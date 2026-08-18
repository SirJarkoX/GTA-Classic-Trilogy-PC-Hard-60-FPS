#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atomic>
#include <cstring>


// GTA III / d3d8to9 hard presentation cap.
//
// GTA III is a D3D8 application. When d3d8to9 is present, GTA III still
// receives IDirect3D8 / IDirect3DDevice8 wrapper interfaces. We hook the
// D3D8 device's Present() instead of trying to hook the underlying D3D9
// device. This keeps the limiter independent of GTA III's own frame limiter
// and of the D3D8to9 implementation details.
//
// Target: 60.000 Hz (16.666666... ms cadence).

namespace
{
    // Minimal D3D8 ABI declarations. We intentionally do not include the
    // legacy DirectX 8 SDK header or link against d3d8.lib; GTA III already
    // loads the D3D8->D3D9 wrapper supplied by the Widescreen Fix.
    using D3DDEVTYPE = UINT;
    constexpr UINT D3D_SDK_VERSION = 120;
    constexpr HRESULT D3DERR_INVALIDCALL = static_cast<HRESULT>(0x8876086CL);

    struct D3DPRESENT_PARAMETERS
    {
        UINT BackBufferWidth;
        UINT BackBufferHeight;
        UINT BackBufferFormat;
        UINT BackBufferCount;
        UINT MultiSampleType;
        DWORD MultiSampleQuality;
        UINT SwapEffect;
        HWND hDeviceWindow;
        BOOL Windowed;
        BOOL EnableAutoDepthStencil;
        UINT AutoDepthStencilFormat;
        DWORD Flags;
        UINT FullScreen_RefreshRateInHz;
        UINT FullScreen_PresentationInterval;
    };

    struct IDirect3D8;
    struct IDirect3DDevice8;

    constexpr double TARGET_FPS = 60.0;
    constexpr double FRAME_SECONDS = 1.0 / TARGET_FPS;

    using Direct3DCreate8Fn = IDirect3D8* (WINAPI*)(UINT);
    using CreateDeviceFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3D8*,
        UINT,
        D3DDEVTYPE,
        HWND,
        DWORD,
        D3DPRESENT_PARAMETERS*,
        IDirect3DDevice8**);

    using PresentFn = HRESULT (STDMETHODCALLTYPE*)(
        IDirect3DDevice8*,
        const RECT*,
        const RECT*,
        HWND,
        const RGNDATA*);

    // IDirect3D8::CreateDevice is vtable slot 14.
    constexpr size_t D3D8_CREATE_DEVICE_SLOT = 15;

    // IDirect3DDevice8::Present is vtable slot 15.
    constexpr size_t D3D8_DEVICE_PRESENT_SLOT = 15;

    CreateDeviceFn g_originalCreateDevice = nullptr;
    PresentFn g_originalPresent = nullptr;

    LARGE_INTEGER g_qpcFreq{};
    LONGLONG g_nextDeadline = 0;
    bool g_timingStarted = false;

    void ResetCadence()
    {
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        const LONGLONG step =
            static_cast<LONGLONG>(
                static_cast<double>(g_qpcFreq.QuadPart) * FRAME_SECONDS);

        g_nextDeadline = now.QuadPart + step;
        g_timingStarted = true;
    }

    void WaitUntil(LONGLONG deadline)
    {
        LARGE_INTEGER now{};

        for (;;)
        {
            QueryPerformanceCounter(&now);

            const LONGLONG remaining = deadline - now.QuadPart;
            if (remaining <= 0)
                return;

            const double remainingMs =
                static_cast<double>(remaining) * 1000.0 /
                static_cast<double>(g_qpcFreq.QuadPart);

            // Sleep while comfortably away from the deadline, then yield
            // briefly for the final portion. This avoids a crude Sleep(16)
            // limiter while avoiding a full-core busy wait.
            if (remainingMs > 2.0)
            {
                const DWORD sleepMs =
                    static_cast<DWORD>(remainingMs - 1.0);

                if (sleepMs > 0)
                    Sleep(sleepMs);
                else
                    SwitchToThread();
            }
            else
            {
                YieldProcessor();
            }
        }
    }

    void LimitPresent()
    {
        if (!g_timingStarted)
            ResetCadence();

        const LONGLONG step =
            static_cast<LONGLONG>(
                static_cast<double>(g_qpcFreq.QuadPart) * FRAME_SECONDS);

        WaitUntil(g_nextDeadline);

        // Keep a stable cadence based on deadlines rather than the time at
        // which Present() happens. If the game falls substantially behind,
        // resynchronize instead of trying to "catch up" with several frames.
        g_nextDeadline += step;

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        if (g_nextDeadline < now.QuadPart - (step * 2))
            ResetCadence();
    }

    bool PatchVtablePointer(
        void*** object,
        size_t slot,
        void* replacement,
        void** original)
    {
        if (!object || !*object)
            return false;

        void** vtable = *object;

        if (original)
            *original = vtable[slot];

        DWORD oldProtect = 0;
        if (!VirtualProtect(
                &vtable[slot],
                sizeof(void*),
                PAGE_READWRITE,
                &oldProtect))
        {
            return false;
        }

        vtable[slot] = replacement;

        DWORD ignored = 0;
        VirtualProtect(
            &vtable[slot],
            sizeof(void*),
            oldProtect,
            &ignored);

        FlushInstructionCache(
            GetCurrentProcess(),
            &vtable[slot],
            sizeof(void*));

        return true;
    }

    HRESULT STDMETHODCALLTYPE HookedPresent(
        IDirect3DDevice8* device,
        const RECT* source,
        const RECT* dest,
        HWND window,
        const RGNDATA* dirty);

    bool CloneAndPatchDevice(
        IDirect3DDevice8* device,
        PresentFn& originalPresent)
    {
        if (!device)
            return false;

        void*** object =
            reinterpret_cast<void***>(device);

        void** originalVtable = *object;
        if (!originalVtable)
            return false;

        // IDirect3DDevice8 has 119 methods in the standard D3D8 interface.
        constexpr size_t VTABLE_SIZE = 119;

        void** clone =
            new void*[VTABLE_SIZE];

        std::memcpy(
            clone,
            originalVtable,
            VTABLE_SIZE * sizeof(void*));

        originalPresent =
            reinterpret_cast<PresentFn>(
                originalVtable[D3D8_DEVICE_PRESENT_SLOT]);

        clone[D3D8_DEVICE_PRESENT_SLOT] =
            reinterpret_cast<void*>(&HookedPresent);

        DWORD oldProtect = 0;
        if (!VirtualProtect(
                object,
                sizeof(void*),
                PAGE_READWRITE,
                &oldProtect))
        {
            delete[] clone;
            return false;
        }

        *object = clone;

        DWORD ignored = 0;
        VirtualProtect(
            object,
            sizeof(void*),
            oldProtect,
            &ignored);

        FlushInstructionCache(
            GetCurrentProcess(),
            object,
            sizeof(void*));

        return true;
    }

    HRESULT STDMETHODCALLTYPE HookedPresent(
        IDirect3DDevice8* device,
        const RECT* source,
        const RECT* dest,
        HWND window,
        const RGNDATA* dirty)
    {
        LimitPresent();

        if (!g_originalPresent)
            return D3DERR_INVALIDCALL;

        return g_originalPresent(
            device,
            source,
            dest,
            window,
            dirty);
    }

    HRESULT STDMETHODCALLTYPE HookedCreateDevice(
        IDirect3D8* self,
        UINT adapter,
        D3DDEVTYPE type,
        HWND window,
        DWORD behavior,
        D3DPRESENT_PARAMETERS* pp,
        IDirect3DDevice8** outDevice)
    {
        if (!g_originalCreateDevice)
            return D3DERR_INVALIDCALL;

        const HRESULT hr = g_originalCreateDevice(
            self,
            adapter,
            type,
            window,
            behavior,
            pp,
            outDevice);

        if (SUCCEEDED(hr) &&
            outDevice &&
            *outDevice &&
            !g_originalPresent)
        {
            CloneAndPatchDevice(
                *outDevice,
                g_originalPresent);
        }

        return hr;
    }

    DWORD WINAPI InitThread(void*)
    {
        if (!QueryPerformanceFrequency(&g_qpcFreq))
            return 0;

        // We intentionally use the D3D8 interface exposed by the game.
        // d3d8to9 supplies this interface when its d3d8.dll is installed.
        HMODULE d3d8 = GetModuleHandleA("d3d8.dll");

        if (!d3d8)
        {
            // The wrapper should already be loaded by the time an ASI
            // initializes, but wait briefly in case the loader runs earlier.
            for (int i = 0; i < 300 && !d3d8; ++i)
            {
                d3d8 = GetModuleHandleA("d3d8.dll");
                if (!d3d8)
                    Sleep(10);
            }
        }

        if (!d3d8)
            return 0;

        auto create8 =
            reinterpret_cast<Direct3DCreate8Fn>(
                GetProcAddress(d3d8, "Direct3DCreate8"));

        if (!create8)
            return 0;

        IDirect3D8* d3d =
            create8(D3D_SDK_VERSION);

        if (!d3d)
            return 0;

        void*** object =
            reinterpret_cast<void***>(d3d);

        void** vtable = *object;

        if (!vtable)
            return 0;

        void* original = vtable[D3D8_CREATE_DEVICE_SLOT];

        DWORD oldProtect = 0;
        if (!VirtualProtect(
                &vtable[D3D8_CREATE_DEVICE_SLOT],
                sizeof(void*),
                PAGE_READWRITE,
                &oldProtect))
        {
            reinterpret_cast<ULONG (STDMETHODCALLTYPE*)(IDirect3D8*)>(vtable[2])(d3d);
            return 0;
        }

        vtable[D3D8_CREATE_DEVICE_SLOT] =
            reinterpret_cast<void*>(&HookedCreateDevice);

        DWORD ignored = 0;
        VirtualProtect(
            &vtable[D3D8_CREATE_DEVICE_SLOT],
            sizeof(void*),
            oldProtect,
            &ignored);

        FlushInstructionCache(
            GetCurrentProcess(),
            &vtable[D3D8_CREATE_DEVICE_SLOT],
            sizeof(void*));

        g_originalCreateDevice =
            reinterpret_cast<CreateDeviceFn>(original);

        using ReleaseFn = ULONG (STDMETHODCALLTYPE*)(IDirect3D8*);
        auto release = reinterpret_cast<ReleaseFn>(vtable[2]);
        release(d3d);
        return 0;
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

        HANDLE thread =
            CreateThread(
                nullptr,
                0,
                InitThread,
                nullptr,
                0,
                nullptr);

        if (thread)
            CloseHandle(thread);
    }

    return TRUE;
}
