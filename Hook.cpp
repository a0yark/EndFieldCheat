#include "Hook.h"
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <atomic>
#include <string>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <MinHook.h>

// Cheat.cpp 中的绘制函数
extern void DrawPlayerDebugWindow();
extern void DrawTreasureEspOverlay();
extern void ShutdownCheatRuntime();
extern bool IsDebugWindowVisible();
extern bool IsForceNoCooldownEnabled();
extern bool IsInfiniteDashEnabled();
extern bool IsInfiniteStaminaEnabled();

// --- 全局状态 ---
namespace Hook {
    bool g_initialized = false;
    HWND g_gameWindow = nullptr;
    HMODULE g_dllModule = nullptr;
}

static std::atomic<bool> g_unloadRequested{false};
static std::atomic<bool> g_unloadCompleted{false};
static std::atomic<bool> g_shutdownStarted{false};
static std::atomic<int32_t> g_observedTeleportLevelId{0};
static std::atomic<int32_t> g_observedTeleportReason{-1};
static std::atomic<int32_t> g_observedTeleportUiType{-1};
static std::atomic<uint32_t> g_observedTeleportHubNodeId{0};
static std::atomic<int32_t> g_observedTeleportSource{0};
static std::atomic<uint64_t> g_observedTeleportSequence{0};
static std::atomic<uintptr_t> g_observedTeleportValidationDataPtr{0};
static std::atomic<int32_t> g_observedTeleportValidationReason{-1};
static std::atomic<int32_t> g_observedTeleportValidationUiType{-1};
static std::atomic<uint64_t> g_observedTeleportValidationSequence{0};
static std::atomic<int32_t> g_manualTeleportCallDepth{0};

static void WriteHookLog(const char* fmt, ...) {
    char tempPath[MAX_PATH] = {};
    DWORD len = GetTempPathA(static_cast<DWORD>(sizeof(tempPath)), tempPath);
    if (len == 0 || len >= sizeof(tempPath)) {
        return;
    }

    char logPath[MAX_PATH] = {};
    std::snprintf(logPath, sizeof(logPath), "%s%s", tempPath, "EndfieldCheat_tp.log");

    FILE* file = nullptr;
    errno_t openErr = fopen_s(&file, logPath, "a");
    if (openErr != 0 || !file) {
        return;
    }

    SYSTEMTIME st{};
    GetLocalTime(&st);
    std::fprintf(file,
                 "[%04u-%02u-%02u %02u:%02u:%02u.%03u][HOOK] ",
                 st.wYear,
                 st.wMonth,
                 st.wDay,
                 st.wHour,
                 st.wMinute,
                 st.wSecond,
                 st.wMilliseconds);

    va_list args;
    va_start(args, fmt);
    std::vfprintf(file, fmt, args);
    va_end(args);

    std::fputc('\n', file);
    std::fclose(file);
}

static void SaveObservedTeleportParams(int32_t levelId,
                                       int32_t reason,
                                       int32_t uiType,
                                       uint32_t hubNodeId,
                                       int32_t source,
                                       const char* sourceText) {
    if (g_manualTeleportCallDepth.load(std::memory_order_acquire) > 0) {
        return;
    }

    g_observedTeleportLevelId.store(levelId, std::memory_order_release);
    g_observedTeleportReason.store(reason, std::memory_order_release);
    g_observedTeleportUiType.store(uiType, std::memory_order_release);
    g_observedTeleportHubNodeId.store(hubNodeId, std::memory_order_release);
    g_observedTeleportSource.store(source, std::memory_order_release);
    const uint64_t seq = g_observedTeleportSequence.fetch_add(1, std::memory_order_acq_rel) + 1;

    WriteHookLog("ObservedTeleport seq=%llu src=%s levelId=%d reason=%d uiType=%d hubNodeId=%u",
                 static_cast<unsigned long long>(seq),
                 sourceText ? sourceText : "Unknown",
                 levelId,
                 reason,
                 uiType,
                 hubNodeId);
}

static void SaveObservedTeleportValidationData(void* validationData,
                                               int32_t reasonFromData,
                                               int32_t uiTypeFromData,
                                               int32_t source,
                                               const char* sourceText) {
    if (g_manualTeleportCallDepth.load(std::memory_order_acquire) > 0) {
        return;
    }

    const uintptr_t ptr = reinterpret_cast<uintptr_t>(validationData);
    g_observedTeleportValidationDataPtr.store(ptr, std::memory_order_release);
    g_observedTeleportValidationReason.store(reasonFromData, std::memory_order_release);
    g_observedTeleportValidationUiType.store(uiTypeFromData, std::memory_order_release);
    const uint64_t seq = g_observedTeleportValidationSequence.fetch_add(1, std::memory_order_acq_rel) + 1;

    WriteHookLog("ObservedTeleportValidation seq=%llu src=%s data=%p reason=%d uiType=%d",
                 static_cast<unsigned long long>(seq),
                 sourceText ? sourceText : "Unknown",
                 validationData,
                 reasonFromData,
                 uiTypeFromData);

    SaveObservedTeleportParams(0,
                               reasonFromData,
                               uiTypeFromData,
                               0u,
                               source,
                               sourceText ? sourceText : "ValidationData");
}

// DX11 相关
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static std::atomic<bool> g_imguiInitialized{false};

// 原始函数指针
typedef HRESULT(STDMETHODCALLTYPE* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* ResizeBuffers_t)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static Present_t oPresent = nullptr;
static ResizeBuffers_t oResizeBuffers = nullptr;

constexpr uintptr_t kAbilityCheckNormalAttackCdRva = 0x39BEF40;
constexpr uintptr_t kAbilityCheckSkillCdByIndexRva = 0x39BEF90;
constexpr uintptr_t kAbilityCheckSkillCdByIdRva = 0x39BF070;
constexpr uintptr_t kSkillCheckCdRva = 0x39AF480;
constexpr uintptr_t kSkillGetAtbCostRva = 0x39AF4F0;
constexpr uintptr_t kSkillCheckCostRva = 0x39AF5E0;
constexpr uintptr_t kSkillCheckTagRva = 0x39AF740;
constexpr uintptr_t kSkillCheckStateRva = 0x39AF8B0;
constexpr uintptr_t kSkillGetCanPressSkillButtonRva = 0x39AA830;
constexpr uintptr_t kSkillIsAvailableRva = 0x39B0FE0;
constexpr uintptr_t kSkillApplyCostRva = 0x39B1880;
constexpr uintptr_t kPlayerCheckCanDashRva = 0x412A550;
constexpr uintptr_t kPlayerDecreaseDashCountRva = 0x412A630;
constexpr uintptr_t kPlayerUpdateDashCdRva = 0x412AAE0;
constexpr uintptr_t kPlayerOnSprintPressedRva = 0x4128830;
constexpr uintptr_t kTeleportProcessorC2STeleportRva = 0x4D5B100;
constexpr uintptr_t kGameplayNetworkC2STeleportRva = 0x4C374F0;
constexpr uintptr_t kGameActionTeleportToPositionRva = 0x510FAA0;
constexpr uintptr_t kTeleportProcessorC2STeleportWithValidationDataRva = 0x4D5B870;
constexpr uintptr_t kGameplayNetworkC2STeleportWithValidationDataRva = 0x4C376E0;

using AbilityCheckNormalAttackCd_t = bool (*)(void*, void*);
using AbilityCheckSkillCdByIndex_t = bool (*)(void*, int, void*);
using AbilityCheckSkillCdById_t = bool (*)(void*, void*, void*);
using SkillCheckCd_t = bool (*)(void*, void*);
using SkillGetAtbCost_t = float (*)(void*, void*, void*);
using SkillCheckCost_t = bool (*)(void*, void*);
using SkillCheckTag_t = bool (*)(void*, void*);
using SkillCheckState_t = bool (*)(void*, void*);
using SkillGetCanPressSkillButton_t = bool (*)(void*, void*);
using SkillIsAvailable_t = bool (*)(void*, void*);
using SkillApplyCost_t = void (*)(void*, void*);
using PlayerCheckCanDash_t = bool (*)(void*, void*);
using PlayerDecreaseDashCount_t = void (*)(void*, void*);
using PlayerUpdateDashCd_t = void (*)(void*, float, void*);
using PlayerOnSprintPressed_t = bool (*)(void*, void*);
using TeleportProcessorC2STeleport_t = void (*)(void*, int32_t, const void*, const void*, int32_t, int32_t, void*, uint32_t, void*);
using GameplayNetworkC2STeleport_t = void (*)(void*, int32_t, const void*, const void*, int32_t, int32_t, void*, uint32_t, void*);
using GameActionTeleportToPosition_t = void (*)(int32_t, void*, const void*, const void*, int32_t, void*, uint32_t, void*);
using TeleportProcessorC2STeleportWithValidationData_t = void (*)(void*, void*, void*, const void*, const void*, void*);
using GameplayNetworkC2STeleportWithValidationData_t = void (*)(void*, void*, void*, const void*, const void*, void*);

static AbilityCheckNormalAttackCd_t oAbilityCheckNormalAttackCd = nullptr;
static AbilityCheckSkillCdByIndex_t oAbilityCheckSkillCdByIndex = nullptr;
static AbilityCheckSkillCdById_t oAbilityCheckSkillCdById = nullptr;
static SkillCheckCd_t oSkillCheckCd = nullptr;
static SkillGetAtbCost_t oSkillGetAtbCost = nullptr;
static SkillCheckCost_t oSkillCheckCost = nullptr;
static SkillCheckTag_t oSkillCheckTag = nullptr;
static SkillCheckState_t oSkillCheckState = nullptr;
static SkillGetCanPressSkillButton_t oSkillGetCanPressSkillButton = nullptr;
static SkillIsAvailable_t oSkillIsAvailable = nullptr;
static SkillApplyCost_t oSkillApplyCost = nullptr;
static PlayerCheckCanDash_t oPlayerCheckCanDash = nullptr;
static PlayerDecreaseDashCount_t oPlayerDecreaseDashCount = nullptr;
static PlayerUpdateDashCd_t oPlayerUpdateDashCd = nullptr;
static PlayerOnSprintPressed_t oPlayerOnSprintPressed = nullptr;
static TeleportProcessorC2STeleport_t oTeleportProcessorC2STeleport = nullptr;
static GameplayNetworkC2STeleport_t oGameplayNetworkC2STeleport = nullptr;
static GameActionTeleportToPosition_t oGameActionTeleportToPosition = nullptr;
static TeleportProcessorC2STeleportWithValidationData_t oTeleportProcessorC2STeleportWithValidationData = nullptr;
static GameplayNetworkC2STeleportWithValidationData_t oGameplayNetworkC2STeleportWithValidationData = nullptr;

static bool hkAbilityCheckNormalAttackCd(void* ability, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oAbilityCheckNormalAttackCd ? oAbilityCheckNormalAttackCd(ability, method) : true;
}

static bool hkAbilityCheckSkillCdByIndex(void* ability, int index, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oAbilityCheckSkillCdByIndex ? oAbilityCheckSkillCdByIndex(ability, index, method) : true;
}

static bool hkAbilityCheckSkillCdById(void* ability, void* skillId, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oAbilityCheckSkillCdById ? oAbilityCheckSkillCdById(ability, skillId, method) : true;
}

static bool hkSkillCheckCd(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillCheckCd ? oSkillCheckCd(skill, method) : true;
}

static float hkSkillGetAtbCost(void* skill, void* costData, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return 0.0f;
    }
    return oSkillGetAtbCost ? oSkillGetAtbCost(skill, costData, method) : 0.0f;
}

static bool hkSkillCheckCost(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillCheckCost ? oSkillCheckCost(skill, method) : true;
}

static bool hkSkillCheckTag(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillCheckTag ? oSkillCheckTag(skill, method) : true;
}

static bool hkSkillCheckState(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillCheckState ? oSkillCheckState(skill, method) : true;
}

static bool hkSkillGetCanPressSkillButton(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillGetCanPressSkillButton ? oSkillGetCanPressSkillButton(skill, method) : true;
}

static bool hkSkillIsAvailable(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return true;
    }
    return oSkillIsAvailable ? oSkillIsAvailable(skill, method) : true;
}

static void hkSkillApplyCost(void* skill, void* method) {
    if (IsForceNoCooldownEnabled()) {
        return;
    }
    if (oSkillApplyCost) {
        oSkillApplyCost(skill, method);
    }
}

static bool hkPlayerCheckCanDash(void* playerController, void* method) {
    if (IsInfiniteDashEnabled() || IsInfiniteStaminaEnabled()) {
        return true;
    }
    return oPlayerCheckCanDash ? oPlayerCheckCanDash(playerController, method) : true;
}

static void hkPlayerDecreaseDashCount(void* playerController, void* method) {
    if (IsInfiniteDashEnabled()) {
        return;
    }
    if (oPlayerDecreaseDashCount) {
        oPlayerDecreaseDashCount(playerController, method);
    }
}

static void hkPlayerUpdateDashCd(void* playerController, float deltaTime, void* method) {
    if (IsInfiniteDashEnabled()) {
        return;
    }
    if (oPlayerUpdateDashCd) {
        oPlayerUpdateDashCd(playerController, deltaTime, method);
    }
}

static bool hkPlayerOnSprintPressed(void* playerController, void* method) {
    return oPlayerOnSprintPressed ? oPlayerOnSprintPressed(playerController, method) : true;
}

static void hkTeleportProcessorC2STeleport(void* teleportProcessor,
                                           int32_t levelId,
                                           const void* position,
                                           const void* rotationEuler,
                                           int32_t reason,
                                           int32_t uiType,
                                           void* callback,
                                           uint32_t hubNodeId,
                                           void* method) {
    SaveObservedTeleportParams(levelId, reason, uiType, hubNodeId, 2, "TeleportProcessor::C2STeleport");
    if (oTeleportProcessorC2STeleport) {
        oTeleportProcessorC2STeleport(teleportProcessor,
                                      levelId,
                                      position,
                                      rotationEuler,
                                      reason,
                                      uiType,
                                      callback,
                                      hubNodeId,
                                      method);
    }
}

static void hkGameplayNetworkC2STeleport(void* gameplayNetwork,
                                         int32_t levelId,
                                         const void* position,
                                         const void* rotationEuler,
                                         int32_t reason,
                                         int32_t uiType,
                                         void* callback,
                                         uint32_t hubNodeId,
                                         void* method) {
    SaveObservedTeleportParams(levelId, reason, uiType, hubNodeId, 1, "GameplayNetwork::C2STeleport");
    if (oGameplayNetworkC2STeleport) {
        oGameplayNetworkC2STeleport(gameplayNetwork,
                                    levelId,
                                    position,
                                    rotationEuler,
                                    reason,
                                    uiType,
                                    callback,
                                    hubNodeId,
                                    method);
    }
}

static void hkGameActionTeleportToPosition(int32_t reason,
                                           void* levelIdStr,
                                           const void* position,
                                           const void* rotationEuler,
                                           int32_t uiType,
                                           void* callback,
                                           uint32_t hubNodeId,
                                           void* method) {
    SaveObservedTeleportParams(0, reason, uiType, hubNodeId, 3, "GameAction::TeleportToPosition");
    if (oGameActionTeleportToPosition) {
        oGameActionTeleportToPosition(reason,
                                      levelIdStr,
                                      position,
                                      rotationEuler,
                                      uiType,
                                      callback,
                                      hubNodeId,
                                      method);
    }
}

static void hkTeleportProcessorC2STeleportWithValidationData(void* teleportProcessor,
                                                             void* validationData,
                                                             void* callback,
                                                             const void* position,
                                                             const void* rotationEuler,
                                                             void* method) {
    int32_t reason = -1;
    int32_t uiType = -1;
    if (validationData) {
        __try {
            reason = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(validationData) + 0x18);
            uiType = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(validationData) + 0x40);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            reason = -1;
            uiType = -1;
        }
    }

    SaveObservedTeleportValidationData(validationData,
                                       reason,
                                       uiType,
                                       4,
                                       "TeleportProcessor::C2STeleportWithValidationData");

    if (oTeleportProcessorC2STeleportWithValidationData) {
        oTeleportProcessorC2STeleportWithValidationData(teleportProcessor,
                                                        validationData,
                                                        callback,
                                                        position,
                                                        rotationEuler,
                                                        method);
    }
}

static void hkGameplayNetworkC2STeleportWithValidationData(void* gameplayNetwork,
                                                           void* validationData,
                                                           void* callback,
                                                           const void* position,
                                                           const void* rotationEuler,
                                                           void* method) {
    int32_t reason = -1;
    int32_t uiType = -1;
    if (validationData) {
        __try {
            reason = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(validationData) + 0x18);
            uiType = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(validationData) + 0x40);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            reason = -1;
            uiType = -1;
        }
    }

    SaveObservedTeleportValidationData(validationData,
                                       reason,
                                       uiType,
                                       5,
                                       "GameplayNetwork::C2STeleportWithValidationData");

    if (oGameplayNetworkC2STeleportWithValidationData) {
        oGameplayNetworkC2STeleportWithValidationData(gameplayNetwork,
                                                      validationData,
                                                      callback,
                                                      position,
                                                      rotationEuler,
                                                      method);
    }
}

static void TryCreateCooldownBypassHooks() {
    HMODULE gameAssembly = GetModuleHandleA("GameAssembly.dll");
    if (!gameAssembly) {
        printf("[Hook] GameAssembly.dll not found, skip cooldown bypass hooks.\n");
        return;
    }

    const uintptr_t base = reinterpret_cast<uintptr_t>(gameAssembly);

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kAbilityCheckNormalAttackCdRva),
            reinterpret_cast<void*>(hkAbilityCheckNormalAttackCd),
            reinterpret_cast<void**>(&oAbilityCheckNormalAttackCd)) != MH_OK) {
        printf("[Hook] Failed to hook AbilitySystem::CheckNormalAttackCd.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kAbilityCheckSkillCdByIndexRva),
            reinterpret_cast<void*>(hkAbilityCheckSkillCdByIndex),
            reinterpret_cast<void**>(&oAbilityCheckSkillCdByIndex)) != MH_OK) {
        printf("[Hook] Failed to hook AbilitySystem::CheckSkillCd(int).\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kAbilityCheckSkillCdByIdRva),
            reinterpret_cast<void*>(hkAbilityCheckSkillCdById),
            reinterpret_cast<void**>(&oAbilityCheckSkillCdById)) != MH_OK) {
        printf("[Hook] Failed to hook AbilitySystem::CheckSkillCd(string).\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillCheckCdRva),
            reinterpret_cast<void*>(hkSkillCheckCd),
            reinterpret_cast<void**>(&oSkillCheckCd)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::CheckCd.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillGetAtbCostRva),
            reinterpret_cast<void*>(hkSkillGetAtbCost),
            reinterpret_cast<void**>(&oSkillGetAtbCost)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::_GetAtbCost.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillCheckCostRva),
            reinterpret_cast<void*>(hkSkillCheckCost),
            reinterpret_cast<void**>(&oSkillCheckCost)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::CheckCost.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillCheckTagRva),
            reinterpret_cast<void*>(hkSkillCheckTag),
            reinterpret_cast<void**>(&oSkillCheckTag)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::CheckTag.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillCheckStateRva),
            reinterpret_cast<void*>(hkSkillCheckState),
            reinterpret_cast<void**>(&oSkillCheckState)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::CheckState.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillGetCanPressSkillButtonRva),
            reinterpret_cast<void*>(hkSkillGetCanPressSkillButton),
            reinterpret_cast<void**>(&oSkillGetCanPressSkillButton)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::get_canPressSkillButton.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillIsAvailableRva),
            reinterpret_cast<void*>(hkSkillIsAvailable),
            reinterpret_cast<void**>(&oSkillIsAvailable)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::IsAvailable.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kSkillApplyCostRva),
            reinterpret_cast<void*>(hkSkillApplyCost),
            reinterpret_cast<void**>(&oSkillApplyCost)) != MH_OK) {
        printf("[Hook] Failed to hook Skill::_ApplyCost.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kPlayerCheckCanDashRva),
            reinterpret_cast<void*>(hkPlayerCheckCanDash),
            reinterpret_cast<void**>(&oPlayerCheckCanDash)) != MH_OK) {
        printf("[Hook] Failed to hook PlayerController::CheckCanDashB.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kPlayerDecreaseDashCountRva),
            reinterpret_cast<void*>(hkPlayerDecreaseDashCount),
            reinterpret_cast<void**>(&oPlayerDecreaseDashCount)) != MH_OK) {
        printf("[Hook] Failed to hook PlayerController::DecreaseDashCount.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kPlayerUpdateDashCdRva),
            reinterpret_cast<void*>(hkPlayerUpdateDashCd),
            reinterpret_cast<void**>(&oPlayerUpdateDashCd)) != MH_OK) {
        printf("[Hook] Failed to hook PlayerController::_UpdateDashCd.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kPlayerOnSprintPressedRva),
            reinterpret_cast<void*>(hkPlayerOnSprintPressed),
            reinterpret_cast<void**>(&oPlayerOnSprintPressed)) != MH_OK) {
        printf("[Hook] Failed to hook PlayerController::OnSprintPressed.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kTeleportProcessorC2STeleportRva),
            reinterpret_cast<void*>(hkTeleportProcessorC2STeleport),
            reinterpret_cast<void**>(&oTeleportProcessorC2STeleport)) != MH_OK) {
        printf("[Hook] Failed to hook TeleportProcessor::C2STeleport.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kGameplayNetworkC2STeleportRva),
            reinterpret_cast<void*>(hkGameplayNetworkC2STeleport),
            reinterpret_cast<void**>(&oGameplayNetworkC2STeleport)) != MH_OK) {
        printf("[Hook] Failed to hook GameplayNetwork::C2STeleport.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kGameActionTeleportToPositionRva),
            reinterpret_cast<void*>(hkGameActionTeleportToPosition),
            reinterpret_cast<void**>(&oGameActionTeleportToPosition)) != MH_OK) {
        printf("[Hook] Failed to hook GameAction::TeleportToPosition.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kTeleportProcessorC2STeleportWithValidationDataRva),
            reinterpret_cast<void*>(hkTeleportProcessorC2STeleportWithValidationData),
            reinterpret_cast<void**>(&oTeleportProcessorC2STeleportWithValidationData)) != MH_OK) {
        printf("[Hook] Failed to hook TeleportProcessor::C2STeleportWithValidationData.\n");
    }

    if (MH_CreateHook(
            reinterpret_cast<void*>(base + kGameplayNetworkC2STeleportWithValidationDataRva),
            reinterpret_cast<void*>(hkGameplayNetworkC2STeleportWithValidationData),
            reinterpret_cast<void**>(&oGameplayNetworkC2STeleportWithValidationData)) != MH_OK) {
        printf("[Hook] Failed to hook GameplayNetwork::C2STeleportWithValidationData.\n");
    }

}

// 原始 WndProc
static WNDPROC oWndProc = nullptr;

// Forward declare
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static void LoadChineseFont(ImGuiIO& io) {
    constexpr float kFontSize = 18.0f;
    const ImWchar* ranges = io.Fonts->GetGlyphRangesChineseFull();
    const char* fontCandidates[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",   // Microsoft YaHei
        "C:\\Windows\\Fonts\\msyhbd.ttc",
        "C:\\Windows\\Fonts\\simhei.ttf",
        "C:\\Windows\\Fonts\\simsun.ttc",
        "C:\\Windows\\Fonts\\arialuni.ttf"
    };

    for (const char* path : fontCandidates) {
        if (io.Fonts->AddFontFromFileTTF(path, kFontSize, nullptr, ranges)) {
            printf("[Hook] Loaded CJK font: %s\n", path);
            return;
        }
    }

    io.Fonts->AddFontDefault();
    printf("[Hook] Warning: no CJK font found, Chinese text may render as '?'.\n");
}

// --- WndProc Hook: 让 ImGui 接收输入 ---
static LRESULT CALLBACK hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    const bool imguiReady = g_imguiInitialized.load(std::memory_order_acquire)
        && (ImGui::GetCurrentContext() != nullptr);

    const bool panelVisible = IsDebugWindowVisible();

    if (!panelVisible) {
        if (uMsg == WM_MOUSEACTIVATE) {
            return MA_NOACTIVATE;
        }
        if (uMsg == WM_NCHITTEST) {
            return HTTRANSPARENT;
        }
    }

    if (imguiReady && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        if (panelVisible) {
            return true;
        }
    }

    // 当 ImGui 想要捕获鼠标/键盘时，不传递给游戏
    if (imguiReady && panelVisible) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse && (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST))
            return true;
        if (io.WantCaptureKeyboard && (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))
            return true;
    }

    if (!oWndProc) {
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    return CallWindowProcW(oWndProc, hWnd, uMsg, wParam, lParam);
}

// --- 初始化 ImGui ---
static void InitImGui(IDXGISwapChain* pSwapChain) {
    // 获取 Device 和 Context
    pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);
    g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

    // 获取游戏窗口句柄
    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);
    Hook::g_gameWindow = desc.OutputWindow;

    // 创建 RenderTargetView
    ID3D11Texture2D* pBackBuffer = nullptr;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();

    // Hook WndProc
    oWndProc = (WNDPROC)SetWindowLongPtrW(Hook::g_gameWindow, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

    // 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    LoadChineseFont(io);

    // 设置深色主题
    ImGui::StyleColorsDark();

    // 初始化后端
    ImGui_ImplWin32_Init(Hook::g_gameWindow);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

    g_imguiInitialized.store(true, std::memory_order_release);
    printf("[Hook] ImGui initialized.\n");
}

// --- Present Hook ---
static HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (g_unloadRequested.load(std::memory_order_acquire)) {
        const HRESULT presentResult = oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
        if (!g_unloadCompleted.load(std::memory_order_acquire)) {
            Hook::Shutdown();
        }
        return presentResult;
    }

    if (!g_imguiInitialized.load(std::memory_order_acquire)) {
        InitImGui(pSwapChain);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 调用 Cheat.cpp 中的绘制逻辑
    DrawTreasureEspOverlay();
    DrawPlayerDebugWindow();

    ImGui::EndFrame();
    ImGui::Render();

    g_pd3dContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// --- ResizeBuffers Hook: 窗口大小改变时重建 RenderTarget ---
static HRESULT STDMETHODCALLTYPE hkResizeBuffers(
    IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width,
    UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (g_pRenderTargetView) {
        g_pRenderTargetView->Release();
        g_pRenderTargetView = nullptr;
    }

    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    // 重建 RenderTargetView
    ID3D11Texture2D* pBackBuffer = nullptr;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
        pBackBuffer->Release();
    }

    return hr;
}

// --- 获取 SwapChain VTable 地址 ---
static bool GetDX11VTable(void** pPresentAddr, void** pResizeAddr) {
    // 创建临时窗口
    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProcA,
                       0, 0, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL,
                       "DXDummy", NULL };
    RegisterClassExA(&wc);
    HWND hWnd = CreateWindowA("DXDummy", NULL, WS_OVERLAPPEDWINDOW,
                              0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ID3D11Device* pDevice = nullptr;
    IDXGISwapChain* pSwap = nullptr;
    ID3D11DeviceContext* pCtx = nullptr;
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
        D3D11_SDK_VERSION, &sd, &pSwap, &pDevice, &featureLevel, &pCtx);

    if (FAILED(hr)) {
        DestroyWindow(hWnd);
        UnregisterClassA("DXDummy", wc.hInstance);
        return false;
    }

    // 从 SwapChain 的虚函数表中获取 Present 和 ResizeBuffers 地址
    void** vtable = *(void***)pSwap;
    *pPresentAddr = vtable[8];   // IDXGISwapChain::Present
    *pResizeAddr = vtable[13];   // IDXGISwapChain::ResizeBuffers

    pCtx->Release();
    pSwap->Release();
    pDevice->Release();
    DestroyWindow(hWnd);
    UnregisterClassA("DXDummy", wc.hInstance);
    return true;
}

// --- Hook::Init ---
bool Hook::Init() {
    printf("[Hook] Initializing MinHook...\n");
    g_unloadRequested.store(false, std::memory_order_release);
    g_unloadCompleted.store(false, std::memory_order_release);
    g_shutdownStarted.store(false, std::memory_order_release);

    if (MH_Initialize() != MH_OK) {
        printf("[Hook] MH_Initialize failed.\n");
        return false;
    }

    void* pPresent = nullptr;
    void* pResize = nullptr;
    if (!GetDX11VTable(&pPresent, &pResize)) {
        printf("[Hook] Failed to get DX11 VTable.\n");
        return false;
    }

    printf("[Hook] Present: %p, ResizeBuffers: %p\n", pPresent, pResize);

    // Hook Present
    if (MH_CreateHook(pPresent, (void*)hkPresent, (void**)&oPresent) != MH_OK) {
        printf("[Hook] Failed to hook Present.\n");
        return false;
    }

    // Hook ResizeBuffers
    if (MH_CreateHook(pResize, (void*)hkResizeBuffers, (void**)&oResizeBuffers) != MH_OK) {
        printf("[Hook] Failed to hook ResizeBuffers.\n");
        return false;
    }

    // Install skill cooldown-check bypass hooks
    TryCreateCooldownBypassHooks();

    MH_EnableHook(MH_ALL_HOOKS);
    Hook::g_initialized = true;
    printf("[Hook] All hooks enabled.\n");
    return true;
}

void Hook::RequestUnload() {
    g_unloadRequested.store(true, std::memory_order_release);
}

bool Hook::IsUnloadCompleted() {
    return g_unloadCompleted.load(std::memory_order_acquire);
}

void Hook::MarkManualTeleportCallBegin() {
    g_manualTeleportCallDepth.fetch_add(1, std::memory_order_acq_rel);
}

void Hook::MarkManualTeleportCallEnd() {
    int32_t old = g_manualTeleportCallDepth.fetch_sub(1, std::memory_order_acq_rel);
    if (old <= 1) {
        g_manualTeleportCallDepth.store(0, std::memory_order_release);
    }
}

bool Hook::GetLastObservedTeleportValidationData(uintptr_t* outValidationDataPtr,
                                                 int32_t* outReason,
                                                 int32_t* outUiType) {
    const uint64_t seq = g_observedTeleportValidationSequence.load(std::memory_order_acquire);
    if (seq == 0) {
        return false;
    }

    if (outValidationDataPtr) {
        *outValidationDataPtr = g_observedTeleportValidationDataPtr.load(std::memory_order_acquire);
    }
    if (outReason) {
        *outReason = g_observedTeleportValidationReason.load(std::memory_order_acquire);
    }
    if (outUiType) {
        *outUiType = g_observedTeleportValidationUiType.load(std::memory_order_acquire);
    }

    return true;
}

bool Hook::GetLastObservedTeleportParams(int32_t* outLevelId,
                                         int32_t* outReason,
                                         int32_t* outUiType,
                                         uint32_t* outHubNodeId) {
    const uint64_t seq = g_observedTeleportSequence.load(std::memory_order_acquire);
    if (seq == 0) {
        return false;
    }

    if (outLevelId) {
        *outLevelId = g_observedTeleportLevelId.load(std::memory_order_acquire);
    }
    if (outReason) {
        *outReason = g_observedTeleportReason.load(std::memory_order_acquire);
    }
    if (outUiType) {
        *outUiType = g_observedTeleportUiType.load(std::memory_order_acquire);
    }
    if (outHubNodeId) {
        *outHubNodeId = g_observedTeleportHubNodeId.load(std::memory_order_acquire);
    }

    return true;
}

// --- Hook::Shutdown ---
void Hook::Shutdown() {
    bool expected = false;
    if (!g_shutdownStarted.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    printf("[Hook] Shutting down...\n");

    // 禁用并移除所有 Hook
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    ShutdownCheatRuntime();

    const bool hadImGui = g_imguiInitialized.exchange(false, std::memory_order_acq_rel);

    // 恢复 WndProc
    if (oWndProc && Hook::g_gameWindow) {
        SetWindowLongPtrW(Hook::g_gameWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        oWndProc = nullptr;
    }

    // 清理 ImGui
    if (hadImGui && ImGui::GetCurrentContext()) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    // 释放 DX11 资源
    if (g_pRenderTargetView) { g_pRenderTargetView->Release(); g_pRenderTargetView = nullptr; }
    if (g_pd3dContext) { g_pd3dContext->Release(); g_pd3dContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

    Hook::g_initialized = false;
    g_unloadCompleted.store(true, std::memory_order_release);
    printf("[Hook] Shutdown complete.\n");
}
