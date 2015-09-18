#ifdef  _ED61_NS_
#define _ED612_NS_
#define _ED6123_NS_
#endif

#ifdef  _ED62_NS_
#define _ED612_NS_
#define _ED623_NS_
#define _ED6123_NS_
#endif

#ifdef  _ED63_NS_
#define _ED623_NS_
#define _ED6123_NS_
#endif

#if 0
//#include "ed6inf.h"
using namespace NED62;
#endif

class CBattleInfoBox
{
public:
    VOID SetTextSize(ULONG size_index)
    {
        DETOUR_METHOD(CBattleInfoBox, SetTextSize, lpfnSetTextSize, size_index);
    }

    VOID DrawSimpleText(LONG x, LONG y, PCSTR text, ULONG color_index = COLOR_WHITE, LONG weight = FW_NORMAL)
    {
        DETOUR_METHOD(CBattleInfoBox, DrawSimpleText, lpfnDrawSimpleText, x, y, text, color_index, weight);
    }

#ifdef _ED61_NS_
    VOID DrawBattleIcon(ULONG_PTR pTexture, PPOINT icon, PPOINT target)
    {
        DETOUR_METHOD(CBattleInfoBox, DrawBattleIcon, lpfnDrawBattleIcon, pTexture, icon, target);
    }
#endif

#ifdef _ED623_NS_
    VOID DrawBattleIcon(ULONG_PTR pTexture, PPOINT icon, PPOINT target, BOOL dark = FALSE)
    {
        DETOUR_METHOD(CBattleInfoBox, DrawBattleIcon, lpfnDrawBattleIcon, pTexture, icon, target, dark);
    }
#endif

    ULONG DrawSimpleTextMultiline(LONG x, LONG y, LONG indent, ULONG height, PCSTR text, INT length = 0, ULONG color_index = COLOR_WHITE, LONG weight = FW_NORMAL);

    VOID ed6DisplayStatus(ED6_CHARACTER_BATTLE_INF* lpBattleInf);
    VOID ed6DisplayItemDrop(ED6_CHARACTER_BATTLE_INF* lpBattleInf);

    VOID DrawMonsterInfo();
    DECL_STATIC_METHOD_POINTER(CBattleInfoBox, DrawMonsterInfo);
};

INIT_STATIC_MEMBER(CBattleInfoBox::StubDrawMonsterInfo);

FORCEINLINE ULONG CDECL EnumCondition(ULONG index)
{
    DETOUR_FUNCTION(EnumCondition, lpfnEnumCondition, index);
}

RESISTANCE ed6GetResistance(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
{
    UINT addr;
    RESISTANCE resist = lpBattleInf->Resistance;
    for (int i = 3; i < 5; ++i)
    {
        if (lpBattleInf->Equip[i] == 0)
        {
            continue;
        }
        addr = getItemInf(lpBattleInf->Equip[i]);

        if (addr == 0)
        {
            continue;
        }

        if (*(byte*)(addr+5) != 1)
        {
            continue;
        }

        __asm xor ebx,ebx;
        switch (*(byte*)(addr+6))
        {
        case 1:
            __asm or BL,0x1;
            break;
        case 2:
            __asm or BL,0x2;
            break;
        case 3:
            __asm or BL,0x4;
            break;
        case 4:
            __asm or BL,0x8;
            break;
        case 5:
            __asm or BL,0x10;
            break;
        case 6:
            __asm or BL,0x20;
            break;
        case 7:
            __asm or BL,0x40;
            break;
        case 8:
            __asm or BL,0x80;
            break;
        case 9:
            __asm or BH,0x1;
            break;
        case 10:
            __asm or BH,0x2;
            break;
        case 11:
            __asm or BL,0x6;
            break;
        case 12:
            __asm or EBX,0x108;
            break;
        case 13:
            __asm or EBX,0x3FF;
            break;
#ifdef _ED623_NS_
        case 14:
            __asm or BL,0x2A;
            break;
        case 15:
            __asm or BL,0x21;
            break;
        case 16:
            __asm or BL,0x88;
            break;
        case 17:
            __asm or BL,0x3;
            break;
        case 18:
            __asm or BL,0x50;
            break;
        case 19:
            __asm or EBX,0x204;
            break;
#endif
#ifdef _ED63_NS_
        case 20:
            __asm or EBX,0x208000;
            break;
        case 21:
            __asm or EBX,0x8A0000;
            break;
        case 22:
            __asm or EBX,0x2AA8000;
            break;
        case 23:
            __asm or EBX,0x2AA8000;
            __asm or EBX,0x3FF;
#endif
        }
        __asm or resist,EBX
    }
    return resist;
}

VOID THISCALL CBattleInfoBox::ed6DisplayStatus(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
{
    if (g_bShowExtraInfo == FALSE)
    {
        return;
    }

    DWORD   dwY = dwYStart;
    int     i;

#if CONSOLE_DEBUG
    QueryPerformanceCounter(&lStartCounter);
#endif

    char szBuffer[0x120];
    static char *szStatus[] = { "Level", " STR", " DEF", " ATS", " ADF", " SPD", " DEX", " AGL", " MOV" };

    /*显示分界线*/
    SetTextSize(0);
    DrawSimpleText(0, dwY, (PCSTR)lpcStatusLine, COLOR_LINE);
    SetTextSize(1);
    dwY += 0xA;

#ifdef _ED612_NS_
    /*物品掉落概率*/
    for (i = 0; i < 2; ++i)
    {
        if (lpBattleInf->DropIndex[i] != 0 && lpBattleInf->DropIndex[i] != 0xFFFF)
        {
            sprintf(szBuffer, "%3d%%", lpBattleInf->DropProbability[i]);
            DrawSimpleText(0, 0xBA + 0xE * i, szBuffer, COLOR_GRAY);
        }
    }
#endif

    for (i = 0; i < countof(szStatus); i++, dwY+=0xC)
    {
        sprintf(szBuffer, "%s", szStatus[i]);
        DrawSimpleText(0, dwY, szBuffer, COLOR_TITLE);

        switch (i)
        {
        case 0:
            sprintf(szBuffer, "%21d", lpBattleInf->StatusBasic.Level);
            break;
        case 1:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.STR, lpBattleInf->StatusSum.STR);
            break;
        case 2:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.DEF, lpBattleInf->StatusSum.DEF);
            break;
        case 3:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.ATS, lpBattleInf->StatusSum.ATS);
            break;
        case 4:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.ADF, lpBattleInf->StatusSum.ADF);
            break;
        case 5:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.SPD, lpBattleInf->StatusSum.SPD);
            break;
        case 6:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.DEX, lpBattleInf->StatusSum.DEX);
            break;
        case 7:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.AGL, lpBattleInf->StatusSum.AGL);
            break;
        case 8:
            sprintf(szBuffer, "       %6d  %6d", lpBattleInf->StatusBasic.MOV, lpBattleInf->StatusSum.MOV);
            break;
        }

        DrawSimpleText(0, dwY, szBuffer);
    }

    /*显示抗性*/
    POINT   target = {0, (LONG)dwY};
    POINT   icon;

    PFLOAT  pf;
    ULONG   nFind = 0;
    ULONG   condition;
    RESISTANCE resist = ed6GetResistance(lpBattleInf);
    for (i = 0, pf = (PFLOAT)addrConditionIconPointBegin; pf < (PFLOAT)addrConditionIconPointEnd; ++i, pf += 2)
    {
        condition = EnumCondition(i);

        if (condition == 0)
        {
            continue;
        }
        else
        {
            nFind++;
        }

        if (FLAG_ON(resist, condition))
        {
            icon.x = LONG(*(pf - 1));
            icon.y = LONG(*pf);
            DrawBattleIcon(*addrBattleTexture, &icon, &target);
        }

        target.x += 0x10;
        if (nFind % 8 == 0)
        {
            target.x = 0;
            target.y += 0x10;
        }
    }

    /*AT delay*/
    //STRdown 16,240 AGLdown 144,240
    //STRup 0,240 AGLup 128,240
    //dwX = 0x70;
    if (FLAG_ON(lpBattleInf->HitFlag, CHR_FLAG_ResistATDelay))
    //if(1)
    {
        dwY = target.y;

        target.x = 0x70;
        target.y = dwY+0x8;
        icon.x = 16;
        icon.y = 240+0x8;
        DrawSimpleText(0x72, dwY, "AT");
        g_bDisplayBattleIcoEx = TRUE;
        DrawBattleIcon(*addrBattleTexture, &icon, &target);
        g_bDisplayBattleIcoEx = FALSE;
    }

#if CONSOLE_DEBUG
    QueryPerformanceCounter(&lStopCounter);
    PrintConsoleW(L"Elapsed time: %lf ms\n", (lStopCounter.QuadPart - lStartCounter.QuadPart) * 1000.0 / lFrequency.QuadPart);
#endif
}

void __cdecl ed6ChangeEnemyStatus(UINT SoldierNo, ED6_STATUS* pStatusSum, ED6_STATUS* pStatusBasic)
{
    if (nDifficulty == 0 || IsSoldierParty(SoldierNo))
    {
        return;
    }

    ED6_CHARACTER_BATTLE_INF* lpBattleInf = (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0 + SoldierNo;

    if (lpBattleInf->StatusSum.HPMax == lpBattleInf->StatusSum.HP)
    {
        SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * sRate.HP_a / 1000 + sRate.HP_b);
        lpBattleInf->StatusSum.HP = lpBattleInf->StatusSum.HPMax;
    }
    else
    {
        SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * sRate.HP_a / 1000 + sRate.HP_b);
        SaturateConvert(&lpBattleInf->StatusSum.HP, (INT64)lpBattleInf->StatusSum.HP * sRate.HP_a / 1000 + sRate.HP_b);
    }
    SaturateConvert(&lpBattleInf->StatusSum.STR, (INT64)lpBattleInf->StatusSum.STR * sRate.STR_a / 1000 + sRate.STR_b);
    SaturateConvert(&lpBattleInf->StatusSum.DEF, (INT64)lpBattleInf->StatusSum.DEF * sRate.DEF_a / 1000 + sRate.DEF_b);
    SaturateConvert(&lpBattleInf->StatusSum.ATS, (INT64)lpBattleInf->StatusSum.ATS * sRate.ATS_a / 1000 + sRate.ATS_b);
    SaturateConvert(&lpBattleInf->StatusSum.ADF, (INT64)lpBattleInf->StatusSum.ADF * sRate.ADF_a / 1000 + sRate.ADF_b);
    SaturateConvert(&lpBattleInf->StatusSum.SPD, (INT64)lpBattleInf->StatusSum.SPD * sRate.SPD_a / 1000 + sRate.SPD_b);
    SaturateConvertEx(&lpBattleInf->StatusSum.DEX, (INT64)lpBattleInf->StatusSum.DEX * sRate.DEX_a / 1000 + sRate.DEX_b, (SHORT)0xCCC);
    SaturateConvertEx(&lpBattleInf->StatusSum.AGL, (INT64)lpBattleInf->StatusSum.AGL * sRate.AGL_a / 1000 + sRate.AGL_b, (SHORT)0xCCC);
    SaturateConvert(&lpBattleInf->StatusSum.MOV, (INT64)lpBattleInf->StatusSum.MOV * sRate.MOV_a / 1000 + sRate.MOV_b);

    if (sRate.ResistNone)
    {
        lpBattleInf->Resistance = 0;
        CLEAR_FLAG(lpBattleInf->HitFlag, CHR_FLAG_ResistATDelay);
    }
    if (sRate.ResistAbnormalCondition)
    {
        lpBattleInf->Resistance |= conditionAbnormal;
    }
    if (sRate.ResistAbilityDown)
    {
        lpBattleInf->Resistance |= conditionDown;
    }
    if (sRate.ResistATDelay)
    {
        SET_FLAG(lpBattleInf->HitFlag, CHR_FLAG_ResistATDelay);
    }
}

ASM void ed6ChangeEnemyStatusPatch()
{
    __asm
    {
        MOV EAX,DWORD PTR SS:[ESP+0x8];
        MOV ECX,DWORD PTR SS:[ESP+0x4];
        MOV EDX,DWORD PTR SS:[ESP];
        push EAX;
        push ECX;
        push EDX;
        call addrChangeEnemyStatusPatch2;
        ADD ESP,0xC;
        call ed6ChangeEnemyStatus;
        jmp addrChangeEnemyStatusPatch1;
    }
}

#undef  _ED61_NS_
#undef  _ED62_NS_
#undef  _ED63_NS_
#undef  _ED612_NS_
#undef  _ED623_NS_
#undef  _ED6123_NS_