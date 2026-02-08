#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdint>

// DX11 Present Hook 初始化与清理
namespace Hook {
    bool Init();
    void Shutdown();
    void RequestUnload();
    bool IsUnloadCompleted();
    void MarkManualTeleportCallBegin();
    void MarkManualTeleportCallEnd();
    bool GetLastObservedTeleportValidationData(uintptr_t* outValidationDataPtr,
                                               int32_t* outReason,
                                               int32_t* outUiType);
    bool GetLastObservedTeleportParams(int32_t* outLevelId,
                                       int32_t* outReason,
                                       int32_t* outUiType,
                                       uint32_t* outHubNodeId);

    // 供外部访问的状态
    extern bool g_initialized;
    extern HWND g_gameWindow;
    extern HMODULE g_dllModule;
}
