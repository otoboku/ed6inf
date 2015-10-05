#ifndef _ed6_common_H_E0FE925A_BC78_484e_B212_2DFA2B7787C9
#define _ed6_common_H_E0FE925A_BC78_484e_B212_2DFA2B7787C9

#include "MyLibrary.h"
//#include "pragma_once.h"
//#include <Windows.h>
//#include "my_headers.h"


typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char UCHAR, ubyte;

#define FSkip(n) DUMMY_STRUCT(n)
#define RESISTANCE UINT

#pragma pack(1)
#pragma warning (disable: 4201)

typedef struct  //DAT DIR文件索引
{
    union
    {
        int file;
        struct
        {
            USHORT  Index;
            USHORT  DatNo;
        };
    };
} FileIndex;

typedef struct  //每组12+24字节
{
    union
    {
        DUMMY_STRUCT(0x24);
        struct
        {
            char    fileName[0x10];
            UINT    fileSize;
        };
    };

} FILE_IN_DIR;

typedef struct  // 一组 20字节
{
    ULONG               ConditionFlags;
    PVOID               Effect;
    //BYTE                Type; // 1 回合; 2 次数; 3 AT条动多少次; 4 永久
    BYTE                CounterType;
    BYTE                Flags;
    SHORT               ConditionRate;
    LONG                ATLeft;
    LONG                Unknown4;

    enum CounterTypes
    {
        ByRounds    = 1,
        ByTimes     = 2,
        ByActions   = 3,
        Infinite    = 4,
    };
} CONDITION, MS_EFFECT_INFO, *PMS_EFFECT_INFO;

typedef struct
{
    ushort  CraftIndex;
    ubyte   CraftAnimation1;
    ubyte   CraftAnimation2;
} CraftLastUsed_INFO;

namespace NED61
{
    typedef struct
    {
        USHORT              Level;      // 0
        USHORT              HPMax;      // 2
        USHORT              HP;         // 4

        USHORT              EPMax;      // 6
        USHORT              EP;         // 8
        USHORT              CP;         // A
        int                 EXP;        // C
        SHORT               STR;        // 10
        SHORT               DEF;        // 12
        SHORT               ATS;        // 14
        SHORT               ADF;        // 16
        SHORT               DEX;        // 18
        SHORT               AGL;        // 1A
        SHORT               MOV;        // 1C
        SHORT               SPD;        // 1E
        USHORT              CPMAX;      // 20
        DUMMY_STRUCT(4);                // 22
        USHORT              RNG;        // 26
        DUMMY_STRUCT(4);                // 2A
        RESISTANCE          condition;  // 2E
        USHORT              CharacterIndex;
        DUMMY_STRUCT(2);
    } ED6_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED6_STATUS;

    namespace CONDITION_BIT
    {
        const ULONG_PTR POISON              = 0x00000001;    // 毒
        const ULONG_PTR FREEZE              = 0x00000002;    // 冻结 FROZEN
        const ULONG_PTR PETRIFY             = 0x00000004;    // 石化 LANDIFICATION STONE
        const ULONG_PTR SLEEP               = 0x00000008;    // 睡眠 SLEEPING
        const ULONG_PTR DISABLE_ARTS        = 0x00000010;    // 封魔 MUTE
        const ULONG_PTR BLIND               = 0x00000020;    // 黑暗 DARKNESS
        const ULONG_PTR DISABLE_CRAFT       = 0x00000040;    // 封技 SEAL
        const ULONG_PTR CONFUSE             = 0x00000080;    // 混乱 CHAOS
        const ULONG_PTR FAINT               = 0x00000100;    // 气绝 STUN
        const ULONG_PTR ONE_HIT_KILL        = 0x00000200;    // 即死
        const ULONG_PTR DEF_DOWN_FORCE      = 0x00000400;    // 绝对降防
        const ULONG_PTR RAGE                = 0x00000800;    // 愤怒
        const ULONG_PTR ARTS_GUARD          = 0x00001000;    // ArtsGuard
        const ULONG_PTR CRAFT_GUARD         = 0x00002000;    // CraftGuard
        const ULONG_PTR MOV_UP              = 0x00004000;
        //nst ULONG_PTR empty1              = 0x00008000;    // 无
        const ULONG_PTR STR_UP              = 0x00010000;
        const ULONG_PTR STR_DOWN            = 0x00020000;
        const ULONG_PTR DEF_UP              = 0x00040000;
        const ULONG_PTR DEF_DOWN            = 0x00080000;
        const ULONG_PTR SPD_UP              = 0x00100000;
        const ULONG_PTR SPD_DOWN            = 0x00200000;
        const ULONG_PTR DEX_UP              = 0x00400000;
        const ULONG_PTR DEX_DOWN            = 0x00800000;
        const ULONG_PTR AGL_UP              = 0x01000000;
        const ULONG_PTR AGL_DOWN            = 0x02000000;
        const ULONG_PTR MAX_GUARD           = 0x04000000;    // 墙 虚无领域 IMMUNE
        const ULONG_PTR VANISH_GUARD        = 0x08000000;    // 隐身GUARD
        const ULONG_PTR DEATH               = 0x10000000;    // 战斗不能 00 00 00 10
    }

    const ULONG_PTR conditionAbnormal       = 0xBFF;
    const ULONG_PTR conditionDown           = 0x2AA0400;

    CONST ULONG SEPITH_UP_LIMIT_ORIGINAL = 70;

    bool IsSoldierParty(UINT SoldierNo)
    {
        return SoldierNo < 6;
    }
}

namespace NED62
{
    typedef struct
    {
        UINT                Level;          // 0
        UINT                HPMax;          // 4
        UINT                HP;             // 8
        USHORT              EPMax;          // C
        USHORT              EP;             // E
        USHORT              CP;             // 10
        DUMMY_STRUCT(2);
        UINT                EXP;            // 14
        SHORT               STR;            // 18
        SHORT               DEF;            // 1A
        SHORT               ATS;            // 1C
        SHORT               ADF;            // 1E
        SHORT               DEX;            // 20
        SHORT               AGL;            // 22
        SHORT               MOV;            // 24
        SHORT               SPD;            // 26
        USHORT              CPMAX;          // 28
        DUMMY_STRUCT(4);
        USHORT              RNG;            // 2E
        DUMMY_STRUCT(4);
        RESISTANCE          condition;      // 34
        USHORT              CharacterIndex; // 38
        DUMMY_STRUCT(2);
    } ED6_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED6_STATUS;

    namespace CONDITION_BIT
    {
        const ULONG_PTR POISON              = 0x00000001;    // 毒
        const ULONG_PTR FREEZE              = 0x00000002;    // 冻结 FROZEN
        const ULONG_PTR PETRIFY             = 0x00000004;    // 石化 LANDIFICATION STONE
        const ULONG_PTR SLEEP               = 0x00000008;    // 睡眠 SLEEPING
        const ULONG_PTR DISABLE_ARTS        = 0x00000010;    // 封魔 MUTE
        const ULONG_PTR BLIND               = 0x00000020;    // 黑暗 DARKNESS
        const ULONG_PTR DISABLE_CRAFT       = 0x00000040;    // 封技 SEAL
        const ULONG_PTR CONFUSE             = 0x00000080;    // 混乱 CHAOS
        const ULONG_PTR FAINT               = 0x00000100;    // 气绝 STUN
        const ULONG_PTR ONE_HIT_KILL        = 0x00000200;    // 即死
        const ULONG_PTR DEF_DOWN_FORCE      = 0x00000400;    // 绝对降防
        const ULONG_PTR RAGE                = 0x00000800;    // 愤怒
        const ULONG_PTR ARTS_GUARD          = 0x00001000;    // ArtsGuard
        const ULONG_PTR CRAFT_GUARD         = 0x00002000;    // CraftGuard
        const ULONG_PTR MOV_UP              = 0x00004000;
        const ULONG_PTR MOV_DOWN            = 0x00008000;
        const ULONG_PTR STR_UP              = 0x00010000;
        const ULONG_PTR STR_DOWN            = 0x00020000;
        const ULONG_PTR DEF_UP              = 0x00040000;
        const ULONG_PTR DEF_DOWN            = 0x00080000;
        const ULONG_PTR SPD_UP              = 0x00100000;
        const ULONG_PTR SPD_DOWN            = 0x00200000;
        const ULONG_PTR ADF_UP              = 0x00400000;
        const ULONG_PTR ADF_DOWN            = 0x00800000;
        const ULONG_PTR AGL_UP              = 0x01000000;
        const ULONG_PTR AGL_DOWN            = 0x02000000;
        const ULONG_PTR MAX_GUARD           = 0x04000000;    // 墙 虚无领域 IMMUNE
        const ULONG_PTR VANISH              = 0x08000000;    // Vanish
        const ULONG_PTR CONDITION_GUARD     = 0x10000000;    // 全状态抵抗
        const ULONG_PTR BODY_ABNORMAL       = 0x20000000;    // 变胖/变小
        const ULONG_PTR ATS_UP              = 0x40000000;
        const ULONG_PTR DEATH               = 0x80000000;    // 战斗不能
    }

    namespace DEBUG_BIT
    {
        const ULONG_PTR ATTACK_MISS         = 0x0400;       // 非必中攻击 100% miss
        const ULONG_PTR ATTACK_HIT          = 0x1000;       // 非必中攻击 100% 命中（0x400回避优先）
        const ULONG_PTR ATTACK_BLIND        = 0x4000;       // 非必中攻击 命中率 *= 30%（黑暗效果）
    }

    const ULONG_PTR conditionAbnormal       = 0x28000BFF;
    const ULONG_PTR conditionDown           = 0x2AA8400;

    CONST ULONG SEPITH_UP_LIMIT_ORIGINAL = 70;

    bool IsSoldierParty(UINT SoldierNo)
    {
        return SoldierNo < 8;
    }
}

namespace NED63
{
    typedef NED62::ED6_CHARACTER_STATUS ED6_CHARACTER_STATUS;
    typedef NED62::ED6_CHARACTER_STATUS ED6_STATUS;

    namespace CONDITION_BIT = NED62::CONDITION_BIT;
    namespace DEBUG_BIT     = NED62::DEBUG_BIT;

    using NED62::conditionAbnormal;
    using NED62::conditionDown;

    CONST ULONG SEPITH_UP_LIMIT_ORIGINAL = 300;
    
    using NED62::IsSoldierParty;
}


typedef struct  // Ai格式
{
    ubyte   Time;
    ubyte   Probability;
    ubyte   Target;
    ubyte   DUMMY_STRUCT_01;        // 01
    ubyte   CraftAnimation1;        // 动画代码，在AS文件中定义。蓄魔法06、放魔法07、C技10-19，S技1A-1F
    ubyte   CraftAnimation2;
    ushort  CraftIndex;             // 技能代码，常规查t_magic._dt，自定义技能从E8 03开始
    int     TimeParameter1;
    int     TimeParameter2;
    int     TargetParameter1;
    int     TargetParameter2;
} ED6_AI_INFO;
typedef ED6_AI_INFO ED7_AI_INFO;


typedef struct  // 自定义技能格式
{
    ushort  AnimationType;
    ubyte   Target;
    ubyte   SpecialEffect;      // 01 强制命中 02 驱动时无法被打断
    ubyte   Att;                // 属性
    // 空轨FCSC       0无 1地 2水 3火 4风 5空 6幻 0时 // FCSC没有 时空幻有效率，所以无所谓
    // 空轨3rd        0无 1地 2水 3火 4风 5空 6幻 7时
    // 零轨碧轨     0无 1地 2水 3火 4风 5时 6空 7幻
    ubyte   ShapeScope;         // 范围形状
    ubyte   Effect1;
    ubyte   Effect2;
#if MAGICSIZE == 32

    ushort  RNG;
    ushort  ScopeRadius;        // 半径
    ushort  STCharge;
    ushort  STCoolDown;
    ushort  CP_EP;
    ushort  DisplayIndex;       // 魔法列表中的显示先后，t_magic._dt中的才有效，ms文件中的一般为1或0。

#elif MAGICSIZE == 28

    ubyte   DisplayIndex;       // 魔法列表中的显示先后，t_magic._dt中的才有效，ms文件中的一般为1或0。
    ubyte   RNG;
    ubyte   STCharge;
    ubyte   STCoolDown;
    ushort  CP_EP;
    ushort  ScopeRadius;        // 半径
#else
#endif

    short   Effect1Parameter;
    ushort  Effect1ST;
    short   Effect2Parameter;
    ushort  Effect2ST;

    //DUMMY_STRUCT(4);
    ushort  addrMagicName;  // 魔法名 地址
    ushort  addrMagicIntro; // 魔法说明 地址
} ED6_CRAFT_INFO, CRAFT_INFO, *PCRAFT_INFO;
typedef ED6_CRAFT_INFO ED7_CRAFT_INFO;

typedef struct
{
    CHAR Description[0x100];
    CHAR Name[0x20];
} ED6_CRAFT_INTRO;
typedef ED6_CRAFT_INTRO ED7_CRAFT_INTRO;

enum
{
    CHR_FLAG_ENEMY  = 0x1000,
    CHR_FLAG_NPC    = 0x2000,
    CHR_FLAG_PARTY  = 0x4000,
    CHR_FLAG_EMPTY  = 0x8000,
};

enum
{
    CHR_FLAG_ResistBeatBack     = 0x0200,
    CHR_FLAG_ResistATDelay      = 0x0800,
    CHR_FLAG_AbsoluteMiss       = 0x2000,
};

enum
{
    ACTION_ATTACK       = 0,
    ACTION_MOVE         = 1,
    ACTION_ARTS         = 2,
    ACTION_CRAFT        = 3,
    ACTION_SCRAFT       = 4,
    ACTION_ITEM         = 5,
    ACTION_ARIA_ARTS    = 6,
    ACTION_CAST_ARTS    = 7,
    ACTION_ARIA_CRAFT   = 8,
    ACTION_CAST_CRAFT   = 9,
};

namespace NED61
{
    typedef struct // 0x23C8
    {
        union
        {
            DUMMY_STRUCT(0x23C8);
            struct
            {
                USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
                USHORT              Flags;                      // 10 敌方 40 己方 ...
                BYTE                DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
                BYTE                UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
                DUMMY_STRUCT(4);
                USHORT              CharacterIndex;
                //DUMMY_STRUCT(2);
                //USHORT              ATActTime;                // AT条动多少次，初始为0
                FileIndex           SYFileIndex;
                FileIndex           MSFileIndex;
                FileIndex           ASFileIndex;                // 0x14
                DUMMY_STRUCT(0x1B8);
                // 0x1D0
                ED6_STATUS          StatusBasic;                // normal难度基础值
                ED6_STATUS          StatusSum;                  // 算上难度、装备、回路
                USHORT              MoveSPD;                    // 移动速度，我方人员也从ms文件中读取
                USHORT              MoveAfterAttack;            // 从ms文件中读取
                CONDITION           Condition[29];
                DUMMY_STRUCT(24);
                int                 AT;
                int                 AT2;
                DUMMY_STRUCT(6);
                USHORT              AIType;
                DUMMY_STRUCT(2);
                ushort              DropIndex[2];               // 掉落物 物品代码
                ubyte               DropProbability[2];         // 掉落概率
                ubyte               Sex;
                DUMMY_STRUCT(1);
                ushort              Equip[5];                   // 装备
                ushort              Orb[6];                     // 回路   7个
                //DUMMY_STRUCT(2);
                ED6_AI_INFO         NormalAttack;
                ED6_AI_INFO         Arts[80];
                ED6_AI_INFO         Craft[10];
                ED6_AI_INFO         SCraft[4];

                CraftLastUsed_INFO  CraftLastUsed;
                BYTE                fleeParameter[4];
                ED6_CRAFT_INFO      CraftInf[16];
                ED6_CRAFT_INTRO     CraftIntro[16];

                //DUMMY_STRUCT(0x203A);
                DUMMY_STRUCT(0xC);
                // 0x21C4
                ubyte               Sepith[7];                  // 掉落耀晶片
                FSkip(1);
                ushort              EXPGet;                     // 战斗胜利 结算经验前暂存
                FSkip(2);
                ushort              ArtsAttResistance[4];       // 七曜属性有效率
                DUMMY_STRUCT(0x84);
                RESISTANCE          Resistance;                 // 0x225C
                DUMMY_STRUCT(0x48);
                char                CharacterIntro[0x100];      // 0x22A8
            };
        };
    } ED6_CHARACTER_BATTLE_INF, MONSTER_STATUS, *PMONSTER_STATUS;
}

namespace NED62
{
    typedef struct // 0x2478
    {
        union
        {
            DUMMY_STRUCT(0x2478);
            struct
            {
                USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
                USHORT              Flags;                      // 10 敌方 40 己方 ...
                BYTE                DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
                BYTE                UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
                DUMMY_STRUCT(4);
                USHORT              CharacterIndex;
                //DUMMY_STRUCT(2);
                //USHORT              ATActTime;                // AT条动多少次，初始为0
                FileIndex           SYFileIndex;
                FileIndex           MSFileIndex;
                FileIndex           ASFileIndex;                // 0x14
                //DUMMY_STRUCT(0x214);
                DUMMY_STRUCT(0x162);
                USHORT              CurrentActionType;          // 0x17A
                DUMMY_STRUCT(0xA);
                USHORT              CurrentCraftIndex;          // 0x186
                DUMMY_STRUCT(0x2C);
                USHORT              Target[0x10];               // 0x1B4
                BYTE                TargetCount;                // 0x1D4
                BYTE                SelectedTargetIndex;        // 0x1D5
                COORD               SelectedTargetPos;          // 0x1D6
                DUMMY_STRUCT(0x36);
                BYTE                IsHitMiss[0x10];            // 0x210
                DUMMY_STRUCT(4);
                PCHAR               ChrName;                    // 0x224
                DUMMY_STRUCT(4);                                // 0x228
                ED6_STATUS          StatusBasic;                // 0x22C
                ED6_STATUS          StatusSum;                  // 0x268
                USHORT              MoveSPD;                    // 移动速度，我方人员也从ms文件中读取
                USHORT              MoveAfterAttack;            // 从ms文件中读取
                CONDITION           Condition[31];
                DUMMY_STRUCT(24);
                int                 AT;
                int                 AT2;
                DUMMY_STRUCT(6);
                USHORT              AIType;
                ushort              DropIndex[2];               // 掉落物 物品代码
                ubyte               DropProbability[2];         // 掉落概率
                ubyte               Sex;
                DUMMY_STRUCT(1);
                ushort              Equip[5];                   // 装备
                ushort              Orb[7];                     // 回路   7个
                //DUMMY_STRUCT(2);
                ED6_AI_INFO         NormalAttack;
                ED6_AI_INFO         Arts[80];
                ED6_AI_INFO         Craft[10];
                ED6_AI_INFO         SCraft[5];

                CraftLastUsed_INFO  CraftLastUsed;
                BYTE                fleeParameter[4];
                ED6_CRAFT_INFO      CraftInf[16];
                ED6_CRAFT_INTRO     CraftIntro[16];

                //DUMMY_STRUCT(0x203A);
                DUMMY_STRUCT(0x4C);
                // 0x22B0
                ubyte               Sepith[7];                  // 掉落耀晶片
                FSkip(1);
                ushort              EXPGet;                     // 战斗胜利 结算经验前暂存
                FSkip(2);
                ushort              ArtsAttResistance[4];       // 七曜属性有效率
                //FSkip(2);
                RESISTANCE          Resistance;                 // 0x22C4
                DUMMY_STRUCT(0x94);
                //char              CharacterIntro[0x120];      // 0x2358
                char                CharacterIntro[0x100];      // 0x2358
            };
        };
    } ED6_CHARACTER_BATTLE_INF, MONSTER_STATUS, *PMONSTER_STATUS;
}

namespace NED63
{
    typedef struct // 0x2490
    {
        union
        {
            DUMMY_STRUCT(0x2490);
            struct
            {
                USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
                USHORT              Flags;                      // 10 敌方 40 己方 ...
                BYTE                DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
                BYTE                UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
                DUMMY_STRUCT(4);
                USHORT              CharacterIndex;
                DUMMY_STRUCT(2);
                USHORT              ATActTime;                  // AT条动多少次，初始为0
                FileIndex           SYFileIndex;
                FileIndex           MSFileIndex;
                FileIndex           ASFileIndex;                // 0x18
                DUMMY_STRUCT(0x162);
                USHORT              CurrentActionType;          // 0x17E
                DUMMY_STRUCT(0xA);
                USHORT              CurrentCraftIndex;          // 0x18A
                DUMMY_STRUCT(0x2C);
                USHORT              Target[0x10];               // 0x1B8
                BYTE                TargetCount;                // 0x1D8
                BYTE                SelectedTargetIndex;        // 0x1D9
                COORD               SelectedTargetPos;          // 0x1DA
                DUMMY_STRUCT(0x32);
                BYTE                IsHitMiss[0x10];            // 0x210
                DUMMY_STRUCT(4);
                PCHAR               ChrName;                    // 0x224
                DUMMY_STRUCT(4);                                // 0x228
                ED6_STATUS          StatusBasic;                // 0x22C
                ED6_STATUS          StatusSum;                  // 0x268
                USHORT              MoveSPD;                    // 移动速度，我方人员也从ms文件中读取
                USHORT              MoveAfterAttack;            // 从ms文件中读取
                CONDITION           Condition[32];
                DUMMY_STRUCT(24);
                int                 AT;
                int                 AT2;
                DUMMY_STRUCT(4);
                USHORT              AIType;
                ushort              DropIndex[2];               // 掉落物 物品代码
                ubyte               DropProbability[2];         // 掉落概率
                ubyte               Sex;
                DUMMY_STRUCT(1);
                ushort              Equip[5];                   // 装备
                ushort              Orb[7];                     // 回路   7个
                DUMMY_STRUCT(2);
                ED6_AI_INFO         NormalAttack;
                ED6_AI_INFO         Arts[80];
                ED6_AI_INFO         Craft[10];
                ED6_AI_INFO         SCraft[5];

                CraftLastUsed_INFO  CraftLastUsed;
                BYTE                fleeParameter[4];
                ED6_CRAFT_INFO      CraftInf[16];
                ED6_CRAFT_INTRO     CraftIntro[16];

                //DUMMY_STRUCT(0x203A);
                DUMMY_STRUCT(0x4B);
                ubyte               Sepith[7];                  // 掉落耀晶片
                FSkip(2);
                ushort              EXPGet;                     // 战斗胜利 结算经验前暂存
                FSkip(2);
                ushort              ArtsAttResistance[7];       // 七曜属性有效率
                FSkip(2);
                RESISTANCE          Resistance;                 // 0x22E0
                DUMMY_STRUCT(0x8C);
                char                CharacterIntro[0x100];      // 0x2370
            };
        };
    } ED6_CHARACTER_BATTLE_INF, MONSTER_STATUS, *PMONSTER_STATUS;

    typedef struct
    {
        ushort  index;
        FSkip(2);
        ushort  item[8];
        byte    probability[8];
    } ED6_DROP_ITEM;
}

/*
namespace NEDZERO
{
    typedef struct
    {
        int                 HPMax;
        int                 HP;
        ushort              Level;
        ushort              EPMax;
        ushort              EP;
        ushort              CP;
        UINT                EXP;
        short               STR;
        short               DEF;
        short               ATS;
        short               ADF;
        short               DEX;
        short               AGL;
        short               MOV;
        short               SPD;
        short               DEXRate;
        short               AGLRate;
        ushort              CPMAX;
        DUMMY_STRUCT(0x2);
        ushort              RNG;        // 内存中改StatusBasic中的才有效
        DUMMY_STRUCT(0x2);
        RESISTANCE          condition;  // RESISTANCE类型，进程中有效; 战斗外包括存档中 DEATH有效
    } ED6_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED6_STATUS;
    typedef ED6_CHARACTER_STATUS ED7_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED7_STATUS;
}

namespace NEDAO
{
    typedef NEDZERO::ED6_CHARACTER_STATUS ED6_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED6_STATUS;
    typedef ED6_CHARACTER_STATUS ED7_CHARACTER_STATUS;
    typedef ED6_CHARACTER_STATUS ED7_STATUS;

    typedef struct // 0x2424
    {
        union
        {
            DUMMY_STRUCT(0x2424);
            struct
            {
                USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              Flags;                      // 10 敌方 40 己方 ...
                BYTE                DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
                BYTE                UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                DUMMY_STRUCT(4);
                USHORT              CharacterIndex;
                DUMMY_STRUCT(2);
                USHORT              ATActTime;                  // AT条动多少次，初始为0
                FileIndex           SYFileIndex;
                FileIndex           MSFileIndex;
                FileIndex           ASFileIndex;                // 0x18
                DUMMY_STRUCT(0x156);
                USHORT              wActType;
                DUMMY_STRUCT(0xA);
                USHORT              wUseMagic;
                DUMMY_STRUCT(0xB4);
                //DUMMY_STRUCT(0x218);
                ED6_STATUS          StatusBasic;                // normal难度基础值
                ED6_STATUS          StatusSum;                  // 算上难度、装备、回路

                ushort              MoveSPD;                // 移动速度，我方人员也从ms文件中读取
                ushort              MoveAfterAttack;        // 从ms文件中读取
                CONDITION           Condition[32];
                DUMMY_STRUCT(24);
                int             AT;
                int             AT2;    // AiTime08用

                ushort              AIType;
                ushort              EXPGet;                     // 战斗胜利 结算经验前暂存
                ushort              DropIndex[2];                 // 掉落物 物品代码
                //ushort                DropIndex2;
                ubyte               DropProbability[2];           // 掉落概率
                //ubyte             DropProbability2;
                ubyte               Sex;
                ubyte               DisplayHighLevelArtsAttResistance;
                // 是否开启时空幻有效率显示，0/8 不开启 1/9 开启，应该是标志位，可能含有更多信息
                ushort              Equip[5];                   // 装备
                ushort              Orb[7];                     // 回路   7个

                ED7_AI_INFO         NormalAttack;
                ED7_AI_INFO         Arts[80];
                ED7_AI_INFO         Craft[10];
                ED7_AI_INFO         SCraft[5];
                ED7_AI_INFO         SupportCraft[3];
                CraftLastUsed_INFO  CraftLastUsed;
                BYTE                fleeParameter[4];
                ED7_CRAFT_INFO      CraftInf[16];
                ED7_CRAFT_INTRO     CraftIntro[16];

                ubyte               Sepith[7];                  // 掉落耀晶片
                FSkip(3);
                ushort              ArtsAttResistance[7];       // 七曜属性有效率
                RESISTANCE          Resistance;                 // 0x2304
                DUMMY_STRUCT(0x78);
                char                CharacterIntro[0x80];       // 0x2380
            };
        };
    } ED6_CHARACTER_BATTLE_INF;
    typedef ED6_CHARACTER_BATTLE_INF ED7_CHARACTER_BATTLE_INF;
}
*/

INT STATUS_REVISE[4] = {0, -4, -9, 6};

typedef struct _SSTATUS_REVISE_SPECIAL // 0x20
{
    ULONG   MSFile;
    union
    {
        USHORT      entry[6];
        struct
        {
            USHORT  HP;     // 0x4
            USHORT  STR;    // 0x6
            USHORT  DEF;    // 0x8
            USHORT  ATS;    // 0xA
            USHORT  ADF;    // 0xC
            USHORT  SPD;    // 0xE
        };
    };
    ULONG   N_RESIST;       // 0x10
    ULONG   E_RESIST;       // 0x14
    ULONG   N_AI;           // 0x18
    ULONG   E_AI;           // 0x1C
} SSTATUS_REVISE_SPECIAL;

typedef union _SSTATUS_REVISE_DIFFICULTY // 0xC
{
    USHORT      entry[6];
    struct
    {
        USHORT  HP;     // 0x0
        USHORT  STR;    // 0x2
        USHORT  DEF;    // 0x4
        USHORT  ATS;    // 0x6
        USHORT  ADF;    // 0x8
        USHORT  SPD;    // 0xA
    };

} SSTATUS_REVISE_DIFFICULTY;

typedef union _SSTATUS_RATE_MINI
{
    int         entry[6];
    struct
    {
        int     HP;
        int     STR;
        int     DEF;
        int     ATS;
        int     ADF;
        int     SPD;
    };

} SSTATUS_RATE_MINI;

enum STATUS_TYPE
{
    STATUS_TYPE_HP      = 0,
    STATUS_TYPE_STR     = 1,
    STATUS_TYPE_DEF     = 2,
    STATUS_TYPE_ATS     = 3,
    STATUS_TYPE_ADF     = 4,
    STATUS_TYPE_SPD     = 5,
    STATUS_TYPE_MOV     = 6,
    STATUS_TYPE_RESIST  = 7,
};

enum BTREV_AI_FLAGS
{
    BTREV_AI_INC_ARTS   = 0x1,
    BTREV_AI_DEC_ARTS   = 0x2,
    BTREV_AI_INC_CRAFT  = 0x4,
    BTREV_AI_DEC_CRAFT  = 0x8,
    BTREV_AI_INC_SCRAFT = 0x10,
    BTREV_AI_DEC_SCRAFT = 0x20,

    BTREV_AI_INC_MOV    = 0x40,
    BTREV_AI_DEC_MOV    = 0x80,

    BTREV_AI_INC_SUMMON = 0x100,
    BTREV_AI_DEC_SUMMON = 0x200,
    BTREV_AI_SET_0      = 0x400,
    BTREV_AI_SET_1      = 0x800,
    BTREV_AI_SET_2      = 0x1000,
};

enum BTREV_AI_PROBABILITY_CHANGE_TYPE
{
    BTREV_AI_PROBABILITY_INC = 0,
    BTREV_AI_PROBABILITY_DEC = 1,
};

#pragma pack()
#pragma warning (default: 4201)

#endif