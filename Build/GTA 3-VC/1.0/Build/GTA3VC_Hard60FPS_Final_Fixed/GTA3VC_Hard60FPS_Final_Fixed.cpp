#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <cstdint>

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

    constexpr LONGLONG TARGET_HZ = 60;

    Direct3DCreate8Fn g_realDirect3DCreate8 = nullptr;
    CreateDeviceFn g_originalCreateDevice = nullptr;
    PresentFn g_originalPresent = nullptr;

    bool g_presentHookInstalled = false;
    LARGE_INTEGER g_qpcFrequency{};
    LONGLONG g_nextFrame = 0;
    bool g_timerStarted = false;

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

    void PaceTo60FPS()
    {
        if (!g_timerStarted)
            StartLimiter();

        const LONGLONG interval =
            (g_qpcFrequency.QuadPart + (TARGET_HZ / 2)) /
            TARGET_HZ;

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        // Schedule this presentation at the next 60 Hz boundary.
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
        PaceTo60FPS();

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
        DisableThreadLibraryCalls(module);
        InstallDirect3DCreate8Hook();
    }

    return TRUE;
}
