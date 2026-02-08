#include <windows.h>
#include <cstdio>
#include <atomic>

#include "Hook.h"

static HMODULE g_hModule = nullptr;
static std::atomic<bool> g_running{true};

static bool IsSafeUnloadHotkeyPressed() {
    if ((GetAsyncKeyState(VK_F10) & 1) == 0) {
        return false;
    }

    const bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    return ctrlDown && shiftDown;
}

DWORD WINAPI MainThread(LPVOID) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    printf("Endfield Cheat (ImGui Mode) Injected.\n");

    if (!Hook::Init()) {
        printf("[Main] Hook init failed!\n");
        FreeConsole();
        FreeLibraryAndExitThread(g_hModule, 0);
        return 0;
    }

    printf("[Main] Hook initialized. Press [Ctrl + Shift + F10] to unload.\n");

    while (g_running.load(std::memory_order_acquire)) {
        if (IsSafeUnloadHotkeyPressed()) {
            g_running.store(false, std::memory_order_release);
            break;
        }
        Sleep(80);
    }

    printf("[Main] Requesting safe unload...\n");
    Hook::RequestUnload();

    for (int i = 0; i < 240; ++i) {
        if (Hook::IsUnloadCompleted()) {
            break;
        }
        Sleep(25);
    }

    if (!Hook::IsUnloadCompleted()) {
        printf("[Main] Unload timeout, fallback shutdown.\n");
        Hook::Shutdown();
    }

    FreeConsole();
    FreeLibraryAndExitThread(g_hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        Hook::g_dllModule = hModule;
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        g_running.store(false, std::memory_order_release);
        break;
    default:
        break;
    }
    return TRUE;
}
