#pragma once
#include <cstdint>

// Basic types
using System_String = void*; // Placeholder for Il2CppString*
using System_Object = void*; // Placeholder
using System_Type = void*;

// Forward declarations
struct Entity;
struct PlayerCommandController;
struct AbilitySystem;
struct Attributes;
struct AttributeDataCache;
struct SyncedValue_Double; // Specialized for double

// Il2Cpp Array definition for Double[]
struct Il2CppArrayDouble {
    void* klass;
    void* monitor;
    void* bounds;
    uint32_t max_length;
    double vector[1]; // Variable length
};

// --- Helper for padding ---
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(size) char CONCAT(_pad_, __COUNTER__)[size];

// 1. PlayerController (dump.cs:375580)
struct PlayerController {
    PAD(0x70);
    Entity* mainCharacter;                  // 0x70
    PlayerCommandController* commandController; // 0x78
    float maxDashCount;                     // 0x80
    float currentDashCount;                 // 0x84
    PAD(0x5);                              // 0x88-0x8C
    bool isInDashOverdraft;                 // 0x8D
    PAD(0x2);                              // 0x8E-0x8F
    float dashOverdraftRecoverTime;         // 0x90
    PAD(0x1C);                             // 0x94-0xAF
    bool isWalk;                            // 0xB0
    PAD(0x2F);                             // 0xB1-0xDF
    float frameMotion_x;                    // 0xE0 (Vector3)
    float frameMotion_y;
    float frameMotion_z;
    PAD(0x34);                             // 0xEC-0x11F
    bool castingNormalAttack;               // 0x120
    bool inUltimateCasting;                 // 0x121
};

// 2. Entity (dump.cs:371681)
struct Entity {
    PAD(0x10);
    System_String name;                     // 0x10
    uint32_t instanceUid;                   // 0x18
    PAD(0x14);
    uint64_t serverId;                      // 0x30
    PAD(0xE8);                              // 0x120 - 0x38 = 0xE8
    AbilitySystem* abilityCom;              // 0x120
};

// 3. AbilitySystem (dump.cs:267324)
struct AbilitySystem {
    PAD(0x130);
    Attributes* m_attributes;               // 0x130
    SyncedValue_Double* m_hp;               // 0x138
    PAD(0x8);
    double m_cachedServerHp;                // 0x148
    float m_ap;                             // 0x150
    float m_ultimateSp;                     // 0x154
};

// Wrapper for SyncedValue<double>
// Assuming standard class layout: vtable + monitor + fields
struct SyncedValue_Double {
    PAD(0x10); // standard object header
    double value; // Most likely at 0x10
};

// 4. Attributes (dump.cs:285890)
struct Attributes {
    PAD(0x18);
    AttributeDataCache* m_finalCachedData;  // 0x18
};

// AttributeDataCache
struct AttributeDataCache {
    PAD(0x20);
    Il2CppArrayDouble* cachedData;          // 0x20 (Double[])
};
