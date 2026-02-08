#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <mutex>
#include <vector>
#include <atomic>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

#include "GameStructs.h"
#include "imgui.h"

namespace Hook {
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
}

namespace {
struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

using GameInstanceGetInstanceFn = void* (*)();
using GameInstanceGetMainCharPtrFn = Entity* (*)(void*, void*);
using GameInstanceGetGameplayNetworkFn = void* (*)(void*, void*);
using EntityGetAbilityComFn = AbilitySystem* (*)(Entity*, void*);
using EntityGetPositionFn = Vec3 (*)(Entity*, void*);
using EntityGetRootComFn = void* (*)(Entity*, void*);
using EntityGetEntityDataFn = void* (*)(Entity*, void*);
using EntityGetInLevelDataFn = void* (*)(Entity*, void*);
using Il2CppStringNewFn = void* (*)(const char*);
using RootComponentGetPositionFn = Vec3 (*)(void*, void*);
using RootComponentSetPositionFn = void (*)(void*, Vec3, void*);
using RootComponentTrySyncTransformToDataFn = void (*)(void*, void*);
using RootComponentOnPositionChangedFn = void (*)(void*, void*);
using GameActionTeleportToPositionFn = void (*)(int32_t, void*, const Vec3&, const Vec3&, int32_t, void*, uint32_t, void*);
using TeleportProcessorC2STeleportFn = void (*)(void*, int32_t, const Vec3&, const Vec3&, int32_t, int32_t, void*, uint32_t, void*);
using GameplayNetworkGetEnteringLevelIdFn = int32_t (*)(void*, void*);
using GameplayNetworkC2STeleportFn = void (*)(void*, int32_t, const Vec3&, const Vec3&, int32_t, int32_t, void*, uint32_t, void*);
using GameplayNetworkC2STeleportWithValidationDataFn = void (*)(void*, void*, void*, const Vec3&, const Vec3&, void*);
using WorldInfoGetCurLevelIdFn = void* (*)(void*, void*);
using WorldInfoGetCurLevelIdNumFn = int32_t (*)(void*, void*);
using MapManagerGetCurrLevelIdFn = void* (*)(void*, void*);
using GameUtilLevelIdStringToNumFn = int32_t (*)(void*, void*);
using MapManagerGetLevelSubHubNodeIdFn = uint32_t (*)(void*, void*);
using TeleportProcessorC2STeleportWithValidationDataFn = void (*)(void*, void*, void*, const Vec3&, const Vec3&, void*);
using AbilitySystemGetAttributesFn = Attributes* (*)(AbilitySystem*, void*);
using AbilitySystemGetHpFn = double (*)(AbilitySystem*, void*);
using AbilitySystemGetServerHpFn = double (*)(AbilitySystem*, void*);
using AbilitySystemGetUltimateSpFn = float (*)(AbilitySystem*, void*);
using AbilitySystemSetUltimateSpFn = void (*)(AbilitySystem*, float, void*);
using AbilitySystemGetSkillCooldownScalarFn = float (*)(AbilitySystem*, void*);
using ObjectContainerGetAllInteractivesFn = void* (*)(void*, void*);
using ObjectContainerGetAllEntitiesFn = void* (*)(void*, void*);
using BaseEntityDataGetTemplateIdFn = void* (*)(void*, void*);
using GameUtilGetMainCameraFn = void* (*)(void*);
using CameraWorldToScreenPointFn = Vec3 (*)(void*, Vec3, void*);

constexpr uintptr_t kGameInstanceGetInstanceRva = 0x49C58A0;
constexpr uintptr_t kGameInstanceGetMainCharPtrRva = 0x49C5C30;
constexpr uintptr_t kGameInstanceGetGameplayNetworkRva = 0x49C5C80;
constexpr uintptr_t kGameActionTeleportToPositionRva = 0x510FAA0;
constexpr uintptr_t kTeleportProcessorC2STeleportRva = 0x4D5B100;
constexpr uintptr_t kTeleportProcessorC2STeleportWithValidationDataRva = 0x4D5B870;
constexpr uintptr_t kGameplayNetworkGetEnteringLevelIdRva = 0x267C0C0;
constexpr uintptr_t kGameplayNetworkC2STeleportRva = 0x4C374F0;
constexpr uintptr_t kGameplayNetworkC2STeleportWithValidationDataRva = 0x4C376E0;
constexpr uintptr_t kWorldInfoGetCurLevelIdRva = 0x41CBC80;
constexpr uintptr_t kWorldInfoGetCurLevelIdNumRva = 0x41CBCA0;
constexpr uintptr_t kMapManagerGetCurrLevelIdRva = 0x410C430;
constexpr uintptr_t kGameUtilLevelIdStringToNumRva = 0x3B47E70;
constexpr uintptr_t kMapManagerGetLevelSubHubNodeIdRva = 0x4DDAFA0;
constexpr uintptr_t kEntityGetAbilityComRva = 0x37EC3A0;
constexpr uintptr_t kEntityGetPositionRva = 0x4102AB0;
constexpr uintptr_t kEntityGetRootComRva = 0x26CE8A0;
constexpr uintptr_t kEntityGetEntityDataRva = 0x26CE790;
constexpr uintptr_t kEntityGetInLevelDataRva = 0x26CE660;
constexpr uintptr_t kRootComponentGetPositionRva = 0x3FEE440;
constexpr uintptr_t kRootComponentSetPositionRva = 0x40125D0;
constexpr uintptr_t kRootComponentTrySyncTransformToDataRva = 0x4013490;
constexpr uintptr_t kRootComponentOnPositionChangedRva = 0x4017E20;
constexpr uintptr_t kAbilitySystemGetAttributesRva = 0x37CCCD0;
constexpr uintptr_t kAbilitySystemGetHpRva = 0x39B68D0;
constexpr uintptr_t kAbilitySystemGetServerHpRva = 0x39B6920;
constexpr uintptr_t kAbilitySystemGetUltimateSpRva = 0x3886D10;
constexpr uintptr_t kAbilitySystemSetUltimateSpRva = 0x39B6970;
constexpr uintptr_t kAbilitySystemGetSkillCooldownScalarRva = 0x39B7060;
constexpr uintptr_t kObjectContainerGetAllInteractivesRva = 0x26CE8C0;
constexpr uintptr_t kObjectContainerGetAllEntitiesRva = 0x2667620;
constexpr uintptr_t kBaseEntityDataGetTemplateIdRva = 0x26CE230;
constexpr uintptr_t kGameUtilGetMainCameraRva = 0x3B45C90;
constexpr uintptr_t kCameraWorldToScreenPointRva = 0xB66F7F0;
constexpr int kAttrIndexSkillCooldownScalar = 0x10;
constexpr int kAttrIndexUltimateSpGainScalar = 0x2D;
constexpr int kAttrIndexAtbCostAddition = 0x2E;
constexpr int kAttrIndexSkillCooldownAddition = 0x2F;
constexpr int kAttrIndexCriticalRate = 0x09;
constexpr int kAttrIndexCriticalDamageIncrease = 0x0A;
constexpr int kAttrIndexNormalAttackDamageIncrease = 0x11;
constexpr int kAttrIndexUltimateSkillDamageIncrease = 0x1C;
constexpr int kAttrIndexNormalSkillDamageIncrease = 0x20;
constexpr int kAttrIndexComboSkillDamageIncrease = 0x21;
constexpr int kAttrIndexPhysicalDamageIncrease = 0x32;
constexpr int kAttrIndexFireDamageIncrease = 0x33;
constexpr int kAttrIndexPulseDamageIncrease = 0x34;
constexpr int kAttrIndexCrystDamageIncrease = 0x35;
constexpr int kAttrIndexNaturalDamageIncrease = 0x36;
constexpr int kAttrIndexEtherDamageIncrease = 0x37;

constexpr float kPresetUltimateSp = 9999.0f;
constexpr float kPresetSkillCooldownScalar = 0.01f;
constexpr float kPresetSkillCooldownAddition = -999.0f;
constexpr float kPresetAtbCostAddition = -999.0f;
constexpr float kPresetUltimateSpGainScalar = 10.0f;

constexpr uintptr_t kGameInstancePlayerControllerOffset = 0x28;
constexpr uintptr_t kGameInstanceGameplayNetworkOffset = 0x38;
constexpr uintptr_t kGameplayNetworkEnteringLevelIdOffset = 0x18;
constexpr uintptr_t kGameplayNetworkTeleportProcessorOffset = 0x30;
constexpr uintptr_t kGameInstancePlayerOffset = 0x18;
constexpr uintptr_t kGamePlayerMapManagerOffset = 0x90;
constexpr uintptr_t kMapManagerCurrLevelIdOffset = 0x218;
constexpr uintptr_t kGamePlayerSquadManagerOffset = 0x48;
constexpr uintptr_t kSquadManagerClientCharsOffset = 0x80;
constexpr uintptr_t kManagedListItemsOffset = 0x10;
constexpr uintptr_t kManagedListSizeOffset = 0x18;
constexpr float kPresetAllSquadUltimateSp = 50000.0f;
constexpr float kPresetInfiniteDashCount = 9.0f;

constexpr uintptr_t kGameInstanceWorldOffset = 0x10;
constexpr uintptr_t kGameWorldWorldInfoOffset = 0x1E0;
constexpr uintptr_t kGameWorldObjectContainerOffset = 0x1F8;
constexpr uintptr_t kGameWorldEntityManagerOffset = 0x1D8;
constexpr uintptr_t kWorldInfoCurLevelOffset = 0xC0;
constexpr uintptr_t kGameLevelIdNumOffset = 0x20;
constexpr std::array<uintptr_t, 5> kGameWorldObjectContainerOffsets = {0x170, 0x1F8, 0x200, 0x208, 0x210};
constexpr std::array<uintptr_t, 5> kObjectContainerInteractivesOffsets = {0xB0, 0x98, 0x90, 0xB8, 0xA8};
constexpr uintptr_t kEntityManagerDataStorageOffset = 0x88;
constexpr uintptr_t kEntityDataStorageContainersOffset = 0x10;
constexpr uintptr_t kDataContainerDataDictOffset = 0x10;
constexpr uintptr_t kBaseEntityDataEntityRefOffset = 0x50;
constexpr uintptr_t kEntityRootComponentOffset = 0xA8;
constexpr uintptr_t kEntityDataOffset = 0xA0;
constexpr uintptr_t kEntityDataTemplateIdOffset = 0x68;
constexpr uintptr_t kEntityObjectTypeOffset = 0x5C;
constexpr uintptr_t kEntityInLevelDataOffset = 0x90;
constexpr uintptr_t kLevelEntityDataIdKeyOffset = 0x30;
constexpr uintptr_t kLevelEntityDataPositionOffset = 0x3C;
constexpr uintptr_t kRootComponentPositionOffset = 0x90;
constexpr uintptr_t kRootComponentSyncedPositionOffset = 0x154;
constexpr int kTeleportReasonClientTeleportBegin = 16;
constexpr int kTeleportReasonClientSpatialCrossingMove = 17;
constexpr int kTeleportReasonClientCutsceneTp = 18;
constexpr int kTeleportReasonClientGuideTp = 19;
constexpr int kTeleportUiTypeDefault = 0;
constexpr int kTeleportUiTypeNoLoading = 3;
constexpr int kTeleportUiTypeWithLoading = 4;
constexpr int kTeleportUiTypeManual = 5;
constexpr uintptr_t kBaseEntityDataPositionOffset = 0x8C;
constexpr uintptr_t kEntityInteractiveRootOffset = 0x1D0;

constexpr std::array<uintptr_t, 3> kStaticFieldsOffsets = {0xB8, 0xB0, 0xC0};
constexpr uintptr_t kGameInstanceStaticInstanceOffset = 0x0;
constexpr uintptr_t kPlayerControllerDumpBytes = 0x200;

static uintptr_t g_gameBaseAddress = 0;
static uintptr_t g_typeInfoOffset = 0;
static char g_offsetInput[32] = "0xFC2FA18";
static char g_statusText[128] = "";
static bool g_statusError = false;
static int g_attributeDisplayLimit = 160;

struct StatEditorState {
    bool initializedFromGame = false;

    double hpClient = 0.0;
    double hpServer = 0.0;
    float skillCooldownScalar = 1.0f;
    float skillCooldownAddition = 0.0f;
    float atbCostAddition = 0.0f;
    float atbValue = 0.0f;
    float ultimateSpGainScalar = 1.0f;
    float ultimateSp = 0.0f;
    float currentDash = 0.0f;
    float maxDash = 0.0f;
    float criticalRate = 1.0f;
    float damageMultiplier = 1.0f;

    bool lockHpClient = false;
    bool lockHpServer = false;
    bool lockSkillCooldown = false;
    bool lockSkillCooldownAddition = false;
    bool lockAtbCostAddition = false;
    bool lockAtbValue = false;
    bool lockUltimateSpGainScalar = false;
    bool lockUltimateSp = false;
    bool lockCurrentDash = false;
    bool lockMaxDash = false;
    bool lockCriticalRate = false;
    bool lockDamageMultiplier = false;
    bool lockInfiniteBlueNoCd = false;
    bool lockNoCooldownBypass = false;
    bool lockAllSquadUltimateNoCd = false;
    bool lockInfiniteStamina = false;
    bool lockInfiniteBlink = false;
};

static StatEditorState g_statEditor;
static char g_editStatusText[128] = "";
static bool g_editStatusError = false;

struct NearbyChestInfo {
    Entity* entity = nullptr;
    uintptr_t entityAddress = 0;
    Vec3 position{};
    float distance = 0.0f;
    uint32_t instanceUid = 0;
    uint64_t serverId = 0;
    bool hasPosition = false;
    bool keywordMatched = false;
    int32_t objectType = -1;
    bool hasInteractiveRoot = false;
    std::string name;
    std::string nameDisplay;
    std::string templateId;
    std::string levelEntityId;
    std::string templateIdDisplay;
};

struct TreasureToolState {
    bool onlyChestKeyword = false;
    bool showRadar = true;
    bool showChestList = true;
    bool enableScreenEsp = true;
    bool espShowCoordinates = true;
    bool enableTeleportHold = false;
    bool autoLockAfterTeleport = false;
    bool teleportLockEnabled = false;
    bool teleportLockHasTarget = false;
    bool drawAllInteractives = true;
    bool limitByDistance = false;
    bool templateIdExactMatch = false;
    int teleportHoldMs = 4500;
    int teleportWriteIntervalMs = 35;
    float scanRange = 6000.0f;
    float radarRange = 120.0f;
    float teleportHeightOffset = 0.3f;
    float customTeleport[3] = {0.0f, 0.0f, 0.0f};
    bool customTeleportInitialized = false;
    int selectedChestIndex = -1;
    int espListMaxCount = 40;
    char templateIdFilter[96] = "";
    Vec3 teleportHoldTarget{};
    Vec3 teleportLockTarget{};
    uint64_t teleportHoldUntilTick = 0;
    uint64_t teleportLastWriteTick = 0;
    char statusText[160] = "";
    bool statusError = false;
};

static TreasureToolState g_treasureTool;
static std::vector<NearbyChestInfo> g_nearbyChests;
static uint64_t g_lastChestRefreshTick = 0;

struct InteractiveScanDebugState {
    int containerCandidateCount = 0;
    int listCandidateCount = 0;
    int listHeaderSuccessCount = 0;
    int directWorldObjectContainerHitCount = 0;
    int directAllEntitiesListHitCount = 0;
    int directAllInteractivesListHitCount = 0;
    int entityManagerDataContainerCount = 0;
    int entityManagerEntityHitCount = 0;
    int uniqueEntityAddressCount = 0;
    int validEntityCount = 0;
    int withPositionCount = 0;
    int withoutPositionCount = 0;
    int posFromGetterCount = 0;
    int posFromRootCount = 0;
    int posFromEntityDataCount = 0;
    int posFromLevelDataCount = 0;
    int filteredByDuplicatePosCount = 0;
    int filteredByDistanceCount = 0;
    int filteredByKeywordCount = 0;
    int filteredByTemplateCount = 0;
    bool autoNearCapApplied = false;
    uint64_t lastRefreshTick = 0;
};

static InteractiveScanDebugState g_interactiveScanDebug;

static std::atomic<uintptr_t> g_fastUltimateAbilityAddress{0};
static std::atomic<uintptr_t> g_fastPlayerControllerAddress{0};
static std::atomic<uintptr_t> g_fastGameInstanceAddress{0};
static std::atomic<bool> g_debugWindowVisible{true};
static std::atomic<uintptr_t> g_mainCharacterRootComponentAddress{0};
static std::atomic<uintptr_t> g_mainCharacterEntityAddress{0};
static std::atomic<int> g_lastTeleportPath{ -1 };
static std::atomic<int32_t> g_cachedTeleportLevelId{0};
static std::atomic<int> g_teleportLogSequence{0};
static std::atomic<int> g_teleportForceActive{0};
static std::atomic<float> g_teleportForceTargetX{0.0f};
static std::atomic<float> g_teleportForceTargetY{0.0f};
static std::atomic<float> g_teleportForceTargetZ{0.0f};
static std::atomic<float> g_teleportForceOffsetX{0.0f};
static std::atomic<float> g_teleportForceOffsetY{0.0f};
static std::atomic<float> g_teleportForceOffsetZ{0.0f};
static std::atomic<bool> g_fastUltimateThreadRunning{false};
static std::atomic<bool> g_fastUltimateThreadStop{false};
static HANDLE g_fastUltimateThread = nullptr;
static std::mutex g_tpLogMutex;

static void WriteTeleportLog(const char* fmt, ...);

enum class CustomValueType {
    Bool = 0,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Float,
    Double,
    Pointer,
    Byte
};

enum class CustomBaseSource {
    PlayerController = 0,
    Entity,
    AbilitySystem,
    GameInstance,
    AbsoluteAddress
};

struct CustomWatchItem {
    int id = 0;
    char label[64] = "";
    char offsetInput[32] = "0x0";
    uintptr_t offsetOrAddress = 0;
    bool parsedOk = true;

    CustomBaseSource baseSource = CustomBaseSource::PlayerController;
    CustomValueType valueType = CustomValueType::Float;
    bool lockWrite = false;

    bool boolValue = false;
    int32_t i32Value = 0;
    uint32_t u32Value = 0;
    int64_t i64Value = 0;
    uint64_t u64Value = 0;
    float f32Value = 0.0f;
    double f64Value = 0.0;
    uintptr_t ptrValue = 0;
    uint8_t u8Value = 0;
};

struct CustomWatchDraft {
    char label[64] = "";
    char offsetInput[32] = "0x0";
    int baseSourceIndex = 0;
    int valueTypeIndex = static_cast<int>(CustomValueType::Float);
};

static const char* kCustomBaseSourceItems[] = {
    u8"\u73a9\u5bb6\u63a7\u5236\u5668 + \u504f\u79fb",
    u8"\u5b9e\u4f53 + \u504f\u79fb",
    u8"\u80fd\u529b\u7cfb\u7edf + \u504f\u79fb",
    u8"\u6e38\u620f\u5b9e\u4f8b + \u504f\u79fb",
    u8"\u7edd\u5bf9\u5730\u5740"
};

static const char* kCustomValueTypeItems[] = {
    u8"\u5e03\u5c14(bool, 1\u5b57\u8282)",
    u8"\u6709\u7b26\u53f7\u6574\u6570(int32)",
    u8"\u65e0\u7b26\u53f7\u6574\u6570(uint32)",
    u8"\u957f\u6574\u6570(int64)",
    u8"\u65e0\u7b26\u53f7\u957f\u6574\u6570(uint64)",
    u8"\u6d6e\u70b9\u6570(float)",
    u8"\u53cc\u7cbe\u5ea6(double)",
    u8"\u6307\u9488(pointer)",
    u8"\u5b57\u8282(byte)"
};

static std::vector<CustomWatchItem> g_customWatchItems;
static CustomWatchDraft g_customWatchDraft;
static int g_customWatchNextId = 1;
static char g_customWatchStatusText[160] = "";
static bool g_customWatchStatusError = false;

struct ResolveResult {
    bool ok = false;
    const char* error = u8"未开始";
    const char* path = u8"无";

    uintptr_t getterFunctionAddress = 0;
    uintptr_t typeInfoStorageAddress = 0;
    uintptr_t typeInfoAddress = 0;
    uintptr_t staticFieldsAddress = 0;
    uintptr_t staticFieldsOffsetUsed = 0;
    uintptr_t gameInstanceAddress = 0;
    uintptr_t playerControllerFieldAddress = 0;
    PlayerController* playerController = nullptr;
};

static bool TryCallAbilitySetUltimateSp(AbilitySystem* ability, float value);
static bool TryReadAttributeDoubleByIndex(AbilitySystem* ability, int index, double& outValue);
static bool TryWriteAttributeDoubleByIndex(AbilitySystem* ability, int index, double value);

template <typename T>
static bool WriteValueSafe(uintptr_t addr, T value);

static bool IsValidPtr(const void* ptr) {
    return ptr != nullptr && !IsBadReadPtr(const_cast<void*>(ptr), sizeof(void*));
}

static bool IsCanonicalUserPtr(uintptr_t addr) {
    return addr >= 0x10000 && addr <= 0x00007FFFFFFFFFFFULL;
}

template <typename T>
static bool ReadValue(uintptr_t addr, T& outValue) {
    if (addr == 0 || IsBadReadPtr(reinterpret_cast<void*>(addr), sizeof(T))) {
        return false;
    }

    outValue = *reinterpret_cast<T*>(addr);
    return true;
}

template <typename T>
static bool ReadPcField(PlayerController* pc, uintptr_t offset, T& outValue) {
    return ReadValue(reinterpret_cast<uintptr_t>(pc) + offset, outValue);
}

static bool ReadPcBoolField(PlayerController* pc, uintptr_t offset, bool& outValue) {
    uint8_t value = 0;
    if (!ReadPcField(pc, offset, value)) {
        return false;
    }

    outValue = (value != 0);
    return true;
}

static bool ReadBytesSafe(uintptr_t address, void* buffer, size_t size) {
    if (!IsCanonicalUserPtr(address) || buffer == nullptr || size == 0) {
        return false;
    }

    SIZE_T bytesRead = 0;
    const BOOL ok = ReadProcessMemory(GetCurrentProcess(),
                                      reinterpret_cast<LPCVOID>(address),
                                      buffer,
                                      size,
                                      &bytesRead);
    return ok != FALSE && bytesRead == static_cast<SIZE_T>(size);
}

static bool IsReasonableFloat(float value) {
    return std::isfinite(value) && value > -10000000.0f && value < 10000000.0f;
}

static bool IsReasonableWorldPosition(const Vec3& value) {
    return IsReasonableFloat(value.x) && IsReasonableFloat(value.y) && IsReasonableFloat(value.z);
}

static bool IsUsableWorldPosition(const Vec3& value) {
    if (!IsReasonableWorldPosition(value)) {
        return false;
    }

    const float absSum = std::fabs(value.x) + std::fabs(value.y) + std::fabs(value.z);
    return absSum > 0.001f;
}

static bool IsLikelyEntityAddress(uintptr_t entityAddress) {
    if (!IsCanonicalUserPtr(entityAddress)) {
        return false;
    }

    int32_t objectType = -1;
    const bool hasObjectType = ReadValue(entityAddress + kEntityObjectTypeOffset, objectType);
    if (hasObjectType && (objectType < -1 || objectType > 2048)) {
        return false;
    }

    uintptr_t entityDataAddress = 0;
    uintptr_t rootAddress = 0;
    uintptr_t levelEntityAddress = 0;
    uintptr_t interactiveRootAddress = 0;
    uintptr_t nameAddress = 0;

    const bool hasEntityData = ReadValue(entityAddress + kEntityDataOffset, entityDataAddress) &&
                               IsCanonicalUserPtr(entityDataAddress);
    const bool hasRootCom = ReadValue(entityAddress + kEntityRootComponentOffset, rootAddress) &&
                            IsCanonicalUserPtr(rootAddress);
    const bool hasLevelEntity = ReadValue(entityAddress + kEntityInLevelDataOffset, levelEntityAddress) &&
                                IsCanonicalUserPtr(levelEntityAddress);
    const bool hasInteractiveRoot = ReadValue(entityAddress + kEntityInteractiveRootOffset, interactiveRootAddress) &&
                                    IsCanonicalUserPtr(interactiveRootAddress);
    const bool hasNameRef = ReadValue(entityAddress + 0x10, nameAddress) &&
                            IsCanonicalUserPtr(nameAddress);

    uint32_t instanceUid = 0;
    uint64_t serverId = 0;
    const bool hasUid = ReadValue(entityAddress + 0x18, instanceUid);
    const bool hasServerId = ReadValue(entityAddress + 0x30, serverId);

    if (!(hasEntityData || hasRootCom || hasLevelEntity)) {
        return false;
    }

    if (!(hasEntityData || hasRootCom)) {
        return false;
    }

    if (!hasNameRef && !((hasUid && instanceUid != 0) || (hasServerId && serverId != 0))) {
        return false;
    }

    bool hasReadableTemplateIdRef = false;
    if (hasEntityData) {
        uintptr_t templateIdAddress = 0;
        hasReadableTemplateIdRef = ReadValue(entityDataAddress + kEntityDataTemplateIdOffset, templateIdAddress) &&
                                   IsCanonicalUserPtr(templateIdAddress);
    }

    int signalCount = 0;
    signalCount += hasEntityData ? 2 : 0;
    signalCount += hasRootCom ? 2 : 0;
    signalCount += hasLevelEntity ? 1 : 0;
    signalCount += hasNameRef ? 1 : 0;
    signalCount += hasInteractiveRoot ? 1 : 0;
    signalCount += (hasObjectType && objectType >= 0) ? 1 : 0;
    signalCount += ((hasUid && instanceUid != 0) || (hasServerId && serverId != 0)) ? 1 : 0;
    signalCount += hasReadableTemplateIdRef ? 1 : 0;

    return signalCount >= 4;
}

static std::string ToLowerAscii(std::string value) {
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
    }
    return value;
}

static bool ContainsAsciiIgnoreCase(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }

    const std::string lowerHaystack = ToLowerAscii(haystack);
    const std::string lowerNeedle = ToLowerAscii(needle);
    return lowerHaystack.find(lowerNeedle) != std::string::npos;
}

static std::string TrimAsciiWhitespace(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }

    size_t head = 0;
    while (head < value.size() && std::isspace(static_cast<unsigned char>(value[head]))) {
        ++head;
    }

    if (head > 0) {
        value = value.substr(head);
    }
    return value;
}

static bool IsLikelyReadableTemplateId(const std::string& value) {
    if (value.empty() || value.size() > 128) {
        return false;
    }

    int validCharCount = 0;
    for (unsigned char ch : value) {
        if (ch < 0x20 || ch > 0x7E) {
            return false;
        }

        const bool isAlphaNum = (ch >= 'a' && ch <= 'z') ||
                                (ch >= 'A' && ch <= 'Z') ||
                                (ch >= '0' && ch <= '9');
        const bool isSymbol = (ch == '_') || (ch == '-') || (ch == '.') ||
                              (ch == '/') || (ch == '\\') || (ch == ':');
        if (!isAlphaNum && !isSymbol) {
            return false;
        }

        ++validCharCount;
    }

    if (validCharCount < 3) {
        return false;
    }

    return value.find("???") == std::string::npos;
}

static bool IsLikelyReadableUtf8Text(const std::string& value) {
    if (value.empty() || value.size() > 192) {
        return false;
    }

    int badAsciiCount = 0;
    for (unsigned char ch : value) {
        if (ch < 0x20 && ch != ' ') {
            ++badAsciiCount;
        }
    }
    if (badAsciiCount > 0) {
        return false;
    }

    const int wideLen = MultiByteToWideChar(CP_UTF8,
                                            MB_ERR_INVALID_CHARS,
                                            value.data(),
                                            static_cast<int>(value.size()),
                                            nullptr,
                                            0);
    return wideLen > 0;
}

static std::string NormalizeNameForDisplay(std::string value) {
    value = TrimAsciiWhitespace(std::move(value));
    if (value.empty()) {
        return {};
    }

    for (char& ch : value) {
        if (ch == '\r' || ch == '\n' || ch == '\t') {
            ch = ' ';
        }
    }

    std::string compact;
    compact.reserve(value.size());
    bool lastSpace = false;
    for (unsigned char ch : value) {
        const bool isSpace = std::isspace(ch) != 0;
        if (isSpace) {
            if (lastSpace) {
                continue;
            }
            compact.push_back(' ');
            lastSpace = true;
            continue;
        }

        compact.push_back(static_cast<char>(ch));
        lastSpace = false;
    }

    compact = TrimAsciiWhitespace(std::move(compact));
    if (compact.empty() || !IsLikelyReadableUtf8Text(compact)) {
        return {};
    }

    constexpr size_t kMaxLabelBytes = 72;
    if (compact.size() <= kMaxLabelBytes) {
        return compact;
    }

    std::string shortened = compact.substr(0, kMaxLabelBytes);
    while (!shortened.empty() && (static_cast<unsigned char>(shortened.back()) & 0xC0) == 0x80) {
        shortened.pop_back();
    }
    shortened += "...";
    return shortened;
}

static std::string BuildUtf8BytePreview(const std::string& value, size_t maxBytes = 12) {
    if (value.empty()) {
        return {};
    }

    const size_t previewLen = (std::min)(value.size(), maxBytes);
    std::string preview;
    preview.reserve(previewLen * 3 + 8);

    char byteHex[4] = {};
    for (size_t i = 0; i < previewLen; ++i) {
        if (i > 0) {
            preview.push_back(' ');
        }
        std::snprintf(byteHex, sizeof(byteHex), "%02X", static_cast<unsigned int>(static_cast<unsigned char>(value[i])));
        preview += byteHex;
    }

    if (value.size() > previewLen) {
        preview += " ...";
    }

    return preview;
}

static std::string NormalizeTemplateIdForDisplay(const std::string& rawValue, uintptr_t sourceAddress) {
    const std::string value = TrimAsciiWhitespace(rawValue);
    if (IsLikelyReadableTemplateId(value)) {
        return value;
    }

    if (value.empty()) {
        if (!IsCanonicalUserPtr(sourceAddress)) {
            return {};
        }

        char pointerOnly[40] = {};
        std::snprintf(pointerOnly, sizeof(pointerOnly), "str@0x%llX", static_cast<unsigned long long>(sourceAddress));
        return pointerOnly;
    }

    const std::string bytePreview = BuildUtf8BytePreview(value);
    char fallback[160] = {};
    if (IsCanonicalUserPtr(sourceAddress)) {
        std::snprintf(fallback,
                      sizeof(fallback),
                      "str@0x%llX [%s]",
                      static_cast<unsigned long long>(sourceAddress),
                      bytePreview.c_str());
    } else {
        std::snprintf(fallback, sizeof(fallback), "[%s]", bytePreview.c_str());
    }
    return fallback;
}

static const char* GetTemplateIdText(const NearbyChestInfo& chest) {
    if (!chest.templateIdDisplay.empty()) {
        return chest.templateIdDisplay.c_str();
    }
    if (IsLikelyReadableTemplateId(chest.templateId)) {
        return chest.templateId.c_str();
    }
    if (IsLikelyReadableTemplateId(chest.levelEntityId)) {
        return chest.levelEntityId.c_str();
    }
    return "-";
}

static const char* GetEntityNameText(const NearbyChestInfo& chest) {
    if (!chest.nameDisplay.empty()) {
        return chest.nameDisplay.c_str();
    }

    if (IsLikelyReadableTemplateId(chest.levelEntityId)) {
        return chest.levelEntityId.c_str();
    }
    if (IsLikelyReadableTemplateId(chest.templateId)) {
        return chest.templateId.c_str();
    }

    static thread_local std::string fallbackName;
    fallbackName = NormalizeNameForDisplay(chest.name);
    if (!fallbackName.empty()) {
        return fallbackName.c_str();
    }

    return "<unnamed>";
}

static std::string ReadManagedStringUtf8(uintptr_t strAddress, int maxChars = 96) {
    if (!IsCanonicalUserPtr(strAddress) || maxChars <= 0) {
        return {};
    }

    int32_t len = 0;
    if (!ReadValue(strAddress + 0x10, len) || len <= 0) {
        return {};
    }

    const int32_t clampedLen = (std::min)(len, maxChars);
    if (clampedLen <= 0 || clampedLen > 1024) {
        return {};
    }

    const uintptr_t contentAddress = strAddress + 0x14;
    const size_t byteCount = static_cast<size_t>(clampedLen) * sizeof(wchar_t);
    if (byteCount == 0 || byteCount > 4096) {
        return {};
    }

    std::wstring wide(static_cast<size_t>(clampedLen), L'\0');
    if (!ReadBytesSafe(contentAddress, wide.data(), byteCount)) {
        return {};
    }

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), clampedLen, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0 || utf8Len > 4096) {
        return {};
    }

    std::string out(static_cast<size_t>(utf8Len), '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, wide.data(), clampedLen, out.data(), utf8Len, nullptr, nullptr) <= 0) {
        return {};
    }

    out.erase(std::remove(out.begin(), out.end(), '\0'), out.end());
    return out;
}

enum class EntityPositionSource {
    None = 0,
    Getter,
    RootComponent,
    EntityData,
    LevelEntityData
};

static bool TryCallEntityGetPosition(Entity* entity, Vec3& outPosition) {
    outPosition = {0.0f, 0.0f, 0.0f};
    if (g_gameBaseAddress == 0 || !IsValidPtr(entity)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kEntityGetPositionRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<EntityGetPositionFn>(fnAddress);
        outPosition = fn(entity, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outPosition = {0.0f, 0.0f, 0.0f};
        return false;
    }

    return IsReasonableWorldPosition(outPosition);
}

static bool TryCallEntityGetRootCom(Entity* entity, uintptr_t& outRootComAddress) {
    outRootComAddress = 0;
    if (g_gameBaseAddress == 0 || !IsValidPtr(entity)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kEntityGetRootComRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<EntityGetRootComFn>(fnAddress);
        outRootComAddress = reinterpret_cast<uintptr_t>(fn(entity, nullptr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outRootComAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outRootComAddress);
}

static bool TryCallEntityGetEntityData(Entity* entity, uintptr_t& outEntityDataAddress) {
    outEntityDataAddress = 0;
    if (g_gameBaseAddress == 0 || !IsValidPtr(entity)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kEntityGetEntityDataRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<EntityGetEntityDataFn>(fnAddress);
        outEntityDataAddress = reinterpret_cast<uintptr_t>(fn(entity, nullptr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outEntityDataAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outEntityDataAddress);
}

static bool TryCallEntityGetInLevelData(Entity* entity, uintptr_t& outLevelDataAddress) {
    outLevelDataAddress = 0;
    if (g_gameBaseAddress == 0 || !IsValidPtr(entity)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kEntityGetInLevelDataRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<EntityGetInLevelDataFn>(fnAddress);
        outLevelDataAddress = reinterpret_cast<uintptr_t>(fn(entity, nullptr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelDataAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outLevelDataAddress);
}

static bool TryCallRootComponentGetPosition(uintptr_t rootComAddress, Vec3& outPosition) {
    outPosition = {0.0f, 0.0f, 0.0f};
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(rootComAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kRootComponentGetPositionRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<RootComponentGetPositionFn>(fnAddress);
        outPosition = fn(reinterpret_cast<void*>(rootComAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outPosition = {0.0f, 0.0f, 0.0f};
        return false;
    }

    return IsUsableWorldPosition(outPosition);
}

static bool TryCallRootComponentSetPosition(uintptr_t rootComAddress, const Vec3& targetPos) {
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(rootComAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kRootComponentSetPositionRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<RootComponentSetPositionFn>(fnAddress);
        fn(reinterpret_cast<void*>(rootComAddress), targetPos, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallRootComponentTrySyncTransformToData(uintptr_t rootComAddress) {
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(rootComAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kRootComponentTrySyncTransformToDataRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<RootComponentTrySyncTransformToDataFn>(fnAddress);
        fn(reinterpret_cast<void*>(rootComAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallRootComponentOnPositionChanged(uintptr_t rootComAddress) {
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(rootComAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kRootComponentOnPositionChangedRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<RootComponentOnPositionChangedFn>(fnAddress);
        fn(reinterpret_cast<void*>(rootComAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallGameInstanceGetGameplayNetwork(uintptr_t gameInstanceAddress, uintptr_t& outGameplayNetworkAddress) {
    outGameplayNetworkAddress = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(gameInstanceAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameInstanceGetGameplayNetworkRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameInstanceGetGameplayNetworkFn>(fnAddress);
        outGameplayNetworkAddress = reinterpret_cast<uintptr_t>(fn(reinterpret_cast<void*>(gameInstanceAddress), nullptr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outGameplayNetworkAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outGameplayNetworkAddress);
}

static bool TryCallGameplayNetworkGetEnteringLevelId(uintptr_t gameplayNetworkAddress, int32_t& outLevelId) {
    outLevelId = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(gameplayNetworkAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameplayNetworkGetEnteringLevelIdRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameplayNetworkGetEnteringLevelIdFn>(fnAddress);
        outLevelId = fn(reinterpret_cast<void*>(gameplayNetworkAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelId = 0;
        return false;
    }

    return outLevelId > 0;
}

static bool TryCallWorldInfoGetCurLevelId(uintptr_t worldInfoAddress, void*& outLevelIdString) {
    outLevelIdString = nullptr;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(worldInfoAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kWorldInfoGetCurLevelIdRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<WorldInfoGetCurLevelIdFn>(fnAddress);
        outLevelIdString = fn(reinterpret_cast<void*>(worldInfoAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelIdString = nullptr;
        return false;
    }

    return IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(outLevelIdString));
}

static bool TryCallWorldInfoGetCurLevelIdNum(uintptr_t worldInfoAddress, int32_t& outLevelIdNum) {
    outLevelIdNum = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(worldInfoAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kWorldInfoGetCurLevelIdNumRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<WorldInfoGetCurLevelIdNumFn>(fnAddress);
        outLevelIdNum = fn(reinterpret_cast<void*>(worldInfoAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelIdNum = 0;
        return false;
    }

    return outLevelIdNum > 0;
}

static bool TryCallMapManagerGetCurrLevelId(uintptr_t mapManagerAddress, void*& outLevelIdString) {
    outLevelIdString = nullptr;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(mapManagerAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kMapManagerGetCurrLevelIdRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<MapManagerGetCurrLevelIdFn>(fnAddress);
        outLevelIdString = fn(reinterpret_cast<void*>(mapManagerAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelIdString = nullptr;
        return false;
    }

    return IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(outLevelIdString));
}

static bool TryCallGameUtilLevelIdStringToNum(void* levelIdString, int32_t& outLevelIdNum) {
    outLevelIdNum = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(levelIdString))) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameUtilLevelIdStringToNumRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameUtilLevelIdStringToNumFn>(fnAddress);
        outLevelIdNum = fn(levelIdString, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outLevelIdNum = 0;
        return false;
    }

    return outLevelIdNum > 0;
}

static bool TryCallMapManagerGetLevelSubHubNodeId(uintptr_t mapManagerAddress,
                                                  void* levelIdString,
                                                  uint32_t& outHubNodeId) {
    outHubNodeId = 0;
    if (g_gameBaseAddress == 0 ||
        !IsCanonicalUserPtr(mapManagerAddress) ||
        !IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(levelIdString))) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kMapManagerGetLevelSubHubNodeIdRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<MapManagerGetLevelSubHubNodeIdFn>(fnAddress);
        outHubNodeId = fn(reinterpret_cast<void*>(mapManagerAddress), levelIdString);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outHubNodeId = 0;
        return false;
    }

    return outHubNodeId != 0;
}

static bool TryCallGameplayNetworkC2STeleport(uintptr_t gameplayNetworkAddress,
                                              int32_t levelId,
                                              const Vec3& targetPos,
                                              const Vec3& rotationEuler,
                                              int32_t reason,
                                              int32_t uiType,
                                              uint32_t hubNodeId) {
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(gameplayNetworkAddress) || levelId <= 0) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameplayNetworkC2STeleportRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameplayNetworkC2STeleportFn>(fnAddress);
        Hook::MarkManualTeleportCallBegin();
        fn(reinterpret_cast<void*>(gameplayNetworkAddress),
           levelId,
           targetPos,
           rotationEuler,
           reason,
           uiType,
           nullptr,
           hubNodeId,
           nullptr);
        Hook::MarkManualTeleportCallEnd();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Hook::MarkManualTeleportCallEnd();
        return false;
    }

    return true;
}

static bool TryCallGameplayNetworkC2STeleportWithValidationData(uintptr_t gameplayNetworkAddress,
                                                                void* validationData,
                                                                const Vec3& targetPos,
                                                                const Vec3& rotationEuler) {
    if (g_gameBaseAddress == 0 ||
        !IsCanonicalUserPtr(gameplayNetworkAddress) ||
        !IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(validationData))) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameplayNetworkC2STeleportWithValidationDataRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameplayNetworkC2STeleportWithValidationDataFn>(fnAddress);
        Hook::MarkManualTeleportCallBegin();
        fn(reinterpret_cast<void*>(gameplayNetworkAddress),
           validationData,
           nullptr,
           targetPos,
           rotationEuler,
           nullptr);
        Hook::MarkManualTeleportCallEnd();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Hook::MarkManualTeleportCallEnd();
        return false;
    }

    return true;
}

static void* TryCreateManagedString(const char* text) {
    if (!text) {
        return nullptr;
    }

    HMODULE gameAssembly = GetModuleHandleA("GameAssembly.dll");
    if (!gameAssembly) {
        return nullptr;
    }

    auto fn = reinterpret_cast<Il2CppStringNewFn>(GetProcAddress(gameAssembly, "il2cpp_string_new"));
    if (!fn) {
        return nullptr;
    }

    __try {
        return fn(text);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

static bool TryCallTeleportProcessorC2STeleport(uintptr_t teleportProcessorAddress,
                                                int32_t levelId,
                                                const Vec3& targetPos,
                                                const Vec3& rotationEuler,
                                                int32_t reason,
                                                int32_t uiType,
                                                uint32_t hubNodeId) {
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(teleportProcessorAddress) || levelId <= 0) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kTeleportProcessorC2STeleportRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<TeleportProcessorC2STeleportFn>(fnAddress);
        Hook::MarkManualTeleportCallBegin();
        fn(reinterpret_cast<void*>(teleportProcessorAddress),
           levelId,
           targetPos,
           rotationEuler,
           reason,
           uiType,
           nullptr,
           hubNodeId,
           nullptr);
        Hook::MarkManualTeleportCallEnd();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Hook::MarkManualTeleportCallEnd();
        return false;
    }

    return true;
}

static bool TryCallTeleportProcessorC2STeleportWithValidationData(uintptr_t teleportProcessorAddress,
                                                                  void* validationData,
                                                                  const Vec3& targetPos,
                                                                  const Vec3& rotationEuler) {
    if (g_gameBaseAddress == 0 ||
        !IsCanonicalUserPtr(teleportProcessorAddress) ||
        !IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(validationData))) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kTeleportProcessorC2STeleportWithValidationDataRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<TeleportProcessorC2STeleportWithValidationDataFn>(fnAddress);
        Hook::MarkManualTeleportCallBegin();
        fn(reinterpret_cast<void*>(teleportProcessorAddress),
           validationData,
           nullptr,
           targetPos,
           rotationEuler,
           nullptr);
        Hook::MarkManualTeleportCallEnd();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Hook::MarkManualTeleportCallEnd();
        return false;
    }

    return true;
}

static bool TryCallGameActionTeleportToPosition(const Vec3& targetPos,
                                                void* levelIdStr,
                                                const Vec3& rotationEuler,
                                                int32_t reason,
                                                int32_t uiType,
                                                uint32_t hubNodeId) {
    if (g_gameBaseAddress == 0) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameActionTeleportToPositionRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameActionTeleportToPositionFn>(fnAddress);
        Hook::MarkManualTeleportCallBegin();
        fn(reason,
           levelIdStr,
           targetPos,
           rotationEuler,
           uiType,
           nullptr,
           hubNodeId,
           nullptr);
        Hook::MarkManualTeleportCallEnd();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Hook::MarkManualTeleportCallEnd();
        return false;
    }

    return true;
}

static bool TryTeleportViaGameplayNetwork(Entity* entity, const Vec3& targetPos, bool writeLog = true) {
    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork begin entity=%p target=(%.3f, %.3f, %.3f)",
                         entity,
                         targetPos.x,
                         targetPos.y,
                         targetPos.z);
    }

    if (!IsValidPtr(entity)) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fail: invalid entity");
        }
        return false;
    }

    const Vec3 rotationEuler{0.0f, 0.0f, 0.0f};
    int32_t observedLevelId = 0;
    int32_t observedReason = -1;
    int32_t observedUiType = -1;
    uint32_t observedHubNodeId = 0;
    uintptr_t observedValidationDataPtr = 0;
    int32_t observedValidationReason = -1;
    int32_t observedValidationUiType = -1;
    const bool hasObservedTeleportParams = Hook::GetLastObservedTeleportParams(
        &observedLevelId,
        &observedReason,
        &observedUiType,
        &observedHubNodeId);
    const bool hasObservedValidationData = Hook::GetLastObservedTeleportValidationData(
        &observedValidationDataPtr,
        &observedValidationReason,
        &observedValidationUiType);
    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork observed params available=%d levelId=%d reason=%d uiType=%d hubNodeId=%u",
                         hasObservedTeleportParams ? 1 : 0,
                         observedLevelId,
                         observedReason,
                         observedUiType,
                         observedHubNodeId);
        WriteTeleportLog("TryTeleportViaGameplayNetwork observed validation available=%d ptr=0x%llX reason=%d uiType=%d",
                         hasObservedValidationData ? 1 : 0,
                         static_cast<unsigned long long>(observedValidationDataPtr),
                         observedValidationReason,
                         observedValidationUiType);
    }

    const uintptr_t gameInstanceAddress = g_fastGameInstanceAddress.load(std::memory_order_acquire);
    if (!IsCanonicalUserPtr(gameInstanceAddress)) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fail: invalid gameInstance=0x%llX",
                             static_cast<unsigned long long>(gameInstanceAddress));
        }
        return false;
    }

    uintptr_t gameplayNetworkAddress = 0;
    const bool gotGameplayNetworkByMethod =
        TryCallGameInstanceGetGameplayNetwork(gameInstanceAddress, gameplayNetworkAddress);
    if (!gotGameplayNetworkByMethod || !IsCanonicalUserPtr(gameplayNetworkAddress)) {
        ReadValue(gameInstanceAddress + kGameInstanceGameplayNetworkOffset, gameplayNetworkAddress);
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fallback gameplayNetwork from field: byMethod=%d addr=0x%llX",
                             gotGameplayNetworkByMethod ? 1 : 0,
                             static_cast<unsigned long long>(gameplayNetworkAddress));
        }
    } else if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork gameplayNetwork by method: addr=0x%llX",
                         static_cast<unsigned long long>(gameplayNetworkAddress));
    }

    if (!IsCanonicalUserPtr(gameplayNetworkAddress)) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fail: invalid gameplayNetwork=0x%llX",
                             static_cast<unsigned long long>(gameplayNetworkAddress));
        }
        return false;
    }

    int32_t levelId = 0;
    void* levelIdStr = nullptr;
    uint32_t hubNodeId = 0;
    const bool gotLevelIdByMethod = TryCallGameplayNetworkGetEnteringLevelId(gameplayNetworkAddress, levelId);
    if (!gotLevelIdByMethod) {
        ReadValue(gameplayNetworkAddress + kGameplayNetworkEnteringLevelIdOffset, levelId);
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fallback levelId from field(+0x%llX): levelId=%d",
                             static_cast<unsigned long long>(kGameplayNetworkEnteringLevelIdOffset),
                             levelId);
        }
    } else if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork levelId by method: levelId=%d", levelId);
    }

    if (levelId <= 0) {
        uintptr_t gamePlayerAddress = 0;
        if (ReadValue(gameInstanceAddress + kGameInstancePlayerOffset, gamePlayerAddress) &&
            IsCanonicalUserPtr(gamePlayerAddress)) {
            uintptr_t mapManagerAddress = 0;
            if (ReadValue(gamePlayerAddress + kGamePlayerMapManagerOffset, mapManagerAddress) &&
                IsCanonicalUserPtr(mapManagerAddress)) {
                void* mapCurrLevelIdStr = nullptr;
                if (!TryCallMapManagerGetCurrLevelId(mapManagerAddress, mapCurrLevelIdStr)) {
                    ReadValue(mapManagerAddress + kMapManagerCurrLevelIdOffset, mapCurrLevelIdStr);
                }

                if (IsCanonicalUserPtr(reinterpret_cast<uintptr_t>(mapCurrLevelIdStr))) {
                    levelIdStr = mapCurrLevelIdStr;
                    if (writeLog) {
                        const std::string levelIdTextFromMap =
                            ReadManagedStringUtf8(reinterpret_cast<uintptr_t>(mapCurrLevelIdStr), 64);
                        WriteTeleportLog("TryTeleportViaGameplayNetwork levelIdStr from MapManager.currLevelId: ptr=%p text=%s",
                                         mapCurrLevelIdStr,
                                         levelIdTextFromMap.empty() ? "<empty>" : levelIdTextFromMap.c_str());
                    }

                    int32_t levelIdFromMap = 0;
                    if (TryCallGameUtilLevelIdStringToNum(mapCurrLevelIdStr, levelIdFromMap)) {
                        levelId = levelIdFromMap;
                        if (writeLog) {
                            WriteTeleportLog("TryTeleportViaGameplayNetwork levelId from GameUtil.LevelIdStringToNum(MapManager.currLevelId): %d",
                                             levelId);
                        }
                    }

                    uint32_t subHubNodeId = 0;
                    if (TryCallMapManagerGetLevelSubHubNodeId(mapManagerAddress, mapCurrLevelIdStr, subHubNodeId)) {
                        hubNodeId = subHubNodeId;
                        if (writeLog) {
                            WriteTeleportLog("TryTeleportViaGameplayNetwork hubNodeId from MapManager.GetLevelSubHubNodeId: %u",
                                             hubNodeId);
                        }
                    } else if (writeLog) {
                        WriteTeleportLog("TryTeleportViaGameplayNetwork MapManager.GetLevelSubHubNodeId unavailable (using 0)");
                    }
                } else if (writeLog) {
                    WriteTeleportLog("TryTeleportViaGameplayNetwork MapManager.currLevelId unavailable (mapManager=0x%llX)",
                                     static_cast<unsigned long long>(mapManagerAddress));
                }
            }
        }
    }

    if (levelId <= 0) {
        uintptr_t gameWorldAddress = 0;
        if (ReadValue(gameInstanceAddress + kGameInstanceWorldOffset, gameWorldAddress) &&
            IsCanonicalUserPtr(gameWorldAddress)) {
            uintptr_t worldInfoAddress = 0;
            if (ReadValue(gameWorldAddress + kGameWorldWorldInfoOffset, worldInfoAddress) &&
                IsCanonicalUserPtr(worldInfoAddress)) {
                int32_t worldLevelIdNum = 0;
                if (TryCallWorldInfoGetCurLevelIdNum(worldInfoAddress, worldLevelIdNum)) {
                    levelId = worldLevelIdNum;
                    if (writeLog) {
                        WriteTeleportLog("TryTeleportViaGameplayNetwork levelId from WorldInfo.get_curLevelIdNum: %d",
                                         levelId);
                    }
                } else {
                    uintptr_t gameLevelAddress = 0;
                    if (ReadValue(worldInfoAddress + kWorldInfoCurLevelOffset, gameLevelAddress) &&
                        IsCanonicalUserPtr(gameLevelAddress) &&
                        ReadValue(gameLevelAddress + kGameLevelIdNumOffset, worldLevelIdNum) &&
                        worldLevelIdNum > 0) {
                        levelId = worldLevelIdNum;
                        if (writeLog) {
                            WriteTeleportLog("TryTeleportViaGameplayNetwork levelId from WorldInfo.curLevel.idNum field: %d",
                                             levelId);
                        }
                    }
                }

                void* worldLevelIdStr = nullptr;
                if (TryCallWorldInfoGetCurLevelId(worldInfoAddress, worldLevelIdStr)) {
                    levelIdStr = worldLevelIdStr;
                    if (writeLog) {
                        WriteTeleportLog("TryTeleportViaGameplayNetwork levelIdStr from WorldInfo.get_curLevelId: %p",
                                         levelIdStr);
                    }
                }
            }
        }
    }

    if (levelId > 0) {
        g_cachedTeleportLevelId.store(levelId, std::memory_order_release);
    } else {
        const int32_t cachedLevelId = g_cachedTeleportLevelId.load(std::memory_order_acquire);
        if (cachedLevelId > 0) {
            levelId = cachedLevelId;
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork levelId fallback from cache: %d", levelId);
            }
        }
    }

    if (levelId <= 0) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fail: invalid levelId=%d", levelId);
        }
        return false;
    }

    if (!levelIdStr) {
        char levelIdText[32] = "";
        std::snprintf(levelIdText, sizeof(levelIdText), "%d", levelId);
        levelIdStr = TryCreateManagedString(levelIdText);
        if (!levelIdStr) {
            levelIdStr = TryCreateManagedString("");
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork levelIdStr fallback to empty string ptr=%p", levelIdStr);
            }
        } else if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork levelIdStr created from num ptr=%p text=%s",
                             levelIdStr,
                             levelIdText);
        }
    }

    int reasonCandidates[5] = {
        kTeleportReasonClientTeleportBegin,
        kTeleportReasonClientSpatialCrossingMove,
        kTeleportReasonClientCutsceneTp,
        kTeleportReasonClientGuideTp,
        kTeleportReasonClientTeleportBegin,
    };
    int reasonCount = 4;
    if (hasObservedTeleportParams && observedReason >= 0) {
        reasonCandidates[0] = observedReason;
        reasonCount = 1;
    }

    int uiTypeCandidates[5] = {
        kTeleportUiTypeManual,
        kTeleportUiTypeWithLoading,
        kTeleportUiTypeNoLoading,
        kTeleportUiTypeDefault,
        kTeleportUiTypeManual,
    };
    int uiTypeCount = 4;
    if (hasObservedTeleportParams && observedUiType >= 0) {
        uiTypeCandidates[0] = observedUiType;
        uiTypeCount = 1;
    }

    uint32_t hubNodeIdCandidates[4] = {
        hubNodeId,
        0u,
        hubNodeId,
        0u,
    };
    int hubNodeIdCount = 2;
    if (hasObservedTeleportParams) {
        hubNodeIdCandidates[0] = observedHubNodeId;
        hubNodeIdCandidates[1] = hubNodeId;
        hubNodeIdCandidates[2] = 0u;
        hubNodeIdCandidates[3] = observedHubNodeId == 0u ? 1u : 0u;
        hubNodeIdCount = 2;
    }

    if (hasObservedTeleportParams && observedLevelId > 0 && observedLevelId != levelId) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork levelId override by observed params: %d -> %d",
                             levelId,
                             observedLevelId);
        }
        levelId = observedLevelId;
        g_cachedTeleportLevelId.store(levelId, std::memory_order_release);
    }

    uintptr_t teleportProcessorAddress = 0;
    ReadValue(gameplayNetworkAddress + kGameplayNetworkTeleportProcessorOffset, teleportProcessorAddress);

    if (hasObservedValidationData && IsCanonicalUserPtr(observedValidationDataPtr)) {
        if (IsCanonicalUserPtr(teleportProcessorAddress)) {
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork try TeleportProcessor::C2STeleportWithValidationData ptr=0x%llX",
                                 static_cast<unsigned long long>(observedValidationDataPtr));
            }
            const bool calledWithValidation = TryCallTeleportProcessorC2STeleportWithValidationData(
                teleportProcessorAddress,
                reinterpret_cast<void*>(observedValidationDataPtr),
                targetPos,
                rotationEuler);
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork TeleportProcessor validation call result=%d",
                                 calledWithValidation ? 1 : 0);
            }
            if (calledWithValidation) {
                g_lastTeleportPath.store(3, std::memory_order_release);
                if (writeLog) {
                    WriteTeleportLog("TryTeleportViaGameplayNetwork success path=TeleportProcessor::C2STeleportWithValidationData");
                }
                return true;
            }
        }

        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork try GameplayNetwork::C2STeleportWithValidationData ptr=0x%llX",
                             static_cast<unsigned long long>(observedValidationDataPtr));
        }
        const bool networkValidationCalled = TryCallGameplayNetworkC2STeleportWithValidationData(
            gameplayNetworkAddress,
            reinterpret_cast<void*>(observedValidationDataPtr),
            targetPos,
            rotationEuler);
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork GameplayNetwork validation call result=%d",
                             networkValidationCalled ? 1 : 0);
        }
        if (networkValidationCalled) {
            g_lastTeleportPath.store(1, std::memory_order_release);
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork success path=GameplayNetwork::C2STeleportWithValidationData");
            }
            return true;
        }
    }

    const int reason = reasonCandidates[0];
    const int uiType = uiTypeCandidates[0];
    const uint32_t tryHubNodeId = hubNodeIdCandidates[0];

    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork single try GameAction::TeleportToPosition reason=%d uiType=%d levelId=%d hubNodeId=%u",
                         reason,
                         uiType,
                         levelId,
                         tryHubNodeId);
    }

    const bool gameActionCalled = TryCallGameActionTeleportToPosition(targetPos,
                                                                       levelIdStr,
                                                                       rotationEuler,
                                                                       reason,
                                                                       uiType,
                                                                       tryHubNodeId);
    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork GameAction single call result=%d",
                         gameActionCalled ? 1 : 0);
    }

    if (gameActionCalled) {
        g_lastTeleportPath.store(2, std::memory_order_release);
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork success path=GameAction::TeleportToPosition");
        }
        return true;
    }

    if (IsCanonicalUserPtr(teleportProcessorAddress)) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork teleportProcessor addr=0x%llX",
                             static_cast<unsigned long long>(teleportProcessorAddress));
        }

        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork single try TeleportProcessor::C2STeleport reason=%d uiType=%d levelId=%d hubNodeId=%u",
                             reason,
                             uiType,
                             levelId,
                             tryHubNodeId);
        }

        const bool processorCalled = TryCallTeleportProcessorC2STeleport(teleportProcessorAddress,
                                                                          levelId,
                                                                          targetPos,
                                                                          rotationEuler,
                                                                          reason,
                                                                          uiType,
                                                                          tryHubNodeId);
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork TeleportProcessor single call result=%d",
                             processorCalled ? 1 : 0);
        }

        if (processorCalled) {
            g_lastTeleportPath.store(3, std::memory_order_release);
            if (writeLog) {
                WriteTeleportLog("TryTeleportViaGameplayNetwork success path=TeleportProcessor::C2STeleport");
            }
            return true;
        }
    } else if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork no valid teleportProcessor (addr=0x%llX)",
                         static_cast<unsigned long long>(teleportProcessorAddress));
    }

    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork single try GameplayNetwork::C2STeleport reason=%d uiType=%d levelId=%d hubNodeId=%u",
                         reason,
                         uiType,
                         levelId,
                         tryHubNodeId);
    }

    const bool networkCalled = TryCallGameplayNetworkC2STeleport(gameplayNetworkAddress,
                                                                  levelId,
                                                                  targetPos,
                                                                  rotationEuler,
                                                                  reason,
                                                                  uiType,
                                                                  tryHubNodeId);
    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork GameplayNetwork single call result=%d",
                         networkCalled ? 1 : 0);
    }

    if (!networkCalled) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportViaGameplayNetwork fail: all service calls failed");
        }
        return false;
    }

    g_lastTeleportPath.store(1, std::memory_order_release);
    if (writeLog) {
        WriteTeleportLog("TryTeleportViaGameplayNetwork success path=GameplayNetwork::C2STeleport");
    }
    return true;
}

static bool TryTeleportByDirectWrite(Entity* entity, const Vec3& targetPos, bool writeLog = true) {
    if (writeLog) {
        WriteTeleportLog("TryTeleportByDirectWrite begin entity=%p target=(%.3f, %.3f, %.3f)",
                         entity,
                         targetPos.x,
                         targetPos.y,
                         targetPos.z);
    }

    if (!IsValidPtr(entity)) {
        if (writeLog) {
            WriteTeleportLog("TryTeleportByDirectWrite fail: invalid entity");
        }
        return false;
    }

    bool wroteAny = false;
    uintptr_t rootAddress = 0;
    if (!TryCallEntityGetRootCom(entity, rootAddress) || !IsCanonicalUserPtr(rootAddress)) {
        ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityRootComponentOffset, rootAddress);
    }

    if (IsCanonicalUserPtr(rootAddress)) {
        const bool setByMethod = TryCallRootComponentSetPosition(rootAddress, targetPos);
        const bool notifyPositionChanged = TryCallRootComponentOnPositionChanged(rootAddress);
        const bool syncToData = TryCallRootComponentTrySyncTransformToData(rootAddress);

        if (writeLog) {
            WriteTeleportLog("TryTeleportByDirectWrite root=0x%llX setByMethod=%d notify=%d sync=%d",
                             static_cast<unsigned long long>(rootAddress),
                             setByMethod ? 1 : 0,
                             notifyPositionChanged ? 1 : 0,
                             syncToData ? 1 : 0);
        }

        if (setByMethod) {
            wroteAny = true;
            WriteValueSafe<Vec3>(rootAddress + kRootComponentSyncedPositionOffset, targetPos);
        } else {
            wroteAny |= WriteValueSafe<Vec3>(rootAddress + kRootComponentPositionOffset, targetPos);
            wroteAny |= WriteValueSafe<Vec3>(rootAddress + kRootComponentSyncedPositionOffset, targetPos);
        }

        wroteAny |= notifyPositionChanged;
        wroteAny |= syncToData;
    }

    uintptr_t entityDataAddress = 0;
    if (ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityDataOffset, entityDataAddress) &&
        IsCanonicalUserPtr(entityDataAddress)) {
        wroteAny |= WriteValueSafe<Vec3>(entityDataAddress + kBaseEntityDataPositionOffset, targetPos);
    }

    if (writeLog) {
        WriteTeleportLog("TryTeleportByDirectWrite entityData=0x%llX wroteAny=%d",
                         static_cast<unsigned long long>(entityDataAddress),
                         wroteAny ? 1 : 0);
    }

    if (wroteAny) {
        g_lastTeleportPath.store(0, std::memory_order_release);
    }

    if (writeLog) {
        WriteTeleportLog("TryTeleportByDirectWrite end result=%d", wroteAny ? 1 : 0);
    }

    return wroteAny;
}

static bool TryReadEntityPosition(Entity* entity, Vec3& outPosition, EntityPositionSource* outSource = nullptr) {
    outPosition = {0.0f, 0.0f, 0.0f};
    if (outSource != nullptr) {
        *outSource = EntityPositionSource::None;
    }

    if (!IsValidPtr(entity)) {
        return false;
    }

    auto tryReadByOffsets = [&outPosition](uintptr_t baseAddress,
                                           std::initializer_list<uintptr_t> offsets) -> bool {
        if (!IsCanonicalUserPtr(baseAddress)) {
            return false;
        }

        for (const uintptr_t offset : offsets) {
            if (ReadValue(baseAddress + offset, outPosition) && IsUsableWorldPosition(outPosition)) {
                return true;
            }
        }
        return false;
    };

    if (TryCallEntityGetPosition(entity, outPosition) && IsUsableWorldPosition(outPosition)) {
        if (outSource != nullptr) {
            *outSource = EntityPositionSource::Getter;
        }
        return true;
    }

    uintptr_t rootAddressFromGetter = 0;
    if (TryCallEntityGetRootCom(entity, rootAddressFromGetter) &&
        TryCallRootComponentGetPosition(rootAddressFromGetter, outPosition)) {
        if (outSource != nullptr) {
            *outSource = EntityPositionSource::RootComponent;
        }
        return true;
    }

    uintptr_t rootAddress = 0;
    if (ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityRootComponentOffset, rootAddress) &&
        IsCanonicalUserPtr(rootAddress)) {
        if (tryReadByOffsets(rootAddress, {kRootComponentPositionOffset, kRootComponentSyncedPositionOffset})) {
            if (outSource != nullptr) {
                *outSource = EntityPositionSource::RootComponent;
            }
            return true;
        }
    }

    uintptr_t entityDataAddress = 0;
    if (!ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityDataOffset, entityDataAddress) ||
        !IsCanonicalUserPtr(entityDataAddress)) {
        TryCallEntityGetEntityData(entity, entityDataAddress);
    }
    if (IsCanonicalUserPtr(entityDataAddress)) {
        if (tryReadByOffsets(entityDataAddress, {kBaseEntityDataPositionOffset})) {
            if (outSource != nullptr) {
                *outSource = EntityPositionSource::EntityData;
            }
            return true;
        }
    }

    uintptr_t levelEntityDataAddress = 0;
    if (!ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityInLevelDataOffset, levelEntityDataAddress) ||
        !IsCanonicalUserPtr(levelEntityDataAddress)) {
        TryCallEntityGetInLevelData(entity, levelEntityDataAddress);
    }
    if (IsCanonicalUserPtr(levelEntityDataAddress)) {
        if (tryReadByOffsets(levelEntityDataAddress, {kLevelEntityDataPositionOffset})) {
            if (outSource != nullptr) {
                *outSource = EntityPositionSource::LevelEntityData;
            }
            return true;
        }
    }

    return false;
}

static bool TeleportEntityTo(Entity* entity, const Vec3& targetPos, bool writeLog = true) {
    if (writeLog) {
        WriteTeleportLog("TeleportEntityTo begin entity=%p target=(%.3f, %.3f, %.3f)",
                         entity,
                         targetPos.x,
                         targetPos.y,
                         targetPos.z);
    }

    if (!IsValidPtr(entity)) {
        g_lastTeleportPath.store(-1, std::memory_order_release);
        if (writeLog) {
            WriteTeleportLog("TeleportEntityTo fail: invalid entity");
        }
        return false;
    }

    if (TryTeleportViaGameplayNetwork(entity, targetPos, writeLog)) {
        if (writeLog) {
            WriteTeleportLog("TeleportEntityTo success by service path=%d",
                             g_lastTeleportPath.load(std::memory_order_acquire));
        }
        return true;
    }

    const bool directResult = TryTeleportByDirectWrite(entity, targetPos, writeLog);
    if (writeLog) {
        WriteTeleportLog("TeleportEntityTo fallback direct result=%d path=%d",
                         directResult ? 1 : 0,
                         g_lastTeleportPath.load(std::memory_order_acquire));
    }
    return directResult;
}

static void BeginTeleportHold(const Vec3& targetPos) {
    g_teleportForceTargetX.store(targetPos.x, std::memory_order_release);
    g_teleportForceTargetY.store(targetPos.y, std::memory_order_release);
    g_teleportForceTargetZ.store(targetPos.z, std::memory_order_release);

    Vec3 currentPos{};
    bool hasCurrentPos = false;
    const uintptr_t entityAddress = g_mainCharacterEntityAddress.load(std::memory_order_acquire);
    if (IsCanonicalUserPtr(entityAddress) && IsValidPtr(reinterpret_cast<void*>(entityAddress))) {
        hasCurrentPos = TryReadEntityPosition(reinterpret_cast<Entity*>(entityAddress), currentPos);
    }

    if (hasCurrentPos) {
        g_teleportForceOffsetX.store(targetPos.x - currentPos.x, std::memory_order_release);
        g_teleportForceOffsetY.store(targetPos.y - currentPos.y, std::memory_order_release);
        g_teleportForceOffsetZ.store(targetPos.z - currentPos.z, std::memory_order_release);
    } else {
        g_teleportForceOffsetX.store(0.0f, std::memory_order_release);
        g_teleportForceOffsetY.store(0.0f, std::memory_order_release);
        g_teleportForceOffsetZ.store(0.0f, std::memory_order_release);
    }

    g_teleportForceActive.store(1, std::memory_order_release);

    g_treasureTool.teleportHoldTarget = targetPos;
    g_treasureTool.teleportLockTarget = targetPos;
    g_treasureTool.teleportLockHasTarget = true;
    g_treasureTool.teleportLastWriteTick = 0;

    if (g_treasureTool.autoLockAfterTeleport) {
        g_treasureTool.teleportLockEnabled = true;
    }

    g_treasureTool.teleportHoldUntilTick = 0;
    if (!g_treasureTool.enableTeleportHold) {
        return;
    }

    const int clampedHoldMs = (std::max)(200, (std::min)(10000, g_treasureTool.teleportHoldMs));
    g_treasureTool.teleportHoldUntilTick = GetTickCount64() + static_cast<uint64_t>(clampedHoldMs);
}

static void ApplyTeleportHoldIfNeeded(Entity* entity) {
    if (!IsValidPtr(entity)) {
        return;
    }

    if (!g_treasureTool.enableTeleportHold) {
        g_treasureTool.teleportHoldUntilTick = 0;
    }

    const uint64_t nowTick = GetTickCount64();
    bool holdActive = false;
    if (g_treasureTool.teleportHoldUntilTick != 0) {
        if (nowTick <= g_treasureTool.teleportHoldUntilTick) {
            holdActive = true;
        } else {
            g_treasureTool.teleportHoldUntilTick = 0;
        }
    }

    const bool lockActive = g_treasureTool.teleportLockEnabled && g_treasureTool.teleportLockHasTarget;
    if (!holdActive && !lockActive) {
        g_teleportForceActive.store(0, std::memory_order_release);
        g_treasureTool.teleportLastWriteTick = 0;
        return;
    }

    const int writeIntervalMs = (std::max)(15, (std::min)(250, g_treasureTool.teleportWriteIntervalMs));
    if (g_treasureTool.teleportLastWriteTick != 0 &&
        nowTick - g_treasureTool.teleportLastWriteTick < static_cast<uint64_t>(writeIntervalMs)) {
        return;
    }

    const int lastTeleportPath = g_lastTeleportPath.load(std::memory_order_acquire);
    if (lastTeleportPath == 1 || lastTeleportPath == 2 || lastTeleportPath == 3) {
        g_teleportForceActive.store(0, std::memory_order_release);
        if (!holdActive) {
            g_treasureTool.teleportHoldUntilTick = 0;
        }
        if (!g_treasureTool.teleportLockEnabled) {
            g_treasureTool.teleportLockHasTarget = false;
        }
        g_treasureTool.teleportLastWriteTick = nowTick;
        return;
    }

    bool wroteAny = false;
    if (holdActive) {
        wroteAny |= TeleportEntityTo(entity, g_treasureTool.teleportHoldTarget, false);
    }
    if (lockActive) {
        wroteAny |= TeleportEntityTo(entity, g_treasureTool.teleportLockTarget, false);
    }

    if (wroteAny) {
        g_treasureTool.teleportLastWriteTick = nowTick;
    }
}

static bool TryCallGameUtilGetMainCamera(uintptr_t& outCameraAddress) {
    outCameraAddress = 0;
    if (g_gameBaseAddress == 0) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameUtilGetMainCameraRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameUtilGetMainCameraFn>(fnAddress);
        outCameraAddress = reinterpret_cast<uintptr_t>(fn(nullptr));
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outCameraAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outCameraAddress);
}

static bool TryWorldToScreen(uintptr_t cameraAddress, const Vec3& worldPos, ImVec2& outScreenPos) {
    outScreenPos = ImVec2(0.0f, 0.0f);
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(cameraAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kCameraWorldToScreenPointRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    Vec3 screenPoint{};
    __try {
        auto fn = reinterpret_cast<CameraWorldToScreenPointFn>(fnAddress);
        screenPoint = fn(reinterpret_cast<void*>(cameraAddress), worldPos, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    if (!(screenPoint.z > 0.01f)) {
        return false;
    }

    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if (!(displaySize.x > 10.0f && displaySize.y > 10.0f)) {
        return false;
    }

    const float screenX = screenPoint.x;
    const float screenY = displaySize.y - screenPoint.y;
    if (screenX < -200.0f || screenX > displaySize.x + 200.0f ||
        screenY < -200.0f || screenY > displaySize.y + 200.0f) {
        return false;
    }

    outScreenPos = ImVec2(screenX, screenY);
    return true;
}

static bool LooksLikeTreasureChest(const std::string& name,
                                  const std::string& templateId,
                                  const std::string& levelEntityId) {
    const std::string lowerName = ToLowerAscii(name);
    const std::string lowerTemplate = ToLowerAscii(templateId);
    const std::string lowerLevelId = ToLowerAscii(levelEntityId);

    static const std::array<const char*, 17> kKeywords = {
        "chest", "treasure", "loot", "box", "cache", "rewardchest", "supplychest",
        "equipformulachest", "interactive_treasure", "treasurehunt", "dropchest",
        "treasure_box", "chest_", "_chest", "container", "supplybox", "rewardbox"};

    for (const char* keyword : kKeywords) {
        if (lowerName.find(keyword) != std::string::npos ||
            lowerTemplate.find(keyword) != std::string::npos ||
            lowerLevelId.find(keyword) != std::string::npos) {
            return true;
        }
    }

    static const std::array<const char*, 4> kChineseKeywords = {
        u8"??", u8"??", u8"???", u8"??"};
    for (const char* keyword : kChineseKeywords) {
        if (name.find(keyword) != std::string::npos ||
            templateId.find(keyword) != std::string::npos ||
            levelEntityId.find(keyword) != std::string::npos) {
            return true;
        }
    }

    return false;
}

static bool TryCallBaseEntityDataGetTemplateId(uintptr_t entityDataAddress, uintptr_t& outStringAddress) {
    outStringAddress = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(entityDataAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kBaseEntityDataGetTemplateIdRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<BaseEntityDataGetTemplateIdFn>(fnAddress);
        auto* strPtr = fn(reinterpret_cast<void*>(entityDataAddress), nullptr);
        outStringAddress = reinterpret_cast<uintptr_t>(strPtr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outStringAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outStringAddress);
}

static bool TryCallObjectContainerGetAllInteractives(uintptr_t objectContainerAddress, uintptr_t& outListAddress) {
    outListAddress = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(objectContainerAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kObjectContainerGetAllInteractivesRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<ObjectContainerGetAllInteractivesFn>(fnAddress);
        auto* listPtr = fn(reinterpret_cast<void*>(objectContainerAddress), nullptr);
        outListAddress = reinterpret_cast<uintptr_t>(listPtr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outListAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outListAddress);
}

static bool TryCallObjectContainerGetAllEntities(uintptr_t objectContainerAddress, uintptr_t& outListAddress) {
    outListAddress = 0;
    if (g_gameBaseAddress == 0 || !IsCanonicalUserPtr(objectContainerAddress)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kObjectContainerGetAllEntitiesRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<ObjectContainerGetAllEntitiesFn>(fnAddress);
        auto* listPtr = fn(reinterpret_cast<void*>(objectContainerAddress), nullptr);
        outListAddress = reinterpret_cast<uintptr_t>(listPtr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outListAddress = 0;
        return false;
    }

    return IsCanonicalUserPtr(outListAddress);
}

static bool TryReadManagedListHeaderWithLimit(uintptr_t listAddress,
                                              int32_t maxAllowedSize,
                                              uintptr_t& outItemsArrayAddress,
                                              int32_t& outSize) {
    outItemsArrayAddress = 0;
    outSize = 0;
    if (!IsCanonicalUserPtr(listAddress) || maxAllowedSize <= 0) {
        return false;
    }

    uintptr_t itemsArrayAddress = 0;
    int32_t size = 0;
    if (!ReadValue(listAddress + 0x10, itemsArrayAddress) || !IsCanonicalUserPtr(itemsArrayAddress) ||
        !ReadValue(listAddress + 0x18, size) || size <= 0 || size > maxAllowedSize) {
        return false;
    }

    int32_t maxLength = 0;
    if (!ReadValue(itemsArrayAddress + 0x18, maxLength) || maxLength < size || maxLength <= 0 ||
        maxLength > 16384) {
        return false;
    }

    outItemsArrayAddress = itemsArrayAddress;
    outSize = size;
    return true;
}

static bool TryExtractEntityListFromUnorderedList(uintptr_t unorderedListAddress,
                                                  uintptr_t& outItemsArrayAddress,
                                                  int32_t& outCount) {
    outItemsArrayAddress = 0;
    outCount = 0;
    if (!IsCanonicalUserPtr(unorderedListAddress)) {
        return false;
    }

    constexpr int32_t kMaxEntityCount = 16384;
    constexpr std::array<uintptr_t, 12> kNestedListOffsets = {
        0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68};

    std::vector<uintptr_t> listCandidates;
    listCandidates.reserve(16);
    const auto pushUniqueLocal = [&listCandidates](uintptr_t value) {
        if (!IsCanonicalUserPtr(value)) {
            return;
        }
        for (uintptr_t existing : listCandidates) {
            if (existing == value) {
                return;
            }
        }
        listCandidates.push_back(value);
    };

    pushUniqueLocal(unorderedListAddress);
    for (const uintptr_t offset : kNestedListOffsets) {
        uintptr_t nested = 0;
        if (ReadValue(unorderedListAddress + offset, nested)) {
            pushUniqueLocal(nested);
        }
    }

    for (uintptr_t candidateAddress : listCandidates) {
        if (TryReadManagedListHeaderWithLimit(candidateAddress, kMaxEntityCount, outItemsArrayAddress, outCount)) {
            return true;
        }

        for (const uintptr_t nestedOffset : kNestedListOffsets) {
            uintptr_t nestedAddress = 0;
            if (!ReadValue(candidateAddress + nestedOffset, nestedAddress)) {
                continue;
            }
            if (TryReadManagedListHeaderWithLimit(nestedAddress, kMaxEntityCount, outItemsArrayAddress, outCount)) {
                return true;
            }
        }
    }

    return false;
}

static bool LooksLikeObjectContainer(uintptr_t containerAddress) {
    if (!IsCanonicalUserPtr(containerAddress)) {
        return false;
    }

    constexpr std::array<uintptr_t, 8> kKnownListFieldOffsets = {
        0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8
    };

    int validKnownListCount = 0;
    for (const uintptr_t offset : kKnownListFieldOffsets) {
        uintptr_t listAddress = 0;
        if (!ReadValue(containerAddress + offset, listAddress) || !IsCanonicalUserPtr(listAddress)) {
            continue;
        }

        uintptr_t itemsArrayAddress = 0;
        int32_t count = 0;
        if (TryExtractEntityListFromUnorderedList(listAddress, itemsArrayAddress, count)) {
            ++validKnownListCount;
        }
    }

    uintptr_t idEntityMapAddress = 0;
    const bool hasIdEntityMap = ReadValue(containerAddress + 0x88, idEntityMapAddress) && IsCanonicalUserPtr(idEntityMapAddress);

    if (hasIdEntityMap && validKnownListCount >= 2) {
        return true;
    }

    uintptr_t allEntitiesListAddress = 0;
    if (TryCallObjectContainerGetAllEntities(containerAddress, allEntitiesListAddress)) {
        uintptr_t itemsArrayAddress = 0;
        int32_t count = 0;
        if (TryExtractEntityListFromUnorderedList(allEntitiesListAddress, itemsArrayAddress, count)) {
            return true;
        }
    }

    return false;
}

static void PushUniquePointer(std::vector<uintptr_t>& values, uintptr_t value) {
    if (!IsCanonicalUserPtr(value)) {
        return;
    }

    for (uintptr_t existing : values) {
        if (existing == value) {
            return;
        }
    }

    values.push_back(value);
}

static bool PushUniquePointerFast(std::vector<uintptr_t>& values,
                                  std::unordered_set<uintptr_t>& valueSet,
                                  uintptr_t value) {
    if (!IsCanonicalUserPtr(value)) {
        return false;
    }

    const auto inserted = valueSet.emplace(value).second;
    if (!inserted) {
        return false;
    }

    values.push_back(value);
    return true;
}

static bool TryExtractDictionaryValuePointers(uintptr_t dictionaryAddress,
                                              std::vector<uintptr_t>& outValuePointers,
                                              int32_t maxValues = 8192) {
    outValuePointers.clear();
    if (!IsCanonicalUserPtr(dictionaryAddress) || maxValues <= 0) {
        return false;
    }

    uintptr_t entriesArrayAddress = 0;
    if (!ReadValue(dictionaryAddress + 0x18, entriesArrayAddress) || !IsCanonicalUserPtr(entriesArrayAddress)) {
        return false;
    }

    int32_t entryCount = 0;
    if (!ReadValue(entriesArrayAddress + 0x18, entryCount) || entryCount <= 0 || entryCount > 262144) {
        return false;
    }

    const int32_t safeCount = (std::min)(entryCount, maxValues);
    if (safeCount <= 0) {
        return false;
    }

    struct EntryLayout {
        uintptr_t stride;
        uintptr_t valueOffset;
    };
    constexpr std::array<EntryLayout, 3> kCandidateLayouts = {
        EntryLayout{0x18, 0x10},
        EntryLayout{0x20, 0x10},
        EntryLayout{0x18, 0x18},
    };

    size_t bestCount = 0;
    std::vector<uintptr_t> bestValues;
    std::vector<uintptr_t> trialValues;
    trialValues.reserve(static_cast<size_t>(safeCount));

    for (const EntryLayout& layout : kCandidateLayouts) {
        trialValues.clear();

        for (int32_t i = 0; i < safeCount; ++i) {
            const uintptr_t entryAddress = entriesArrayAddress + 0x20 + static_cast<uintptr_t>(i) * layout.stride;

            int32_t hashCode = -1;
            if (!ReadValue(entryAddress, hashCode) || hashCode < 0) {
                continue;
            }

            uintptr_t valuePointer = 0;
            if (!ReadValue(entryAddress + layout.valueOffset, valuePointer) || !IsCanonicalUserPtr(valuePointer)) {
                continue;
            }

            trialValues.push_back(valuePointer);
        }

        if (trialValues.size() > bestCount) {
            bestCount = trialValues.size();
            bestValues = trialValues;
        }
    }

    if (bestValues.empty()) {
        return false;
    }

    std::unordered_set<uintptr_t> uniqueSet;
    uniqueSet.reserve(bestValues.size() * 2 + 1);
    outValuePointers.reserve(bestValues.size());
    for (uintptr_t ptr : bestValues) {
        if (uniqueSet.emplace(ptr).second) {
            outValuePointers.push_back(ptr);
        }
    }

    return !outValuePointers.empty();
}

static bool TryAppendEntityAddressesFromEntityManager(uintptr_t gameInstanceAddress,
                                                      std::vector<uintptr_t>& ioEntityAddresses,
                                                      std::unordered_set<uintptr_t>& ioEntityAddressSet,
                                                      size_t maxUniqueEntities,
                                                      int& outDataContainerCount,
                                                      int& outEntityHitCount) {
    outDataContainerCount = 0;
    outEntityHitCount = 0;

    if (!IsCanonicalUserPtr(gameInstanceAddress) || maxUniqueEntities == 0) {
        return false;
    }

    uintptr_t worldAddress = 0;
    if (!ReadValue(gameInstanceAddress + kGameInstanceWorldOffset, worldAddress) || !IsCanonicalUserPtr(worldAddress)) {
        return false;
    }

    uintptr_t entityManagerAddress = 0;
    if (!ReadValue(worldAddress + kGameWorldEntityManagerOffset, entityManagerAddress) ||
        !IsCanonicalUserPtr(entityManagerAddress)) {
        return false;
    }

    uintptr_t dataStorageAddress = 0;
    if (!ReadValue(entityManagerAddress + kEntityManagerDataStorageOffset, dataStorageAddress) ||
        !IsCanonicalUserPtr(dataStorageAddress)) {
        return false;
    }

    uintptr_t containersArrayAddress = 0;
    if (!ReadValue(dataStorageAddress + kEntityDataStorageContainersOffset, containersArrayAddress) ||
        !IsCanonicalUserPtr(containersArrayAddress)) {
        return false;
    }

    int32_t containerCount = 0;
    if (!ReadValue(containersArrayAddress + 0x18, containerCount) || containerCount <= 0 || containerCount > 1024) {
        return false;
    }

    const int32_t safeContainerCount = (std::min)(containerCount, 128);
    std::vector<uintptr_t> baseEntityDataAddresses;
    baseEntityDataAddresses.reserve(2048);

    for (int32_t i = 0; i < safeContainerCount; ++i) {
        uintptr_t dataContainerAddress = 0;
        if (!ReadValue(containersArrayAddress + 0x20 + static_cast<uintptr_t>(i) * sizeof(uintptr_t), dataContainerAddress) ||
            !IsCanonicalUserPtr(dataContainerAddress)) {
            continue;
        }

        ++outDataContainerCount;

        uintptr_t dataDictAddress = 0;
        if (!ReadValue(dataContainerAddress + kDataContainerDataDictOffset, dataDictAddress) ||
            !IsCanonicalUserPtr(dataDictAddress)) {
            continue;
        }

        if (!TryExtractDictionaryValuePointers(dataDictAddress, baseEntityDataAddresses, 16384)) {
            continue;
        }

        for (uintptr_t baseEntityDataAddress : baseEntityDataAddresses) {
            uintptr_t entityAddress = 0;
            if (!ReadValue(baseEntityDataAddress + kBaseEntityDataEntityRefOffset, entityAddress) ||
                !IsCanonicalUserPtr(entityAddress)) {
                continue;
            }

            if (!IsLikelyEntityAddress(entityAddress)) {
                continue;
            }

            if (PushUniquePointerFast(ioEntityAddresses, ioEntityAddressSet, entityAddress)) {
                ++outEntityHitCount;
                if (ioEntityAddresses.size() >= maxUniqueEntities) {
                    return true;
                }
            }
        }
    }

    return outEntityHitCount > 0;
}

static bool TryReadPointerArray(uintptr_t pointerArrayAddress,
                                size_t count,
                                std::vector<uintptr_t>& outValues) {
    outValues.clear();
    if (!IsCanonicalUserPtr(pointerArrayAddress) || count == 0 || count > 16384) {
        return false;
    }

    const size_t byteCount = count * sizeof(uintptr_t);
    if (byteCount == 0 || byteCount > (1u << 20)) {
        return false;
    }

    outValues.resize(count, 0);
    if (!ReadBytesSafe(pointerArrayAddress, outValues.data(), byteCount)) {
        outValues.clear();
        return false;
    }

    return true;
}

static bool TryExtractEntityPointersFromArrayLike(uintptr_t ownerAddress,
                                                  std::vector<uintptr_t>& outEntityAddresses,
                                                  int32_t maxArrayLength = 4096) {
    outEntityAddresses.clear();
    if (!IsCanonicalUserPtr(ownerAddress) || maxArrayLength <= 0) {
        return false;
    }

    constexpr std::array<uintptr_t, 14> kCandidateOffsets = {
        0x0, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
        0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70};

    std::unordered_set<uintptr_t> uniqueEntitySet;
    uniqueEntitySet.reserve(512);
    std::vector<uintptr_t> pointerValues;

    for (const uintptr_t offset : kCandidateOffsets) {
        uintptr_t arrayAddress = ownerAddress;
        if (offset != 0) {
            if (!ReadValue(ownerAddress + offset, arrayAddress) || !IsCanonicalUserPtr(arrayAddress)) {
                continue;
            }
        }

        int32_t arrayLength = 0;
        if (!ReadValue(arrayAddress + 0x18, arrayLength) || arrayLength <= 0 || arrayLength > maxArrayLength) {
            continue;
        }

        if (!TryReadPointerArray(arrayAddress + 0x20, static_cast<size_t>(arrayLength), pointerValues)) {
            continue;
        }

        for (uintptr_t value : pointerValues) {
            if (!IsLikelyEntityAddress(value)) {
                continue;
            }

            const bool inserted = uniqueEntitySet.emplace(value).second;
            if (!inserted) {
                continue;
            }

            outEntityAddresses.push_back(value);
            if (outEntityAddresses.size() >= static_cast<size_t>(maxArrayLength)) {
                return true;
            }
        }
    }

    return !outEntityAddresses.empty();
}

static void CollectObjectContainerCandidates(uintptr_t baseAddress, std::vector<uintptr_t>& outCandidates) {
    if (!IsCanonicalUserPtr(baseAddress)) {
        return;
    }

    constexpr std::array<uintptr_t, 12> kKnownOffsets = {
        0x170, 0x188, 0x190, 0x198, 0x1A0, 0x1B8, 0x1F8, 0x200, 0x208, 0x210, 0x218, 0x260};
    for (const uintptr_t offset : kKnownOffsets) {
        uintptr_t candidate = 0;
        if (ReadValue(baseAddress + offset, candidate) && IsCanonicalUserPtr(candidate)) {
            PushUniquePointer(outCandidates, candidate);
        }
    }

    for (uintptr_t offset = 0; offset <= 0x420; offset += sizeof(uintptr_t)) {
        uintptr_t candidate = 0;
        if (!ReadValue(baseAddress + offset, candidate) || !IsCanonicalUserPtr(candidate)) {
            continue;
        }

        if (LooksLikeObjectContainer(candidate)) {
            PushUniquePointer(outCandidates, candidate);
        }
    }
}

static bool TryResolveObjectContainerAddresses(uintptr_t gameInstanceAddress,
                                               std::vector<uintptr_t>& outContainerAddresses,
                                               int* outDirectWorldHitCount = nullptr) {
    outContainerAddresses.clear();
    if (outDirectWorldHitCount != nullptr) {
        *outDirectWorldHitCount = 0;
    }
    if (!IsCanonicalUserPtr(gameInstanceAddress)) {
        return false;
    }

    constexpr std::array<uintptr_t, 6> kDirectContainerOffsets = {
        0x170, 0x188, 0x1F8, 0x200, 0x208, 0x210
    };
    for (const uintptr_t offset : kDirectContainerOffsets) {
        uintptr_t candidate = 0;
        if (ReadValue(gameInstanceAddress + offset, candidate) && LooksLikeObjectContainer(candidate)) {
            PushUniquePointer(outContainerAddresses, candidate);
        }
    }

    uintptr_t worldAddress = 0;
    if (ReadValue(gameInstanceAddress + kGameInstanceWorldOffset, worldAddress) && IsCanonicalUserPtr(worldAddress)) {
        uintptr_t directWorldContainer = 0;
        if (ReadValue(worldAddress + kGameWorldObjectContainerOffset, directWorldContainer) &&
            IsCanonicalUserPtr(directWorldContainer)) {
            bool trusted = LooksLikeObjectContainer(directWorldContainer);
            if (!trusted) {
                uintptr_t allEntitiesAddress = 0;
                uintptr_t allInteractivesAddress = 0;
                const bool hasAllEntities = ReadValue(directWorldContainer + 0x98, allEntitiesAddress) &&
                                            IsCanonicalUserPtr(allEntitiesAddress);
                const bool hasAllInteractives = ReadValue(directWorldContainer + 0xB0, allInteractivesAddress) &&
                                                IsCanonicalUserPtr(allInteractivesAddress);
                trusted = hasAllEntities || hasAllInteractives;
            }

            if (trusted) {
                PushUniquePointer(outContainerAddresses, directWorldContainer);
                if (outDirectWorldHitCount != nullptr) {
                    *outDirectWorldHitCount += 1;
                }
            }
        }

        for (const uintptr_t offset : kDirectContainerOffsets) {
            uintptr_t candidate = 0;
            if (ReadValue(worldAddress + offset, candidate) && LooksLikeObjectContainer(candidate)) {
                PushUniquePointer(outContainerAddresses, candidate);
            }
        }

        CollectObjectContainerCandidates(worldAddress, outContainerAddresses);

        for (uintptr_t offset = 0; offset <= 0x180; offset += sizeof(uintptr_t)) {
            uintptr_t nested = 0;
            if (!ReadValue(worldAddress + offset, nested) || !IsCanonicalUserPtr(nested)) {
                continue;
            }
            CollectObjectContainerCandidates(nested, outContainerAddresses);
            if (outContainerAddresses.size() >= 96) {
                break;
            }
        }
    }

    CollectObjectContainerCandidates(gameInstanceAddress, outContainerAddresses);

    if (outContainerAddresses.empty()) {
        for (uintptr_t offset = 0; offset <= 0x420; offset += sizeof(uintptr_t)) {
            uintptr_t candidate = 0;
            if (!ReadValue(gameInstanceAddress + offset, candidate) || !IsCanonicalUserPtr(candidate)) {
                continue;
            }
            if (LooksLikeObjectContainer(candidate)) {
                PushUniquePointer(outContainerAddresses, candidate);
            }
            if (outContainerAddresses.size() >= 64) {
                break;
            }
        }
    }

    if (outContainerAddresses.size() > 96) {
        outContainerAddresses.resize(96);
    }

    return !outContainerAddresses.empty();
}

static bool TryGetInteractiveEntityAddresses(uintptr_t gameInstanceAddress, std::vector<Entity*>& outEntities) {
    outEntities.clear();
    g_interactiveScanDebug.containerCandidateCount = 0;
    g_interactiveScanDebug.listCandidateCount = 0;
    g_interactiveScanDebug.listHeaderSuccessCount = 0;
    g_interactiveScanDebug.directWorldObjectContainerHitCount = 0;
    g_interactiveScanDebug.directAllEntitiesListHitCount = 0;
    g_interactiveScanDebug.directAllInteractivesListHitCount = 0;
    g_interactiveScanDebug.entityManagerDataContainerCount = 0;
    g_interactiveScanDebug.entityManagerEntityHitCount = 0;
    g_interactiveScanDebug.uniqueEntityAddressCount = 0;
    g_interactiveScanDebug.validEntityCount = 0;

    std::vector<uintptr_t> containerAddresses;
    int directWorldObjectContainerHitCount = 0;
    const bool hasObjectContainers =
        TryResolveObjectContainerAddresses(gameInstanceAddress, containerAddresses, &directWorldObjectContainerHitCount);
    g_interactiveScanDebug.directWorldObjectContainerHitCount = directWorldObjectContainerHitCount;

    const bool fullScanMode = g_treasureTool.drawAllInteractives;
    if (!hasObjectContainers && !fullScanMode) {
        return false;
    }

    const size_t maxContainerCount = fullScanMode ? 96 : 48;
    if (containerAddresses.size() > maxContainerCount) {
        containerAddresses.resize(maxContainerCount);
    }
    g_interactiveScanDebug.containerCandidateCount = static_cast<int>(containerAddresses.size());
    int listCandidateCount = 0;
    int listHeaderSuccessCount = 0;

    constexpr std::array<uintptr_t, 17> kExtraContainerListOffsets = {
        0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8,
        0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x100};
    const int32_t maxEntityPerList = fullScanMode ? 4096 : 1024;
    const size_t maxUniqueEntities = fullScanMode ? 20000 : 2048;
    const size_t maxListCandidatesPerContainer = fullScanMode ? 72 : 24;
    const uint64_t scanBudgetMs = fullScanMode ? 55 : 12;
    const bool useContainerMethods = true;

    std::vector<uintptr_t> uniqueEntityAddresses;
    uniqueEntityAddresses.reserve(512);
    std::unordered_set<uintptr_t> uniqueEntityAddressSet;
    uniqueEntityAddressSet.reserve(maxUniqueEntities * 2);
    std::vector<uintptr_t> entityAddressBatch;
    entityAddressBatch.reserve(static_cast<size_t>(maxEntityPerList));
    std::vector<uintptr_t> arrayLikeEntityBatch;
    arrayLikeEntityBatch.reserve(static_cast<size_t>(maxEntityPerList));

    const auto appendFromUnorderedListAddress = [&](uintptr_t unorderedListAddress) {
        uintptr_t itemsArrayAddress = 0;
        int32_t count = 0;
        if (!TryExtractEntityListFromUnorderedList(unorderedListAddress, itemsArrayAddress, count) || count <= 0) {
            return;
        }

        ++listHeaderSuccessCount;
        const int32_t safeCount = (std::min)(count, maxEntityPerList);
        const uintptr_t firstEntityAddress = itemsArrayAddress + 0x20;

        if (TryReadPointerArray(firstEntityAddress, static_cast<size_t>(safeCount), entityAddressBatch)) {
            for (uintptr_t entityAddress : entityAddressBatch) {
                PushUniquePointerFast(uniqueEntityAddresses, uniqueEntityAddressSet, entityAddress);
                if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
                    break;
                }
            }
            return;
        }

        for (int32_t i = 0; i < safeCount; ++i) {
            uintptr_t entityAddress = 0;
            if (!ReadValue(firstEntityAddress + static_cast<uintptr_t>(i) * sizeof(uintptr_t), entityAddress)) {
                continue;
            }
            PushUniquePointerFast(uniqueEntityAddresses, uniqueEntityAddressSet, entityAddress);
            if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
                break;
            }
        }
    };

    const uint64_t scanStartTick = GetTickCount64();
    int methodCallBudget = useContainerMethods ? (fullScanMode ? 48 : 6) : 0;
    int directAllEntitiesListHitCount = 0;
    int directAllInteractivesListHitCount = 0;

    for (uintptr_t objectContainerAddress : containerAddresses) {
        if (GetTickCount64() - scanStartTick > scanBudgetMs) {
            break;
        }

        std::vector<uintptr_t> unorderedCandidates;
        unorderedCandidates.reserve(32);
        std::unordered_set<uintptr_t> unorderedCandidateSet;
        unorderedCandidateSet.reserve(64);

        if (useContainerMethods && methodCallBudget > 0) {
            --methodCallBudget;

            uintptr_t listByMethod = 0;
            if (TryCallObjectContainerGetAllInteractives(objectContainerAddress, listByMethod)) {
                PushUniquePointerFast(unorderedCandidates, unorderedCandidateSet, listByMethod);
            }

            uintptr_t allEntitiesByMethod = 0;
            if (TryCallObjectContainerGetAllEntities(objectContainerAddress, allEntitiesByMethod)) {
                PushUniquePointerFast(unorderedCandidates, unorderedCandidateSet, allEntitiesByMethod);
            }
        }

        // Direct object-container field path (ObjectContainer::allEntities/allInteractives)
        if (fullScanMode) {
            uintptr_t directAllEntities = 0;
            if (ReadValue(objectContainerAddress + 0x98, directAllEntities) && IsCanonicalUserPtr(directAllEntities)) {
                ++directAllEntitiesListHitCount;
                appendFromUnorderedListAddress(directAllEntities);
            }

            uintptr_t directAllInteractives = 0;
            if (ReadValue(objectContainerAddress + 0xB0, directAllInteractives) && IsCanonicalUserPtr(directAllInteractives)) {
                ++directAllInteractivesListHitCount;
                appendFromUnorderedListAddress(directAllInteractives);
            }

            if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
                break;
            }
        }

        for (const uintptr_t offset : kObjectContainerInteractivesOffsets) {
            uintptr_t listAddress = 0;
            if (ReadValue(objectContainerAddress + offset, listAddress)) {
                PushUniquePointerFast(unorderedCandidates, unorderedCandidateSet, listAddress);
            }
        }

        for (const uintptr_t offset : kExtraContainerListOffsets) {
            uintptr_t listAddress = 0;
            if (ReadValue(objectContainerAddress + offset, listAddress)) {
                PushUniquePointerFast(unorderedCandidates, unorderedCandidateSet, listAddress);
            }
        }

        if (fullScanMode) {
            for (uintptr_t offset = 0x20; offset <= 0x120; offset += sizeof(uintptr_t)) {
                uintptr_t listAddress = 0;
                if (ReadValue(objectContainerAddress + offset, listAddress)) {
                    PushUniquePointerFast(unorderedCandidates, unorderedCandidateSet, listAddress);
                }
            }
        }

        if (unorderedCandidates.size() > maxListCandidatesPerContainer) {
            unorderedCandidates.resize(maxListCandidatesPerContainer);
        }

        listCandidateCount += static_cast<int>(unorderedCandidates.size());

        for (uintptr_t unorderedListAddress : unorderedCandidates) {
            if (GetTickCount64() - scanStartTick > scanBudgetMs) {
                break;
            }

            uintptr_t itemsArrayAddress = 0;
            int32_t count = 0;
            if (TryExtractEntityListFromUnorderedList(unorderedListAddress, itemsArrayAddress, count) && count > 0) {
                appendFromUnorderedListAddress(unorderedListAddress);
            } else if (fullScanMode &&
                       TryExtractEntityPointersFromArrayLike(unorderedListAddress, arrayLikeEntityBatch, maxEntityPerList)) {
                ++listHeaderSuccessCount;
                for (uintptr_t entityAddress : arrayLikeEntityBatch) {
                    PushUniquePointerFast(uniqueEntityAddresses, uniqueEntityAddressSet, entityAddress);
                    if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
                        break;
                    }
                }
            } else {
                continue;
            }

            if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
                break;
            }
        }

        if (uniqueEntityAddresses.size() >= maxUniqueEntities) {
            break;
        }
    }

    g_interactiveScanDebug.listCandidateCount = listCandidateCount;
    g_interactiveScanDebug.listHeaderSuccessCount = listHeaderSuccessCount;
    g_interactiveScanDebug.directAllEntitiesListHitCount = directAllEntitiesListHitCount;
    g_interactiveScanDebug.directAllInteractivesListHitCount = directAllInteractivesListHitCount;

    int emDataContainerCount = 0;
    int emEntityHitCount = 0;
    if (fullScanMode) {
        TryAppendEntityAddressesFromEntityManager(gameInstanceAddress,
                                                  uniqueEntityAddresses,
                                                  uniqueEntityAddressSet,
                                                  maxUniqueEntities,
                                                  emDataContainerCount,
                                                  emEntityHitCount);
    }
    g_interactiveScanDebug.entityManagerDataContainerCount = emDataContainerCount;
    g_interactiveScanDebug.entityManagerEntityHitCount = emEntityHitCount;

    g_interactiveScanDebug.uniqueEntityAddressCount = static_cast<int>(uniqueEntityAddresses.size());

    if (uniqueEntityAddresses.empty()) {
        return false;
    }

    outEntities.reserve(uniqueEntityAddresses.size());
    for (uintptr_t entityAddress : uniqueEntityAddresses) {
        if (!IsLikelyEntityAddress(entityAddress)) {
            continue;
        }

        auto* entity = reinterpret_cast<Entity*>(entityAddress);
        if (IsValidPtr(entity)) {
            outEntities.push_back(entity);
        }
    }

    g_interactiveScanDebug.validEntityCount = static_cast<int>(outEntities.size());
    return !outEntities.empty();
}

static void RefreshNearbyChestCache(uintptr_t gameInstanceAddress,
                                    const Vec3& playerPos,
                                    bool playerPosValid,
                                    uintptr_t playerEntityAddress,
                                    float range,
                                    bool onlyChestKeyword,
                                    std::vector<NearbyChestInfo>& outChests) {
    outChests.clear();
    g_interactiveScanDebug.withPositionCount = 0;
    g_interactiveScanDebug.withoutPositionCount = 0;
    g_interactiveScanDebug.posFromGetterCount = 0;
    g_interactiveScanDebug.posFromRootCount = 0;
    g_interactiveScanDebug.posFromEntityDataCount = 0;
    g_interactiveScanDebug.posFromLevelDataCount = 0;
    g_interactiveScanDebug.filteredByDuplicatePosCount = 0;
    g_interactiveScanDebug.filteredByDistanceCount = 0;
    g_interactiveScanDebug.filteredByKeywordCount = 0;
    g_interactiveScanDebug.filteredByTemplateCount = 0;
    g_interactiveScanDebug.autoNearCapApplied = false;
    g_interactiveScanDebug.lastRefreshTick = GetTickCount64();

    std::vector<Entity*> interactives;
    if (!TryGetInteractiveEntityAddresses(gameInstanceAddress, interactives)) {
        return;
    }

    const bool hasValidPlayerPos = playerPosValid && IsReasonableWorldPosition(playerPos);
    const bool distanceLimited = hasValidPlayerPos && g_treasureTool.limitByDistance && range > 1.0f;
    const float clampedRange = (std::max)(10.0f, range);
    const float rangeSq = clampedRange * clampedRange;

    std::string templateFilter = g_treasureTool.templateIdFilter;
    while (!templateFilter.empty() && std::isspace(static_cast<unsigned char>(templateFilter.back()))) {
        templateFilter.pop_back();
    }
    size_t headIndex = 0;
    while (headIndex < templateFilter.size() && std::isspace(static_cast<unsigned char>(templateFilter[headIndex]))) {
        ++headIndex;
    }
    if (headIndex > 0) {
        templateFilter = templateFilter.substr(headIndex);
    }
    const bool hasTemplateFilter = !templateFilter.empty();
    const std::string templateFilterLower = ToLowerAscii(templateFilter);
    const bool forceShowAllItems = g_treasureTool.drawAllInteractives;
    const bool drawAllNearOnly = false;
    const bool skipWithoutPositionInAllMode = false;
    const float drawAllNearRangeSq = 0.0f;
    g_interactiveScanDebug.autoNearCapApplied = drawAllNearOnly;

    int withPositionCount = 0;
    int withoutPositionCount = 0;
    int posFromGetterCount = 0;
    int posFromRootCount = 0;
    int posFromEntityDataCount = 0;
    int posFromLevelDataCount = 0;
    int filteredByDuplicatePosCount = 0;
    int filteredByDistanceCount = 0;
    int filteredByKeywordCount = 0;
    int filteredByTemplateCount = 0;
    size_t inspectedEntityCount = 0;
    const size_t maxInspectedEntities = forceShowAllItems ? 12000 : 2400;
    std::unordered_map<uint64_t, int> positionBucketCount;
    positionBucketCount.reserve(512);

    for (Entity* interactive : interactives) {
        if (inspectedEntityCount >= maxInspectedEntities) {
            break;
        }
        ++inspectedEntityCount;
        const uintptr_t interactiveAddress = reinterpret_cast<uintptr_t>(interactive);
        if (playerEntityAddress != 0 && interactiveAddress == playerEntityAddress) {
            continue;
        }

        Vec3 interactivePos{};
        EntityPositionSource positionSource = EntityPositionSource::None;
        const bool hasPosition = TryReadEntityPosition(interactive, interactivePos, &positionSource);

        float distance = -1.0f;
        float distSq = 0.0f;
        if (hasPosition) {
            ++withPositionCount;

            switch (positionSource) {
            case EntityPositionSource::Getter:
                ++posFromGetterCount;
                break;
            case EntityPositionSource::RootComponent:
                ++posFromRootCount;
                break;
            case EntityPositionSource::EntityData:
                ++posFromEntityDataCount;
                break;
            case EntityPositionSource::LevelEntityData:
                ++posFromLevelDataCount;
                break;
            default:
                break;
            }

            if (hasValidPlayerPos) {
                const float dx = interactivePos.x - playerPos.x;
                const float dy = interactivePos.y - playerPos.y;
                const float dz = interactivePos.z - playerPos.z;
                distSq = dx * dx + dy * dy + dz * dz;
                distance = std::sqrt(distSq);

                if (distanceLimited && distSq > rangeSq) {
                    ++filteredByDistanceCount;
                    continue;
                }
            }
        } else {
            ++withoutPositionCount;
        }

        if (skipWithoutPositionInAllMode && !hasPosition) {
            ++filteredByDistanceCount;
            continue;
        }

        if (hasPosition) {
            const int quantX = static_cast<int>(std::lround(interactivePos.x * 2.0f));
            const int quantY = static_cast<int>(std::lround(interactivePos.y * 2.0f));
            const int quantZ = static_cast<int>(std::lround(interactivePos.z * 2.0f));
            const uint64_t key =
                (static_cast<uint64_t>(static_cast<uint32_t>(quantX)) << 42) ^
                (static_cast<uint64_t>(static_cast<uint32_t>(quantY)) << 21) ^
                static_cast<uint64_t>(static_cast<uint32_t>(quantZ));

            int& bucketHits = positionBucketCount[key];
            ++bucketHits;
            const bool looksLikeStaticFallback = (positionSource == EntityPositionSource::EntityData ||
                                                  positionSource == EntityPositionSource::LevelEntityData);
            const int staticFallbackBucketLimit = forceShowAllItems ? 2 : 6;
            const int genericBucketLimit = forceShowAllItems ? 4 : 12;
            if (looksLikeStaticFallback && bucketHits > staticFallbackBucketLimit) {
                ++filteredByDuplicatePosCount;
                continue;
            }
            if (bucketHits > genericBucketLimit) {
                ++filteredByDuplicatePosCount;
                continue;
            }
        }

        if (drawAllNearOnly) {
            if (!hasValidPlayerPos || !hasPosition || distSq > drawAllNearRangeSq) {
                ++filteredByDistanceCount;
                continue;
            }
        }

        std::string interactiveName = ReadManagedStringUtf8(reinterpret_cast<uintptr_t>(interactive->name), 80);
        uintptr_t entityDataAddress = 0;
        uintptr_t templateIdStringAddress = 0;
        std::string templateId;
        std::string levelEntityId;
        std::string templateIdDisplay;

        if (ReadValue(reinterpret_cast<uintptr_t>(interactive) + kEntityDataOffset, entityDataAddress) &&
            IsCanonicalUserPtr(entityDataAddress)) {
            if (ReadValue(entityDataAddress + kEntityDataTemplateIdOffset, templateIdStringAddress) &&
                IsCanonicalUserPtr(templateIdStringAddress)) {
                templateId = TrimAsciiWhitespace(ReadManagedStringUtf8(templateIdStringAddress, 96));
            }

            if (!IsLikelyReadableTemplateId(templateId)) {
                uintptr_t getterTemplateIdString = 0;
                if (TryCallBaseEntityDataGetTemplateId(entityDataAddress, getterTemplateIdString)) {
                    const std::string fromGetter = TrimAsciiWhitespace(ReadManagedStringUtf8(getterTemplateIdString, 96));
                    if (IsLikelyReadableTemplateId(fromGetter) || templateId.empty()) {
                        templateId = fromGetter;
                        templateIdStringAddress = getterTemplateIdString;
                    }
                }
            }
        }

        uintptr_t levelEntityDataAddress = 0;
        if (ReadValue(reinterpret_cast<uintptr_t>(interactive) + kEntityInLevelDataOffset, levelEntityDataAddress) &&
            IsCanonicalUserPtr(levelEntityDataAddress)) {
            uintptr_t levelEntityIdAddress = 0;
            if (ReadValue(levelEntityDataAddress + kLevelEntityDataIdKeyOffset, levelEntityIdAddress) &&
                IsCanonicalUserPtr(levelEntityIdAddress)) {
                levelEntityId = TrimAsciiWhitespace(ReadManagedStringUtf8(levelEntityIdAddress, 96));
            }
        }

        if (IsLikelyReadableTemplateId(templateId)) {
            templateIdDisplay = templateId;
        } else if (IsLikelyReadableTemplateId(levelEntityId)) {
            templateIdDisplay = levelEntityId;
        } else {
            templateIdDisplay = NormalizeTemplateIdForDisplay(templateId, templateIdStringAddress);
        }

        const uintptr_t entityAddress = interactiveAddress;
        int32_t objectType = -1;
        ReadValue(entityAddress + kEntityObjectTypeOffset, objectType);

        uintptr_t interactiveRootAddress = 0;
        const bool hasInteractiveRoot = ReadValue(entityAddress + kEntityInteractiveRootOffset, interactiveRootAddress) &&
                                        IsCanonicalUserPtr(interactiveRootAddress);

        NearbyChestInfo info;
        info.entity = interactive;
        info.entityAddress = entityAddress;
        info.position = interactivePos;
        info.hasPosition = hasPosition;
        info.distance = distance;
        info.instanceUid = interactive->instanceUid;
        info.serverId = interactive->serverId;
        info.objectType = objectType;
        info.hasInteractiveRoot = hasInteractiveRoot;
        info.name = std::move(interactiveName);
        info.nameDisplay = NormalizeNameForDisplay(info.name);
        info.templateId = std::move(templateId);
        info.levelEntityId = std::move(levelEntityId);
        info.templateIdDisplay = std::move(templateIdDisplay);
        info.keywordMatched = LooksLikeTreasureChest(info.name, info.templateId, info.levelEntityId) || info.hasInteractiveRoot;

        if (!forceShowAllItems && onlyChestKeyword && !info.keywordMatched) {
            ++filteredByKeywordCount;
            continue;
        }

        if (!g_treasureTool.drawAllInteractives && !info.keywordMatched) {
            ++filteredByKeywordCount;
            continue;
        }

        if (hasTemplateFilter && !forceShowAllItems) {
            const std::string templateLower = ToLowerAscii(info.templateId);
            const std::string levelIdLower = ToLowerAscii(info.levelEntityId);
            const bool matched = g_treasureTool.templateIdExactMatch
                                     ? (templateLower == templateFilterLower || levelIdLower == templateFilterLower)
                                     : (templateLower.find(templateFilterLower) != std::string::npos ||
                                        levelIdLower.find(templateFilterLower) != std::string::npos);
            if (!matched) {
                ++filteredByTemplateCount;
                continue;
            }
        }

        outChests.push_back(std::move(info));
    }

    g_interactiveScanDebug.withPositionCount = withPositionCount;
    g_interactiveScanDebug.withoutPositionCount = withoutPositionCount;
    g_interactiveScanDebug.posFromGetterCount = posFromGetterCount;
    g_interactiveScanDebug.posFromRootCount = posFromRootCount;
    g_interactiveScanDebug.posFromEntityDataCount = posFromEntityDataCount;
    g_interactiveScanDebug.posFromLevelDataCount = posFromLevelDataCount;
    g_interactiveScanDebug.filteredByDuplicatePosCount = filteredByDuplicatePosCount;
    g_interactiveScanDebug.filteredByDistanceCount = filteredByDistanceCount;
    g_interactiveScanDebug.filteredByKeywordCount = filteredByKeywordCount;
    g_interactiveScanDebug.filteredByTemplateCount = filteredByTemplateCount;

    std::sort(outChests.begin(), outChests.end(), [](const NearbyChestInfo& lhs, const NearbyChestInfo& rhs) {
        if (lhs.hasPosition != rhs.hasPosition) {
            return lhs.hasPosition && !rhs.hasPosition;
        }
        if (lhs.hasInteractiveRoot != rhs.hasInteractiveRoot) {
            return lhs.hasInteractiveRoot && !rhs.hasInteractiveRoot;
        }
        if (lhs.keywordMatched != rhs.keywordMatched) {
            return lhs.keywordMatched && !rhs.keywordMatched;
        }
        if (lhs.hasPosition && rhs.hasPosition &&
            lhs.distance >= 0.0f && rhs.distance >= 0.0f &&
            lhs.distance != rhs.distance) {
            return lhs.distance < rhs.distance;
        }
        return lhs.entityAddress < rhs.entityAddress;
    });

    if (outChests.size() > 20000) {
        outChests.resize(20000);
    }
}

static void DrawChestRadar(const std::vector<NearbyChestInfo>& chests,
                           const Vec3& playerPos,
                           float radarRange,
                           int selectedIndex) {
    const float clampedRange = (std::max)(20.0f, radarRange);
    const float canvasSize = 260.0f;
    ImGui::BeginChild("chest_radar_canvas", ImVec2(canvasSize, canvasSize), true);

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 center = ImVec2(origin.x + canvasSize * 0.5f, origin.y + canvasSize * 0.5f);
    const float radius = canvasSize * 0.46f;

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddCircle(center, radius, IM_COL32(90, 180, 255, 180), 64, 1.5f);
    draw->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(90, 180, 255, 100));
    draw->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(90, 180, 255, 100));
    draw->AddCircleFilled(center, 3.5f, IM_COL32(120, 255, 120, 240));

    for (size_t i = 0; i < chests.size(); ++i) {
        const auto& chest = chests[i];
        if (!chest.hasPosition) {
            continue;
        }

        const float dx = chest.position.x - playerPos.x;
        const float dz = chest.position.z - playerPos.z;
        float nx = dx / clampedRange;
        float ny = dz / clampedRange;
        const float len = std::sqrt(nx * nx + ny * ny);
        if (len > 1.0f && len > 0.0001f) {
            nx /= len;
            ny /= len;
        }

        const ImVec2 dot = ImVec2(center.x + nx * radius, center.y + ny * radius);
        const bool selected = static_cast<int>(i) == selectedIndex;
        const ImU32 color = selected ? IM_COL32(255, 200, 50, 255) : IM_COL32(255, 80, 80, 230);
        draw->AddCircleFilled(dot, selected ? 5.0f : 4.0f, color);
    }

    ImGui::Dummy(ImVec2(canvasSize - 12.0f, canvasSize - 12.0f));
    ImGui::EndChild();
}

template <typename T>
static bool WriteValueSafe(uintptr_t addr, T value) {
    if (addr == 0) {
        return false;
    }

    __try {
        *reinterpret_cast<T*>(addr) = value;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool AnyLockEnabled() {
    return g_statEditor.lockHpClient || g_statEditor.lockHpServer || g_statEditor.lockSkillCooldown ||
           g_statEditor.lockSkillCooldownAddition || g_statEditor.lockAtbCostAddition ||
           g_statEditor.lockAtbValue || g_statEditor.lockUltimateSpGainScalar ||
           g_statEditor.lockUltimateSp || g_statEditor.lockCurrentDash ||
           g_statEditor.lockMaxDash || g_statEditor.lockCriticalRate ||
           g_statEditor.lockDamageMultiplier || g_statEditor.lockNoCooldownBypass ||
           g_statEditor.lockAllSquadUltimateNoCd || g_statEditor.lockInfiniteStamina ||
           g_statEditor.lockInfiniteBlink;
}

static void SetInfiniteBlueNoCdPreset(bool enabled) {
    g_statEditor.lockInfiniteBlueNoCd = enabled;
    if (enabled) {
        g_statEditor.lockSkillCooldown = true;
        g_statEditor.lockSkillCooldownAddition = true;
        g_statEditor.lockAtbCostAddition = true;
        g_statEditor.lockAtbValue = true;
        g_statEditor.lockUltimateSpGainScalar = true;
        g_statEditor.lockUltimateSp = true;
        g_statEditor.lockNoCooldownBypass = true;

        g_statEditor.skillCooldownScalar = kPresetSkillCooldownScalar;
        g_statEditor.skillCooldownAddition = kPresetSkillCooldownAddition;
        g_statEditor.atbCostAddition = kPresetAtbCostAddition;
        g_statEditor.atbValue = (std::max)(g_statEditor.atbValue, 9999.0f);
        g_statEditor.ultimateSpGainScalar = kPresetUltimateSpGainScalar;
        g_statEditor.ultimateSp = (std::max)(g_statEditor.ultimateSp, kPresetUltimateSp);
    } else {
        g_statEditor.lockSkillCooldown = false;
        g_statEditor.lockSkillCooldownAddition = false;
        g_statEditor.lockAtbCostAddition = false;
        g_statEditor.lockAtbValue = false;
        g_statEditor.lockUltimateSpGainScalar = false;
        g_statEditor.lockUltimateSp = false;
        g_statEditor.lockNoCooldownBypass = false;
    }
}

static void SetAllSquadUltimateNoCdPreset(bool enabled) {
    g_statEditor.lockAllSquadUltimateNoCd = enabled;
    if (enabled) {
        g_statEditor.lockNoCooldownBypass = true;
        g_statEditor.lockUltimateSp = true;
        g_statEditor.ultimateSp = (std::max)(g_statEditor.ultimateSp, kPresetAllSquadUltimateSp);
    }
}

static void SetInfiniteStaminaPreset(bool enabled) {
    g_statEditor.lockInfiniteStamina = enabled;
    if (enabled) {
        g_statEditor.lockCurrentDash = true;
        g_statEditor.lockMaxDash = true;
        g_statEditor.currentDash = (std::max)(g_statEditor.currentDash, kPresetInfiniteDashCount);
        g_statEditor.maxDash = (std::max)(g_statEditor.maxDash, kPresetInfiniteDashCount);
    }
}

static void SetInfiniteBlinkPreset(bool enabled) {
    g_statEditor.lockInfiniteBlink = enabled;
    if (enabled) {
        SetInfiniteStaminaPreset(true);
    }
}

static int LockAllSquadUltimateSp(uintptr_t gameInstanceAddress, float spValue);

static bool ApplyEditedValues(PlayerController* pc, AbilitySystem* ability, uintptr_t gameInstanceAddress, bool onlyLocked) {
    bool wroteAny = false;
    const auto shouldWrite = [onlyLocked](bool lockFlag) {
        return !onlyLocked || lockFlag;
    };

    if (IsValidPtr(ability)) {
        const uintptr_t abilityBase = reinterpret_cast<uintptr_t>(ability);

        if (shouldWrite(g_statEditor.lockHpClient)) {
            wroteAny |= WriteValueSafe<double>(abilityBase + 0x138, g_statEditor.hpClient);
        }
        if (shouldWrite(g_statEditor.lockHpServer)) {
            wroteAny |= WriteValueSafe<double>(abilityBase + 0x140, g_statEditor.hpServer);
        }
        if (shouldWrite(g_statEditor.lockSkillCooldown)) {
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownScalar, static_cast<double>(g_statEditor.skillCooldownScalar));
        }
        if (shouldWrite(g_statEditor.lockSkillCooldownAddition)) {
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownAddition, static_cast<double>(g_statEditor.skillCooldownAddition));
        }
        if (shouldWrite(g_statEditor.lockAtbCostAddition)) {
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexAtbCostAddition, static_cast<double>(g_statEditor.atbCostAddition));
        }
        if (shouldWrite(g_statEditor.lockAtbValue)) {
            wroteAny |= WriteValueSafe<float>(abilityBase + 0x150, g_statEditor.atbValue);
        }
        if (shouldWrite(g_statEditor.lockUltimateSpGainScalar)) {
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexUltimateSpGainScalar, static_cast<double>(g_statEditor.ultimateSpGainScalar));
        }
        if (shouldWrite(g_statEditor.lockUltimateSp)) {
            bool setOk = TryCallAbilitySetUltimateSp(ability, g_statEditor.ultimateSp);
            if (!setOk) {
                setOk = WriteValueSafe<float>(abilityBase + 0x154, g_statEditor.ultimateSp);
            }
            wroteAny |= setOk;
        }
        if (shouldWrite(g_statEditor.lockCriticalRate)) {
            const float clampedCriticalRate = (std::max)(0.0f, g_statEditor.criticalRate);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexCriticalRate, static_cast<double>(clampedCriticalRate));
        }
        if (shouldWrite(g_statEditor.lockDamageMultiplier)) {
            const float clampedMultiplier = (std::max)(1.0f, g_statEditor.damageMultiplier);
            const double damageAdd = static_cast<double>(clampedMultiplier - 1.0f);

            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexNormalAttackDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexNormalSkillDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexComboSkillDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexUltimateSkillDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexPhysicalDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexFireDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexPulseDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexCrystDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexNaturalDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexEtherDamageIncrease, damageAdd);
            wroteAny |= TryWriteAttributeDoubleByIndex(ability, kAttrIndexCriticalDamageIncrease, damageAdd);
        }
    }

    if (IsValidPtr(pc)) {
        const uintptr_t pcBase = reinterpret_cast<uintptr_t>(pc);
        if (shouldWrite(g_statEditor.lockMaxDash)) {
            wroteAny |= WriteValueSafe<float>(pcBase + 0x80, g_statEditor.maxDash);
        }
        if (shouldWrite(g_statEditor.lockCurrentDash)) {
            wroteAny |= WriteValueSafe<float>(pcBase + 0x84, g_statEditor.currentDash);
        }

        if (g_statEditor.lockInfiniteStamina || g_statEditor.lockInfiniteBlink) {
            const float dashTarget = (std::max)((std::max)(g_statEditor.currentDash, g_statEditor.maxDash), kPresetInfiniteDashCount);
            wroteAny |= WriteValueSafe<float>(pcBase + 0x80, dashTarget);
            wroteAny |= WriteValueSafe<float>(pcBase + 0x84, dashTarget);
            wroteAny |= WriteValueSafe<float>(pcBase + 0x88, dashTarget);
        }
    }

    if (g_statEditor.lockAllSquadUltimateNoCd && IsCanonicalUserPtr(gameInstanceAddress)) {
        const float squadUltimateSp = (std::max)(g_statEditor.ultimateSp, kPresetAllSquadUltimateSp);
        const int applied = LockAllSquadUltimateSp(gameInstanceAddress, squadUltimateSp);
        wroteAny |= (applied > 0);
    }

    return wroteAny;
}

static bool TryCallGameInstanceGetMainChar(uintptr_t gameInstanceAddress, Entity*& outEntity) {
    outEntity = nullptr;
    if (g_gameBaseAddress == 0 || gameInstanceAddress == 0) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kGameInstanceGetMainCharPtrRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<GameInstanceGetMainCharPtrFn>(fnAddress);
        outEntity = fn(reinterpret_cast<void*>(gameInstanceAddress), nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return IsValidPtr(outEntity);
}

static bool TryCallEntityGetAbilityCom(Entity* entity, AbilitySystem*& outAbility) {
    outAbility = nullptr;
    if (g_gameBaseAddress == 0 || !IsValidPtr(entity)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kEntityGetAbilityComRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<EntityGetAbilityComFn>(fnAddress);
        outAbility = fn(entity, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return IsValidPtr(outAbility);
}

static bool TryCallAbilityGetAttributes(AbilitySystem* ability, Attributes*& outAttr) {
    outAttr = nullptr;
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemGetAttributesRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemGetAttributesFn>(fnAddress);
        outAttr = fn(ability, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return IsValidPtr(outAttr);
}

static bool TryCallAbilityGetHp(AbilitySystem* ability, double& outHp) {
    outHp = 0.0;
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemGetHpRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemGetHpFn>(fnAddress);
        outHp = fn(ability, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallAbilityGetServerHp(AbilitySystem* ability, double& outHp) {
    outHp = 0.0;
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemGetServerHpRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemGetServerHpFn>(fnAddress);
        outHp = fn(ability, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallAbilityGetUltimateSp(AbilitySystem* ability, float& outSp) {
    outSp = 0.0f;
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemGetUltimateSpRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemGetUltimateSpFn>(fnAddress);
        outSp = fn(ability, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryReadAbilityHpPair(AbilitySystem* ability, double& clientValue, double& serverValue) {
    clientValue = 0.0;
    serverValue = 0.0;

    if (!IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t base = reinterpret_cast<uintptr_t>(ability);
    const bool directClient = ReadValue(base + 0x138, clientValue);
    const bool directServer = ReadValue(base + 0x140, serverValue);
    if (directClient || directServer) {
        return true;
    }

    if (!IsValidPtr(ability->m_hp)) {
        return false;
    }

    const uintptr_t legacyBase = reinterpret_cast<uintptr_t>(ability->m_hp);
    const bool legacyClient = ReadValue(legacyBase + 0x10, clientValue);
    const bool legacyServer = ReadValue(legacyBase + 0x18, serverValue);
    return legacyClient || legacyServer;
}

static bool TryCallAbilitySetUltimateSp(AbilitySystem* ability, float value) {
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemSetUltimateSpRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemSetUltimateSpFn>(fnAddress);
        fn(ability, value, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryCallAbilityGetSkillCooldownScalar(AbilitySystem* ability, float& outScalar) {
    outScalar = 1.0f;
    if (g_gameBaseAddress == 0 || !IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t fnAddress = g_gameBaseAddress + kAbilitySystemGetSkillCooldownScalarRva;
    if (IsBadReadPtr(reinterpret_cast<void*>(fnAddress), 16)) {
        return false;
    }

    __try {
        auto fn = reinterpret_cast<AbilitySystemGetSkillCooldownScalarFn>(fnAddress);
        outScalar = fn(ability, nullptr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

static bool TryGetAttributeArrayAddress(AbilitySystem* ability, uintptr_t& outArrayAddress) {
    outArrayAddress = 0;
    if (!IsValidPtr(ability)) {
        return false;
    }

    const uintptr_t abilityBase = reinterpret_cast<uintptr_t>(ability);
    uintptr_t attrAddress = 0;
    if (!ReadValue(abilityBase + 0x130, attrAddress) || !IsCanonicalUserPtr(attrAddress)) {
        return false;
    }

    uintptr_t arrayAddress = 0;
    if (!ReadValue(attrAddress + 0x28, arrayAddress) || !IsCanonicalUserPtr(arrayAddress)) {
        uintptr_t cacheAddress = 0;
        if (!ReadValue(attrAddress + 0x18, cacheAddress) || !IsCanonicalUserPtr(cacheAddress) ||
            !ReadValue(cacheAddress + 0x20, arrayAddress) || !IsCanonicalUserPtr(arrayAddress)) {
            return false;
        }
    }

    int32_t maxLength = 0;
    if (!ReadValue(arrayAddress + 0x18, maxLength) || maxLength <= 0 || maxLength > 4096) {
        return false;
    }

    outArrayAddress = arrayAddress;
    return true;
}

static bool TryReadAttributeDoubleByIndex(AbilitySystem* ability, int index, double& outValue) {
    outValue = 0.0;
    if (index < 0) {
        return false;
    }

    uintptr_t arrayAddress = 0;
    if (!TryGetAttributeArrayAddress(ability, arrayAddress)) {
        return false;
    }

    int32_t maxLength = 0;
    if (!ReadValue(arrayAddress + 0x18, maxLength) || index >= maxLength) {
        return false;
    }

    const uintptr_t valueAddress = arrayAddress + 0x20 + static_cast<uintptr_t>(index) * sizeof(double);
    return ReadValue(valueAddress, outValue);
}

static bool TryWriteAttributeDoubleByIndex(AbilitySystem* ability, int index, double value) {
    if (index < 0) {
        return false;
    }

    uintptr_t arrayAddress = 0;
    if (!TryGetAttributeArrayAddress(ability, arrayAddress)) {
        return false;
    }

    int32_t maxLength = 0;
    if (!ReadValue(arrayAddress + 0x18, maxLength) || index >= maxLength) {
        return false;
    }

    const uintptr_t valueAddress = arrayAddress + 0x20 + static_cast<uintptr_t>(index) * sizeof(double);
    return WriteValueSafe<double>(valueAddress, value);
}

static bool TryReadManagedListHeader(uintptr_t listAddress, uintptr_t& outItemsArrayAddress, int32_t& outSize) {
    outItemsArrayAddress = 0;
    outSize = 0;
    if (!IsCanonicalUserPtr(listAddress)) {
        return false;
    }

    if (!ReadValue(listAddress + kManagedListItemsOffset, outItemsArrayAddress) ||
        !IsCanonicalUserPtr(outItemsArrayAddress) ||
        !ReadValue(listAddress + kManagedListSizeOffset, outSize) ||
        outSize <= 0 || outSize > 128) {
        return false;
    }

    int32_t maxLength = 0;
    if (!ReadValue(outItemsArrayAddress + 0x18, maxLength) || maxLength <= 0) {
        return false;
    }

    outSize = (std::min)(outSize, maxLength);
    return outSize > 0;
}

static bool TryResolveAbilityAddressFromSquadItem(uintptr_t itemAddress, uintptr_t& outAbilityAddress) {
    outAbilityAddress = 0;
    if (!IsCanonicalUserPtr(itemAddress)) {
        return false;
    }

    const auto tryEntityAddress = [&outAbilityAddress](uintptr_t entityAddress) {
        if (!IsCanonicalUserPtr(entityAddress)) {
            return false;
        }

        uintptr_t abilityAddress = 0;
        if (!ReadValue(entityAddress + 0x120, abilityAddress) || !IsCanonicalUserPtr(abilityAddress)) {
            return false;
        }

        outAbilityAddress = abilityAddress;
        return true;
    };

    if (tryEntityAddress(itemAddress)) {
        return true;
    }

    constexpr std::array<uintptr_t, 8> kNestedEntityOffsets = {0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48};
    for (const uintptr_t nestedOffset : kNestedEntityOffsets) {
        uintptr_t nestedAddress = 0;
        if (!ReadValue(itemAddress + nestedOffset, nestedAddress)) {
            continue;
        }
        if (tryEntityAddress(nestedAddress)) {
            return true;
        }
    }

    return false;
}

static int LockAllSquadUltimateSp(uintptr_t gameInstanceAddress, float spValue) {
    uintptr_t gamePlayerAddress = 0;
    if (!ReadValue(gameInstanceAddress + kGameInstancePlayerOffset, gamePlayerAddress) ||
        !IsCanonicalUserPtr(gamePlayerAddress)) {
        return 0;
    }

    uintptr_t squadManagerAddress = 0;
    if (!ReadValue(gamePlayerAddress + kGamePlayerSquadManagerOffset, squadManagerAddress) ||
        !IsCanonicalUserPtr(squadManagerAddress)) {
        return 0;
    }

    uintptr_t clientCharsListAddress = 0;
    if (!ReadValue(squadManagerAddress + kSquadManagerClientCharsOffset, clientCharsListAddress) ||
        !IsCanonicalUserPtr(clientCharsListAddress)) {
        return 0;
    }

    uintptr_t itemsArrayAddress = 0;
    int32_t count = 0;
    if (!TryReadManagedListHeader(clientCharsListAddress, itemsArrayAddress, count)) {
        return 0;
    }

    int appliedCount = 0;
    for (int32_t i = 0; i < count; ++i) {
        uintptr_t itemAddress = 0;
        if (!ReadValue(itemsArrayAddress + 0x20 + static_cast<uintptr_t>(i) * sizeof(uintptr_t), itemAddress) ||
            !IsCanonicalUserPtr(itemAddress)) {
            continue;
        }

        uintptr_t abilityAddress = 0;
        if (!TryResolveAbilityAddressFromSquadItem(itemAddress, abilityAddress)) {
            continue;
        }

        auto* squadAbility = reinterpret_cast<AbilitySystem*>(abilityAddress);
        bool ok = TryCallAbilitySetUltimateSp(squadAbility, spValue);
        ok |= WriteValueSafe<float>(abilityAddress + 0x154, spValue);

        if (ok) {
            ++appliedCount;
        }
    }

    return appliedCount;
}

static DWORD WINAPI FastUltimateThreadProc(LPVOID) {
    while (!g_fastUltimateThreadStop.load()) {
        const uintptr_t abilityAddress = g_fastUltimateAbilityAddress.load();
        const uintptr_t pcAddress = g_fastPlayerControllerAddress.load();
        const uintptr_t gameInstanceAddress = g_fastGameInstanceAddress.load();

        if (abilityAddress != 0) {
            auto* ability = reinterpret_cast<AbilitySystem*>(abilityAddress);

            if (g_statEditor.lockSkillCooldown) {
                TryWriteAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownScalar, static_cast<double>(g_statEditor.skillCooldownScalar));
            }
            if (g_statEditor.lockSkillCooldownAddition) {
                TryWriteAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownAddition, static_cast<double>(g_statEditor.skillCooldownAddition));
            }
            if (g_statEditor.lockAtbCostAddition) {
                TryWriteAttributeDoubleByIndex(ability, kAttrIndexAtbCostAddition, static_cast<double>(g_statEditor.atbCostAddition));
            }
            if (g_statEditor.lockAtbValue) {
                WriteValueSafe<float>(abilityAddress + 0x150, g_statEditor.atbValue);
            }
            if (g_statEditor.lockUltimateSpGainScalar) {
                TryWriteAttributeDoubleByIndex(ability, kAttrIndexUltimateSpGainScalar, static_cast<double>(g_statEditor.ultimateSpGainScalar));
            }
            if (g_statEditor.lockUltimateSp) {
                if (!TryCallAbilitySetUltimateSp(ability, g_statEditor.ultimateSp)) {
                    WriteValueSafe<float>(abilityAddress + 0x154, g_statEditor.ultimateSp);
                }
            }
        }

        if (pcAddress != 0 && (g_statEditor.lockInfiniteStamina || g_statEditor.lockInfiniteBlink)) {
            const float dashTarget = (std::max)((std::max)(g_statEditor.currentDash, g_statEditor.maxDash), kPresetInfiniteDashCount);
            WriteValueSafe<float>(pcAddress + 0x80, dashTarget);
            WriteValueSafe<float>(pcAddress + 0x84, dashTarget);
            WriteValueSafe<float>(pcAddress + 0x88, dashTarget);
        }

        if (gameInstanceAddress != 0 && g_statEditor.lockAllSquadUltimateNoCd) {
            const float squadUltimateSp = (std::max)(g_statEditor.ultimateSp, kPresetAllSquadUltimateSp);
            LockAllSquadUltimateSp(gameInstanceAddress, squadUltimateSp);
        }

        Sleep(8);
    }

    g_fastUltimateThreadRunning.store(false);
    return 0;
}

static void EnsureFastUltimateThread() {
    if (g_fastUltimateThreadRunning.load()) {
        return;
    }

    g_fastUltimateThreadStop.store(false);
    g_fastUltimateThread = CreateThread(nullptr, 0, FastUltimateThreadProc, nullptr, 0, nullptr);
    if (g_fastUltimateThread) {
        g_fastUltimateThreadRunning.store(true);
    }
}

static void UpdateFastUltimateTarget(AbilitySystem* ability, PlayerController* pc, uintptr_t gameInstanceAddress) {
    g_fastUltimateAbilityAddress.store(reinterpret_cast<uintptr_t>(ability));
    g_fastPlayerControllerAddress.store(reinterpret_cast<uintptr_t>(pc));
    g_fastGameInstanceAddress.store(gameInstanceAddress);
}

static void StopFastUltimateThread() {
    g_fastUltimateThreadStop.store(true);
    g_fastUltimateAbilityAddress.store(0);
    g_fastPlayerControllerAddress.store(0);
    g_fastGameInstanceAddress.store(0);

    if (g_fastUltimateThread) {
        WaitForSingleObject(g_fastUltimateThread, 3000);
        CloseHandle(g_fastUltimateThread);
        g_fastUltimateThread = nullptr;
    }

    g_fastUltimateThreadRunning.store(false);
}

static void InitGameBase() {
    if (g_gameBaseAddress != 0) {
        return;
    }

    HMODULE mod = GetModuleHandleA("GameAssembly.dll");
    if (!mod) {
        mod = GetModuleHandleA("UnityPlayer.dll");
    }

    if (mod) {
        g_gameBaseAddress = reinterpret_cast<uintptr_t>(mod);
    }
}

static bool TryParseHex(const char* text, uintptr_t& outValue) {
    if (!text) {
        return false;
    }

    const char* p = text;
    while (*p && std::isspace(static_cast<unsigned char>(*p))) {
        ++p;
    }

    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }

    if (*p == '\0') {
        return false;
    }

    errno = 0;
    char* end = nullptr;
    const unsigned long long value = std::strtoull(p, &end, 16);
    if (p == end || errno == ERANGE) {
        return false;
    }

    while (*end && std::isspace(static_cast<unsigned char>(*end))) {
        ++end;
    }

    if (*end != '\0') {
        return false;
    }

    outValue = static_cast<uintptr_t>(value);
    return true;
}

static void EnsureTypeInfoOffsetInitialized() {
    if (g_typeInfoOffset != 0) {
        return;
    }

    uintptr_t parsed = 0;
    if (TryParseHex(g_offsetInput, parsed)) {
        g_typeInfoOffset = parsed;
    }
}

static CustomBaseSource ToCustomBaseSource(int index) {
    const int clamped = (std::max)(0, (std::min)(index, IM_ARRAYSIZE(kCustomBaseSourceItems) - 1));
    return static_cast<CustomBaseSource>(clamped);
}

static CustomValueType ToCustomValueType(int index) {
    const int clamped = (std::max)(0, (std::min)(index, IM_ARRAYSIZE(kCustomValueTypeItems) - 1));
    return static_cast<CustomValueType>(clamped);
}

static bool IsBlankText(const char* text) {
    if (!text) {
        return true;
    }

    for (const char* p = text; *p; ++p) {
        if (!std::isspace(static_cast<unsigned char>(*p))) {
            return false;
        }
    }

    return true;
}

static void SetCustomWatchStatus(const char* text, bool isError) {
    std::snprintf(g_customWatchStatusText, sizeof(g_customWatchStatusText), "%s", text ? text : "");
    g_customWatchStatusError = isError;
}

static void SetTreasureStatus(const char* text, bool isError) {
    std::snprintf(g_treasureTool.statusText, sizeof(g_treasureTool.statusText), "%s", text ? text : "");
    g_treasureTool.statusError = isError;
}

static void WriteTeleportLog(const char* fmt, ...) {
    if (!fmt) {
        return;
    }

    char tempPath[MAX_PATH] = "";
    if (GetTempPathA(static_cast<DWORD>(sizeof(tempPath)), tempPath) == 0) {
        std::snprintf(tempPath, sizeof(tempPath), ".\\");
    }

    char logPath[MAX_PATH] = "";
    std::snprintf(logPath, sizeof(logPath), "%s%s", tempPath, "EndfieldCheat_tp.log");

    char message[1024] = "";
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    SYSTEMTIME st{};
    GetLocalTime(&st);
    const int seq = g_teleportLogSequence.fetch_add(1, std::memory_order_acq_rel) + 1;

    std::lock_guard<std::mutex> lock(g_tpLogMutex);
    FILE* fp = nullptr;
    if (fopen_s(&fp, logPath, "a") != 0 || !fp) {
        return;
    }

    std::fprintf(fp,
                 "[%04u-%02u-%02u %02u:%02u:%02u.%03u][%d] %s\n",
                 st.wYear,
                 st.wMonth,
                 st.wDay,
                 st.wHour,
                 st.wMinute,
                 st.wSecond,
                 st.wMilliseconds,
                 seq,
                 message);
    std::fclose(fp);
}

static bool UpdateCustomWatchOffset(CustomWatchItem& item) {
    uintptr_t parsed = 0;
    if (!TryParseHex(item.offsetInput, parsed)) {
        item.parsedOk = false;
        return false;
    }

    item.offsetOrAddress = parsed;
    item.parsedOk = true;
    std::snprintf(item.offsetInput, sizeof(item.offsetInput), "0x%llX", static_cast<unsigned long long>(parsed));
    return true;
}

static uintptr_t ResolveCustomWatchAddress(
    const CustomWatchItem& item,
    PlayerController* pc,
    Entity* entity,
    AbilitySystem* ability,
    uintptr_t gameInstanceAddress) {
    if (!item.parsedOk) {
        return 0;
    }

    if (item.baseSource == CustomBaseSource::AbsoluteAddress) {
        return item.offsetOrAddress;
    }

    uintptr_t baseAddress = 0;
    switch (item.baseSource) {
    case CustomBaseSource::PlayerController:
        baseAddress = reinterpret_cast<uintptr_t>(pc);
        break;
    case CustomBaseSource::Entity:
        baseAddress = reinterpret_cast<uintptr_t>(entity);
        break;
    case CustomBaseSource::AbilitySystem:
        baseAddress = reinterpret_cast<uintptr_t>(ability);
        break;
    case CustomBaseSource::GameInstance:
        baseAddress = gameInstanceAddress;
        break;
    case CustomBaseSource::AbsoluteAddress:
        baseAddress = 0;
        break;
    }

    if (baseAddress == 0) {
        return 0;
    }

    return baseAddress + item.offsetOrAddress;
}

static bool ReadCustomWatchValue(CustomWatchItem& item, uintptr_t address) {
    switch (item.valueType) {
    case CustomValueType::Bool: {
        uint8_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        item.boolValue = (value != 0);
        return true;
    }
    case CustomValueType::Int32:
        return ReadValue(address, item.i32Value);
    case CustomValueType::UInt32:
        return ReadValue(address, item.u32Value);
    case CustomValueType::Int64:
        return ReadValue(address, item.i64Value);
    case CustomValueType::UInt64:
        return ReadValue(address, item.u64Value);
    case CustomValueType::Float:
        return ReadValue(address, item.f32Value);
    case CustomValueType::Double:
        return ReadValue(address, item.f64Value);
    case CustomValueType::Pointer:
        return ReadValue(address, item.ptrValue);
    case CustomValueType::Byte:
        return ReadValue(address, item.u8Value);
    }

    return false;
}

static bool ReadCustomWatchValueText(const CustomWatchItem& item, uintptr_t address, char* outText, size_t outTextSize) {
    if (!outText || outTextSize == 0) {
        return false;
    }

    outText[0] = '\0';
    switch (item.valueType) {
    case CustomValueType::Bool: {
        uint8_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%s", value ? "true" : "false");
        return true;
    }
    case CustomValueType::Int32: {
        int32_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%d", value);
        return true;
    }
    case CustomValueType::UInt32: {
        uint32_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%u", value);
        return true;
    }
    case CustomValueType::Int64: {
        int64_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%lld", static_cast<long long>(value));
        return true;
    }
    case CustomValueType::UInt64: {
        uint64_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%llu", static_cast<unsigned long long>(value));
        return true;
    }
    case CustomValueType::Float: {
        float value = 0.0f;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%.6f", value);
        return true;
    }
    case CustomValueType::Double: {
        double value = 0.0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%.6f", value);
        return true;
    }
    case CustomValueType::Pointer: {
        uintptr_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "0x%llX", static_cast<unsigned long long>(value));
        return true;
    }
    case CustomValueType::Byte: {
        uint8_t value = 0;
        if (!ReadValue(address, value)) {
            return false;
        }
        std::snprintf(outText, outTextSize, "%u (0x%02X)", value, value);
        return true;
    }
    }

    return false;
}

static bool WriteCustomWatchValue(const CustomWatchItem& item, uintptr_t address) {
    switch (item.valueType) {
    case CustomValueType::Bool:
        return WriteValueSafe<uint8_t>(address, item.boolValue ? 1 : 0);
    case CustomValueType::Int32:
        return WriteValueSafe<int32_t>(address, item.i32Value);
    case CustomValueType::UInt32:
        return WriteValueSafe<uint32_t>(address, item.u32Value);
    case CustomValueType::Int64:
        return WriteValueSafe<int64_t>(address, item.i64Value);
    case CustomValueType::UInt64:
        return WriteValueSafe<uint64_t>(address, item.u64Value);
    case CustomValueType::Float:
        return WriteValueSafe<float>(address, item.f32Value);
    case CustomValueType::Double:
        return WriteValueSafe<double>(address, item.f64Value);
    case CustomValueType::Pointer:
        return WriteValueSafe<uintptr_t>(address, item.ptrValue);
    case CustomValueType::Byte:
        return WriteValueSafe<uint8_t>(address, item.u8Value);
    }

    return false;
}

static int ApplyLockedCustomWatchItems(
    PlayerController* pc,
    Entity* entity,
    AbilitySystem* ability,
    uintptr_t gameInstanceAddress) {
    int writeSuccessCount = 0;

    for (CustomWatchItem& item : g_customWatchItems) {
        if (!item.lockWrite || !item.parsedOk) {
            continue;
        }

        const uintptr_t address = ResolveCustomWatchAddress(item, pc, entity, ability, gameInstanceAddress);
        if (address == 0) {
            continue;
        }

        if (WriteCustomWatchValue(item, address)) {
            ++writeSuccessCount;
        }
    }

    return writeSuccessCount;
}

static bool ResolveViaGetter(ResolveResult& result) {
    result.getterFunctionAddress = g_gameBaseAddress + kGameInstanceGetInstanceRva;

    if (result.getterFunctionAddress == 0 || IsBadReadPtr(reinterpret_cast<void*>(result.getterFunctionAddress), 16)) {
        result.error = u8"getter 函数地址无效";
        return false;
    }

    const uint8_t* code = reinterpret_cast<const uint8_t*>(result.getterFunctionAddress);
    if (code[0] != 0x48 || code[1] != 0x83 || code[2] != 0xEC) {
        result.error = u8"getter RVA 已变化（函数特征不匹配）";
        return false;
    }

    uintptr_t gameInstance = 0;
    __try {
        auto fn = reinterpret_cast<GameInstanceGetInstanceFn>(result.getterFunctionAddress);
        gameInstance = reinterpret_cast<uintptr_t>(fn());
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        result.error = u8"调用 GameInstance_get_instance 时异常";
        return false;
    }

    if (!IsCanonicalUserPtr(gameInstance) || !IsValidPtr(reinterpret_cast<void*>(gameInstance))) {
        result.error = u8"getter 返回的 GameInstance 指针无效";
        return false;
    }

    result.gameInstanceAddress = gameInstance;
    result.playerControllerFieldAddress = gameInstance + kGameInstancePlayerControllerOffset;

    if (!ReadValue(result.playerControllerFieldAddress, result.playerController) || !IsValidPtr(result.playerController)) {
        result.error = u8"m_playerController 为空（请先进入地图）";
        return false;
    }

    result.path = u8"RVA 函数";
    result.ok = true;
    result.error = u8"成功";
    return true;
}

static bool ResolveViaTypeInfo(ResolveResult& result) {
    if (g_typeInfoOffset == 0) {
        result.error = u8"TypeInfo 偏移为 0";
        return false;
    }

    result.typeInfoStorageAddress = g_gameBaseAddress + g_typeInfoOffset;
    if (!ReadValue(result.typeInfoStorageAddress, result.typeInfoAddress) || result.typeInfoAddress == 0) {
        result.error = u8"读取 TypeInfo 指针失败";
        return false;
    }

    if (!IsCanonicalUserPtr(result.typeInfoAddress)) {
        result.error = u8"TypeInfo 指针异常（可能尚未初始化）";
        return false;
    }

    for (uintptr_t staticOffset : kStaticFieldsOffsets) {
        const uintptr_t staticFieldsPtrAddress = result.typeInfoAddress + staticOffset;
        uintptr_t staticFields = 0;
        if (!ReadValue(staticFieldsPtrAddress, staticFields) || !IsCanonicalUserPtr(staticFields)) {
            continue;
        }

        uintptr_t gameInstance = 0;
        if (!ReadValue(staticFields + kGameInstanceStaticInstanceOffset, gameInstance) || !IsCanonicalUserPtr(gameInstance)) {
            continue;
        }

        uintptr_t playerControllerField = gameInstance + kGameInstancePlayerControllerOffset;
        PlayerController* playerController = nullptr;
        if (!ReadValue(playerControllerField, playerController) || !IsValidPtr(playerController)) {
            continue;
        }

        result.staticFieldsAddress = staticFields;
        result.staticFieldsOffsetUsed = staticOffset;
        result.gameInstanceAddress = gameInstance;
        result.playerControllerFieldAddress = playerControllerField;
        result.playerController = playerController;
        result.path = u8"TypeInfo 链";
        result.ok = true;
        result.error = u8"成功";
        return true;
    }

    result.error = u8"通过 TypeInfo 解析 static_fields/GameInstance 失败";
    return false;
}

static ResolveResult ResolvePlayerController() {
    ResolveResult result;

    if (g_gameBaseAddress == 0) {
        result.error = u8"未找到 GameAssembly.dll";
        return result;
    }

    if (ResolveViaGetter(result)) {
        return result;
    }

    ResolveResult fallback = result;
    if (ResolveViaTypeInfo(fallback)) {
        return fallback;
    }

    result.error = fallback.error;
    return result;
}

static void DrawFloatField(PlayerController* pc, uintptr_t offset, const char* name) {
    float value = 0.0f;
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): %.4f", name, static_cast<unsigned long long>(offset), value);
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawBoolField(PlayerController* pc, uintptr_t offset, const char* name) {
    bool value = false;
    if (ReadPcBoolField(pc, offset, value)) {
        ImGui::Text(u8"%s (0x%llX): %s", name, static_cast<unsigned long long>(offset), value ? u8"是" : u8"否");
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawUInt32Field(PlayerController* pc, uintptr_t offset, const char* name) {
    uint32_t value = 0;
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): %u", name, static_cast<unsigned long long>(offset), value);
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawInt32Field(PlayerController* pc, uintptr_t offset, const char* name) {
    int32_t value = 0;
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): %d", name, static_cast<unsigned long long>(offset), value);
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawPointerField(PlayerController* pc, uintptr_t offset, const char* name) {
    uintptr_t value = 0;
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): %p", name, static_cast<unsigned long long>(offset), reinterpret_cast<void*>(value));
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawVec2Field(PlayerController* pc, uintptr_t offset, const char* name) {
    Vec2 value{};
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): (%.3f, %.3f)", name, static_cast<unsigned long long>(offset), value.x, value.y);
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawVec3Field(PlayerController* pc, uintptr_t offset, const char* name) {
    Vec3 value{};
    if (ReadPcField(pc, offset, value)) {
        ImGui::Text("%s (0x%llX): (%.3f, %.3f, %.3f)", name, static_cast<unsigned long long>(offset), value.x, value.y, value.z);
    } else {
        ImGui::TextDisabled(u8"%s (0x%llX): <读取失败>", name, static_cast<unsigned long long>(offset));
    }
}

static void DrawPlayerControllerHexDump(PlayerController* pc) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(pc);
    const size_t dumpSize = static_cast<size_t>(kPlayerControllerDumpBytes);

    if (base == 0 || IsBadReadPtr(reinterpret_cast<void*>(base), dumpSize)) {
        ImGui::TextDisabled(u8"内存不可读");
        return;
    }

    std::array<unsigned char, kPlayerControllerDumpBytes> bytes{};
    std::memcpy(bytes.data(), reinterpret_cast<void*>(base), dumpSize);

    ImGui::BeginChild("pc_hexdump", ImVec2(0.0f, 240.0f), true);
    for (size_t i = 0; i < bytes.size(); i += 16) {
        char line[256] = {};
        int written = std::snprintf(line, sizeof(line), "+%04llX: ", static_cast<unsigned long long>(i));
        if (written < 0) {
            continue;
        }

        for (size_t j = 0; j < 16 && (i + j) < bytes.size(); ++j) {
            if (written >= static_cast<int>(sizeof(line))) {
                break;
            }
            written += std::snprintf(line + written, sizeof(line) - written, "%02X ", bytes[i + j]);
        }

        ImGui::TextUnformatted(line);
    }
    ImGui::EndChild();
}

static void DrawKnownPlayerControllerFields(PlayerController* pc) {
    DrawPointerField(pc, 0x70, u8"主角色实体(mainCharacter)");
    DrawPointerField(pc, 0x78, u8"命令控制器(commandController)");

    DrawFloatField(pc, 0x80, u8"最大冲刺次数(maxDashCount)");
    DrawFloatField(pc, 0x84, u8"当前冲刺次数(currentDashCount)");
    DrawFloatField(pc, 0x88, u8"界面最大冲刺(lastShownMaxDashCount)");
    DrawBoolField(pc, 0x8C, u8"冲刺动画显示(dashCountChangeAnimShowing)");
    DrawBoolField(pc, 0x8D, u8"冲刺透支(isInDashOverdraft)");
    DrawFloatField(pc, 0x90, u8"透支恢复时间(dashOverdraftRecoverTime)");
    DrawFloatField(pc, 0x94, u8"主角SP空闲时间(lastMainCharacterSpIdleTime)");
    DrawFloatField(pc, 0x98, u8"队友SP空闲时间(lastSquadMemberSpIdleTime)");
    DrawFloatField(pc, 0x9C, u8"距上次冲刺时间(timeFromLastDash)");

    DrawBoolField(pc, 0xA0, u8"上次是否首次冲刺(isLastFirstDash)");
    DrawBoolField(pc, 0xA1, u8"技能中冲刺(dashInSkill)");
    DrawBoolField(pc, 0xA2, u8"可刷新偏移(canRefreshOffset)");
    DrawBoolField(pc, 0xA3, u8"冲刺中(inSprint)");
    DrawVec2Field(pc, 0xA4, u8"上次冲刺输入(lastSprintInput)");
    DrawFloatField(pc, 0xAC, u8"无输入冲刺时间(sprintNoInputTime)");

    DrawBoolField(pc, 0xB0, u8"行走状态(isWalk)");
    DrawBoolField(pc, 0xB1, u8"锁定移动坐标(fixMoveCoordinate)");
    DrawVec2Field(pc, 0xB4, u8"上次限制输入(lastLimitInput)");
    DrawVec3Field(pc, 0xBC, u8"固定前向(fixedMoveForward)");
    DrawVec3Field(pc, 0xC8, u8"固定右向(fixedMoveRight)");

    DrawVec3Field(pc, 0xE0, u8"帧运动(frameMotion)");
    DrawBoolField(pc, 0xEC, u8"安全区内(isInSaveZone)");
    DrawBoolField(pc, 0xED, u8"禁用位移推挤(disableMovementPush)");
    DrawPointerField(pc, 0xF0, u8"输入屏蔽集合(blockPlayerInputHashSet)");

    DrawUInt32Field(pc, 0xF8, u8"当前句柄ID(currentHandleId)");
    DrawInt32Field(pc, 0xFC, u8"动作启用掩码(playerActionEnableMask)");
    DrawUInt32Field(pc, 0x100, u8"动作启用句柄(playerActionEnableHandle)");
    DrawBoolField(pc, 0x104, u8"限制输入X(limitInputX)");
    DrawVec2Field(pc, 0x108, u8"输入X范围(limitInputRangeX)");
    DrawBoolField(pc, 0x110, u8"限制输入Y(limitInputY)");
    DrawVec2Field(pc, 0x114, u8"输入Y范围(limitInputRangeY)");
    DrawUInt32Field(pc, 0x11C, u8"当前限制轴句柄(currentLimitInputAxisHandle)");

    DrawBoolField(pc, 0x120, u8"普攻施放中(castingNormalAttack)");
    DrawBoolField(pc, 0x121, u8"终结技施放中(inUltimateCasting)");
    DrawPointerField(pc, 0x128, u8"普攻事件(onCastingNormalAttack)");
    DrawFloatField(pc, 0x130, u8"可持续攻击计时(canKeepAttackingTimer)");
    DrawFloatField(pc, 0x134, u8"持续攻击计时(keepAttackingTimer)");
    DrawFloatField(pc, 0x138, u8"触屏可持续攻击计时(touchCanKeepAttackTimer)");
    DrawFloatField(pc, 0x13C, u8"触屏持续攻击计时(touchKeepAttackTimer)");
    DrawVec2Field(pc, 0x140, u8"缓存原始输入(cachedRawInput)");
    DrawPointerField(pc, 0x148, u8"冲刺失败事件(onDashFailed)");

    DrawBoolField(pc, 0x150, u8"上次主角是否冲刺(lastIsMainCharacterSprinting)");
    DrawInt32Field(pc, 0x154, u8"摇杆状态(joystickState)");
    DrawFloatField(pc, 0x158, u8"摇杆保护计时(joystickProtectionTimer)");
    DrawVec3Field(pc, 0x15C, u8"摇杆保护位移(joystickProtectionMotion)");
    DrawFloatField(pc, 0x168, u8"特效上次命中时间(effectLastHitTime)");
    DrawVec3Field(pc, 0x16C, u8"特效上次命中位置(effectLastHitPos)");
}
static void DrawCustomWatchValueEditor(CustomWatchItem& item) {
    switch (item.valueType) {
    case CustomValueType::Bool:
        ImGui::Checkbox(u8"\u5199\u5165\u503c", &item.boolValue);
        break;
    case CustomValueType::Int32:
        ImGui::InputScalar(u8"\u5199\u5165\u503c", ImGuiDataType_S32, &item.i32Value);
        break;
    case CustomValueType::UInt32:
        ImGui::InputScalar(u8"\u5199\u5165\u503c", ImGuiDataType_U32, &item.u32Value);
        break;
    case CustomValueType::Int64:
        ImGui::InputScalar(u8"\u5199\u5165\u503c", ImGuiDataType_S64, &item.i64Value);
        break;
    case CustomValueType::UInt64:
        ImGui::InputScalar(u8"\u5199\u5165\u503c", ImGuiDataType_U64, &item.u64Value);
        break;
    case CustomValueType::Float:
        ImGui::InputFloat(u8"\u5199\u5165\u503c", &item.f32Value, 1.0f, 10.0f, "%.6f");
        break;
    case CustomValueType::Double:
        ImGui::InputDouble(u8"\u5199\u5165\u503c", &item.f64Value, 1.0, 10.0, "%.6f");
        break;
    case CustomValueType::Pointer: {
        uint64_t value = static_cast<uint64_t>(item.ptrValue);
        if (ImGui::InputScalar(u8"\u5199\u5165\u503c", ImGuiDataType_U64, &value, nullptr, nullptr, "0x%016llX", ImGuiInputTextFlags_CharsHexadecimal)) {
            item.ptrValue = static_cast<uintptr_t>(value);
        }
        break;
    }
    case CustomValueType::Byte: {
        int value = static_cast<int>(item.u8Value);
        if (ImGui::InputInt(u8"\u5199\u5165\u503c", &value, 1, 16)) {
            value = (std::max)(0, (std::min)(255, value));
            item.u8Value = static_cast<uint8_t>(value);
        }
        break;
    }
    }
}

static void DrawCustomWatchPanel(
    PlayerController* pc,
    Entity* entity,
    AbilitySystem* ability,
    uintptr_t gameInstanceAddress) {
    if (!ImGui::CollapsingHeader(u8"\u81ea\u5b9a\u4e49\u5730\u5740\u76d1\u89c6 / \u4fee\u6539", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    ImGui::TextDisabled(u8"\u7528\u4e8e\u8ffd\u8e2a\u7ed3\u6784\u4f53\u91cc\u672a\u5217\u51fa\u7684\u5b57\u6bb5\uff0c\u5e76\u652f\u6301\u5b9e\u65f6\u5199\u5165/\u9501\u5b9a\u3002");
    ImGui::TextDisabled(
        u8"\u57fa\u5740: PC=%p  \u5b9e\u4f53=%p  \u80fd\u529b=%p  \u6e38\u620f\u5b9e\u4f8b=0x%llX",
        pc,
        entity,
        ability,
        static_cast<unsigned long long>(gameInstanceAddress));

    int lockCount = 0;
    for (const CustomWatchItem& item : g_customWatchItems) {
        if (item.lockWrite) {
            ++lockCount;
        }
    }

    if (lockCount > 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), u8"\u5df2\u542f\u7528\u9501\u5b9a\u5199\u5165: %d", lockCount);
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.75f, 0.9f, 1.0f, 1.0f), u8"\u65b0\u589e\u76d1\u89c6\u9879");

    ImGui::PushItemWidth(220.0f);
    ImGui::InputText(u8"\u540d\u79f0", g_customWatchDraft.label, sizeof(g_customWatchDraft.label));
    ImGui::InputText(u8"\u504f\u79fb/\u5730\u5740(HEX)", g_customWatchDraft.offsetInput, sizeof(g_customWatchDraft.offsetInput));
    ImGui::PopItemWidth();
    ImGui::Combo(u8"\u57fa\u5740", &g_customWatchDraft.baseSourceIndex, kCustomBaseSourceItems, IM_ARRAYSIZE(kCustomBaseSourceItems));
    ImGui::Combo(u8"\u7c7b\u578b", &g_customWatchDraft.valueTypeIndex, kCustomValueTypeItems, IM_ARRAYSIZE(kCustomValueTypeItems));

    if (ImGui::Button(u8"\u6dfb\u52a0\u9879\u76ee")) {
        uintptr_t parsed = 0;
        if (!TryParseHex(g_customWatchDraft.offsetInput, parsed)) {
            SetCustomWatchStatus(u8"\u6dfb\u52a0\u5931\u8d25: HEX \u5730\u5740\u683c\u5f0f\u9519\u8bef", true);
        } else {
            CustomWatchItem item;
            item.id = g_customWatchNextId++;
            item.offsetOrAddress = parsed;
            item.parsedOk = true;
            item.baseSource = ToCustomBaseSource(g_customWatchDraft.baseSourceIndex);
            item.valueType = ToCustomValueType(g_customWatchDraft.valueTypeIndex);
            std::snprintf(item.offsetInput, sizeof(item.offsetInput), "0x%llX", static_cast<unsigned long long>(parsed));

            if (IsBlankText(g_customWatchDraft.label)) {
                std::snprintf(item.label, sizeof(item.label), u8"\u9879\u76ee%d", item.id);
            } else {
                std::snprintf(item.label, sizeof(item.label), "%s", g_customWatchDraft.label);
            }

            g_customWatchItems.push_back(item);
            SetCustomWatchStatus(u8"\u5df2\u6dfb\u52a0\u76d1\u89c6\u9879", false);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(u8"\u6e05\u7a7a\u5168\u90e8")) {
        g_customWatchItems.clear();
        SetCustomWatchStatus(u8"\u5df2\u6e05\u7a7a\u5168\u90e8\u76d1\u89c6\u9879", false);
    }

    if (g_customWatchStatusText[0] != '\0') {
        const ImVec4 color = g_customWatchStatusError ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f) : ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
        ImGui::TextColored(color, u8"\u76d1\u89c6\u72b6\u6001: %s", g_customWatchStatusText);
    }

    if (g_customWatchItems.empty()) {
        ImGui::TextDisabled(u8"\u5f53\u524d\u6ca1\u6709\u81ea\u5b9a\u4e49\u76d1\u89c6\u9879");
        return;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.75f, 0.9f, 1.0f, 1.0f), u8"\u76d1\u89c6\u9879\u5217\u8868");

    int removeIndex = -1;
    for (size_t i = 0; i < g_customWatchItems.size(); ++i) {
        CustomWatchItem& item = g_customWatchItems[i];
        ImGui::PushID(item.id);

        const char* displayLabel = item.label[0] != '\0' ? item.label : u8"<\u672a\u547d\u540d>";
        if (ImGui::TreeNodeEx(
                "custom_watch_item",
                ImGuiTreeNodeFlags_DefaultOpen,
                "%s [%s]",
                displayLabel,
                kCustomValueTypeItems[static_cast<int>(item.valueType)])) {
            ImGui::PushItemWidth(220.0f);
            ImGui::InputText(u8"\u540d\u79f0", item.label, sizeof(item.label));
            const bool pressedEnter = ImGui::InputText(
                u8"\u504f\u79fb/\u5730\u5740(HEX)",
                item.offsetInput,
                sizeof(item.offsetInput),
                ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(u8"\u66f4\u65b0\u5730\u5740") || pressedEnter) {
                if (UpdateCustomWatchOffset(item)) {
                    SetCustomWatchStatus(u8"\u5730\u5740\u5df2\u66f4\u65b0", false);
                } else {
                    SetCustomWatchStatus(u8"\u5730\u5740\u66f4\u65b0\u5931\u8d25: HEX \u683c\u5f0f\u9519\u8bef", true);
                }
            }

            int baseSourceIndex = static_cast<int>(item.baseSource);
            if (ImGui::Combo(u8"\u57fa\u5740", &baseSourceIndex, kCustomBaseSourceItems, IM_ARRAYSIZE(kCustomBaseSourceItems))) {
                item.baseSource = ToCustomBaseSource(baseSourceIndex);
            }

            int valueTypeIndex = static_cast<int>(item.valueType);
            if (ImGui::Combo(u8"\u7c7b\u578b", &valueTypeIndex, kCustomValueTypeItems, IM_ARRAYSIZE(kCustomValueTypeItems))) {
                item.valueType = ToCustomValueType(valueTypeIndex);
            }

            if (!item.parsedOk) {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), u8"HEX \u5730\u5740\u683c\u5f0f\u9519\u8bef");
            }

            const uintptr_t resolvedAddress = ResolveCustomWatchAddress(item, pc, entity, ability, gameInstanceAddress);
            ImGui::Text(u8"\u89e3\u6790\u540e\u7684\u5730\u5740: 0x%llX", static_cast<unsigned long long>(resolvedAddress));

            if (resolvedAddress == 0) {
                ImGui::TextDisabled(u8"\u5f53\u524d\u9009\u5b9a\u57fa\u5740\u4e0d\u53ef\u7528");
            } else {
                char currentValueText[128] = {};
                if (ReadCustomWatchValueText(item, resolvedAddress, currentValueText, sizeof(currentValueText))) {
                    ImGui::Text(u8"\u5f53\u524d\u503c: %s", currentValueText);
                } else {
                    ImGui::TextDisabled(u8"\u5f53\u524d\u503c: <\u8bfb\u53d6\u5931\u8d25>");
                }

                DrawCustomWatchValueEditor(item);

                if (ImGui::Button(u8"\u8bfb\u53d6\u5230\u5199\u5165\u503c")) {
                    const bool ok = ReadCustomWatchValue(item, resolvedAddress);
                    SetCustomWatchStatus(ok ? u8"\u5df2\u4ece\u5185\u5b58\u540c\u6b65\u5230\u5199\u5165\u503c" : u8"\u540c\u6b65\u8bfb\u53d6\u5931\u8d25", !ok);
                }
                ImGui::SameLine();
                if (ImGui::Button(u8"\u5199\u5165\u4e00\u6b21")) {
                    const bool ok = WriteCustomWatchValue(item, resolvedAddress);
                    SetCustomWatchStatus(ok ? u8"\u5199\u5165\u6210\u529f" : u8"\u5199\u5165\u5931\u8d25", !ok);
                }
                ImGui::SameLine();
                ImGui::Checkbox(u8"\u9501\u5b9a\u5199\u5165", &item.lockWrite);
            }

            if (ImGui::Button(u8"\u5220\u9664\u9879\u76ee")) {
                removeIndex = static_cast<int>(i);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    if (removeIndex >= 0) {
        g_customWatchItems.erase(g_customWatchItems.begin() + removeIndex);
        SetCustomWatchStatus(u8"\u76d1\u89c6\u9879\u5df2\u5220\u9664", false);
    }
}

} // namespace

void DrawPlayerDebugWindow() {

    static bool showWindow = true;
    static bool initializedDefaultOffset = false;
    static bool unloadRequestedFromUi = false;

    const bool ctrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    if (ctrlDown && shiftDown && (GetAsyncKeyState(VK_F10) & 1)) {
        showWindow = !showWindow;
    }

    g_debugWindowVisible.store(showWindow, std::memory_order_release);

    if (!showWindow) {
        return;
    }

    InitGameBase();
    EnsureFastUltimateThread();

    if (!initializedDefaultOffset) {
        uintptr_t parsed = 0;
        if (TryParseHex(g_offsetInput, parsed)) {
            g_typeInfoOffset = parsed;
            std::snprintf(g_statusText, sizeof(g_statusText), u8"已加载默认偏移");
            g_statusError = false;
        }
        initializedDefaultOffset = true;
    }

    ImGui::SetNextWindowSize(ImVec2(680, 920), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(u8"终末地 角色数据面板", &showWindow)) {
        g_debugWindowVisible.store(showWindow, std::memory_order_release);
        ImGui::End();
        return;
    }

    g_debugWindowVisible.store(showWindow, std::memory_order_release);

    if (ImGui::Button(u8"安全退出并卸载")) {
        Hook::RequestUnload();
        unloadRequestedFromUi = true;
    }
    ImGui::SameLine();
    ImGui::TextDisabled(u8"快捷键: Ctrl + Shift + F10");

    if (unloadRequestedFromUi) {
        if (Hook::IsUnloadCompleted()) {
            ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), u8"卸载完成，可关闭注入器。\n");
            showWindow = false;
            g_debugWindowVisible.store(false, std::memory_order_release);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), u8"正在安全卸载，请稍候...");
        }
        ImGui::End();
        return;
    }

    const int lastTeleportPathTop = g_lastTeleportPath.load(std::memory_order_acquire);
    const char* teleportPathText = u8"未触发";
    if (lastTeleportPathTop == 0) {
        teleportPathText = u8"本地写坐标";
    } else if (lastTeleportPathTop == 1) {
        teleportPathText = u8"GameplayNetwork::C2STeleport";
    } else if (lastTeleportPathTop == 2) {
        teleportPathText = u8"GameAction::TeleportToPosition";
    } else if (lastTeleportPathTop == 3) {
        teleportPathText = u8"TeleportProcessor::C2STeleport";
    }

    ImGui::Separator();
    ImGui::Text(u8"最近传送路径: %s", teleportPathText);
    if (g_treasureTool.statusText[0] != '\0') {
        const ImVec4 statusColorTop = g_treasureTool.statusError
                                          ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
                                          : ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
        ImGui::TextColored(statusColorTop, u8"传送状态: %s", g_treasureTool.statusText);
    } else {
        ImGui::TextDisabled(u8"传送状态: <暂无>");
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader(u8"高级定位设置")) {
        ImGui::PushItemWidth(240.0f);
        const bool pressedEnter = ImGui::InputText(u8"TypeInfo偏移(HEX)", g_offsetInput, sizeof(g_offsetInput), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        bool clickedApply = ImGui::Button(u8"应用");
        ImGui::SameLine();
        if (ImGui::Button(u8"使用 0xFC2FA18")) {
            std::snprintf(g_offsetInput, sizeof(g_offsetInput), "0xFC2FA18");
            clickedApply = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"清空")) {
            g_typeInfoOffset = 0;
            std::snprintf(g_offsetInput, sizeof(g_offsetInput), "0x0");
            std::snprintf(g_statusText, sizeof(g_statusText), u8"已清空");
            g_statusError = false;
        }

        if (pressedEnter || clickedApply) {
            uintptr_t parsed = 0;
            if (TryParseHex(g_offsetInput, parsed)) {
                g_typeInfoOffset = parsed;
                std::snprintf(g_statusText, sizeof(g_statusText), u8"偏移已应用");
                g_statusError = false;
            } else {
                std::snprintf(g_statusText, sizeof(g_statusText), u8"十六进制格式错误");
                g_statusError = true;
            }
        }

        if (g_statusText[0] != '\0') {
            const ImVec4 color = g_statusError ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f) : ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
            ImGui::TextColored(color, u8"状态: %s", g_statusText);
        }

        ImGui::Text(u8"游戏基址: 0x%llX", static_cast<unsigned long long>(g_gameBaseAddress));
    }

    ImGui::Separator();

    const ResolveResult resolved = ResolvePlayerController();
    if (!resolved.ok) {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), u8"解析失败: %s", resolved.error);
        ImGui::End();
        return;
    }

    PlayerController* pc = resolved.playerController;
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), u8"解析成功，角色指针: %p", pc);

    Entity* entity = pc->mainCharacter;
    Entity* entityFromGameInstance = nullptr;
    if (TryCallGameInstanceGetMainChar(resolved.gameInstanceAddress, entityFromGameInstance)) {
        entity = entityFromGameInstance;
    }

    if (IsValidPtr(entity)) {
        g_mainCharacterEntityAddress.store(reinterpret_cast<uintptr_t>(entity), std::memory_order_release);
    } else {
        g_mainCharacterEntityAddress.store(0, std::memory_order_release);
    }

    AbilitySystem* abilityForCustomPanel = nullptr;
    if (IsValidPtr(entity)) {
        abilityForCustomPanel = entity->abilityCom;

        AbilitySystem* abilityFromGetter = nullptr;
        if (TryCallEntityGetAbilityCom(entity, abilityFromGetter)) {
            abilityForCustomPanel = abilityFromGetter;
        }
    }

    UpdateFastUltimateTarget(abilityForCustomPanel, pc, resolved.gameInstanceAddress);
    ApplyLockedCustomWatchItems(pc, entity, abilityForCustomPanel, resolved.gameInstanceAddress);
    if (IsValidPtr(entity)) {
        ApplyTeleportHoldIfNeeded(entity);
    }

    if (!IsValidPtr(entity) || !IsValidPtr(abilityForCustomPanel)) {
        ImGui::TextDisabled(u8"当前无法获取角色能力组件，请进入战斗后再试。");
        DrawCustomWatchPanel(pc, entity, abilityForCustomPanel, resolved.gameInstanceAddress);
        ImGui::End();
        return;
    }

    AbilitySystem* ability = abilityForCustomPanel;

    double hpMethod = 0.0;
    double serverHpMethod = 0.0;
    float spMethod = 0.0f;
    const bool hpByMethod = TryCallAbilityGetHp(ability, hpMethod);
    const bool serverHpByMethod = TryCallAbilityGetServerHp(ability, serverHpMethod);
    const bool spByMethod = TryCallAbilityGetUltimateSp(ability, spMethod);

    float skillCooldownMethod = 1.0f;
    const bool skillCooldownByMethod = TryCallAbilityGetSkillCooldownScalar(ability, skillCooldownMethod);
    double skillCooldownRaw = static_cast<double>(skillCooldownMethod);
    if (!skillCooldownByMethod && !TryReadAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownScalar, skillCooldownRaw)) {
        skillCooldownRaw = 1.0;
    }
    const float skillCooldownFinal = static_cast<float>(skillCooldownRaw);

    double skillCooldownAdditionRaw = 0.0;
    if (!TryReadAttributeDoubleByIndex(ability, kAttrIndexSkillCooldownAddition, skillCooldownAdditionRaw)) {
        skillCooldownAdditionRaw = 0.0;
    }
    const float skillCooldownAdditionFinal = static_cast<float>(skillCooldownAdditionRaw);

    double atbCostAdditionRaw = 0.0;
    if (!TryReadAttributeDoubleByIndex(ability, kAttrIndexAtbCostAddition, atbCostAdditionRaw)) {
        atbCostAdditionRaw = 0.0;
    }
    const float atbCostAdditionFinal = static_cast<float>(atbCostAdditionRaw);

    double ultimateSpGainRaw = 1.0;
    if (!TryReadAttributeDoubleByIndex(ability, kAttrIndexUltimateSpGainScalar, ultimateSpGainRaw)) {
        ultimateSpGainRaw = 1.0;
    }
    const float ultimateSpGainFinal = static_cast<float>(ultimateSpGainRaw);

    double criticalRateRaw = 1.0;
    if (!TryReadAttributeDoubleByIndex(ability, kAttrIndexCriticalRate, criticalRateRaw)) {
        criticalRateRaw = 1.0;
    }
    const float criticalRateFinal = static_cast<float>(criticalRateRaw);

    std::array<double, 11> damageIncreaseValues = {0.0};
    const std::array<int, 11> damageIncreaseIndices = {
        kAttrIndexNormalAttackDamageIncrease,
        kAttrIndexNormalSkillDamageIncrease,
        kAttrIndexComboSkillDamageIncrease,
        kAttrIndexUltimateSkillDamageIncrease,
        kAttrIndexPhysicalDamageIncrease,
        kAttrIndexFireDamageIncrease,
        kAttrIndexPulseDamageIncrease,
        kAttrIndexCrystDamageIncrease,
        kAttrIndexNaturalDamageIncrease,
        kAttrIndexEtherDamageIncrease,
        kAttrIndexCriticalDamageIncrease};

    double maxDamageIncrease = 0.0;
    for (size_t i = 0; i < damageIncreaseIndices.size(); ++i) {
        double value = 0.0;
        if (TryReadAttributeDoubleByIndex(ability, damageIncreaseIndices[i], value)) {
            damageIncreaseValues[i] = value;
            if (value > maxDamageIncrease) {
                maxDamageIncrease = value;
            }
        }
    }
    const float damageMultiplierFinal = static_cast<float>((std::max)(1.0, 1.0 + maxDamageIncrease));

    double hpClientRaw = 0.0;
    double hpServerRaw = 0.0;
    TryReadAbilityHpPair(ability, hpClientRaw, hpServerRaw);

    const double hpFinal = hpByMethod ? hpMethod : hpClientRaw;
    const double serverHpFinal = serverHpByMethod ? serverHpMethod : hpServerRaw;
    const float ultimateSpFinal = spByMethod ? spMethod : ability->m_ultimateSp;
    float atbFinal = ability->m_ap;
    ReadValue<float>(reinterpret_cast<uintptr_t>(ability) + 0x150, atbFinal);

    if (!g_statEditor.initializedFromGame) {
        g_statEditor.hpClient = hpFinal;
        g_statEditor.hpServer = serverHpFinal;
        g_statEditor.skillCooldownScalar = skillCooldownFinal;
        g_statEditor.skillCooldownAddition = skillCooldownAdditionFinal;
        g_statEditor.atbCostAddition = atbCostAdditionFinal;
        g_statEditor.atbValue = atbFinal;
        g_statEditor.ultimateSpGainScalar = ultimateSpGainFinal;
        g_statEditor.ultimateSp = ultimateSpFinal;
        g_statEditor.currentDash = pc->currentDashCount;
        g_statEditor.maxDash = pc->maxDashCount;
        g_statEditor.criticalRate = criticalRateFinal;
        g_statEditor.damageMultiplier = damageMultiplierFinal;
        g_statEditor.initializedFromGame = true;
    }

    if (AnyLockEnabled()) {
        ApplyEditedValues(pc, ability, resolved.gameInstanceAddress, true);
    }

    ImGui::Text(u8"生命值(HP): %.2f %s", hpFinal, hpByMethod ? u8"(函数)" : u8"(内存)");
    ImGui::Text(u8"服务器HP: %.2f %s", serverHpFinal, serverHpByMethod ? u8"(函数)" : u8"(内存)");
    ImGui::Text(u8"技能CD倍率[0x10]: %.3f %s", skillCooldownFinal, skillCooldownByMethod ? u8"(函数)" : u8"(属性)");
    ImGui::Text(u8"技能CD附加[0x2F]: %.3f", skillCooldownAdditionFinal);
    ImGui::Text(u8"蓝耗附加[0x2E]: %.3f", atbCostAdditionFinal);
    ImGui::Text(u8"\u84dd\u6761\u503c(AP/ATB)[0x150]: %.3f", atbFinal);
    ImGui::Text(u8"终结技SP回复倍率[0x2D]: %.3f", ultimateSpGainFinal);
    ImGui::Text(u8"终结技SP: %.3f %s", ultimateSpFinal, spByMethod ? u8"(函数)" : u8"(内存)");
    ImGui::Text(u8"\u66b4\u51fb\u7387[0x09]: %.3f (%.1f%%)", criticalRateFinal, criticalRateFinal * 100.0f);
    ImGui::Text(u8"\u4f24\u5bb3\u500d\u7387(\u7efc\u5408): x%.3f", damageMultiplierFinal);

    if (ImGui::TreeNode(u8"数值修改与锁定")) {
        if (ImGui::Button(u8"读取当前值")) {
            g_statEditor.hpClient = hpFinal;
            g_statEditor.hpServer = serverHpFinal;
            g_statEditor.skillCooldownScalar = skillCooldownFinal;
            g_statEditor.skillCooldownAddition = skillCooldownAdditionFinal;
            g_statEditor.atbCostAddition = atbCostAdditionFinal;
            g_statEditor.atbValue = atbFinal;
            g_statEditor.ultimateSpGainScalar = ultimateSpGainFinal;
            g_statEditor.ultimateSp = ultimateSpFinal;
            g_statEditor.currentDash = pc->currentDashCount;
            g_statEditor.maxDash = pc->maxDashCount;
            g_statEditor.criticalRate = criticalRateFinal;
            g_statEditor.damageMultiplier = damageMultiplierFinal;
            std::snprintf(g_editStatusText, sizeof(g_editStatusText), u8"已读取当前数值");
            g_editStatusError = false;
            if (g_statEditor.lockInfiniteBlueNoCd) {
                SetInfiniteBlueNoCdPreset(true);
            }
            if (g_statEditor.lockAllSquadUltimateNoCd) {
                SetAllSquadUltimateNoCdPreset(true);
            }
            if (g_statEditor.lockInfiniteStamina) {
                SetInfiniteStaminaPreset(true);
            }
            if (g_statEditor.lockInfiniteBlink) {
                SetInfiniteBlinkPreset(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"应用一次")) {
            const bool ok = ApplyEditedValues(pc, ability, resolved.gameInstanceAddress, false);
            std::snprintf(g_editStatusText, sizeof(g_editStatusText), ok ? u8"应用成功" : u8"应用失败");
            g_editStatusError = !ok;
        }

        if (ImGui::Checkbox(u8"无限蓝条 + 无技能CD(推荐)", &g_statEditor.lockInfiniteBlueNoCd)) {
            SetInfiniteBlueNoCdPreset(g_statEditor.lockInfiniteBlueNoCd);
        }
        ImGui::TextDisabled(u8"预设会持续写入 0x10/0x2F/0x2E/0x2D 及 终结技SP 值。");

        if (ImGui::Checkbox(u8"\u5f53\u524d4\u4eba\u7ec8\u7ed3\u6280\u65e0CD", &g_statEditor.lockAllSquadUltimateNoCd)) {
            SetAllSquadUltimateNoCdPreset(g_statEditor.lockAllSquadUltimateNoCd);
        }

        if (ImGui::Checkbox(u8"\u65e0\u9650\u8010\u529b", &g_statEditor.lockInfiniteStamina)) {
            SetInfiniteStaminaPreset(g_statEditor.lockInfiniteStamina);
        }

        if (ImGui::Checkbox(u8"\u65e0\u9650\u95ea\u73b0", &g_statEditor.lockInfiniteBlink)) {
            SetInfiniteBlinkPreset(g_statEditor.lockInfiniteBlink);
        }

        ImGui::Checkbox(u8"强制无CD校验绕过(全技能)", &g_statEditor.lockNoCooldownBypass);

        ImGui::InputDouble(u8"生命值(客户端)", &g_statEditor.hpClient, 10.0, 100.0, "%.2f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_hp_client", &g_statEditor.lockHpClient);

        ImGui::InputDouble(u8"生命值(服务器)", &g_statEditor.hpServer, 10.0, 100.0, "%.2f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_hp_server", &g_statEditor.lockHpServer);

        ImGui::InputFloat(u8"技能CD倍率(0x10)", &g_statEditor.skillCooldownScalar, 0.01f, 0.1f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_skill_cd", &g_statEditor.lockSkillCooldown);

        ImGui::InputFloat(u8"技能CD附加(0x2F)", &g_statEditor.skillCooldownAddition, 0.5f, 5.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_skill_cd_add", &g_statEditor.lockSkillCooldownAddition);

        ImGui::InputFloat(u8"蓝耗附加(0x2E)", &g_statEditor.atbCostAddition, 0.5f, 5.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_atb_add", &g_statEditor.lockAtbCostAddition);

        ImGui::InputFloat(u8"\u84dd\u6761\u503c(AP/ATB,0x150)", &g_statEditor.atbValue, 1.0f, 10.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"\u9501\u5b9a##lock_atb_value", &g_statEditor.lockAtbValue);

        ImGui::InputFloat(u8"终结技SP回复倍率(0x2D)", &g_statEditor.ultimateSpGainScalar, 0.1f, 1.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_sp_gain", &g_statEditor.lockUltimateSpGainScalar);

        ImGui::InputFloat(u8"终结技SP", &g_statEditor.ultimateSp, 1.0f, 10.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_usp", &g_statEditor.lockUltimateSp);

        ImGui::InputFloat(u8"\u66b4\u51fb\u7387(0x09, 1.0=100%)", &g_statEditor.criticalRate, 0.01f, 0.05f, "%.3f");
        g_statEditor.criticalRate = (std::max)(0.0f, g_statEditor.criticalRate);
        ImGui::SameLine();
        ImGui::Checkbox(u8"\u9501\u5b9a##lock_critical_rate", &g_statEditor.lockCriticalRate);

        ImGui::InputFloat(u8"\u4f24\u5bb3\u500d\u7387(1.0=\u539f\u59cb)", &g_statEditor.damageMultiplier, 0.1f, 0.5f, "%.3f");
        g_statEditor.damageMultiplier = (std::max)(1.0f, g_statEditor.damageMultiplier);
        ImGui::SameLine();
        ImGui::Checkbox(u8"\u9501\u5b9a##lock_damage_multiplier", &g_statEditor.lockDamageMultiplier);

        ImGui::InputFloat(u8"当前冲刺次数", &g_statEditor.currentDash, 1.0f, 5.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_cur_dash", &g_statEditor.lockCurrentDash);

        ImGui::InputFloat(u8"最大冲刺次数", &g_statEditor.maxDash, 1.0f, 5.0f, "%.3f");
        ImGui::SameLine();
        ImGui::Checkbox(u8"锁定##lock_max_dash", &g_statEditor.lockMaxDash);

        if (g_editStatusText[0] != '\0') {
            const ImVec4 statusColor = g_editStatusError ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f) : ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
            ImGui::TextColored(statusColor, u8"修改状态: %s", g_editStatusText);
        }

        if (AnyLockEnabled()) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), u8"锁定已启用：每帧持续写入选中项");
        } else {
            ImGui::TextDisabled(u8"当前未启用锁定");
        }

        ImGui::TreePop();
    }

    if (ImGui::TreeNode(u8"\u4f20\u9001\u5de5\u5177")) {
        Vec3 playerPos{};
        const bool hasPlayerPos = TryReadEntityPosition(entity, playerPos);
        if (hasPlayerPos) {
            ImGui::Text(u8"\u5f53\u524d\u4f4d\u7f6e: X %.2f  Y %.2f  Z %.2f", playerPos.x, playerPos.y, playerPos.z);
            if (!g_treasureTool.customTeleportInitialized) {
                g_treasureTool.customTeleport[0] = playerPos.x;
                g_treasureTool.customTeleport[1] = playerPos.y;
                g_treasureTool.customTeleport[2] = playerPos.z;
                g_treasureTool.customTeleportInitialized = true;
            }
        } else {
            ImGui::TextDisabled(u8"\u5f53\u524d\u4f4d\u7f6e\u8bfb\u53d6\u5931\u8d25\uff0c\u8bf7\u5148\u8fdb\u5165\u53ef\u64cd\u4f5c\u573a\u666f");
        }

        ImGui::Checkbox(u8"\u4f20\u9001\u6297\u62c9\u56de\u4fdd\u6301", &g_treasureTool.enableTeleportHold);
        ImGui::SliderInt(u8"\u4fdd\u6301\u6beb\u79d2", &g_treasureTool.teleportHoldMs, 200, 15000);
        ImGui::SliderInt(u8"\u4fdd\u6301\u5199\u5165\u95f4\u9694(ms)", &g_treasureTool.teleportWriteIntervalMs, 15, 250);
        g_treasureTool.teleportHoldMs = (std::max)(200, g_treasureTool.teleportHoldMs);
        g_treasureTool.teleportWriteIntervalMs = (std::max)(15, (std::min)(250, g_treasureTool.teleportWriteIntervalMs));

        ImGui::Checkbox(u8"\u4f20\u9001\u540e\u81ea\u52a8\u5f00\u542f\u5750\u6807\u9501\u5b9a", &g_treasureTool.autoLockAfterTeleport);
        ImGui::SameLine();
        ImGui::Checkbox(u8"\u6301\u7eed\u5750\u6807\u9501\u5b9a(\u9632\u62c9\u56de)", &g_treasureTool.teleportLockEnabled);

        if (ImGui::Button(u8"\u9501\u5b9a\u76ee\u6807=\u5f53\u524d\u5750\u6807")) {
            if (hasPlayerPos) {
                g_treasureTool.teleportLockTarget = playerPos;
                g_treasureTool.teleportLockHasTarget = true;
                g_treasureTool.teleportLockEnabled = true;
                SetTreasureStatus(u8"\u5df2\u5c06\u5f53\u524d\u5750\u6807\u8bbe\u4e3a\u9501\u5b9a\u76ee\u6807", false);
            } else {
                SetTreasureStatus(u8"\u9501\u5b9a\u5931\u8d25\uff1a\u5f53\u524d\u5750\u6807\u4e0d\u53ef\u8bfb", true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"\u505c\u6b62\u5750\u6807\u9501\u5b9a")) {
            g_treasureTool.teleportLockEnabled = false;
            SetTreasureStatus(u8"\u5df2\u5173\u95ed\u5750\u6807\u9501\u5b9a", false);
        }

        if (g_treasureTool.teleportLockHasTarget) {
            ImGui::Text(u8"\u9501\u5b9a\u76ee\u6807: X %.2f  Y %.2f  Z %.2f",
                        g_treasureTool.teleportLockTarget.x,
                        g_treasureTool.teleportLockTarget.y,
                        g_treasureTool.teleportLockTarget.z);
        } else {
            ImGui::TextDisabled(u8"\u9501\u5b9a\u76ee\u6807: <\u672a\u8bbe\u7f6e>");
        }

        if (ImGui::Button(u8"\u5750\u6807\u586b\u5165\u5f53\u524d\u4f4d\u7f6e")) {
            if (hasPlayerPos) {
                g_treasureTool.customTeleport[0] = playerPos.x;
                g_treasureTool.customTeleport[1] = playerPos.y;
                g_treasureTool.customTeleport[2] = playerPos.z;
                g_treasureTool.customTeleportInitialized = true;
                SetTreasureStatus(u8"\u5df2\u540c\u6b65\u5f53\u524d\u4f4d\u7f6e\u5230\u4f20\u9001\u5750\u6807", false);
            } else {
                SetTreasureStatus(u8"\u540c\u6b65\u5931\u8d25\uff1a\u5f53\u524d\u4f4d\u7f6e\u4e0d\u53ef\u7528", true);
            }
        }

        ImGui::InputFloat(u8"\u4f20\u9001\u9ad8\u5ea6\u504f\u79fb", &g_treasureTool.teleportHeightOffset, 0.1f, 1.0f, "%.2f");
        ImGui::InputFloat3(u8"\u81ea\u5b9a\u4e49\u4f20\u9001\u5750\u6807(XYZ)", g_treasureTool.customTeleport, "%.3f");
        if (ImGui::Button(u8"\u4f20\u9001\u5230\u5750\u6807")) {
            Vec3 target = {
                g_treasureTool.customTeleport[0],
                g_treasureTool.customTeleport[1],
                g_treasureTool.customTeleport[2]};
            target.y += g_treasureTool.teleportHeightOffset;
            const bool ok = TeleportEntityTo(entity, target);
            const int usedPath = g_lastTeleportPath.load(std::memory_order_acquire);
            if (ok && usedPath != 1 && usedPath != 2 && usedPath != 3) {
                BeginTeleportHold(target);
            }
            if (ok && (usedPath == 1 || usedPath == 2 || usedPath == 3)) {
                SetTreasureStatus(u8"\u5df2\u53d1\u9001\u670d\u52a1\u7aef\u4f20\u9001\u8bf7\u6c42", false);
            } else if (ok && usedPath == 0) {
                SetTreasureStatus(u8"\u4ec5\u672c\u5730\u5199\u5750\u6807\uff08\u670d\u52a1\u7aef\u53ef\u80fd\u62c9\u56de\uff09", false);
            } else {
                SetTreasureStatus(ok ? u8"\u5df2\u4f20\u9001\u5230\u81ea\u5b9a\u4e49\u5750\u6807" : u8"\u4f20\u9001\u5931\u8d25\uff1a\u5750\u6807\u6216\u5b9e\u4f53\u65e0\u6548", !ok);
            }
        }

        if (g_treasureTool.statusText[0] != '\0') {
            const ImVec4 statusColor = g_treasureTool.statusError
                                           ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f)
                                           : ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
            ImGui::TextColored(statusColor, u8"\u4f20\u9001\u5de5\u5177\u72b6\u6001: %s", g_treasureTool.statusText);
        }

        ImGui::TreePop();
    }

    DrawCustomWatchPanel(pc, entity, abilityForCustomPanel, resolved.gameInstanceAddress);

    ImGui::End();
}



void DrawTreasureEspOverlay() {
    InitGameBase();
    EnsureTypeInfoOffsetInitialized();
    if (g_gameBaseAddress == 0) {
        return;
    }

    const ResolveResult resolved = ResolvePlayerController();
    if (!resolved.ok || !IsValidPtr(resolved.playerController)) {
        return;
    }

    Entity* entity = resolved.playerController->mainCharacter;
    Entity* entityFromGameInstance = nullptr;
    if (TryCallGameInstanceGetMainChar(resolved.gameInstanceAddress, entityFromGameInstance)) {
        entity = entityFromGameInstance;
    }
    if (!IsValidPtr(entity)) {
        return;
    }

    // Keep writing teleport target even when menu window is closed.
    uintptr_t rootAddress = 0;
    if (!TryCallEntityGetRootCom(entity, rootAddress) || !IsCanonicalUserPtr(rootAddress)) {
        ReadValue(reinterpret_cast<uintptr_t>(entity) + kEntityRootComponentOffset, rootAddress);
    }

    if (IsCanonicalUserPtr(rootAddress)) {
        g_mainCharacterRootComponentAddress.store(rootAddress, std::memory_order_release);
    } else {
        g_mainCharacterRootComponentAddress.store(0, std::memory_order_release);
    }

    ApplyTeleportHoldIfNeeded(entity);

}

uintptr_t GetMainCharacterRootComponentAddress() {
    return g_mainCharacterRootComponentAddress.load(std::memory_order_acquire);
}

bool GetTeleportForceState(bool* outActive, float* outX, float* outY, float* outZ) {
    if (!outActive || !outX || !outY || !outZ) {
        return false;
    }

    const bool active = g_teleportForceActive.load(std::memory_order_acquire) != 0;
    *outActive = active;
    *outX = g_teleportForceTargetX.load(std::memory_order_acquire);
    *outY = g_teleportForceTargetY.load(std::memory_order_acquire);
    *outZ = g_teleportForceTargetZ.load(std::memory_order_acquire);
    return true;
}

bool GetTeleportForceOffset(float* outX, float* outY, float* outZ) {
    if (!outX || !outY || !outZ) {
        return false;
    }

    *outX = g_teleportForceOffsetX.load(std::memory_order_acquire);
    *outY = g_teleportForceOffsetY.load(std::memory_order_acquire);
    *outZ = g_teleportForceOffsetZ.load(std::memory_order_acquire);
    return true;
}

bool IsDebugWindowVisible() {
    return g_debugWindowVisible.load(std::memory_order_acquire);
}


bool IsForceNoCooldownEnabled() {
    return g_statEditor.lockNoCooldownBypass || g_statEditor.lockInfiniteBlueNoCd || g_statEditor.lockAllSquadUltimateNoCd;
}

bool IsInfiniteDashEnabled() {
    return g_statEditor.lockInfiniteBlink;
}

bool IsInfiniteStaminaEnabled() {
    return g_statEditor.lockInfiniteStamina || g_statEditor.lockInfiniteBlink;
}

void ShutdownCheatRuntime() {
    StopFastUltimateThread();
}
