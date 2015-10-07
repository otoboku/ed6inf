#define MAGICSIZE 32

#include "ed6_common.h"
#include "ed6inf_data.h"
#include "misc_aki.h"

BOOL Hooked = FALSE;

#if CONSOLE_DEBUG
LARGE_INTEGER lFrequency, lStopCounter, lStartCounter;
#endif

#if ENABLE_LOG
void WriteLog(PCWSTR Format, ...)
{
    NtFileDisk log;
    WCHAR Buffer[0xFF0];
    
    log.CreateIfNotExist(L"log.txt");
    if (log.GetSize32() == 0)
    {
        ULONG BOM = BOM_UTF16_LE;
        log.Write(&BOM, 2);
    }
    
    log.Seek(0, FILE_END);
    
    log.Write(Buffer, vswprintf(Buffer, Format, (va_list)(&Format + 1)) * 2);
    log.Write(L"\r\n", 4);
}
#else
#define WriteLog(...)
#endif

#define METHOD_PTR(_method) PtrAdd((PVOID)NULL, _method)
#define INIT_STATIC_MEMBER(x) DECL_SELECTANY TYPE_OF(x) x = nullptr
#define DECL_STATIC_METHOD_POINTER(cls, method) static TYPE_OF(&cls::method) Stub##method
#define DETOUR_METHOD(cls, method, addr, ...) TYPE_OF(&cls::method) (method); *(PULONG_PTR)&(method) = addr; return (this->*method)(__VA_ARGS__)
#define DECL_METHOD_POINTER_AND_INIT(cls, method, addr) TYPE_OF(&cls::method) Stub##method; *(PULONG_PTR)&(Stub##method) = addr
//#define DETOUR_FUNCTION(function, addr, ...) TYPE_OF(&function) (stubfun); *(PULONG_PTR)&(stubfun) = addr; return (*stubfun)(__VA_ARGS__)
#define DETOUR_FUNCTION(function, addr, ...) return ((TYPE_OF(&function))(addr))(__VA_ARGS__)

typedef int (__cdecl *pSprintf)(char *_Dest, const char* _Format, ...);
typedef ULONG (__cdecl *pGetAddr)(ULONG);
typedef void (*pHandleKeyUp)(USHORT key);

pHandleKeyUp lpfnHandleKeyUp = [] (USHORT key) {};
WNDPROC WindowProc;
USHORT  CodePage = 936;

BOOL    g_bShowExtraInfo = TRUE;
BOOL    g_bDisplayBattleIcoEx = FALSE;
int     g_bShowInfoPage2 = 0;
POINT   battleIcoRec = {16, 8};


typedef struct _SStatusRate
{
    INT     HP_a;
    INT     STR_a;
    INT     DEF_a;
    INT     ATS_a;
    INT     ADF_a;
    INT     SPD_a;
    INT     DEX_a;
    INT     AGL_a;
    INT     MOV_a;
    //INT     DEXRate_a;
    //INT     AGLRate_a;
    INT     HP_b;
    INT     STR_b;
    INT     DEF_b;
    INT     ATS_b;
    INT     ADF_b;
    INT     SPD_b;
    INT     DEX_b;
    INT     AGL_b;
    INT     MOV_b;
    //INT     DEXRate_b;
    //INT     AGLRate_b;
    BOOL    ResistNone;
    BOOL    ResistAbnormalCondition;
    BOOL    ResistAbilityDown;
    BOOL    ResistATDelay;
} SStatusRate;

SStatusRate sRate;

// ini [Battle]

int     nDifficulty = 0;
int     nSepithUpLimit = 0;
int     nShowAT = 0;
int     nShowConditionAT = 0;
int     nConditionATColor = 0;

BOOL    bShowCraftName = FALSE;
BOOL    bForceShowMonsInf = FALSE;
BOOL    bUnlimitedSummon = FALSE;
BOOL    bFixEnemyStatusBug = FALSE;

enum
{
    DIFFICULTY_PC_ORIGINAL      = -1,
    DIFFICULTY_PS_NORMAL        = 0,
    DIFFICULTY_PS_HARD          = 1,
    DIFFICULTY_PS_NIGHTMARE     = 2,
    DIFFICULTY_PS_EASY          = 3,
    DIFFICULTY_PS_CUSTOM        = 4,
    DIFFICULTY_PC_CUSTOM        = 5,
    DIFFICULTY_INI_MAX,

    DIFFICULTY_DEFAULT          = 0,
};
BOOL    bPSP_MODE = FALSE;

enum
{
    COLOR_WHITE     = 0,
    COLOR_GRAY      = 6,

    COLOR_LINE      = 8,
    COLOR_TITLE     = 0xB,

    COLOR_MAXIMUM   = 21,
};

enum
{
    SHOW_AT_NONE        = 0,
    SHOW_AT_SIMPLE      = 1,
    SHOW_AT_ORIGINAL    = 2,

    SHOW_AT_DEFAULT     = SHOW_AT_SIMPLE,
};

enum
{
    SHOW_CONDITION_AT_NONE      = 0,
    SHOW_CONDITION_AT_HIDE99    = 1,
    SHOW_CONDITION_AT_MAX99     = 3,
    SHOW_CONDITION_AT_ORIGINAL  = 5,

    SHOW_CONDITION_AT_DEFAULT   = SHOW_CONDITION_AT_HIDE99,
};

enum
{
    CONDITION_AT_COLOR_ORIGINAL = 0xFFFFFFFF,
    CONDITION_AT_COLOR_GREEN    = 0xFF00FF00,
};

VOID ConfigInit();

LRESULT NTAPI MainWndProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_KEYUP:
            switch (LOWORD(wParam))
            {
                case VK_F5:
                    ConfigInit();
                    break;

                case VK_F2:
                    g_bShowExtraInfo ^= TRUE;
                    break;

                default:
                    lpfnHandleKeyUp(LOWORD(wParam));
                    break;
            }
            break;
    }

    return WindowProc(Window, Message, wParam, lParam);
}

VOID ChangeMainWindowProc(HWND GameWindow)
{
    if (GameWindow != nullptr)
        WindowProc = (WNDPROC)SetWindowLongPtrA(GameWindow, GWL_WNDPROC, (LONG_PTR)MainWndProc);
}

namespace NED6123
{
    pSprintf    sprintf                     = (pSprintf)0;
    pGetAddr    getItemInf                  = (pGetAddr)0;

    ULONG_PTR   lpfnDrawSimpleText          = 0;
    ULONG_PTR   lpfnFtol                    = 0;
    ULONG_PTR   lpfnSetTextSize             = 0;
    ULONG_PTR   lpfnDrawBattleIcon          = 0;

    ULONG_PTR   addrDisplaySkipCondition0   = (ULONG_PTR)-1; // patch at
    ULONG_PTR   addrDisplaySkipCondition1   = 0; // addrDisplaySkipCondition0 + 5; // ret1
    ULONG_PTR   addrDisplaySkipCondition2   = 0; // ret2

    ULONG_PTR   lpcStatusLine               = 0;
    ULONG_PTR   lpfnEnumCondition           = 0;
    ULONG_PTR   addrConditionIconPointBegin = 0;
    ULONG_PTR   addrConditionIconPointEnd   = 0;
    PULONG_PTR  addrBattleTexture           = (PULONG_PTR)0;

    ULONG_PTR   addrDisplayStatusPatch0     = (ULONG_PTR)-1;
    ULONG_PTR   addrDisplayStatusPatch1     = 0; // addrDisplayStatusPatch0 + 5;
    ULONG_PTR   addrDisplayStatusPatch2     = 0; // call

    ULONG_PTR   addrDisplayBattleIcoEx0     = (ULONG_PTR)-1;
    DWORD       dwYStart                    = 0;

    ULONG_PTR   addrDisplayItemDropPatch0   = (ULONG_PTR)-1;
    ULONG_PTR   addrDisplayItemDropPatch1   = 0; // addrDisplayItemDropPatch0 + 5;

    ULONG_PTR   addrSoldierNo0              = 0;
    PULONG_PTR  addrLPDir0                  = (PULONG_PTR)0;
    PULONG_PTR  addrBattleNowIndex          = (PULONG_PTR)0;
    pGetAddr    getItemDrop                 = (pGetAddr)0;
    pGetAddr    getItemName                 = (pGetAddr)0;

    ULONG_PTR   addrChangeEnemyStatusPatch0 = (ULONG_PTR)-1;
    ULONG_PTR   addrChangeEnemyStatusPatch1 = 0; // addrChangeEnemyStatusPatch0 + 5;
    ULONG_PTR   addrChangeEnemyStatusPatch2 = 0; // call

    ULONG_PTR   addrCheckQuartzPatch        = (ULONG_PTR)-1;
    ULONG_PTR   lpfnCheckEquipment          = 0;
    ULONG_PTR   lpfnGetCraftInf             = 0;
    ULONG_PTR   lpfnFindCondition           = 0;
    ULONG_PTR   lpfnRand                    = 0;
    ULONG_PTR   lpfnRandX                   = 0;
    PULONG_PTR  lpDebugFlags                = (PULONG_PTR)0;
    PULONG_PTR  lpBattleFlags               = (PULONG_PTR)0;
    ULONG_PTR   addrGetHitResultPatch       = (ULONG_PTR)-1;

    ULONG_PTR   addrDrive3Patch             = (ULONG_PTR)-1;
    ULONG_PTR   addrDrive3PatchTrue;
    ULONG_PTR   addrDrive3PatchFalse;

    ULONG_PTR   addrInitBeforeAtIconUpPatch = (ULONG_PTR)-1;
    ULONG_PTR   addrCheckCraftMirrorPatch   = (ULONG_PTR)-1;
    ULONG_PTR   addrCheckArtsMirrorPatch    = (ULONG_PTR)-1;
    ULONG_PTR   addrCheckMirrorWPADPatch    = (ULONG_PTR)-1;
    ULONG_PTR   addrCheckMirrorWPADBack;
    ULONG_PTR   addrFixMirrorBugPatch       = (ULONG_PTR)-1;

    ULONG_PTR   StubCheckQuartz;
    ULONG_PTR   StubGetHitResult;
    ULONG_PTR   StubCheckCraftMirror;
    ULONG_PTR   StubCheckArtsMirror;

    ULONG_PTR   addrSetChrStatusByEquipPatch= (ULONG_PTR)-1;
    ULONG_PTR   StubSetChrStatusByEquip;

    PSIZE       resolution                  = (PSIZE)0; // 分辨率

    bool        half_mirror_rand[0x10];

    namespace ITEM_ID
    {
        CONST USHORT CRAFT_MIRROR   = 0x28A;    // 650 物理反射
        CONST USHORT ARTS_MIRROR    = 0x28B;    // 651 魔法反射

        CONST USHORT INFORMATION    = 0x291;    // 657 情报
        CONST USHORT SHINKYO        = 0x292;    // 658 神鏡
        CONST USHORT MAKYO          = 0x293;    // 659 魔鏡

        CONST USHORT DRIVE1         = 0x2C6;    // 710 驱动1
        CONST USHORT DRIVE2         = 0x2C7;    // 711 驱动2
    }

    FORCEINLINE int CDECL rand()
    {
        DETOUR_FUNCTION(rand, lpfnRand);
    }

    FORCEINLINE USHORT CDECL randX(USHORT max)
    {
        DETOUR_FUNCTION(randX, lpfnRandX, max);
    }

    NoInline
    bool CDECL CheckQuartz(ULONG ChrPosition, USHORT ItemId, PULONG EquippedIndex = nullptr)
    {
        TYPE_OF(&CheckQuartz) lpCheckQuartz = (TYPE_OF(&CheckQuartz))StubCheckQuartz;
        //ULONG   who;
        switch (ItemId)
        {
            case ITEM_ID::INFORMATION:
                if (bForceShowMonsInf)
                {
                    if (EquippedIndex)
                    {
                        *EquippedIndex = 1;
                    }
                    return true;
                }
        	    break;
        }

        return lpCheckQuartz(ChrPosition, ItemId, EquippedIndex);
    }
    FORCEINLINE
    bool CDECL CheckEquipment(ULONG ChrPosition, USHORT ItemId, PULONG EquippedIndex = nullptr)
    {
        DETOUR_FUNCTION(CheckEquipment, lpfnCheckEquipment, ChrPosition, ItemId, EquippedIndex);
    }
}

namespace NED61
{
    using namespace NED6123;
    using NED6123::sprintf;
    using NED6123::rand;

    #define _ED61_NS_
    #include "ed6_ns_common.h"
    #undef  _ED61_NS_

    ASM void ed6DisplaySkipCondition()
    {
        __asm
        {
            cmp DWORD PTR DS:[ESI+0x4870],0x0;
            je L01;
            //push 0x004341B8;
            jmp addrDisplaySkipCondition2;
L01:
            push 0x190;
            //push 0x00434121;
            jmp addrDisplaySkipCondition1;
        }
    }

    ASM void ed6DisplayStatusPatch()
    {
        __asm
        {
            call addrDisplayStatusPatch2;
            mov edx, ebx;
            sub edx, 0x22A8;
            push edx;
            mov ecx, esi;
            call CBattleInfoBox::ed6DisplayStatus;
            jmp addrDisplayStatusPatch1;
        }
    }

    NoInline
    void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
    {
        ASM_DUMMY_AUTO();
    }
    void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
    {
        if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99)
        {
            if (AT > 99)
            {
                if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99) return;
                AT = 99;
            }

            if (AT < 10)
            {
                x -= (8.f * 2 - (16.f * resolution->cx / 640.f - 8.f * 1) / 2);
            }
            else
            {
                x -= (8.f * 1 - (16.f * resolution->cx / 640.f - 8.f * 2) / 2);
            }
        }

        // x-=16 y+=16
        y = (y - 16.f) + 16.f * resolution->cy / 480.f;
        ed6ShowConditionAtOld(AT, x, y, width, height, a6, a7, nConditionATColor);
    }

    void ed6SetEnemyFinalStatusPsp(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
    {
#define CALC_STATUS(_status, _rate) \
        result = pStatusSum->_status * rate_final._rate / 100; \
        if (pStatusSum->_status > 0 && result == 0) \
        { \
            result = 1; \
        } \
        pStatusSum->_status = (TYPE_OF(pStatusSum->_status))result;
//#end def

        int difficulty = get_difficulty();
        int revise = STATUS_REVISE[difficulty];
        ED6_STATUS* pStatusSum = &lpBattleInf->StatusSum;
      
        if (revise == 0)
        {
            return;
        }
        int rate = 100 - 5 * revise;
        if (rate < 10)
        {
            rate = 10;
        }
        SSTATUS_REVISE_DIFFICULTY   revise_dif;
        SSTATUS_RATE_MINI           rate_final;

        if (rate > 100)
        {
            if (lpBattleInf->MSFileIndex.file == 0x1001ED)
            {
                return;
            }
            if (difficulty == DIFFICULTY_PS_NIGHTMARE)
            {
                revise_dif = { 100, 100,  50, 100,  50, 100 };
            } 
            else
            {
                revise_dif = { 100, 100, 100, 100, 100, 100 };
            }
            for (int i = 0; i < sizeof(revise_dif) / sizeof(revise_dif.entry[0]); ++i)
            {
                rate_final.entry[i] = (rate - 100) * revise_dif.entry[i] / 100 + 100;
            }
        }
        else
        {
            rate_final = { rate, rate, rate, rate, rate, rate };
        }
        int result;
        CALC_STATUS(HPMax, HP);
        pStatusSum->HP = (TYPE_OF(pStatusSum->HP))result;
        CALC_STATUS(STR, STR);
        CALC_STATUS(DEF, DEF);
        CALC_STATUS(ATS, ATS);
        CALC_STATUS(ADF, ADF);
        CALC_STATUS(SPD, SPD);

        if (rate > 100)
        {
            if (difficulty == DIFFICULTY_PS_NIGHTMARE)
            {
                pStatusSum->ADF += pStatusSum->Level >> 1;
            }
            else if (difficulty == DIFFICULTY_PS_HARD)
            {
                pStatusSum->ADF += pStatusSum->Level >> 2;
            }
        }
#undef  CALC_STATUS
    }

}

namespace NED62
{
    using namespace NED6123;
    using NED6123::sprintf;
    using NED6123::rand;

    namespace ITEM_ID
    {
        using namespace NED6123::ITEM_ID;
        CONST USHORT NEKO_SUIT      = 0x10C;    // 268 猫咪服
        CONST USHORT NEKO_SHOE      = 0x12B;    // 299 猫爪靴
        CONST USHORT NEKO_BAND      = 0x13D;    // 317 猫耳帽
        CONST USHORT NEKO_TAIL      = 0x13E;    // 318 猫尾

        CONST USHORT DEX[5]         = { 0x26A, 0x26B, 0x26C, 0x2A0, 0x2A1 };
        CONST USHORT AGL[5]         = { 0x26D, 0x26E, 0x26F, 0x2A2, 0x2A3 };

        CONST USHORT DRIVE3         = 0x2CD;    // 717 刻耀珠
        CONST USHORT DRIVE[3]       = { 0x2C6, 0x2C7, 0x2CD };
    }

    VOID CDECL SetChrStatusByEquip(ULONG ChrNo, CHAR_STATUS* pStatus);

    #define _ED62_NS_
    #include "ed6_ns_common.h"
    #undef  _ED62_NS_

    ASM void ed6DisplaySkipCondition()
    {
        __asm
        {
            cmp DWORD PTR DS:[ESI+0x4870],0;
            je L01;
            jmp addrDisplaySkipCondition2;
L01:
            push 0x190;
            jmp addrDisplaySkipCondition1;
        }
    }

    ASM void ed6DisplayStatusPatch()
    {
        __asm
        {
            call addrDisplayStatusPatch2;
            mov edx, ebp;
            sub edx, 0x2358;
            push edx;
            mov ecx, esi;
            call CBattleInfoBox::ed6DisplayStatus;
            jmp addrDisplayStatusPatch1;
        }
    }

    NoInline
    void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
    {
        ASM_DUMMY_AUTO();
    }
    void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
    {
        if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99)
        {
            if (AT > 99)
            {
                if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99) return;
                AT = 99;
            }

            if (AT < 10)
            {
                x -= (8.f * 2 - (16.f * resolution->cx / 640.f - 8.f * 1) / 2);
                //x += -10.f;
                //x += -6.8f;
            }
            else
            {
                x -= (8.f * 1 - (16.f * resolution->cx / 640.f - 8.f * 2) / 2);
                //x += -6.f;
                //x += -3.3f;
            }
        }

        // x-=16 y+=16
        y = (y - 16.f) + 16.f * resolution->cy / 480.f;

        //ed6ShowConditionAtOld(AT, x, y, width * resolution->cx / 640.f, height * resolution->cy / 480.f, a6, a7, a8);
        ed6ShowConditionAtOld(AT, x, y, width, height, a6, a7, nConditionATColor);
    }
    void __cdecl ed6ShowConditionAtNewLi(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
    {
        ed6ShowConditionAtNew(AT, x, y - 8.f * resolution->cy / 480.f, width, height, a6, a7, color);
    }

    void ed6SetEnemyFinalStatusPsp(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
    {
#define CALC_STATUS(_status, _rate) \
        result = pStatusSum->_status * rate_final._rate / 100; \
        if (pStatusSum->_status > 0 && result == 0) \
        { \
            result = 1; \
        } \
        pStatusSum->_status = (TYPE_OF(pStatusSum->_status))result;
//#end def

        int difficulty = get_difficulty();
        int revise = STATUS_REVISE[difficulty];
        ED6_STATUS* pStatusSum = &lpBattleInf->StatusSum;
      
        //if (battle_index == 0x7A0)
        if (lpBattleInf->MSFileIndex.file == 0x3001A5 || lpBattleInf->MSFileIndex.file == 0x3001AA)
        {
            switch (difficulty)
            {
            case DIFFICULTY_PS_NIGHTMARE:
                revise += 5;
                break;
            case DIFFICULTY_PS_HARD:
                revise += 2;
                break;
            case DIFFICULTY_PS_EASY:
                revise += 3;
                break;
            }
        }
        //else if (battle_index == 0x394 && difficulty != DIFFICULTY_PS_NORMAL)
        else if (lpBattleInf->MSFileIndex.file == 0x3002A7 && difficulty != DIFFICULTY_PS_NORMAL)
        {
            revise += 2;
        }

        if (revise == 0)
        {
            return;
        }
        int rate = 100 - 5 * revise;
        if (rate < 10)
        {
            rate = 10;
        }
        SSTATUS_REVISE_DIFFICULTY   revise_dif;
        SSTATUS_RATE_MINI           rate_final;

        if (rate > 100)
        { 
            if (difficulty == DIFFICULTY_PS_NIGHTMARE)
            {
                revise_dif = {  50, 185,  40, 185,  40, 185 };
            } 
            else
            {
                revise_dif = {  50, 185,  80, 185,  80, 185 };
            }
            for (int i = 0; i < sizeof(revise_dif) / sizeof(revise_dif.entry[0]); ++i)
            {
                rate_final.entry[i] = (rate - 100) * revise_dif.entry[i] / 100 + 100;
            }
        }
        else
        {
            rate_final = { (rate - 100) * 50 / 100 + 100, rate, rate, rate, rate, rate };
        }
        int result;
        CALC_STATUS(HPMax, HP);
        pStatusSum->HP = (TYPE_OF(pStatusSum->HP))result;
        CALC_STATUS(STR, STR);
        CALC_STATUS(DEF, DEF);
        CALC_STATUS(ATS, ATS);
        CALC_STATUS(ADF, ADF);
        CALC_STATUS(SPD, SPD);
        result = pStatusSum->MOV * rate / 100;
        if (difficulty == DIFFICULTY_PS_NIGHTMARE && result != 0)
        {
            ++result;
        }
        else if (difficulty == DIFFICULTY_PS_EASY && (UINT)result > 1)
        {
            --result;
        }
        if (pStatusSum->MOV != 0 && result == 0)
        {
            result = 1;
        }
        pStatusSum->MOV = (TYPE_OF(pStatusSum->MOV))result;
#undef  CALC_STATUS
    }

    VOID CDECL SetChrStatusByEquip(ULONG ChrNo, CHAR_STATUS* pStatus)
    {
        ((TYPE_OF(&SetChrStatusByEquip))StubSetChrStatusByEquip)(ChrNo, pStatus);
        if (bPSP_MODE &&
            CheckEquipment(ChrNo, ITEM_ID::NEKO_SUIT) &&
            CheckEquipment(ChrNo, ITEM_ID::NEKO_SHOE) &&
            CheckEquipment(ChrNo, ITEM_ID::NEKO_BAND) &&
            CheckEquipment(ChrNo, ITEM_ID::NEKO_TAIL))
        {
            pStatus->SPD += 15;
        }
    }

}

namespace NED63
{
    using namespace NED6123;
    using NED6123::sprintf;
    using NED6123::rand;

    namespace ITEM_ID
    {
        using namespace NED6123::ITEM_ID;
        CONST USHORT NEKO_SHOE      = 0x12B;    // 299 猫爪靴
        CONST USHORT NEKO_TAIL      = 0x13E;    // 318 猫尾

        using NED62::ITEM_ID::DEX;
        using NED62::ITEM_ID::AGL;

        CONST USHORT DRIVE3         = 0x2EE;    // 750 刻耀珠
        CONST USHORT DRIVE[3]       = { 0x2C6, 0x2C7, 0x2EE };
    }

    BOOL get_status_rev_special(SSTATUS_REVISE_SPECIAL* rev, SSTATUS_REVISE_SPECIAL* rev_out, ULONG ms_file);
    INT  calc_specific_status(int type, int revise, int value, SSTATUS_REVISE_SPECIAL* revise_special);
    bool STDCALL CheckCraftMirror(PMONSTER_STATUS src, PMONSTER_STATUS dst);
    bool STDCALL CheckArtsMirror (PMONSTER_STATUS src, PMONSTER_STATUS dst);
    VOID CheckMirrorWhenPreviewAtDelay();

    NoInline
    bool CheckCraftMirrorWithType(PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type = -1);
    NoInline
    bool CheckArtsMirrorWithType (PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type = -1);
    bool FASTCALL CheckMirrorWithType(PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type);

    #define _ED63_NS_
    #include "ed6_ns_common.h"
    #undef  _ED63_NS_

    ASM void ed6DisplaySkipCondition()
    {
        __asm
        {
            cmp DWORD PTR DS:[ESI+0x60B0],0x1;
            jne L01;
            jmp addrDisplaySkipCondition2;
            //ret;
L01:
            push 0x190;
            jmp addrDisplaySkipCondition1;
            //push 0x44A6FE;
            //ret;
        }
    }

    ASM void ed6DisplayBattleIcoEx()
    {
        __asm
        {
            test g_bDisplayBattleIcoEx,0x1;
            je L01;
            mov ecx,battleIcoRec.x;
            mov eax,battleIcoRec.y;
            add ecx,edx;
            add eax,ebp;
            and g_bDisplayBattleIcoEx,0x0;
            ret;
L01:
            LEA EAX,DWORD PTR SS:[EBP+0x10];
            LEA ECX,DWORD PTR DS:[EDX+0x10];
            ret;
        }
    }

    FILE_IN_DIR* getFileInDirInf(FileIndex* fileIndex)
    {
        return (FILE_IN_DIR*)(*(addrLPDir0 + (fileIndex->DatNo))) + fileIndex->Index;
    }

    ASM void ed6DisplayStatusPatch()
    {
        __asm
        {
            cmp DWORD PTR DS:[ESI+0x1C38],0x14;
            jne L01;
            mov DWORD PTR DS:[ESI+0x1C38],0xE; //行间距 仿p bug？ [ESI+15] 02757E6D
L01:
            call addrDisplayStatusPatch2;
            mov edx, ebx;
            sub edx, 0x2370;
            push edx;
            mov ecx, esi;
            call CBattleInfoBox::ed6DisplayStatus;
            jmp addrDisplayStatusPatch1;
        }
    }

    VOID THISCALL CBattleInfoBox::ed6DisplayItemDrop(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
    {

#if 0
        {
            SetTextSize(5);
            int i = 0, x = 0, y = 0x80;
            while (i < 0x16)
            {
                DrawSimpleText(x, y, "■", i);
                ++i;
                x += 0x1C;

                if (i % 4 == 0)
                {
                    x = 0;
                    y += 0x1C;
                }
            }
            return;
        }
#endif

        DWORD   dwY = 0x16;
        ED6_DROP_ITEM* lpDropItem[2];

        char szBuffer[0x120];

        int i, j, stealMin, steal[2];
        int length;
        ULONG line_count;
        stealMin = 100;

        /*偷窃 位置*/
        for (i = 0; i < 2; i++)
        {
            if (lpBattleInf->DropProbability[i] == 0 || lpBattleInf->DropIndex[i] == 0xFFFF)
            {
                lpDropItem[i] = nullptr;
                continue;
            }
;
            lpDropItem[i] = (ED6_DROP_ITEM*)getItemDrop(lpBattleInf->DropIndex[i]);
            if (lpDropItem[i] == nullptr)
            {
                continue;
            }

            for (j = 0; j < 8; j++)
            {
                if (lpDropItem[i]->item[j] == 0 || lpDropItem[i]->item[j] == 0xFFFF || lpDropItem[i]->probability[j] == 0)
                {
                    continue;
                }
                if (lpDropItem[i]->probability[j] <= stealMin)
                {
                    stealMin = lpDropItem[i]->probability[j];
                    steal[0] = i;
                    steal[1] = j;
                }
            }
        }

        DrawSimpleText(0, dwY, "Item:", COLOR_TITLE);
        dwY+=0xC;

        for (i = 0; i < 2; i++)
        {
            if (lpDropItem[i] == nullptr)
            {
                continue;
            }

            if (i == 1)
            {
                dwY+=0x8;
            }

            //sprintf(szBuffer, "[%3d%%]Group%d", lpBattleInf->DropProbability[i], lpBattleInf->DropIndex[i]);
            if (bShowCraftName)
            {
                sprintf(szBuffer, "[%3d%%]%s%d", lpBattleInf->DropProbability[i], CodePage == 936? "宝箱" : "曮敔", lpBattleInf->DropIndex[i]);
            }
            else
            {
                sprintf(szBuffer, "[%3d%%]%s%d", lpBattleInf->DropProbability[i], CodePage == 936? "宝箱" : "曮敔", i+1);
            }

            DrawSimpleText(0, dwY, szBuffer, COLOR_TITLE);
            dwY+=0xC;

            for (j = 0; j < 8; j++)
            {
                if (lpDropItem[i]->item[j] == 0 || lpDropItem[i]->item[j] == 0xFFFF || lpDropItem[i]->probability[j] == 0)
                {
                    continue;
                }

                length = sprintf(szBuffer, "[%3d%%]%s", lpDropItem[i]->probability[j], getItemName(lpDropItem[i]->item[j]));

                if (steal[0] == i && steal[1] == j)
                {
                    //DrawSimpleText(0, dwY, szBuffer, COLOR_GRAY);
                    line_count = DrawSimpleTextMultiline(0, dwY, 6, 0xC, szBuffer, length, COLOR_GRAY);
                }
                else
                {
                    //DrawSimpleText(0, dwY, szBuffer);
                    line_count = DrawSimpleTextMultiline(0, dwY, 6, 0xC, szBuffer, length);
                }
                dwY += 0xC * line_count;
            }
        }

        /*显示分界线*/
        SetTextSize(0);
        DrawSimpleText(0, dwY, (PCSTR)lpcStatusLine, COLOR_LINE);
        SetTextSize(1);
        dwY += 0xA;

        if (!bShowCraftName)
        {
            return;
        }

        DrawSimpleText(0, dwY, "Craft Named:", COLOR_TITLE);
        dwY += 0xC;

        for (i = 0; i < 16; i++)
        {
            if (lpBattleInf->CraftIntro[i].Name[0] == 0 || *(USHORT*)(lpBattleInf->CraftIntro[i].Name) == 0x20)
            {
                continue;
            }
            length = sprintf(szBuffer, "%X:%s", i, lpBattleInf->CraftIntro[i].Name);
            //DrawSimpleText(0, dwY, szBuffer);
            line_count = DrawSimpleTextMultiline(0, dwY, 2, 0xC, szBuffer, length);
            dwY += 0xC * line_count;
        }

        /*显示分界线*/
        SetTextSize(0);
        DrawSimpleText(0, dwY, (PCSTR)lpcStatusLine, COLOR_LINE);
        SetTextSize(1);
        dwY += 0xA;

        DrawSimpleText(0, dwY, "Misc:", COLOR_TITLE);
        dwY += 0xC;

        for (i = 0; i < 4; i++)
        {
            switch (i)
            {
            case 0:
                sprintf(szBuffer, "BattleIndex:0x%X", *addrBattleNowIndex);
                break;
            case 1:
                sprintf(szBuffer, "DT%02X.%03X:%s", lpBattleInf->MSFileIndex.DatNo, lpBattleInf->MSFileIndex.Index,
                        getFileInDirInf(&lpBattleInf->MSFileIndex)->fileName);
                break;
            case 2:
                sprintf(szBuffer, "DT%02X.%03X:%s", lpBattleInf->ASFileIndex.DatNo, lpBattleInf->ASFileIndex.Index,
                        getFileInDirInf(&lpBattleInf->ASFileIndex)->fileName);
                break;
            case 3:
                sprintf(szBuffer, "DT%02X.%03X:%s", lpBattleInf->SYFileIndex.DatNo, lpBattleInf->SYFileIndex.Index,
                        getFileInDirInf(&lpBattleInf->SYFileIndex)->fileName);
                break;
            }

            DrawSimpleText(0, dwY, szBuffer);
            dwY += 0xC;
        }

    }

    VOID THISCALL CBattleInfoBox::DrawMonsterInfo()
    {
        static int previous;
        PBYTE pFlag = (PBYTE)PtrAdd(this, 0x60B5);  // re draw?

        if ((*pFlag & 1) || (g_bShowExtraInfo == FALSE) || (*(PULONG)PtrAdd(this, 0x60B0) == 0))
        {
            g_bShowInfoPage2 = 0;
            previous = 0;
        }
        else if (previous != g_bShowInfoPage2)
        {
            *pFlag |= 1;
            previous = g_bShowInfoPage2;
        }

        (this->*StubDrawMonsterInfo)();
    }

    ASM void ed6DisplayBattleSepith()
    {
        __asm
        {
            mov     ecx, 0x21C;
            cmp     ebp, ecx;
            cmova   ebp, ecx;
            retn;
        }
    }

    ASM void ed6DisplayItemDropPatch()
    {
        __asm
        {
/*          cmp DWORD PTR DS:[ESI+0x60B0],0x1;
            jne L02;
            MOV ECX,DWORD PTR DS:[ESI+0x6080];
            call getSoldierAddr;
            mov ax,[EAX]ED6_CHARACTER_BATTLE_INF.wUseMagic
            cmp ax, 0x5E;
            je L01;
            cmp ax, 0x5F;
            je L01;
            cmp ax, 0x41;
            je L01;
L02:
*/
            cmp g_bShowInfoPage2,1;
            je L01;
            push 0x190;
            jmp addrDisplayItemDropPatch1;
L01:
            push ebx;
            mov ecx, esi;
            call CBattleInfoBox::ed6DisplayItemDrop;
            ; mov g_bShowInfoPage2, 0;
            jmp addrDisplayStatusPatch1;
        }
    }

    NoInline
    void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, ULONG color)
    {
        ASM_DUMMY_AUTO();
    }
    void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, ULONG color)
    {
        if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99)
        {
            if (AT > 99)
            {
                if (nShowConditionAT == SHOW_CONDITION_AT_HIDE99) return;
                AT = 99;
            }

            if (AT < 10)
            {
                x += (16.f * resolution->cx / 640.f - 8.f * 1) / 2;
                //x += 8.0f;
            }
            else
            {
                x += (16.f * resolution->cx / 640.f - 8.f * 2) / 2;
                //x += 3.5f;
            }
        }
        //x += 14.0f;
        //y += 2.0f;

        x += 16.f;
        y = (y - 16.f) + 16.f * resolution->cy / 480.f;
        ed6ShowConditionAtOld(AT, x, y, nConditionATColor);
    }

    VOID HandleKeyUp(USHORT key)
    {
        switch (key)
        {
            case VK_PRIOR:
            case VK_NEXT:
                g_bShowInfoPage2 ^= TRUE;
                break;
        }
    }

    ULONG CBattleInfoBox::DrawSimpleTextMultiline(LONG x, LONG y, LONG indent, ULONG height, PCSTR text, INT length /* = 0 */, ULONG color_index /* = COLOR_WHITE */, LONG weight /* = FW_NORMAL */)
    {
        if (text == nullptr || length < 0 || *text == 0)
        {
            return 0;
        }
        if (length == 0)
        {
            length = strlen(text);
        }

        INT line1_length_max = 10 * 0xC * 2 / height;
        if (length <= line1_length_max + 1)
        {
            DrawSimpleText(x, y, text, color_index, weight);
            return 1;
        }

        if (indent >= line1_length_max)
        {
            indent = 0;
        }
        INT line2_length_max = line1_length_max - indent;
        INT line_length;

        BOOL    is_end = FALSE;
        ULONG   line_no = 0;
        CHAR    buffer[32];
        PCHAR   line_start = (PCHAR)text;
        PCHAR   current, next;

        current = line_start;
        for (;;)
        {
            for (;;)
            {
                next = CharNextExA(CodePage, current, 0);

                if (next - line_start > (line_no == 0 ? line1_length_max : line2_length_max))
                {
                    break;
                }
                if (*next == 0)
                {
                    is_end = TRUE;
                    current = next;
                    break;
                }
                current = next;
            }
            line_length = current - line_start;
            memcpy(buffer, line_start, line_length);
            buffer[line_length] = 0;
            length -= line_length;
            DrawSimpleText(x + (line_no == 0 ? 0 : indent * height / 2), y + line_no * height, buffer, color_index, weight);

            if (is_end)
            {
                return line_no + 1;
            }
            ++line_no;
            if (length <= line2_length_max + 1)
            {
                DrawSimpleText(x + indent * height / 2, y + line_no * height, current, color_index, weight);
                return line_no + 1;
            }
            line_start = current;
        }
    }

    BOOL get_status_rev_special(SSTATUS_REVISE_SPECIAL* rev, SSTATUS_REVISE_SPECIAL* rev_out, ULONG ms_file)
    {
        while (rev->MSFile)
        {
            if (rev->MSFile == ms_file)
            {
                if(rev_out)
                {
                    memcpy(rev_out, rev, sizeof(*rev_out));
                }
                return TRUE;
            }
            ++rev;
        }
        if(rev_out)
        {
            memset(rev_out, 0, sizeof(*rev_out));
            rev_out->MSFile = ms_file;
            for(int i = 0; i < countof(rev_out->entry); ++i)
            {
                rev_out->entry[i] = 100;
            }
        }
        return FALSE;
    }

    //NoInline
    INT calc_specific_status(int type, int revise, int value, SSTATUS_REVISE_SPECIAL* revise_special)
    {
        int difficulty = get_difficulty();
        if (type == STATUS_TYPE_RESIST)
        {
            if (difficulty == DIFFICULTY_PS_NIGHTMARE)
            {
                return value | (revise_special->N_RESIST & 0xFBFFFFFF);
            }
            else if (difficulty == DIFFICULTY_PS_EASY)
            {
                return value & ~(revise_special->E_RESIST & 0xFBFFFFFF);
            }
            else
            {
                return value;
            }
        }
        if (revise == 0)
        {
            return value;
        }
        int rate = 100 - 5 * revise;
        int result = value;
        if (rate < 10)
        {
            rate = 10;
        }
        SSTATUS_REVISE_DIFFICULTY *revise_dif = t_btrev1;
        if (type < STATUS_TYPE_MOV)
        { 
            if (rate > 100)
            {
                USHORT revise1 = revise_dif[difficulty].entry[type];
                USHORT revise2 = revise_special->entry[type];
                // bug?
                if (type == STATUS_TYPE_SPD && !bFixEnemyStatusBug)
                {
                    revise1 = revise2;
                }
                rate = (rate - 100) * revise1 / 100 + 100; 

                if (rate > 100)
                {
                    rate = (rate - 100) * revise2 / 100 + 100;
                }
            }
            if (rate < 0)
            {
                rate = 0;
            }
            result = value * rate / 100;
        }
        else if (type == STATUS_TYPE_MOV)
        {
            for(;;)
            {
                INT AI;
                if (difficulty == DIFFICULTY_PS_NIGHTMARE)
                {
                    AI = revise_special->N_AI;
                }
                else if (difficulty == DIFFICULTY_PS_EASY)
                {
                    AI = revise_special->E_AI;
                }
                else
                {
                    break;
                }

                if (FLAG_ON(AI, BTREV_AI_INC_MOV))
                {
                    result = value + 2;
                }
                else if (FLAG_ON(AI, BTREV_AI_DEC_MOV) && value != 0)
                {
                    result = (value + 1) >> 1;
                }
                break;
            }
        }

        if (value != 0 && result == 0)
        {
            result = 1;
        }
        return result;
    }

    //NoInline
    VOID change_ai_probability(ED6_AI_INFO* ai_info, int count, int type)
    {
        int result;
        for (; count; --count, ++ai_info)
        {
            result = ai_info->Probability;
            if (result == 0)
            {
                continue;
            }
            if (type == BTREV_AI_PROBABILITY_INC)
            {
                if (result >= 75)
                {
                    continue;
                }
                result += result >> 1;
                if (result > 75)
                {
                    result = 75;
                }
                ai_info->Probability = (ubyte)result;
            }
            else if (type == BTREV_AI_PROBABILITY_DEC)
            {
                result >>= 1;
                if (result == 0)
                {
                    result = 1;
                }
                ai_info->Probability = (ubyte)result;
            }
        }
    }

    VOID ed6SetEnemyFinalStatusPsp(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
    {
        int difficulty = get_difficulty();
        int revise = STATUS_REVISE[difficulty];
        ED6_STATUS* pStatusSum = &lpBattleInf->StatusSum;

        if (revise == 0)
        {
            return;
        }
        SSTATUS_REVISE_SPECIAL revise_special;
        get_status_rev_special(t_btrev2, &revise_special, lpBattleInf->MSFileIndex.file);
        if (FLAG_ON(revise_special.N_RESIST, 0x4000000) && difficulty == DIFFICULTY_PS_NIGHTMARE)
        {
            SET_FLAG(lpBattleInf->HitFlag, CHR_FLAG_ResistATDelay);
        }
        if (FLAG_ON(revise_special.E_RESIST, 0x4000000) && difficulty == DIFFICULTY_PS_EASY)
        {
            CLEAR_FLAG(lpBattleInf->HitFlag, CHR_FLAG_ResistATDelay);
        }
        pStatusSum->HPMax       = (TYPE_OF(pStatusSum->HPMax))calc_specific_status(STATUS_TYPE_HP, revise, pStatusSum->HPMax, &revise_special);
        pStatusSum->HP          = pStatusSum->HPMax;
        pStatusSum->STR         = (TYPE_OF(pStatusSum->STR))calc_specific_status(STATUS_TYPE_STR, revise, pStatusSum->STR, &revise_special);
        pStatusSum->DEF         = (TYPE_OF(pStatusSum->DEF))calc_specific_status(STATUS_TYPE_DEF, revise, pStatusSum->DEF, &revise_special);
        pStatusSum->ATS         = (TYPE_OF(pStatusSum->ATS))calc_specific_status(STATUS_TYPE_ATS, revise, pStatusSum->ATS, &revise_special);
        pStatusSum->ADF         = (TYPE_OF(pStatusSum->ADF))calc_specific_status(STATUS_TYPE_ADF, revise, pStatusSum->ADF, &revise_special);
        pStatusSum->SPD         = (TYPE_OF(pStatusSum->SPD))calc_specific_status(STATUS_TYPE_SPD, revise, pStatusSum->SPD, &revise_special);
        pStatusSum->MOV         = (TYPE_OF(pStatusSum->MOV))calc_specific_status(STATUS_TYPE_MOV, revise, pStatusSum->MOV, &revise_special);
        lpBattleInf->Resistance = calc_specific_status(STATUS_TYPE_RESIST, revise, lpBattleInf->Resistance, &revise_special);
        // flag not set yet
        /*if (FLAG_OFF(lpBattleInf->RoleFlag, CHR_FLAG_ENEMY))
        {
            return;
        }*/
        INT revise_ai = 0;
        if (difficulty == DIFFICULTY_PS_NIGHTMARE)
        {
            revise_ai = revise_special.N_AI;
        }
        else if (difficulty == DIFFICULTY_PS_EASY)
        {
            revise_ai = revise_special.E_AI;
        }
        if (revise_ai == 0)
        {
            return;
        }

        // change AIType
        if (lpBattleInf->AIType < 3)
        {
            if (FLAG_ON(revise_ai, BTREV_AI_SET_0))
            {
                lpBattleInf->AIType = 0;
            }
            if (FLAG_ON(revise_ai, BTREV_AI_SET_1))
            {
                lpBattleInf->AIType = 1;
            }
            if (FLAG_ON(revise_ai, BTREV_AI_SET_2))
            {
                lpBattleInf->AIType = 2;
            }
        }

        // change ai probability
        if (FLAG_ON(revise_ai, BTREV_AI_INC_ARTS))
        {
            change_ai_probability(lpBattleInf->Arts, countof(lpBattleInf->Arts), BTREV_AI_PROBABILITY_INC);
        }
        if (FLAG_ON(revise_ai, BTREV_AI_DEC_ARTS))
        {
            change_ai_probability(lpBattleInf->Arts, countof(lpBattleInf->Arts), BTREV_AI_PROBABILITY_DEC);
        }
        if (FLAG_ON(revise_ai, BTREV_AI_INC_CRAFT))
        {
            change_ai_probability(lpBattleInf->Craft, countof(lpBattleInf->Craft), BTREV_AI_PROBABILITY_INC);
        }
        if (FLAG_ON(revise_ai, BTREV_AI_DEC_CRAFT))
        {
            change_ai_probability(lpBattleInf->Craft, countof(lpBattleInf->Craft), BTREV_AI_PROBABILITY_DEC);
        }
        if (FLAG_ON(revise_ai, BTREV_AI_INC_SCRAFT))
        {
            change_ai_probability(lpBattleInf->SCraft, countof(lpBattleInf->SCraft), BTREV_AI_PROBABILITY_INC);
        }
        if (FLAG_ON(revise_ai, BTREV_AI_DEC_SCRAFT))
        {
            change_ai_probability(lpBattleInf->SCraft, countof(lpBattleInf->SCraft), BTREV_AI_PROBABILITY_DEC);
        }

        // summon number
        if (FLAG_ON(revise_ai, BTREV_AI_INC_SUMMON | BTREV_AI_DEC_SUMMON))
        {
            ED6_CRAFT_INFO* craft;
            FOR_EACH_ARRAY(craft, lpBattleInf->CraftInf)
            {
                if (craft->AnimationType == 0x134 || craft->AnimationType == 0x138)
                {
                    if (FLAG_ON(revise_ai, BTREV_AI_INC_SUMMON))
                    {
                        if (craft->Effect2Parameter != 0)
                        {
                            craft->Effect2Parameter = 0xFF;
                        }
                    }
                    else if (craft->Effect2Parameter > 1)
                    {
                        craft->Effect2Parameter = 1;
                    }
                }
            }
        }
    }
/*
    VOID print_status_rev_special(SSTATUS_REVISE_SPECIAL* p)
    {
        for(;;)
        {
            WriteLogA("{ %#x,\t%3d,\t%3d,\t%3d,\t%3d,\t%3d,\t%3d,\t%#10x,\t%#10x,\t%#10x,\t%#10x, },\r\n", 
                p->MSFile, p->HP, p->STR, p->DEF, p->ATS, p->ADF, p->SPD,
                p->N_RESIST, p->E_RESIST, p->N_AI, p->E_AI);
            if (p->MSFile == 0)
            {
                break;
            }
            ++p;
        }
    }

    bool THISCALL CBattle::LoadStatusData(ULONG MSFile, ULONG ChrPosition, ULONG a3)
    {
        static BOOL done = FALSE;
        if (!done)
        {
            done = TRUE;
            dump_status_rev_special(ChrPosition);
        }
        return (this->*StubLoadStatusData)(MSFile, ChrPosition, a3);      
    }

    VOID THISCALL CBattle::dump_status_rev_special(ULONG ChrPosition)
    {
        SSTATUS_REVISE_SPECIAL* p = t_btrev2;
        ED6_CHARACTER_BATTLE_INF* lpBattleInf = getChrBattleInf() + ChrPosition;
        for(;;)
        {
            WriteLogA("{ %#x,\t%3d,\t%3d,\t%3d,\t%3d,\t%3d,\t%3d,\t%#10x,\t%#10x,\t%#10x,\t%#10x, },", 
                p->MSFile, p->HP, p->STR, p->DEF, p->ATS, p->ADF, p->SPD,
                p->N_RESIST, p->E_RESIST, p->N_AI, p->E_AI);
            if (p->MSFile == 0)
            {
                break;
            }
            LoadStatusData(p->MSFile, ChrPosition);
            WriteLogA(" // %s\r\n", lpBattleInf->ChrName);
            ++p;
        }
    }*/

    VOID THISCALL CBattle::InitBeforeAtIconUp()
    {
        PMONSTER_STATUS lpBattleInf;
        FOR_EACH(lpBattleInf, getChrBattleInf(), 0x10)
        {
            if (FLAG_OFF(lpBattleInf->RoleFlag, CHR_FLAG_EMPTY))
            {
                half_mirror_rand[lpBattleInf->SoldierNo] = rand() & 1;
                WriteConsoleLogW(L"%d.%d ", lpBattleInf->SoldierNo, half_mirror_rand[lpBattleInf->SoldierNo]);
            }
        }
        WriteConsoleLogW(L"\r\n");
        return (this->*StubInitBeforeAtIconUp)();
    }

    bool STDCALL CheckCraftMirror(PMONSTER_STATUS src, PMONSTER_STATUS dst)
    {
        return CheckCraftMirrorWithType(src, dst);
    }
    bool CheckCraftMirrorWithType(PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type /* = -1 */)
    {
        if (!bPSP_MODE)
        {
            return ((TYPE_OF(&CheckCraftMirror))StubCheckCraftMirror)(src, dst);
        }

        if (src == dst ||
            (FLAG_ON(src->RoleFlag, CHR_FLAG_PARTY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_PARTY)))
        {
            return false;
        }

        if (action_type == -1)
        {
            action_type= src->CurrentActionType;
        }
        if (action_type == ACTION_ATTACK || 
            action_type == ACTION_CRAFT  || 
            action_type == ACTION_SCRAFT ||
            action_type == ACTION_CAST_CRAFT)
        {
            if ((CheckQuartz(dst->SoldierNo, ITEM_ID::CRAFT_MIRROR) ||
                (CheckQuartz(dst->SoldierNo, ITEM_ID::SHINKYO) && half_mirror_rand[dst->SoldierNo])) &&
                FLAG_OFF(GetCraftInf(src, src->CurrentCraftIndex)->SpecialEffect, CRAFT_INFO::SpecialEffects::IGNORE_MIRROR))
            {
                WriteConsoleLogA("CRAFT_MIRROR:[%d]%s hit [%d]%s\r\n", src->SoldierNo, src->ChrName, dst->SoldierNo, dst->ChrName);
                return true;
            }
        }
        return false;
    }

    bool STDCALL CheckArtsMirror(PMONSTER_STATUS src, PMONSTER_STATUS dst)
    {
        return CheckArtsMirrorWithType(src, dst);
    }
    bool CheckArtsMirrorWithType(PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type /* = -1 */)
    {
        if (!bPSP_MODE)
        {
            return ((TYPE_OF(&CheckArtsMirror))StubCheckArtsMirror)(src, dst);
        }

        if (src == dst ||
            (FLAG_ON(src->RoleFlag, CHR_FLAG_PARTY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_PARTY)))
        {
            return false;
        }

        if (action_type == -1)
        {
            action_type= src->CurrentActionType;
        }
        if (action_type == ACTION_ARTS)
        {
            if ((CheckQuartz(dst->SoldierNo, ITEM_ID::ARTS_MIRROR) ||
                (CheckQuartz(dst->SoldierNo, ITEM_ID::MAKYO) && half_mirror_rand[dst->SoldierNo])) &&
                FLAG_OFF(GetCraftInf(src, src->CurrentCraftIndex)->SpecialEffect, CRAFT_INFO::SpecialEffects::IGNORE_MIRROR))
            {
                WriteConsoleLogA("ARTS_MIRROR:[%d]%s hit [%d]%s\r\n", src->SoldierNo, src->ChrName, dst->SoldierNo, dst->ChrName);
                return true;
            }
        }
        return false;
    }
    bool FASTCALL CheckMirrorWithType(PMONSTER_STATUS src, PMONSTER_STATUS dst, LONG action_type)
    {
        WriteConsoleLogA("Check_MIRROR:[%d]%s hit [%d]%s type:%d\r\n", src->SoldierNo, src->ChrName, dst->SoldierNo, dst->ChrName, action_type);
        if (CheckCraftMirrorWithType(src, dst, action_type) || CheckArtsMirrorWithType(src, dst, action_type))
        {
            return true;
        }
        return false;
    }
    ASM VOID CheckMirrorWhenPreviewAtDelay()
    {
        INLINE_ASM
        {
            movzx   eax, ax;
            push    eax;
            mov     edx, esi;
            mov     ecx, ebp;
            call    CheckMirrorWithType;
            test    al, al;
            je      jumpback;
            xor     bl, bl;
        jumpback:
            jmp     addrCheckMirrorWPADBack;
        }
    }
}

enum GameVersion
{
    ed6VersionUnknown = 0,
    ed61min = 0x10000,
    ed61cn7,
    ed61jp7,

    ed62min = 0x20000,
    ed62cn7,
    ed62jp7,
    ed62jp1020,
    ed62cn1020fc,
    ed62cn1020fcli,

    ed63min = 0x40000,
    ed63cn7,
    ed63jp7,
    ed63jp1002,
} g_GameVersion;

GameVersion getGameVersion()
{
    GameVersion gameVersion;
    __try
    {
        //__asm INT 3;
        // battleInf[0] t_magic
        if (*(UINT*)0x401C93 == 0x672828 && *(UINT*)0x4A01F2 == 0x2DED814)
        {
            gameVersion = ed63cn7;
        }
        else if (*(UINT*)0x00401C33 == 0x60EF38 && *(UINT*)0x004C6342 == 0x2F2B9F8)
        {
            gameVersion = ed62cn7;
        }
        else if (*(UINT*)0x004019FF == 0x5A58D0 && *(UINT*)0x004A8492 == 0x1941780)
        {
            gameVersion = ed61cn7;
        }
        else if (*(UINT*)0x00401C93 == 0x66E780 && *(UINT*)0x0049F6F2 == 0x2DEADD0)
        {
            gameVersion = ed63jp7;
        }
        else if (*(UINT*)0x00401C33 == 0x60EDD8 && *(UINT*)0x004C5E92 == 0x2F2CDD8)
        {
            gameVersion = ed62jp7;
        }
        else if (*(UINT*)0x004019FF == 0x5A8370 && *(UINT*)0x004A8442 == 0x1944220)
        {
            gameVersion = ed61jp7;
        }
        else if (*(UINT*)0x00401C93 == 0x66E640 && *(UINT*)0x0049F4C2 == 0x2DE9338)
        {
            gameVersion = ed63jp1002;
        }
        else if (*(UINT*)0x00401C36 == 0x60DC98 && *(UINT*)0x004C5992 == 0x2F2A338)
        {
            gameVersion = ed62jp1020;
        }
        else
        {
            gameVersion = ed6VersionUnknown;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        gameVersion = ed6VersionUnknown;
    }

    return gameVersion;
}

void ConfigInit()
{
    WCHAR szConfigExPath[] = L".\\configEx.ini";
    typedef struct
    {
        PVOID       Var;
        CHAR        Type;
        LPCWSTR     lpAppName;
        LPCWSTR     lpKeyName;
        union
        {
            BOOL    bDefault;
            INT     nDefault;
            LPCWSTR lpDefault;
        };
        //LPCWSTR     lpFileName;
    } CONFIG_ENTRY;

    static CONFIG_ENTRY Config[] =
    {
        { (INT*)&nDifficulty,                   'i',    L"Battle",  L"Difficulty",              DIFFICULTY_DEFAULT,      },
        { (INT*)&sRate.HP_a,                    'i',    L"Battle",  L"HP_a",                    1000,   },
        { (INT*)&sRate.STR_a,                   'i',    L"Battle",  L"STR_a",                   1000,   },
        { (INT*)&sRate.DEF_a,                   'i',    L"Battle",  L"DEF_a",                   1000,   },
        { (INT*)&sRate.ATS_a,                   'i',    L"Battle",  L"ATS_a",                   1000,   },
        { (INT*)&sRate.ADF_a,                   'i',    L"Battle",  L"ADF_a",                   1000,   },
        { (INT*)&sRate.SPD_a,                   'i',    L"Battle",  L"SPD_a",                   1000,   },
        { (INT*)&sRate.DEX_a,                   'i',    L"Battle",  L"DEX_a",                   1000,   },
        { (INT*)&sRate.AGL_a,                   'i',    L"Battle",  L"AGL_a",                   1000,   },
        { (INT*)&sRate.MOV_a,                   'i',    L"Battle",  L"MOV_a",                   1000,   },

        { (INT*)&sRate.HP_b,                    'i',    L"Battle",  L"HP_b",                    0,      },
        { (INT*)&sRate.STR_b,                   'i',    L"Battle",  L"STR_b",                   0,      },
        { (INT*)&sRate.DEF_b,                   'i',    L"Battle",  L"DEF_b",                   0,      },
        { (INT*)&sRate.ATS_b,                   'i',    L"Battle",  L"ATS_b",                   0,      },
        { (INT*)&sRate.ADF_b,                   'i',    L"Battle",  L"ADF_b",                   0,      },
        { (INT*)&sRate.SPD_b,                   'i',    L"Battle",  L"SPD_b",                   0,      },
        { (INT*)&sRate.DEX_b,                   'i',    L"Battle",  L"DEX_b",                   0,      },
        { (INT*)&sRate.AGL_b,                   'i',    L"Battle",  L"AGL_b",                   0,      },
        { (INT*)&sRate.MOV_b,                   'i',    L"Battle",  L"MOV_b",                   0,      },

        { (BOOL*)&sRate.ResistNone,             'b',    L"Battle",  L"ResistNone",              FALSE,  },
        { (BOOL*)&sRate.ResistAbnormalCondition,'b',    L"Battle",  L"ResistAbnormalCondition", FALSE,  },
        { (BOOL*)&sRate.ResistAbilityDown,      'b',    L"Battle",  L"ResistAbilityDown",       FALSE,  },
        { (BOOL*)&sRate.ResistATDelay,          'b',    L"Battle",  L"ResistATDelay",           FALSE,  },

        { (INT*)&nSepithUpLimit,                'i',    L"Battle",  L"SepithUpLimit",           0,      },
        { (INT*)&nShowAT,                       'i',    L"Battle",  L"ShowAT",                  SHOW_AT_DEFAULT,      },
        { (INT*)&nShowConditionAT,              'i',    L"Battle",  L"ShowConditionAT",         SHOW_CONDITION_AT_DEFAULT,      },
        { (INT*)&nConditionATColor,             'i',    L"Battle",  L"ConditionATColor",        0,      },

        { (BOOL*)&bShowCraftName,               'b',    L"Battle",  L"ShowCraftName",           TRUE,   },
        { (BOOL*)&bForceShowMonsInf,            'b',    L"Battle",  L"ForceShowMonsInf",        FALSE,  },
        { (BOOL*)&bUnlimitedSummon,             'b',    L"Battle",  L"UnlimitedSummon",         FALSE,  },
        { (BOOL*)&bFixEnemyStatusBug,           'b',    L"Battle",  L"FixEnemyStatusBug",       FALSE,  },
    };

    CONFIG_ENTRY *Entry;
    FOR_EACH(Entry, Config, countof(Config))
    {
        switch (Entry->Type)
        {
        case 'b':
            *(BOOL*)Entry->Var  = NINI::GetPrivateProfileBoolW(Entry->lpAppName, Entry->lpKeyName, Entry->bDefault, szConfigExPath);
            break;
        case 'i':
            *(INT*)Entry->Var   = NINI::GetPrivateProfileIntW(Entry->lpAppName, Entry->lpKeyName, Entry->nDefault, szConfigExPath);
            break;
        default:
            ;
        }
    }

    // 0-6
    if ((ULONG)nDifficulty > DIFFICULTY_INI_MAX)
    {
        nDifficulty = DIFFICULTY_DEFAULT;
    }
    --nDifficulty;
    if (nDifficulty == DIFFICULTY_PC_ORIGINAL || nDifficulty == DIFFICULTY_PC_CUSTOM)
    {
        bPSP_MODE = FALSE;
    }
    else
    {
        bPSP_MODE = TRUE;
    }

    // sc psp 所有难度不限制分身次数
    if(bPSP_MODE && FLAG_ON(g_GameVersion, ed62min))
    {
        bUnlimitedSummon = TRUE;
    }

    SaturateConvertEx(&nSepithUpLimit, nSepithUpLimit, 9999, 0);
    if (nSepithUpLimit == 0)
    {
        nSepithUpLimit = FLAG_ON(g_GameVersion, ed63min) ? NED63::SEPITH_UP_LIMIT_ORIGINAL : NED62::SEPITH_UP_LIMIT_ORIGINAL;
    }

    if (nShowAT != SHOW_AT_NONE &&
        nShowAT != SHOW_AT_SIMPLE &&
        nShowAT != SHOW_AT_ORIGINAL)
    {
        nShowAT = SHOW_AT_DEFAULT;
    }

    if (nShowConditionAT != SHOW_CONDITION_AT_NONE &&
        nShowConditionAT != SHOW_CONDITION_AT_HIDE99 &&
        nShowConditionAT != SHOW_CONDITION_AT_MAX99 &&
        nShowConditionAT != SHOW_CONDITION_AT_ORIGINAL)
    {
        nShowConditionAT = SHOW_CONDITION_AT_DEFAULT;
    }

    if (nConditionATColor == 0)
    {
        nConditionATColor = FLAG_ON(g_GameVersion, ed63min) ? CONDITION_AT_COLOR_ORIGINAL : CONDITION_AT_COLOR_GREEN;
    }

#if CONSOLE_DEBUG
    FOR_EACH(Entry, Config, countof(Config))
    {
        switch (Entry->Type)
        {
        case 'b':
            PrintConsoleW(L"b:%s:%d\r\n", Entry->lpKeyName, *(BOOL*)Entry->Var);
            break;
        case 'i':
            PrintConsoleW(L"i:%s:%d\r\n", Entry->lpKeyName, *(INT*)Entry->Var);
            break;
        default:
            ;
        }
    }
#endif
}

void patch_ed63cn7(PVOID hModule)
{
    using namespace NED63;

    NED63::sprintf              = (pSprintf)0x59234F;
    getItemInf                  = (pGetAddr)0x4A0270;

    lpfnDrawSimpleText          = 0x0052B170;
    lpfnFtol                    = 0x5922F0;
    lpfnSetTextSize             = 0x529530;
    lpfnDrawBattleIcon          = 0x52BCB0;

    addrDisplaySkipCondition0   = 0x0044A6F9; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x0044A792; // ret2

    lpcStatusLine               = 0x5B7410;
    lpfnEnumCondition           = 0x41B070;
    addrConditionIconPointBegin = 0x5B310C;
    addrConditionIconPointEnd   = 0x5B3214;
    addrBattleTexture           = (PULONG_PTR)0x6A27FC;

    addrDisplayStatusPatch0     = 0x44AC31;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x52B1E0; // call

    addrDisplayBattleIcoEx0     = 0x0052BCFA;
    dwYStart                    = 0x110;

    addrDisplayItemDropPatch0   = 0x0044A68E;
    addrDisplayItemDropPatch1   = addrDisplayItemDropPatch0 + 5;

    addrSoldierNo0              = 0x672828;
    addrLPDir0                  = (PULONG_PTR)0x2CAD950;
    addrBattleNowIndex          = (PULONG_PTR)0x721A38;
    getItemDrop                 = (pGetAddr)0x004A0E60;
    getItemName                 = (pGetAddr)0x004A0370;

    addrChangeEnemyStatusPatch0 = 0x0044CDAF;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004A4360; // call

    addrCheckQuartzPatch        = 0x004A8840;
    lpfnCheckEquipment          = 0x004A3EF0;
    lpfnGetCraftInf             = 0x0040A1C0;
    lpfnFindCondition           = 0x0041C0D0;
    lpfnRand                    = 0x004A1710;
    lpfnRandX                   = 0x004A1790;
    lpDebugFlags                = (PULONG_PTR)0x2DA4DD4;
    lpBattleFlags               = (PULONG_PTR)0x66A438;
    addrGetHitResultPatch       = 0x00416FB0;

    addrDrive3Patch             = 0x0042E32E;

    addrInitBeforeAtIconUpPatch = 0x00402D80;
    addrCheckCraftMirrorPatch   = 0x0042E500;
    addrCheckArtsMirrorPatch    = 0x0042E4A0;
    addrCheckMirrorWPADPatch    = 0x0043E964;
    addrCheckMirrorWPADBack     = 0x0043E9B3;
    addrFixMirrorBugPatch       = 0x0043A400 - 0x00400000;

    resolution                  = (PSIZE)0x005BDFF0; // 分辨率

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2DA4DD8],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x0043CF5F -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x0041C652 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x1B73A0),   // Exp %4d->%5d
        PATCH_MEMORY(0xFC,      1, 0x04A9B7),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0x00,      1, 0x04A9D3),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0xEB,      1, 0x006185),   // party display extra inf
        //  PATCH_MEMORY(0xACE9,    4, 0x04A6E2),   // not display Condition
        PATCH_MEMORY(0x00,      1, 0x04A986),   // display cp
        PATCH_MEMORY(0x30,      1, 0x04A53F),   // height
        PATCH_MEMORY(0x30,      1, 0x04A547),   // height2
        PATCH_MEMORY(0x00,      1, 0x0061BE),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408E97 -0x00400000),
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00406F98 -0x00400000),
        PATCH_MEMORY(0x5A33F0,  4, 0x0041C672 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x5A33F0,  4, 0x0041C67F -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x0041C643 -0x00400000), // 是否显状态AT

        PATCH_MEMORY(0x01,      1, 0x004A64D3 -0x00400000), // LG_Font2
        //PATCH_MEMORY(0xEB,      1, 0x004A64D4 -0x00400000),   // LG_Font1
        //PATCH_MEMORY(0xEB,      1, 0x004A65A6 -0x00400000),   // LG_Font4
        //PATCH_MEMORY(0xEB,      1, 0x004A65EC -0x00400000),   // LG_Font5
        //PATCH_MEMORY(0xEB,      1, 0x004A6630 -0x00400000),   // LG_Font6
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        //PATCH_FUNCTION(JUMP, 0, 0x04A6F9, ed6DisplaySkipCondition, 0),
        //PATCH_FUNCTION(CALL, 0, 0x12BCFA, NED63::ed6DisplayBattleIcoEx, 1),
        //PATCH_FUNCTION(JUMP, 0, 0x04AC31, ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,    ed6DisplayItemDropPatch, 0),
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,           ed6DisplayGetPar, 0, ed6DisplayGetParOld),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, 0x00446DDF,   ed6DisplayBattleSepith, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048D310,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),

        INLINE_HOOK_JUMP(0x0044A5E0,    METHOD_PTR(&CBattleInfoBox::DrawMonsterInfo),   CBattleInfoBox::StubDrawMonsterInfo),
        //INLINE_HOOK_JUMP(0x0041D950,    METHOD_PTR(&CBattle::LoadStatusData), CBattle::StubLoadStatusData),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2DED7AC);
    lpfnHandleKeyUp = HandleKeyUp;
}

void patch_ed63jp7(PVOID hModule)
{
    using namespace NED63;

    NED63::sprintf              = (pSprintf)0x0058C7AF;
    getItemInf                  = (pGetAddr)0x0049F750;

    lpfnDrawSimpleText          = 0x00528B10;
    lpfnFtol                    = 0x0058C750;
    lpfnSetTextSize             = 0x00526F20;
    lpfnDrawBattleIcon          = 0x00529640;

    addrDisplaySkipCondition0   = 0x0044A0B9; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x0044A152; // ret2

    lpcStatusLine               = 0x5B18E8;
    lpfnEnumCondition           = 0x41ADC0;
    addrConditionIconPointBegin = 0x5AD1F4;
    addrConditionIconPointEnd   = 0x5AD2FC;
    addrBattleTexture           = (PULONG_PTR)0x69E750;

    addrDisplayStatusPatch0     = 0x0044A5F2;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x00528B80; // call

    addrDisplayBattleIcoEx0     = 0x0052968A;
    dwYStart                    = 0x112;

    addrDisplayItemDropPatch0   = 0x0044A04E;
    addrDisplayItemDropPatch1   = addrDisplayItemDropPatch0 + 5;

    addrSoldierNo0              = 0x66E780;
    addrLPDir0                  = (PULONG_PTR)0x2CAAFD0;
    addrBattleNowIndex          = (PULONG_PTR)0x71D900;
    getItemDrop                 = (pGetAddr)0x004A0320;
    getItemName                 = (pGetAddr)0x0049F850;

    addrChangeEnemyStatusPatch0 = 0x0044C72F;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004A36A0; // call

    addrCheckQuartzPatch        = 0x004A6EF0;
    lpfnCheckEquipment          = 0x004A3230;
    lpfnGetCraftInf             = 0x0040A1D0;
    lpfnFindCondition           = 0x0041BE20;
    lpfnRand                    = 0x004A0B10;
    lpfnRandX                   = 0x004A0B90;
    lpDebugFlags                = (PULONG_PTR)0x2DA2420;
    lpBattleFlags               = (PULONG_PTR)0x6663D0;
    addrGetHitResultPatch       = 0x00416DC0;

    addrDrive3Patch             = 0x0042DF0E;

    addrInitBeforeAtIconUpPatch = 0x00402D80;
    addrCheckCraftMirrorPatch   = 0x0042E0E0;
    addrCheckArtsMirrorPatch    = 0x0042E080;
    addrCheckMirrorWPADPatch    = 0x0043E454;
    addrCheckMirrorWPADBack     = 0x0043E4A3;
    addrFixMirrorBugPatch       = 0x00439F00 - 0x00400000;

    resolution                  = (PSIZE)0x005B86C0; // 分辨率
    CodePage                    = 932;

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2DA2424],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x0043CA4F -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x0041C3A2 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x1B186C),   // Exp %4d->%5d
        PATCH_MEMORY(0xD4,      1, 0x04A377),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0x00,      1, 0x04A393),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0xEB,      1, 0x006195),   // party display extra inf
        //  PATCH_MEMORY(0xACE9,    4, 0x04A6E2),   // not display Condition
        PATCH_MEMORY(0x00,      1, 0x04A346),   // display cp
        PATCH_MEMORY(0x30,      1, 0x049EFF),   // height
        PATCH_MEMORY(0x30,      1, 0x049F07),   // height2
        PATCH_MEMORY(0x00,      1, 0x0061CE),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408EA7 -0x00400000),
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00406FA8 -0x00400000),
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x0041C393 -0x00400000), // 是否显状态AT
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,    ed6DisplayItemDropPatch, 0),
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,           ed6DisplayGetPar, 0, ed6DisplayGetParOld),
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,       ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, 0x0044677F,   ed6DisplayBattleSepith, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048C710,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),

        INLINE_HOOK_JUMP(0x00449FA0,    METHOD_PTR(&CBattleInfoBox::DrawMonsterInfo),   CBattleInfoBox::StubDrawMonsterInfo),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2DEAD68);
    lpfnHandleKeyUp = HandleKeyUp;
}

void patch_ed63jp1002(PVOID hModule)
{
    using namespace NED63;

    NED63::sprintf              = (pSprintf)0x0058BEEF;
    getItemInf                  = (pGetAddr)0x0049F520;

    lpfnDrawSimpleText          = 0x00528A30;
    lpfnFtol                    = 0x0058BE90;
    lpfnSetTextSize             = 0x00526D60;
    lpfnDrawBattleIcon          = 0x00529560;

    addrDisplaySkipCondition0   = 0x0044A039; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x0044A0D2; // ret2

    lpcStatusLine               = 0x5B18E8;
    lpfnEnumCondition           = 0x0041ADC0;
    addrConditionIconPointBegin = 0x5AD1F4;
    addrConditionIconPointEnd   = 0x5AD2FC;
    addrBattleTexture           = (PULONG_PTR)0x69E610;

    addrDisplayStatusPatch0     = 0x0044A572;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x00528AA0; // call

    addrDisplayBattleIcoEx0     = 0x005295AA;
    dwYStart                    = 0x112;

    addrDisplayItemDropPatch0   = 0x00449FCE;
    addrDisplayItemDropPatch1   = addrDisplayItemDropPatch0 + 5;

    addrSoldierNo0              = 0x66E640;
    addrLPDir0                  = (PULONG_PTR)0x2CA9538;
    addrBattleNowIndex          = (PULONG_PTR)0x71D7C0;
    getItemDrop                 = (pGetAddr)0x004A00F0;
    getItemName                 = (pGetAddr)0x0049F620;

    addrChangeEnemyStatusPatch0 = 0x0044C6AF;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004A35B0; // call

    addrCheckQuartzPatch        = 0x004A6DF0;
    lpfnCheckEquipment          = 0x004A3140;
    lpfnGetCraftInf             = 0x0040A1D0;
    lpfnFindCondition           = 0x0041BE20;
    lpfnRand                    = 0x004A08E0;
    lpfnRandX                   = 0x004A0960;
    lpDebugFlags                = (PULONG_PTR)0x2DA0988;
    lpBattleFlags               = (PULONG_PTR)0x666290;
    addrGetHitResultPatch       = 0x00416DC0;

    addrDrive3Patch             = 0x0042DEFE;

    addrInitBeforeAtIconUpPatch = 0x00402D80;
    addrCheckCraftMirrorPatch   = 0x0042E0D0;
    addrCheckArtsMirrorPatch    = 0x0042E070;
    addrCheckMirrorWPADPatch    = 0x0043E444;
    addrCheckMirrorWPADBack     = 0x0043E493;
    addrFixMirrorBugPatch       = 0x00439EF0 - 0x00400000;

    resolution                  = (PSIZE)0x005B8644; // 分辨率

    unsigned char p0044A548[9] = { 0x6A, 0x01, 0x8B, 0xCE, 0xE8, 0x0F, 0xC8, 0x0D, 0x00 };
    unsigned char p00528A34[5] = { 0xE8, 0xD7, 0xFD, 0xFF, 0xFF };
    unsigned char p0043CA3F[6] = { 0xE9, 0x2E, 0x01, 0x00, 0x00, 0x90 };

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (*(UINT*)0x005B8654 == 0x59977089 && *(UINT*)0x004A0C34 == 0x5B863C) //日版 windows名称
    {
        CodePage = 932;
        MEMORY_FUNCTION_PATCH f1[] =
        {
            PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
            //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,       ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only
        };
        Nt_PatchMemory(NULL, 0, f1, countof(f1), hModule);
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2DA098C],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x0043CA3F -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    //if (*(PUCHAR)0x004035E9 == 0xE9) // 3rd The Other Side
    if (*(PUCHAR)0x0041C398 == 0x75)
    {
        nShowConditionAT |= 1;
    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x0041C3A2 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(p0044A548, 9, 0x04A548),   // 仿p恢复
        //PATCH_MEMORY(0xB4,        1, 0x04A041),   // 仿p恢复 condition
        PATCH_MEMORY(p00528A34, 5, 0x128A34),   // 仿p恢复 display + 0xF8
        PATCH_MEMORY(0x00,      1, 0x04A3C5),   // 仿p恢复 Exp -6
        //PATCH_MEMORY(p0043CA3F,   6, 0x03CA3F),   // AT 显示减少
        PATCH_MEMORY(0x74,      1, 0x0041C398 -0x00400000), // The Other Side 恢复

        PATCH_MEMORY(0x643525,  4, 0x1B186C),   // Exp %4d->%5d
        PATCH_MEMORY(0xD4,      1, 0x04A2F7),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0x00,      1, 0x04A313),   // CP %3d->%6d, use HP's
        PATCH_MEMORY(0xEB,      1, 0x006195),   // party display extra inf
        //  PATCH_MEMORY(0xACE9,    4, 0x04A6E2),   // not display Condition
        PATCH_MEMORY(0x74,      2, 0x04A2C5),   // display cp
        PATCH_MEMORY(0x30,      1, 0x049ECF),   // height
        PATCH_MEMORY(0x30,      1, 0x049ED7),   // height2
        PATCH_MEMORY(0x00,      1, 0x0061CE),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408EA7 -0x00400000),
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00406FA8 -0x00400000),
        PATCH_MEMORY(0x59D3F0,  4, 0x0041C3C2 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x59D3F0,  4, 0x0041C3CF -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x25D8,    2, 0x0041C3CD -0x00400000), // 状态AT调整恢复 FSUB
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x0041C393 -0x00400000), // 是否显状态AT
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,  ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,    ed6DisplayItemDropPatch, 0),
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,           ed6DisplayGetPar, 0, ed6DisplayGetParOld),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, 0x0044677F,   ed6DisplayBattleSepith, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048C4B0,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,     ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only

        INLINE_HOOK_JUMP(0x00449F20,    METHOD_PTR(&CBattleInfoBox::DrawMonsterInfo),   CBattleInfoBox::StubDrawMonsterInfo),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2DE92D0);
    lpfnHandleKeyUp = HandleKeyUp;
}

void patch_ed62cn7(PVOID hModule)
{
    using namespace NED62;

    NED62::sprintf              = (pSprintf)0x53A93F;
    getItemInf                  = (pGetAddr)0x4C63A0;

    lpfnDrawSimpleText          = 0x0048C3A0;
    lpfnFtol                    = 0x0053A8DC;
    lpfnSetTextSize             = 0x48A7A0;
    lpfnDrawBattleIcon          = 0x48CE60;

    addrDisplaySkipCondition0   = 0x004412B7; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x00441350; // ret2

    lpcStatusLine               = 0x55C1DC;
    lpfnEnumCondition           = 0x418610;
    addrConditionIconPointBegin = 0x558E9C;
    addrConditionIconPointEnd   = 0x558F9C;
    addrBattleTexture          = (PULONG_PTR)0x63ED14;

    addrDisplayStatusPatch0     = 0x4418D0;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x0048C410; // call

    addrDisplayBattleIcoEx0     = 0x0048CEAA;
    dwYStart                    = 0x110;

    addrSoldierNo0              = 0x60EF38;
    addrChangeEnemyStatusPatch0 = 0x00443592;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004CE170; // call

    addrCheckQuartzPatch        = 0x004CE300;
    lpfnCheckEquipment          = 0x004C9F60;
    lpfnGetCraftInf             = 0x00409B80;
    lpfnFindCondition           = 0x004194B0;
    lpfnRand                    = 0x004C7650;
    lpfnRandX                   = 0x004C76D0;
    lpDebugFlags                = (PULONG_PTR)0x2EE0D5C;
    lpBattleFlags               = (PULONG_PTR)0x607A20;
    addrGetHitResultPatch       = 0x00414830;

    addrDrive3Patch             = 0x00428D8E;

    addrSetChrStatusByEquipPatch= 0x004CA0A0;

    resolution                  = (PSIZE)0x005643F8; // 分辨率

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2EE0D5E],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x00433DC9 -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x00419A11 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x15C168),   // Exp %4d->%5d
        PATCH_MEMORY(0xEB,      1, 0x005F65),   // party display extra inf
        PATCH_MEMORY(0x00,      1, 0x04155B),   // display cp
        PATCH_MEMORY(0x30,      1, 0x041156),   // height
        PATCH_MEMORY(0x00,      1, 0x005F96),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408A97 -0x00400000),
        PATCH_MEMORY(0x54A330,  4, 0x00419A36 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x54A330,  4, 0x00419A43 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x004199F9 -0x00400000), // 是否显状态AT

        PATCH_MEMORY(0x01,      1, 0x004CBA86 -0x00400000), // LG_Font2
        //PATCH_MEMORY(0xEB,      1, 0x004CBA87 -0x00400000),   // LG_Font1
        //PATCH_MEMORY(0xEB,      1, 0x004CBB59 -0x00400000),   // LG_Font4
        //PATCH_MEMORY(0xEB,      1, 0x004CBB9F -0x00400000),   // LG_Font5
        //PATCH_MEMORY(0xEB,      1, 0x004CBBE3 -0x00400000),   // LG_Font6
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        //PATCH_FUNCTION(JUMP, 0, 0x0412B7, ed6DisplaySkipCondition, 0),
        //PATCH_FUNCTION(CALL, 0, 0x08CEAA, NED63::ed6DisplayBattleIcoEx, 1),
        //PATCH_FUNCTION(JUMP, 0, 0x0418D0, ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9FD0,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2F2BA20);
}

void patch_ed62jp7(PVOID hModule)
{
    using namespace NED62;

    NED62::sprintf              = (pSprintf)0x005389AF;
    getItemInf                  = (pGetAddr)0x004C5EF0;

    lpfnDrawSimpleText          = 0x0048BDA0;
    lpfnFtol                    = 0x00538950;
    lpfnSetTextSize             = 0x0048A170;
    lpfnDrawBattleIcon          = 0x0048C860;

    addrDisplaySkipCondition0   = 0x00440E77; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x00440F10; // ret2

    lpcStatusLine               = 0x55B6C4;
    lpfnEnumCondition           = 0x418410;
    addrConditionIconPointBegin = 0x557FC4;
    addrConditionIconPointEnd   = 0x5580C4;
    addrBattleTexture           = (PULONG_PTR)0x63EBB0;

    addrDisplayStatusPatch0     = 0x00441499;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x0048BE10; // call

    addrDisplayBattleIcoEx0     = 0x0048C8AA;
    dwYStart                    = 0x112;

    addrSoldierNo0              = 0x60EDD8;
    addrChangeEnemyStatusPatch0 = 0x004431C2;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004CCF00; // call

    addrCheckQuartzPatch        = 0x004CD090;
    lpfnCheckEquipment          = 0x004C9940;
    lpfnGetCraftInf             = 0x00409BD0;
    lpfnFindCondition           = 0x004192B0;
    lpfnRand                    = 0x004C70E0;
    lpfnRandX                   = 0x004C7160;
    lpDebugFlags                = (PULONG_PTR)0x2EE2210;
    lpBattleFlags               = (PULONG_PTR)0x6078F8;
    addrGetHitResultPatch       = 0x00414700;

    addrDrive3Patch             = 0x00428AAE;

    addrSetChrStatusByEquipPatch= 0x004C9A80;

    resolution                  = (PSIZE)0x00563CB0; // 分辨率
    CodePage                    = 932;

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2EE2212],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x00433A19 -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x00419811 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x15B644),   // Exp %4d->%5d
        PATCH_MEMORY(0xEB,      1, 0x005F65),   // party display extra inf
        PATCH_MEMORY(0x00,      1, 0x04111B),   // display cp
        PATCH_MEMORY(0x30,      1, 0x040D16),   // height
        PATCH_MEMORY(0x00,      1, 0x005F96),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408A97 -0x00400000),
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x004197F9 -0x00400000), // 是否显状态AT

    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9970,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2F2CDFC);
}

void patch_ed62jp1020(PVOID hModule)
{
    using namespace NED62;

    NED62::sprintf              = (pSprintf)0x00537EEF;
    getItemInf                  = (pGetAddr)0x004C59F0;

    lpfnDrawSimpleText          = 0x0048B7D0;
    lpfnFtol                    = 0x00537E8C;
    lpfnSetTextSize             = 0x00489C70;
    lpfnDrawBattleIcon          = 0x0048C290;

    addrDisplaySkipCondition0   = 0x00440DC7; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x00440E60; // ret2

    lpcStatusLine               = 0x55A6C4;
    lpfnEnumCondition           = 0x418420;
    addrConditionIconPointBegin = 0x556FC4;
    addrConditionIconPointEnd   = 0x5570C4;
    addrBattleTexture           = (PULONG_PTR)0x63DA70;

    addrDisplayStatusPatch0     = 0x004413E9;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x0048B840; // call

    addrDisplayBattleIcoEx0     = 0x0048C2DA;
    dwYStart                    = 0x112;

    addrSoldierNo0              = 0x60DC98;
    addrChangeEnemyStatusPatch0 = 0x00443112;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004CCB30; // call

    addrCheckQuartzPatch        = 0x004CCCC0;
    lpfnCheckEquipment          = 0x004C9580;
    lpfnGetCraftInf             = 0x00409BD0;
    lpfnFindCondition           = 0x004192C0;
    lpfnRand                    = 0x004C6BE0;
    lpfnRandX                   = 0x004C6C60;
    lpDebugFlags                = (PULONG_PTR)0x2EDF778;
    lpBattleFlags               = (PULONG_PTR)0x6067B8;
    addrGetHitResultPatch       = 0x00414710;

    addrDrive3Patch             = 0x00428ABE;

    addrSetChrStatusByEquipPatch= 0x004C96C0;

    resolution                  = (PSIZE)0x00562BF4; // 分辨率

    unsigned char p004339F9[6] = { 0xE9, 0xF2, 0x00, 0x00, 0x00, 0x90 };

    unsigned char p004CBDCE[11] = { 0x6A, 0x0A, 0xFF, 0x15, 0x24, 0x90, 0x54, 0x00, 0x32, 0xC0, 0xC3 };
    unsigned char p004CDEE2[10] = { 0xE8, 0x4D, 0xA2, 0x07, 0x00, 0xEB, 0x3D, 0x90, 0x90, 0x90 };
    unsigned char p00548120[35] = {
        0xE8, 0x0B, 0xEA, 0xF7, 0xFF, 0x85, 0xC0, 0x75, 0x0A, 0x60, 0x6A, 0x01, 0xFF, 0x15, 0x24, 0x90,
        0x54, 0x00, 0x61, 0xC3, 0xE8, 0xF7, 0xE9, 0xF7, 0xFF, 0xE8, 0x42, 0x3C, 0xF8, 0xFF, 0x84, 0xC0,
        0x74, 0xF2, 0xC3};

    unsigned char p00419839[7]  = { 0xDB, 0x44, 0x24, 0x44, 0x8B, 0x43, 0x0C };

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (*(PULONG)0x0047A7C2 == 0x02CC11CA)  // 整合 理补
    {
        //g_GameVersion = ed62cn1020fc;
        addrSetChrStatusByEquipPatch = (ULONG_PTR)-1;
    }

    if (*(UINT*)0x00562C04 == 0x59977089 && *(UINT*)0x004C6F01 == 0x562BEC) //日版 windows名称
    {
        CodePage = 932;
        MEMORY_FUNCTION_PATCH f1[] =
        {
            PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0), // jp ver only
        };
        Nt_PatchMemory(NULL, 0, f1, countof(f1), hModule);
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x2EDF77A],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   6, 0x004339F9 -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x00419821 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    if (*(UINT*)0x004413BC == 0x02CFE5DB) // 理之补丁 at调整为一列
    {
        if (nShowConditionAT != SHOW_CONDITION_AT_NONE)
        {
            __asm OR BYTE PTR DS:[0x2EDF77C],0x1;
        }

        //nShowConditionAT = SHOW_CONDITION_AT_MAX99;
        //nConditionATColor = CONDITION_AT_COLOR_ORIGINAL;
        MEMORY_PATCH p1[] =
        {
            //PATCH_MEMORY(p004339F9,   6, 0x0339F9),   // AT 显示减少
            PATCH_MEMORY(0x549330,  4, 0x00419846 -0x00400000), // 状态AT调整恢复
            PATCH_MEMORY(0x549330,  4, 0x00419853 -0x00400000), // 状态AT调整恢复
            PATCH_MEMORY(p00419839, sizeof(p00419839), 0x00419839 -0x00400000), // 状态AT max99恢复

            PATCH_MEMORY(0x00077201,4, 0x004D0F1B -0x00400000), // CPUtest
            PATCH_MEMORY(p004CDEE2, sizeof(p004CDEE2), 0x004CDEE2 -0x00400000), // CPU手册
            PATCH_MEMORY(p00548120, sizeof(p00548120), 0x00548120 -0x00400000), // new Code
        };
        MEMORY_FUNCTION_PATCH f1[] =
        {
            PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
            PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9530,   ed6ShowConditionAtNewLi, 5, ed6ShowConditionAtOld), // 状态AT不同分辨率居中显示，li状态图标上移，所以特殊对待
        };
        Nt_PatchMemory(p1, countof(p1), f1, countof(f1),  hModule);

        ChangeMainWindowProc(*(HWND*)0x2F2A35C);
        return;
    }

    MEMORY_PATCH p[] =
    {
        //PATCH_MEMORY(0x43,        1, 0x15A68C),   // 整合恢复 condition
        //PATCH_MEMORY(0x22,        1, 0x040E13),   // 整合恢复 condition

        PATCH_MEMORY(0x643525,  4, 0x15A644),   // Exp %4d->%5d
        PATCH_MEMORY(0xEB,      1, 0x005F65),   // party display extra inf
        PATCH_MEMORY(0x74,      2, 0x04106A),   // display cp
        PATCH_MEMORY(0x30,      1, 0x040C66),   // height
        PATCH_MEMORY(0x00,      1, 0x005F96),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00408A97 -0x00400000),
        PATCH_MEMORY(0x549330,  4, 0x00419846 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x549330,  4, 0x00419853 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x00419809 -0x00400000), // 是否显状态AT

        //PATCH_MEMORY(p004339F9,   6, 0x0339F9),   // AT 显示减少
        PATCH_MEMORY(0x36,      1, 0x004C6B54 -0x00400000), // CPU恢复
        PATCH_MEMORY(0x00145FD2,4, 0x0040215E -0x00400000), // CPU战斗
        PATCH_MEMORY(0x00077201,4, 0x004D0F1B -0x00400000), // CPUtest
        PATCH_MEMORY(p004CBDCE, sizeof(p004CBDCE), 0x004CBDCE -0x00400000), // sleep@no active
        PATCH_MEMORY(p004CDEE2, sizeof(p004CDEE2), 0x004CDEE2 -0x00400000), // CPU手册
        PATCH_MEMORY(p00548120, sizeof(p00548120), 0x00548120 -0x00400000), // new Code
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        //PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,  ed6DisplaySkipCondition, 0), //整合放右上角了
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9530,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x2F2A35C);
}

void patch_ed61cn7(PVOID hModule)
{
    using namespace NED61;

    NED61::sprintf              = (pSprintf)0x004FC9FF;
    getItemInf                  = (pGetAddr)0x4A84F0;

    lpfnDrawSimpleText          = 0x00474DC0;
    lpfnFtol                    = 0x004FC99C;
    lpfnSetTextSize             = 0x00473260;
    lpfnDrawBattleIcon          = 0x00475880;

    addrDisplaySkipCondition0   = 0x0043411C; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x004341B8; // ret2

    lpcStatusLine               = 0x51A294;
    lpfnEnumCondition           = 0x413220;
    addrConditionIconPointBegin = 0x517BCC;
    addrConditionIconPointEnd   = 0x517CBC;
    addrBattleTexture          = (PULONG_PTR)0x5CB918;

    addrDisplayStatusPatch0     = 0x434737;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x00474E30; // call

    addrDisplayBattleIcoEx0     = 0x004758CA;
    dwYStart                    = 0x112;

    addrSoldierNo0              = 0x5A58D0;
    addrChangeEnemyStatusPatch0 = 0x00435036;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004AE7E0; // call

    addrCheckQuartzPatch        = 0x004AE990;

    resolution                  = (PSIZE)0x005204DC; // 分辨率

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x18FC9D4],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   8, 0x004286C3 -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x00414525 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x11A214),   // Exp %4d->%5d
        PATCH_MEMORY(0xEB,      1, 0x0054A1),   // party display extra inf
        PATCH_MEMORY(0x00,      1, 0x0343C2),   // display cp
        PATCH_MEMORY(0x30,      1, 0x033F76),   // height
        PATCH_MEMORY(0x00,      1, 0x0054D2),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00407D07 -0x00400000),
        PATCH_MEMORY(0x50C2FC,  4, 0x0041454A -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY(0x50C2FC,  4, 0x00414557 -0x00400000), // 状态AT调整恢复
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x0041450D -0x00400000), // 是否显状态AT

        //PATCH_MEMORY(0x01,      1, 0x004CBA86 -0x00400000),   // LG_Font2
        //PATCH_MEMORY(0xEB,      1, 0x004AC469 -0x00400000),   // LG_Font1
        //PATCH_MEMORY(0xEB,      1, 0x004AC4F4 -0x00400000),   // LG_Font4
        //PATCH_MEMORY(0xEB,      1, 0x004AC51F -0x00400000),   // LG_Font5
        //PATCH_MEMORY(0xEB,      1, 0x004AC548 -0x00400000),   // LG_Font6
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        //PATCH_FUNCTION(JUMP, 0, 0x03411C, ed6DisplaySkipCondition, 0),
        //PATCH_FUNCTION(CALL, 0, 0x0758CA, NED63::ed6DisplayBattleIcoEx, 1),
        //PATCH_FUNCTION(JUMP, 0, 0x034737, ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        //PATCH_FUNCTION(JUMP, NOT_RVA, 0x00490A50,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
        PATCH_CALL    (CALL, NOT_RVA, 0x00414560,   ed6ShowConditionAtNew, 0, ed6ShowConditionAtOld),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x19417A4);
}

void patch_ed61jp7(PVOID hModule)
{
    using namespace NED61;

    NED61::sprintf              = (pSprintf)0x004FBA2F;
    getItemInf                  = (pGetAddr)0x004A84A0;

    lpfnDrawSimpleText          = 0x00474C00;
    lpfnFtol                    = 0x004FB9D0;
    lpfnSetTextSize             = 0x00473010;
    lpfnDrawBattleIcon          = 0x004756C0;

    addrDisplaySkipCondition0   = 0x00433D9C; // patch at
    addrDisplaySkipCondition1   = addrDisplaySkipCondition0 + 5; // ret1
    addrDisplaySkipCondition2   = 0x00433E38; // ret2

    lpcStatusLine               = 0x51A714;
    lpfnEnumCondition           = 0x004130B0;
    addrConditionIconPointBegin = 0x517C7C;
    addrConditionIconPointEnd   = 0x517D6C;
    addrBattleTexture           = (PULONG_PTR)0x5CE3B8;

    addrDisplayStatusPatch0     = 0x004343C0;
    addrDisplayStatusPatch1     = addrDisplayStatusPatch0 + 5;
    addrDisplayStatusPatch2     = 0x00474C70; // call

    addrDisplayBattleIcoEx0     = 0x0047570A;
    dwYStart                    = 0x112;

    addrSoldierNo0              = 0x5A8370;
    addrChangeEnemyStatusPatch0 = 0x00434C56;
    addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
    addrChangeEnemyStatusPatch2 = 0x004AE440; // call

    addrCheckQuartzPatch        = 0x004AE5F0;

    resolution                  = (PSIZE)0x00520F04; // 分辨率
    CodePage                    = 932;

    if (*(PBYTE)addrChangeEnemyStatusPatch0 == 0xE9) // 0xE8 call 0xE9 jump; 防止重复补丁
    {
        Hooked = TRUE;
        return;
    }

    if (nShowAT != 0)   // 显AT
    {
        __asm OR BYTE PTR DS:[0x18FF474],0x1;
        if (nShowAT == 1)
        {
            pMemPatchShowAtData = pMemPatchShowAtDataNew;
        }
        else
        {
            pMemPatchShowAtData = pMemPatchShowAtDataOld;
        }

        MEMORY_PATCH memPatchShowAT[] =
        {
            PATCH_MEMORY(pMemPatchShowAtData,   8, 0x004284D3 -0x00400000), // AT 显示减少恢复
        };
        Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

    }

    if ((nShowConditionAT == SHOW_CONDITION_AT_HIDE99 || nShowConditionAT == SHOW_CONDITION_AT_MAX99) && resolution->cx >= 800)  // 状态AT 调整到一行
    {
        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0, 4, 0x004143B5 -0x00400000), // 状态AT 调整到一行
        };
        Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
    }

    MEMORY_PATCH p[] =
    {
        PATCH_MEMORY(0x643525,  4, 0x11A694),   // Exp %4d->%5d
        PATCH_MEMORY(0xEB,      1, 0x0054A1),   // party display extra inf
        PATCH_MEMORY(0x00,      1, 0x034042),   // display cp
        PATCH_MEMORY(0x30,      1, 0x033C46),   // height
        PATCH_MEMORY(0x00,      1, 0x0054D2),   // up board
        PATCH_MEMORY(nSepithUpLimit,    4, 0x00407D17 -0x00400000),
        PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,  4, 0x0041439D -0x00400000), // 是否显状态AT
    };

    MEMORY_FUNCTION_PATCH f[] =
    {
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,    ed6DisplaySkipCondition, 0),
        PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,      NED63::ed6DisplayBattleIcoEx, 1),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,      ed6DisplayStatusPatch, 0),
        PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,  ed6ChangeEnemyStatusPatch, 0),
        //PATCH_FUNCTION(JUMP, NOT_RVA, 0x00490930,   ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
        PATCH_CALL    (CALL, NOT_RVA, 0x004143F0,   ed6ShowConditionAtNew, 0, ed6ShowConditionAtOld),
    };
    Nt_PatchMemory(p, countof(p), f, countof(f), hModule);

    ChangeMainWindowProc(*(HWND*)0x1944244);
}

void patch_ed6123(PVOID hModule)
{
    {
        using namespace NED6123;

        addrDrive3PatchFalse        = addrDrive3Patch + 5 + 0x34;
        addrDrive3PatchTrue         = addrDrive3Patch + 5 + 0x9;
        if (!bPSP_MODE)
        {
            addrCheckMirrorWPADPatch    = (ULONG_PTR)-1;
            addrFixMirrorBugPatch       = (ULONG_PTR)-1;
        }

        MEMORY_FUNCTION_PATCH f[] =
        {
            INLINE_HOOK_JUMP        (addrCheckQuartzPatch,      CheckQuartz,        StubCheckQuartz),
        };
        Nt_PatchMemory(nullptr, 0, f, countof(f), hModule);
    }

    if (FLAG_ON(g_GameVersion, ed61min))
    {
        using namespace NED61;
    }
    else if (FLAG_ON(g_GameVersion, ed62min))
    {
        using namespace NED62;
        MEMORY_FUNCTION_PATCH f[] =
        {
            // PSP 命中率/回避率设定
            INLINE_HOOK_JUMP        (addrGetHitResultPatch,     GetHitResult,       StubGetHitResult),

            // PSP 驱动3
            PATCH_FUNCTION(JUMP, NOT_RVA, addrDrive3Patch,      ed6Drive3Patch, 0),

            // PSP 猫咪套装+SPD
            INLINE_HOOK_JUMP        (addrSetChrStatusByEquipPatch,  SetChrStatusByEquip,    StubSetChrStatusByEquip),
        };
        Nt_PatchMemory(nullptr, 0, f, countof(f), hModule);
    }
    else if (FLAG_ON(g_GameVersion, ed63min))
    {
        using namespace NED63;

        MEMORY_PATCH p[] =
        {
            PATCH_MEMORY(0x32EB,        2,      addrFixMirrorBugPatch), // 裂甲断 反射卡住bug
        };

        MEMORY_FUNCTION_PATCH f[] =
        {
            // PSP 命中率/回避率设定
            INLINE_HOOK_JUMP        (addrGetHitResultPatch,     GetHitResult,       StubGetHitResult),

            // PSP 驱动3
            PATCH_FUNCTION(JUMP, NOT_RVA, addrDrive3Patch,      ed6Drive3Patch, 0),

            // PSP 神镜 魔镜
            INLINE_HOOK_JUMP        (addrInitBeforeAtIconUpPatch,   METHOD_PTR(&CBattle::InitBeforeAtIconUp),   CBattle::StubInitBeforeAtIconUp),
            INLINE_HOOK_JUMP        (addrCheckCraftMirrorPatch,     CheckCraftMirror,   StubCheckCraftMirror),
            INLINE_HOOK_JUMP        (addrCheckArtsMirrorPatch,      CheckArtsMirror,    StubCheckArtsMirror),
            INLINE_HOOK_JUMP_NULL   (addrCheckMirrorWPADPatch,      CheckMirrorWhenPreviewAtDelay),
        };
        Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
    }
}

void Init()
{
    GameVersion gameVersion = getGameVersion();
    if (gameVersion == ed6VersionUnknown)
    {
        return;
    }
    g_GameVersion = gameVersion;
    ConfigInit();

    HMODULE hModule = Nt_GetExeModuleHandle();

#if CONSOLE_DEBUG
    QueryPerformanceFrequency(&lFrequency);
    PrintConsoleW(L"ED63 Is 0x17E, 0x18A, 0x210, 0x268?: %#x %#x %#x %#x\r\n", 
        FIELD_OFFSET(NED63::MONSTER_STATUS, CurrentActionType), 
        FIELD_OFFSET(NED63::MONSTER_STATUS, CurrentCraftIndex), 
        FIELD_OFFSET(NED63::MONSTER_STATUS, IsHitMiss),
        FIELD_OFFSET(NED63::MONSTER_STATUS, StatusSum));
    PrintConsoleW(L"ED62 Is 0x17A, 0x186, 0x210, 0x268?: %#x %#x %#x %#x\r\n", 
        FIELD_OFFSET(NED62::MONSTER_STATUS, CurrentActionType), 
        FIELD_OFFSET(NED62::MONSTER_STATUS, CurrentCraftIndex), 
        FIELD_OFFSET(NED62::MONSTER_STATUS, IsHitMiss),
        FIELD_OFFSET(NED62::MONSTER_STATUS, StatusSum));
#endif
    //NED63::print_status_rev_base(t_btrev2);

    switch (gameVersion)
    {
        case ed63cn7:
            patch_ed63cn7(hModule);
            break;
        case ed63jp7:
            patch_ed63jp7(hModule);
            break;
        case ed63jp1002:
            patch_ed63jp1002(hModule);
            break;

        case ed62cn7:
            patch_ed62cn7(hModule);
            break;
        case ed62jp7:
            patch_ed62jp7(hModule);
            break;
        case ed62jp1020:
            patch_ed62jp1020(hModule);
            break;

        case ed61cn7:
            patch_ed61cn7(hModule);
            break;
        case ed61jp7:
            patch_ed61jp7(hModule);
            break;
    }

    if (!Hooked)
    {
        patch_ed6123(hModule);
        Hooked = TRUE;
    }
}