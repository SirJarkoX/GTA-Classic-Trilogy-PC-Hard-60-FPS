#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <cerrno>

#pragma comment(lib, "d3d9.lib")

namespace
{
    using GetProcAddressFn = FARPROC (WINAPI*)(HMODULE, LPCSTR);
    using Direct3DCreate9Fn = IDirect3D9* (WINAPI*)(UINT);
    using CreateDeviceFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD,
        D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
    using PresentFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3DDevice9*, const RECT*, const RECT*, HWND,
        const RGNDATA*);

    constexpr size_t D3D9_VTABLE_SIZE = 119;
    constexpr size_t CREATE_DEVICE_SLOT = 16;
    constexpr size_t PRESENT_SLOT = 17;
    constexpr int DEFAULT_FPS = 30;
    constexpr int MIN_FPS = 2;
    constexpr int MAX_FPS = 1000;

    int g_targetFPS = DEFAULT_FPS;

    GetProcAddressFn g_originalGetProcAddress = nullptr;
    Direct3DCreate9Fn g_realDirect3DCreate9 = nullptr;
    CreateDeviceFn g_originalCreateDevice = nullptr;
    PresentFn g_originalPresent = nullptr;

    bool g_presentHookInstalled = false;
    LARGE_INTEGER g_qpcFrequency{};
    LONGLONG g_nextFrame = 0;
    bool g_timerStarted = false;


    int ReadTargetFPS(HMODULE module)
    {
        char modulePath[MAX_PATH]{};

        if (!module ||
            !GetModuleFileNameA(module, modulePath, MAX_PATH))
            return DEFAULT_FPS;

        char* lastSlash = std::strrchr(modulePath, '\\');

        if (!lastSlash)
            return DEFAULT_FPS;

        *lastSlash = '\0';

        std::string iniPath =
            std::string(modulePath) + "\\HardFPS.ini";

        std::ifstream file(iniPath);
        if (!file.is_open())
            return DEFAULT_FPS;

        std::string line;

        while (std::getline(file, line))
        {
            const size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos)
                continue;

            const size_t last = line.find_last_not_of(" \t\r\n");
            const std::string trimmed = line.substr(first, last - first + 1);

            if (trimmed.size() < 5 ||
                trimmed.compare(0, 4, "fps=") != 0)
                continue;

            const char* value = trimmed.c_str() + 4;
            char* end = nullptr;

            errno = 0;
            const long parsed = std::strtol(value, &end, 10);

            if (errno == ERANGE ||
                end == value ||
                *end != '\0')
                return DEFAULT_FPS;

            if (parsed < MIN_FPS)
                return MIN_FPS;

            if (parsed > MAX_FPS)
                return MAX_FPS;

            return static_cast<int>(parsed);
        }

        return DEFAULT_FPS;
    }

    void StartLimiter()
    {
        if (g_timerStarted)
            return;

        QueryPerformanceFrequency(&g_qpcFrequency);

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        g_nextFrame = now.QuadPart;
        g_timerStarted = true;
    }

    void PaceToFPS()
    {
        if (!g_timerStarted)
            StartLimiter();

        const LONGLONG interval =
            static_cast<LONGLONG>(
                static_cast<double>(g_qpcFrequency.QuadPart) /
                static_cast<double>(g_targetFPS));

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        g_nextFrame += interval;

        if (g_nextFrame <= now.QuadPart)
        {
            g_nextFrame = now.QuadPart + interval;
            return;
        }

        for (;;)
        {
            QueryPerformanceCounter(&now);

            if (now.QuadPart >= g_nextFrame)
                break;

            const LONGLONG remaining =
                g_nextFrame - now.QuadPart;

            const double remainingMs =
                (static_cast<double>(remaining) /
                 static_cast<double>(g_qpcFrequency.QuadPart)) *
                1000.0;

            if (remainingMs > 2.0)
            {
                DWORD sleepMs =
                    static_cast<DWORD>(remainingMs - 1.0);

                if (sleepMs > 0)
                    Sleep(sleepMs);
            }
            else
            {
                SwitchToThread();
            }
        }
    }

    HRESULT STDMETHODCALLTYPE PresentHook(
        IDirect3DDevice9* device,
        const RECT* sourceRect,
        const RECT* destRect,
        HWND overrideWindow,
        const RGNDATA* dirtyRegion)
    {
        PaceToFPS();

        return g_originalPresent(
            device,
            sourceRect,
            destRect,
            overrideWindow,
            dirtyRegion);
    }

    void HookPresentDirectly(IDirect3DDevice9* device)
    {
        if (!device || g_presentHookInstalled)
            return;

        void** vtable =
            *reinterpret_cast<void***>(device);

        if (!vtable)
            return;

        g_originalPresent =
            reinterpret_cast<PresentFn>(
                vtable[PRESENT_SLOT]);

        if (!g_originalPresent)
            return;

        DWORD oldProtect = 0;

        if (!VirtualProtect(
                &vtable[PRESENT_SLOT],
                sizeof(void*),
                PAGE_READWRITE,
                &oldProtect))
            return;

        vtable[PRESENT_SLOT] =
            reinterpret_cast<void*>(&PresentHook);

        FlushInstructionCache(
            GetCurrentProcess(),
            &vtable[PRESENT_SLOT],
            sizeof(void*));

        DWORD ignored = 0;
        VirtualProtect(
            &vtable[PRESENT_SLOT],
            sizeof(void*),
            oldProtect,
            &ignored);

        g_presentHookInstalled = true;
        StartLimiter();
    }

    HRESULT STDMETHODCALLTYPE CreateDeviceHook(
        IDirect3D9* self,
        UINT adapter,
        D3DDEVTYPE type,
        HWND focus,
        DWORD behavior,
        D3DPRESENT_PARAMETERS* params,
        IDirect3DDevice9** outDevice)
    {
        HRESULT hr =
            g_originalCreateDevice(
                self,
                adapter,
                type,
                focus,
                behavior,
                params,
                outDevice);

        if (SUCCEEDED(hr) &&
            outDevice &&
            *outDevice)
        {
            HookPresentDirectly(*outDevice);
        }

        return hr;
    }

    IDirect3D9* WINAPI Direct3DCreate9Hook(
        UINT sdkVersion)
    {
        IDirect3D9* object =
            g_realDirect3DCreate9(sdkVersion);

        if (!object)
            return nullptr;

        void** original =
            *reinterpret_cast<void***>(object);

        void** clone =
            static_cast<void**>(
                VirtualAlloc(
                    nullptr,
                    D3D9_VTABLE_SIZE * sizeof(void*),
                    MEM_COMMIT | MEM_RESERVE,
                    PAGE_READWRITE));

        if (!clone)
            return object;

        std::memcpy(
            clone,
            original,
            D3D9_VTABLE_SIZE * sizeof(void*));

        g_originalCreateDevice =
            reinterpret_cast<CreateDeviceFn>(
                clone[CREATE_DEVICE_SLOT]);

        clone[CREATE_DEVICE_SLOT] =
            reinterpret_cast<void*>(&CreateDeviceHook);

        DWORD oldProtect = 0;

        VirtualProtect(
            clone,
            D3D9_VTABLE_SIZE * sizeof(void*),
            PAGE_READONLY,
            &oldProtect);

        void*** vtablePointer =
            reinterpret_cast<void***>(object);

        *vtablePointer = clone;

        return object;
    }

    FARPROC WINAPI GetProcAddressHook(
        HMODULE module,
        LPCSTR name)
    {
        FARPROC result =
            g_originalGetProcAddress(
                module,
                name);

        if (!result ||
            !name ||
            reinterpret_cast<uintptr_t>(name) <= 0xFFFFu)
            return result;

        if (std::strcmp(
                name,
                "Direct3DCreate9") != 0)
            return result;

        HMODULE d3d9 =
            GetModuleHandleA("d3d9.dll");

        if (!d3d9 || module != d3d9)
            return result;

        g_realDirect3DCreate9 =
            reinterpret_cast<Direct3DCreate9Fn>(
                result);

        return reinterpret_cast<FARPROC>(
            &Direct3DCreate9Hook);
    }

    bool InstallHook()
    {
        HMODULE game =
            GetModuleHandleW(nullptr);

        if (!game)
            return false;

        uintptr_t base =
            reinterpret_cast<uintptr_t>(game);

        auto dos =
            reinterpret_cast<IMAGE_DOS_HEADER*>(
                base);

        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            return false;

        auto nt =
            reinterpret_cast<IMAGE_NT_HEADERS32*>(
                base + dos->e_lfanew);

        if (nt->Signature != IMAGE_NT_SIGNATURE)
            return false;

        const auto& dir =
            nt->OptionalHeader.DataDirectory[
                IMAGE_DIRECTORY_ENTRY_IMPORT];

        if (!dir.VirtualAddress)
            return false;

        auto imports =
            reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
                base + dir.VirtualAddress);

        for (; imports->Name; ++imports)
        {
            const char* dllName =
                reinterpret_cast<const char*>(
                    base + imports->Name);

            if (_stricmp(dllName, "KERNEL32.dll") != 0 &&
                _stricmp(dllName, "kernel32.dll") != 0)
                continue;

            if (!imports->OriginalFirstThunk)
                continue;

            auto originalThunk =
                reinterpret_cast<IMAGE_THUNK_DATA32*>(
                    base + imports->OriginalFirstThunk);

            auto firstThunk =
                reinterpret_cast<IMAGE_THUNK_DATA32*>(
                    base + imports->FirstThunk);

            for (; originalThunk->u1.AddressOfData;
                 ++originalThunk, ++firstThunk)
            {
                if (originalThunk->u1.Ordinal &
                    IMAGE_ORDINAL_FLAG32)
                    continue;

                auto byName =
                    reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                        base +
                        originalThunk->u1.AddressOfData);

                if (std::strcmp(
                        reinterpret_cast<const char*>(
                            byName->Name),
                        "GetProcAddress") != 0)
                    continue;

                g_originalGetProcAddress =
                    reinterpret_cast<GetProcAddressFn>(
                        static_cast<uintptr_t>(
                            firstThunk->u1.Function));

                DWORD oldProtect = 0;

                if (!VirtualProtect(
                        &firstThunk->u1.Function,
                        sizeof(firstThunk->u1.Function),
                        PAGE_READWRITE,
                        &oldProtect))
                    return false;

                firstThunk->u1.Function =
                    reinterpret_cast<uintptr_t>(
                        &GetProcAddressHook);

                FlushInstructionCache(
                    GetCurrentProcess(),
                    &firstThunk->u1.Function,
                    sizeof(firstThunk->u1.Function));

                VirtualProtect(
                    &firstThunk->u1.Function,
                    sizeof(firstThunk->u1.Function),
                    oldProtect,
                    &oldProtect);

                return true;
            }
        }

        return false;
    }
}

BOOL APIENTRY DllMain(
    HMODULE module,
    DWORD reason,
    LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        g_targetFPS = ReadTargetFPS(module);
        InstallHook();
    }

    return TRUE;
}
