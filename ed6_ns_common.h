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

#define ITEM_ID_DRIVE1  0x2C6
#define ITEM_ID_DRIVE2  0x2C7

#ifdef  _ED62_NS_
#define ITEM_ID_DRIVE3  0x2CD
#endif

#ifdef  _ED63_NS_
#define ITEM_ID_DRIVE3  0x2EE
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

class CBattle
{
public:
    bool LoadStatusData(ULONG MSFile, ULONG ChrPosition, ULONG a3 = 0);
    VOID dump_status_rev_special(ULONG ChrPosition);
    VOID InitBeforeAtIconUp();

    DECL_STATIC_METHOD_POINTER(CBattle, LoadStatusData);
    DECL_STATIC_METHOD_POINTER(CBattle, InitBeforeAtIconUp);
};

INIT_STATIC_MEMBER(CBattle::StubLoadStatusData);
INIT_STATIC_MEMBER(CBattle::StubInitBeforeAtIconUp);

ED6_CHARACTER_BATTLE_INF* getChrBattleInf()
{
    return (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0;
}

FORCEINLINE ULONG CDECL EnumCondition(ULONG index)
{
    DETOUR_FUNCTION(EnumCondition, lpfnEnumCondition, index);
}

FORCEINLINE CONDITION* CDECL FindCondition(PMONSTER_STATUS lpBattleInf, ULONG_PTR condition)
{
    DETOUR_FUNCTION(FindCondition, lpfnFindCondition, lpBattleInf, condition);
}

FORCEINLINE CRAFT_INFO* CDECL GetCraftInf(PMONSTER_STATUS lpBattleInf, ULONG_PTR craft)
{
    DETOUR_FUNCTION(GetCraftInf, lpfnGetCraftInf, lpBattleInf, craft);
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
    //PrintConsoleW(L"Elapsed time: %lf ms\n", (lStopCounter.QuadPart - lStartCounter.QuadPart) * 1000.0 / lFrequency.QuadPart);
#endif
}

INT get_difficulty()
{
    return nDifficulty;
}

VOID ed6SetEnemyFinalStatusPsp(ED6_CHARACTER_BATTLE_INF* lpBattleInf);

void __cdecl ed6ChangeEnemyStatus(UINT SoldierNo, ED6_STATUS* pStatusSum, ED6_STATUS* pStatusBasic)
{
    if (nDifficulty == DIFFICULTY_PC_ORIGINAL || IsSoldierParty(SoldierNo))
    {
        return;
    }

    ED6_CHARACTER_BATTLE_INF* lpBattleInf = CONTAINING_RECORD(pStatusSum, ED6_CHARACTER_BATTLE_INF, StatusSum);
    //ED6_CHARACTER_BATTLE_INF* lpBattleInf = (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0 + SoldierNo;

    if (nDifficulty >= DIFFICULTY_PS_NORMAL && nDifficulty <= DIFFICULTY_PS_EASY)
    {
#if 0
        pStatusSum->HPMax = 1000;
        pStatusSum->STR = 1000;
        pStatusSum->DEF = 1000;
        pStatusSum->ATS = 1000;
        pStatusSum->ADF = 1000;
        pStatusSum->SPD = 1000;
        pStatusSum->MOV = 1000;
#endif
        ed6SetEnemyFinalStatusPsp(lpBattleInf);
    }
    else
    {
        if (pStatusSum->HPMax == pStatusSum->HP)
        {
            SaturateConvert(&pStatusSum->HPMax, (INT64)pStatusSum->HPMax * sRate.HP_a / 1000 + sRate.HP_b);
            pStatusSum->HP = pStatusSum->HPMax;
        }
        else
        {
            SaturateConvert(&pStatusSum->HPMax, (INT64)pStatusSum->HPMax * sRate.HP_a / 1000 + sRate.HP_b);
            SaturateConvert(&pStatusSum->HP, (INT64)pStatusSum->HP * sRate.HP_a / 1000 + sRate.HP_b);
        }
        SaturateConvert(&pStatusSum->STR, (INT64)pStatusSum->STR * sRate.STR_a / 1000 + sRate.STR_b);
        SaturateConvert(&pStatusSum->DEF, (INT64)pStatusSum->DEF * sRate.DEF_a / 1000 + sRate.DEF_b);
        SaturateConvert(&pStatusSum->ATS, (INT64)pStatusSum->ATS * sRate.ATS_a / 1000 + sRate.ATS_b);
        SaturateConvert(&pStatusSum->ADF, (INT64)pStatusSum->ADF * sRate.ADF_a / 1000 + sRate.ADF_b);
        SaturateConvert(&pStatusSum->SPD, (INT64)pStatusSum->SPD * sRate.SPD_a / 1000 + sRate.SPD_b);
        SaturateConvertEx(&pStatusSum->DEX, (INT64)pStatusSum->DEX * sRate.DEX_a / 1000 + sRate.DEX_b, (SHORT)0xCCC);
        SaturateConvertEx(&pStatusSum->AGL, (INT64)pStatusSum->AGL * sRate.AGL_a / 1000 + sRate.AGL_b, (SHORT)0xCCC);
        SaturateConvert(&pStatusSum->MOV, (INT64)pStatusSum->MOV * sRate.MOV_a / 1000 + sRate.MOV_b);
    }

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

    if (bUnlimitedSummon)
    {
        ED6_CRAFT_INFO* craft;
        FOR_EACH_ARRAY(craft, lpBattleInf->CraftInf)
        {
            if (craft->AnimationType == 0x134 || craft->AnimationType == 0x138)
            {
                if (craft->Effect2Parameter != 0)
                {
                    craft->Effect2Parameter = 0xFF;
                }
            }
        }
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

#ifdef _ED623_NS_
bool CDECL GetHitResultOriginal(PMONSTER_STATUS src, PMONSTER_STATUS dst)
{
    bool result = false;
    for (;;)
    {
        // 绝对回避
        if (FLAG_ON(dst->HitFlag, CHR_FLAG_AbsoluteMiss))
        {
            result = false;
            break;
        }
        CRAFT_INFO* craft_inf = GetCraftInf(src, src->CurrentCraftIndex);
        CONDITION*  condition = nullptr;
        // 技能必中
        if (FLAG_ON(craft_inf->SpecialEffect, 1))
        {
            result = true;
            break;
        }
        // 打自己人必中
        if((FLAG_ON(src->RoleFlag, CHR_FLAG_PARTY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_PARTY)) ||
           (FLAG_ON(src->RoleFlag, CHR_FLAG_ENEMY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_ENEMY)) )
        {
            result = true;
            break;
        }
        if (FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_MISS))
        {
            result = false;
            break;
        }
        if (FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_HIT) || 
            FLAG_ON(*lpBattleFlags, 0x8020) ||
            src->CurrentActionType == ACTION_ARTS ||
            FindCondition(dst, 0x8800030E))
        {
            result = true;
            break;
        }

        int dex = src->StatusSum.DEX;
        int agl = dst->StatusSum.AGL;
        condition = FindCondition(dst, CONDITION_BIT::AGL_DOWN);
        if (condition)
        {
            agl -= condition->ConditionRate;
        }
        condition = FindCondition(dst, CONDITION_BIT::AGL_UP);
        if (condition)
        {
            agl += condition->ConditionRate;
        }
        if (agl < 0)
        {
            agl = 0;
        }
        if (dex < 0)
        {
            dex = 0;
        }
        if (src->CurrentActionType == ACTION_SCRAFT)
        {
            agl /= 2;
        }
        dex *= 10;
        agl *= 10;
        int rand_result = randX(USHORT(dex + agl));
        WriteConsoleLogW(L"Hit %d(HIT*10) Mis %d(MIS*10) 命中(%d - %d) 结果[%d]\r\n", dex, agl, dex + agl, agl, rand_result);
        if (rand_result < agl)
        {
            result = false;
            break;
        }

        if (FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_BLIND) || FindCondition(src, CONDITION_BIT::BLIND))
        {
            if (randX(10000) < 7000)
            {
                result = false;
                break;
            }
        }
        result = true;
        break;
    }
    src->IsHitMiss[dst->SoldierNo] = result ? 0 : 1;
    return result;
}

bool CDECL GetHitResult(PMONSTER_STATUS src, PMONSTER_STATUS dst)
{
    if (!bPSP_MODE)
    {
        return ((TYPE_OF(&GetHitResult))StubGetHitResult)(src, dst);
    }
    bool result = false;
    for(;;)
    {
        // 绝对回避
        if (FLAG_ON(dst->HitFlag, CHR_FLAG_AbsoluteMiss))
        {
            result = false;
            break;
        }
        CRAFT_INFO* craft_inf = GetCraftInf(src, src->CurrentCraftIndex);
        CONDITION*  condition = nullptr;
        // 技能必中
        if (FLAG_ON(craft_inf->SpecialEffect, 1))
        {
            result = true;
            break;
        }
        // 打自己人必中
        if((FLAG_ON(src->RoleFlag, CHR_FLAG_PARTY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_PARTY)) ||
           (FLAG_ON(src->RoleFlag, CHR_FLAG_ENEMY) && FLAG_ON(dst->RoleFlag, CHR_FLAG_ENEMY)) )
        {
            result = true;
            break;
        }
        if (FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_MISS))
        {
            result = false;
            break;
        }
        if (FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_HIT) || 
            FLAG_ON(*lpBattleFlags, 0x8020) ||
            src->CurrentActionType == ACTION_ARTS ||
            FindCondition(dst, 0x8800030E))
        {
            result = true;
            break;
        }

        int dex = src->StatusSum.DEX;
        int agl = dst->StatusSum.AGL;
        int dex_rate = 0;
        int agl_rate = 0;
        int i;
        int rand_result;
        condition = FindCondition(dst, CONDITION_BIT::AGL_DOWN);
        if (condition)
        {
            agl -= condition->ConditionRate;
        }
        condition = FindCondition(dst, CONDITION_BIT::AGL_UP);
        if (condition)
        {
            agl_rate += condition->ConditionRate / 2;
        }
        for (i = 0; i < countof(ITEM_ID::AGL); ++i)
        {
            if (CheckQuartz(dst->SoldierNo, ITEM_ID::AGL[i]))
            {
                agl_rate += ++i * 3;
                break;
            }
        }
        for (i = 0; i < countof(ITEM_ID::DEX); ++i)
        {
            if (CheckQuartz(src->SoldierNo, ITEM_ID::DEX[i]))
            {
                dex_rate += ++i * 5;
                break;
            }
        }
        if (CheckEquipment(dst->SoldierNo, ITEM_ID::NEKO_SHOE))
        {
            agl_rate += 5;
#ifdef _ED62_NS_
            if (CheckEquipment(dst->SoldierNo, ITEM_ID::NEKO_SUIT) &&
                //CheckEquipment(dst->SoldierNo, ITEM_ID::NEKO_SHOE) &&
                CheckEquipment(dst->SoldierNo, ITEM_ID::NEKO_BAND) &&
                CheckEquipment(dst->SoldierNo, ITEM_ID::NEKO_TAIL))
            {
                agl_rate += 5;
            }
#endif
        }
        if (CheckEquipment(src->SoldierNo, ITEM_ID::NEKO_TAIL))
        {
            dex_rate += 5;
#ifdef _ED62_NS_
            if (CheckEquipment(src->SoldierNo, ITEM_ID::NEKO_SUIT) &&
                CheckEquipment(src->SoldierNo, ITEM_ID::NEKO_SHOE) &&
                CheckEquipment(src->SoldierNo, ITEM_ID::NEKO_BAND))
                //CheckEquipment(src->SoldierNo, ITEM_ID::NEKO_TAIL))
            {
                dex_rate += 5;
            }
#endif
        }
        bool is_blind = FLAG_ON(*lpDebugFlags, DEBUG_BIT::ATTACK_BLIND) || FindCondition(src, CONDITION_BIT::BLIND) ? true : false;

        if (agl < 0)
        {
            agl = 0;
        }
        if (dex < 0)
        {
            dex = 0;
        }
        if (src->CurrentActionType == ACTION_SCRAFT)
        {
            agl /= 2;
        }
        else
        {
            // dex_rate VS rand
            if (!is_blind)
            {
                rand_result = randX(100);
                WriteConsoleLogW(L"dex_rate:%d rand_result:%d\r\n", dex_rate, rand_result);
                if (rand_result < dex_rate)
                {
                    result = true;
                    break;
                }
            }

            // agl_rate VS rand
            rand_result = randX(100);
            WriteConsoleLogW(L"agl_rate:%d rand_result:%d\r\n", agl_rate, rand_result);
            if (rand_result < agl_rate)
            {
                result = false;
                break;
            }
        }

        // dex VS agl
        dex *= 10;
        agl *= 10;
        rand_result = randX((dex + agl) & 0xFFFE);
        WriteConsoleLogW(L"Hit %d(HIT*10) Mis %d(MIS*10) 命中(%d - %d) 结果[%d]\r\n", dex, agl, dex + agl, agl, rand_result);
        if (rand_result < agl)
        {
            result = false;
            break;
        }

        if (is_blind)
        {
            if (randX(10000) < 7000)
            {
                result = false;
                break;
            }
        }
        result = true;
        break;
    }
    src->IsHitMiss[dst->SoldierNo] = result ? 0 : 1;
    return result;
}

ASM VOID ed6Drive3Patch()
{
    __asm
    {
        cmp eax, ITEM_ID_DRIVE1;
        jl  drive_false;
        cmp eax, ITEM_ID_DRIVE2;
        jle drive_true;
        cmp eax, ITEM_ID_DRIVE3;
        je  drive_true
    drive_false:
        //add dword ptr[esp], 0x34;
        jmp addrDrive3PatchFalse
        //ret;
    drive_true:
        //add dword ptr[esp], 0x9;
        jmp addrDrive3PatchTrue
        //ret;
    }

}
#endif

#undef  ITEM_ID_DRIVE1
#undef  ITEM_ID_DRIVE2
#undef  ITEM_ID_DRIVE3
#undef  _ED61_NS_
#undef  _ED62_NS_
#undef  _ED63_NS_
#undef  _ED612_NS_
#undef  _ED623_NS_
#undef  _ED6123_NS_