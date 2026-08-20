#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>

struct IDirect3D8;
struct IDirect3DDevice8;
struct D3DPRESENT_PARAMETERS8;

namespace
{
    using Direct3DCreate8Fn = IDirect3D8* (WINAPI*)(UINT);

    using CreateDeviceFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3D8*, UINT, UINT, HWND, DWORD,
        D3DPRESENT_PARAMETERS8*, IDirect3DDevice8**);

    using PresentFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3DDevice8*, const void*, const void*, HWND, const void*);

    constexpr size_t D3D8_VTABLE_SIZE = 34;
    constexpr size_t CREATE_DEVICE_SLOT = 15;
    constexpr size_t PRESENT_SLOT = 15;

    constexpr int DEFAULT_FPS = 30;
    constexpr int MIN_FPS = 2;
    constexpr int MAX_FPS = 1000;

    Direct3DCreate8Fn g_realDirect3DCreate8 = nullptr;
    CreateDeviceFn g_originalCreateDevice = nullptr;
    PresentFn g_originalPresent = nullptr;

    bool g_presentHookInstalled = false;
    LARGE_INTEGER g_qpcFrequency{};
    LONGLONG g_nextFrame = 0;
    bool g_timerStarted = false;

    HMODULE g_module = nullptr;
    int g_targetFPS = DEFAULT_FPS;
    bool g_configLoaded = false;

    int ClampFPS(int fps)
    {
        if (fps < MIN_FPS)
            return MIN_FPS;

        if (fps > MAX_FPS)
            return MAX_FPS;

        return fps;
    }

    int LoadTargetFPS()
    {
        if (!g_module)
            return DEFAULT_FPS;

        char modulePath[MAX_PATH]{};

        DWORD length =
            GetModuleFileNameA(
                g_module,
                modulePath,
                static_cast<DWORD>(sizeof(modulePath)));

        if (length == 0 || length >= sizeof(modulePath))
            return DEFAULT_FPS;

        std::string iniPath(modulePath);

        const size_t slash = iniPath.find_last_of("\\/");

        if (slash == std::string::npos)
            return DEFAULT_FPS;

        iniPath.resize(slash + 1);
        iniPath += "HardFPS.ini";

        std::ifstream file(iniPath);

        if (!file)
            return DEFAULT_FPS;

        std::string line;

        while (std::getline(file, line))
        {
            // Allow a simple "fps=value" line, with optional whitespace.
            size_t start = line.find_first_not_of(" \t\r\n");

            if (start == std::string::npos)
                continue;

            line.erase(0, start);

            size_t end = line.find_last_not_of(" \t\r\n");

            if (end != std::string::npos)
                line.resize(end + 1);

            if (line.size() < 4)
                continue;

            if ((line[0] != 'f' && line[0] != 'F') ||
                (line[1] != 'p' && line[1] != 'P') ||
                (line[2] != 's' && line[2] != 'S') ||
                line[3] != '=')
                continue;

            const char* value = line.c_str() + 4;
            char* parseEnd = nullptr;

            long parsed = std::strtol(value, &parseEnd, 10);

            if (parseEnd == value)
                return DEFAULT_FPS;

            while (*parseEnd == ' ' || *parseEnd == '\t')
                ++parseEnd;

            if (*parseEnd != '\0')
                return DEFAULT_FPS;

            if (parsed < MIN_FPS)
                return MIN_FPS;

            if (parsed > MAX_FPS)
                return MAX_FPS;

            return static_cast<int>(parsed);
        }

        return DEFAULT_FPS;
    }

    void LoadConfig()
    {
        if (g_configLoaded)
            return;

        g_targetFPS = LoadTargetFPS();
        g_configLoaded = true;
    }

    void StartLimiter()
    {
        if (g_timerStarted)
            return;

        LoadConfig();

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
            (g_qpcFrequency.QuadPart + (g_targetFPS / 2)) /
            g_targetFPS;

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        // Schedule this presentation at the next configured FPS boundary.
        g_nextFrame += interval;

        // If rendering fell behind, don't accumulate a backlog of sleeps.
        if (g_nextFrame <= now.QuadPart)
        {
            g_nextFrame = now.QuadPart + interval;
            return;
        }

        for (;;)
        {
            QueryPerformanceCounter(&now);

            const LONGLONG remaining =
                g_nextFrame - now.QuadPart;

            if (remaining <= 0)
                break;

            const LONGLONG ms =
                (remaining * 1000) /
                g_qpcFrequency.QuadPart;

            if (ms > 2)
            {
                Sleep(static_cast<DWORD>(ms - 1));
            }
            else
            {
                SwitchToThread();
            }
        }
    }

    HRESULT STDMETHODCALLTYPE PresentHook(
        IDirect3DDevice8* device,
        const void* sourceRect,
        const void* destRect,
        HWND overrideWindow,
        const void* dirtyRegion)
    {
        PaceToFPS();

        return g_originalPresent(
            device,
            sourceRect,
            destRect,
            overrideWindow,
            dirtyRegion);
    }

    void HookPresentDirectly(IDirect3DDevice8* device)
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
        IDirect3D8* self,
        UINT adapter,
        UINT deviceType,
        HWND focusWindow,
        DWORD behaviorFlags,
        D3DPRESENT_PARAMETERS8* presentationParameters,
        IDirect3DDevice8** returnedDevice)
    {
        HRESULT hr =
            g_originalCreateDevice(
                self,
                adapter,
                deviceType,
                focusWindow,
                behaviorFlags,
                presentationParameters,
                returnedDevice);

        if (SUCCEEDED(hr) &&
            returnedDevice &&
            *returnedDevice)
        {
            HookPresentDirectly(*returnedDevice);
        }

        return hr;
    }

    IDirect3D8* WINAPI Direct3DCreate8Hook(UINT sdkVersion)
    {
        IDirect3D8* object =
            g_realDirect3DCreate8(sdkVersion);

        if (!object)
            return nullptr;

        void** original =
            *reinterpret_cast<void***>(object);

        void** clone =
            static_cast<void**>(
                VirtualAlloc(
                    nullptr,
                    D3D8_VTABLE_SIZE * sizeof(void*),
                    MEM_COMMIT | MEM_RESERVE,
                    PAGE_READWRITE));

        if (!clone)
            return object;

        std::memcpy(
            clone,
            original,
            D3D8_VTABLE_SIZE * sizeof(void*));

        g_originalCreateDevice =
            reinterpret_cast<CreateDeviceFn>(
                clone[CREATE_DEVICE_SLOT]);

        if (!g_originalCreateDevice)
        {
            VirtualFree(clone, 0, MEM_RELEASE);
            return object;
        }

        clone[CREATE_DEVICE_SLOT] =
            reinterpret_cast<void*>(&CreateDeviceHook);

        DWORD oldProtect = 0;

        VirtualProtect(
            clone,
            D3D8_VTABLE_SIZE * sizeof(void*),
            PAGE_READONLY,
            &oldProtect);

        *reinterpret_cast<void***>(object) = clone;

        return object;
    }

    bool InstallDirect3DCreate8Hook()
    {
        HMODULE game = GetModuleHandleW(nullptr);

        if (!game)
            return false;

        uintptr_t base =
            reinterpret_cast<uintptr_t>(game);

        auto dos =
            reinterpret_cast<IMAGE_DOS_HEADER*>(base);

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

            if (_stricmp(dllName, "d3d8.dll") != 0)
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
                        base + originalThunk->u1.AddressOfData);

                if (std::strcmp(
                        reinterpret_cast<const char*>(
                            byName->Name),
                        "Direct3DCreate8") != 0)
                    continue;

                g_realDirect3DCreate8 =
                    reinterpret_cast<Direct3DCreate8Fn>(
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
                        &Direct3DCreate8Hook);

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
        g_module = module;
        DisableThreadLibraryCalls(module);
        InstallDirect3DCreate8Hook();
    }

    return TRUE;
}
