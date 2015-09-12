#define MAGICSIZE 32

#include "ed6_common.h"
#include "ed6inf_data.h"
#include "misc_aki.h"

bool  g_bShowExtraInfo = true;
bool  g_bDisplayBattleIcoEx = false;
int parShowExtraInfo = 0;

#if CONSOLE_DEBUG
LARGE_INTEGER lFrequency, lStopCounter, lStartCounter;
#endif


typedef int (__cdecl *pSprintf)(char *_Dest, const char* _Format, ...);
typedef ULONG (__cdecl *pGetAddr)(ULONG);

__declspec(align(4))
typedef struct
{
	int a;
	int b;
} structAB;

structAB battleIcoRec = {16, 8};

int		nDifficulty = 0;
int		nSepithUpLimit = 0;
int		nShowAT = 0;
int		nShowConditionAT = 0;
UINT    nConditionATColor = 0xFF00FF00;

typedef struct
{
	INT	HP;
	INT	STR;
	INT	DEF;
	INT	ATS;
	INT	ADF;
	INT	SPD;
	INT	DEX;
	INT	AGL;
	INT	MOV;
	BOOL	ResistAbnormalCondition;
	BOOL	ResistAbilityDown;
} structStatusRate;

structStatusRate statusRateUserDefined;

BOOL WINAPI PeekMessageAOld(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	ASM_DUMMY_AUTO();
}

DWORD dwCount = 4;

BOOL WINAPI PeekMessageANew(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	BOOL bResult = PeekMessageAOld(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	if (_ReturnAddress() > (void*)0x600000 || _ReturnAddress() < (void*)0x400000)
	{
		return bResult;
	}

    if (bResult == False)
    {
//		SleepEx(1, FALSE);

        DWORD dwTickCount;
        static DWORD dwLastTickCount;

        dwTickCount = GetTickCount();
        if (dwTickCount - dwLastTickCount < 10 && --dwCount == 0)
        {
            dwCount = 4;
            SleepEx(1, FALSE);
        }
        dwLastTickCount = dwTickCount;

    }
    else if (lpMsg->message == WM_KEYDOWN)
    {
/*
        switch (lpMsg->wParam)
        {
            case VK_F1:
                ChangeFont();
                break;

            case VK_F2:
                g_bShowExtraInfo = True;
                break;

            case VK_F3:
                g_bShowExtraInfo = False;
                break;
        }
*/
    }

    return bResult;
}

HMODULE
WINAPI
GetModuleHandleAOld(
    LPCSTR lpModuleName
    )
{
	ASM_DUMMY_AUTO();
}

HMODULE
WINAPI
GetModuleHandleWOld(
    LPCWSTR lpModuleName
    )
{
	ASM_DUMMY_AUTO();
}

HMODULE
WINAPI
GetModuleHandleANew(
    LPCSTR lpModuleName
    )
{
	if (lpModuleName == NULL)
	{
		return GetModuleHandleAOld(lpModuleName);
	}
	if(*(lpModuleName+1) == '3' && *(lpModuleName+3) == '8')
	{
		if (_stricmp(lpModuleName, "d3d8.dll") == 0 || _stricmp(lpModuleName, "d3d8") == 0)
		{
			return GetModuleHandleAOld("system32\\d3d8.dll");
		}
	}
	return GetModuleHandleAOld(lpModuleName);
}

HMODULE
WINAPI
GetModuleHandleWNew(
    LPCWSTR lpModuleName
    )
{
	if (lpModuleName == NULL)
	{
		return GetModuleHandleWOld(lpModuleName);
	}
	if(*(lpModuleName+1) == L'3' && *(lpModuleName+3) == L'8')
	{
		if (_wcsicmp(lpModuleName, L"d3d8.dll") == 0 || _wcsicmp(lpModuleName, L"d3d8") == 0)
		{
			return GetModuleHandleWOld(L"system32\\d3d8.dll");
		}
	}
	return GetModuleHandleWOld(lpModuleName);
}

namespace NED63
{
	pSprintf sprintf = (pSprintf) 0x59234F;
	pGetAddr getItemInf = (pGetAddr) 0x4A0270;

	LPVOID	lpfnDisplayText = (LPVOID)0x0052B170;
	LPVOID	lpfnFtoL = (LPVOID)0x5922F0;
	LPVOID	lpfnResetRightBoard = (LPVOID)0x529530;
	LPVOID	lpfnDisplayBattleIco = (LPVOID)0x52BCB0;

	ULONG	addrDisplaySkipCondition0 = 0x0044A6F9;	// patch at
	//ULONG	addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
	ULONG	addrDisplaySkipCondition1 = 0x0044A6FE;
	ULONG	addrDisplaySkipCondition2 = 0x0044A792; // ret2

	ULONG	addrDisplayStatusLine = 0x5B7410;
	ULONG	addrDisplayStatusResist1 = 0x41B070;
	ULONG	addrDisplayStatusResist2 = 0x5B310C;
	ULONG	addrDisplayStatusResist3 = 0x5B3214;
	ULONG	addrDisplayStatusResist4 = 0x6A27FC;

	ULONG	addrDisplayStatusPatch0 = 0x44AC31;
	//ULONG	addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
	ULONG	addrDisplayStatusPatch1 = 0x44AC36;
	ULONG	addrDisplayStatusPatch2 = 0x52B1E0; // call

	ULONG	addrDisplayBattleIcoEx0 = 0x0052BCFA;
	DWORD	dwYStart = 0x110;

	ULONG	addrDisplayItemDropPatch0 = 0x0044A68E;
	//ULONG	addrDisplayItemDropPatch1 = addrDisplayItemDropPatch0 + 5;
	ULONG	addrDisplayItemDropPatch1 = 0x0044A693;
	ULONG	addrDisplayGetPar0 = 0x004061C0;
	//ULONG	addrDisplayGetPar1 = addrDisplayGetPar0 + 5;
	ULONG	addrDisplayGetPar2 = 0x27621F8; // 情报？
	ULONG	addrDisplayResetWidth0 = 0x0044A556;

	ULONG	addrSoldierNo0 = 0x672828;
	ULONG*	addrLPDir0 = (ULONG*)0x2CAD950;
	ULONG*	addrBattleNowIndex = (ULONG*)0x721A38;
	pGetAddr getItemDrop = (pGetAddr) 0x004A0E60;
	pGetAddr getItemName = (pGetAddr) 0x004A0370;

	ULONG	addrChangeEnemyStatusPatch0 = 0x0044CDAF;
	//ULONG	addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
	ULONG	addrChangeEnemyStatusPatch1 = 0x0044CDB4;
	ULONG	addrChangeEnemyStatusPatch2 = 0x004A4360; // call

    PSIZE   resolution = (PSIZE)0x005BDFF0; // 分辨率

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
			mov ecx,battleIcoRec.a;
			mov eax,battleIcoRec.b;
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

	FILE_IN_DIR*  __stdcall getFileInDirInf(FileIndex* fileIndex)
	{
		return (FILE_IN_DIR*)(*(addrLPDir0 + (fileIndex->DatNo))) + fileIndex->Index;
	}

	RESISTANCE __stdcall ed6GetResistance(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		UINT addr;
		RESISTANCE resist = lpBattleInf->Resistance;
		for (int i = 3; i < 5; i++)
		{
			if (lpBattleInf->Equip[i] == 0)
			{
				continue;
			}
			addr = getItemInf(lpBattleInf->Equip[i]);

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
			}
			__asm or resist,EBX
		}
		return resist;
	}

	ASM ED6_CHARACTER_BATTLE_INF* getSoldierAddr()
	{
		__asm
		{
			MOV EAX,ECX;
			SHL EAX,0x6;
			ADD EAX,ECX;
			LEA EAX,DWORD PTR DS:[EAX+EAX*8];
			SHL EAX,0x4;
			ADD EAX,addrSoldierNo0;
			ret;
		}
	}

	void FASTCALL ed6DisplayStatus(ULONG dwThis, ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		if (g_bShowExtraInfo == false)
		{
			return;
		}
		//DWORD	dwThis;
		DWORD	dwY = dwYStart + 0xA;

		//ED6_CHARACTER_BATTLE_INF*	lpBattleInf;

		//__asm mov dwThis,esi;
		//__asm sub ebx,0x2370
		//__asm mov lpBattleInf,ebx;

#if CONSOLE_DEBUG
		QueryPerformanceCounter(&lStartCounter);
#endif

		char szBuffer[0x120];
		static char *szStatus[] = { "Level", " STR", " DEF", " ATS", " ADF", " SPD", " DEX", " AGL", " MOV" };

		/*显示分界线*/
		__asm
		{
			push 0x0;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
			push 0x190;
			push 0x8;
			push addrDisplayStatusLine;
			push dwYStart;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
			push 0x1;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
		}

		int i;
		//char *p;
		for (i = 0; i < countof(szStatus); i++, dwY+=0xC)
		{
			sprintf(szBuffer, "%s", szStatus[i]);
			__asm
			{
				push 0x190;
				push 0xB;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

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

			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

		}

		/*显示抗性*/
		structAB position, ab;
		position.a = 0;
		position.b = dwY;

		ULONG_PTR	j;
		int nFind = 0;
		ULONG	condition;
		RESISTANCE resist = ed6GetResistance(lpBattleInf);
		for (i = 0, j = addrDisplayStatusResist2; j < addrDisplayStatusResist3; i++, j+=0x8)
		{
			__asm
			{
				push i;
				//mov eax,0x41B070;
				call addrDisplayStatusResist1;
				add esp,4;
				mov condition,eax;
			}

			if (condition == 0)
			{
				continue;
			}
			else
			{
				nFind++;
			}

			if ((condition & resist) != 0)
			{
				__asm
				{
					MOV EDI, j;
					FLD DWORD PTR DS:[EDI-0x4];
					call lpfnFtoL;
					MOV ab.a,EAX;

					FLD DWORD PTR DS:[EDI];
					call lpfnFtoL;
					MOV ab.b,EAX;

					LEA ECX,position;
					MOV EAX,addrDisplayStatusResist4
					MOV EAX,DWORD PTR DS:[EAX];
					push 0x0;
					LEA EDX,ab;
					push ECX;
					push EDX;
					push EAX;
					MOV ECX,dwThis;
					call lpfnDisplayBattleIco;
				}
			}

			position.a+=0x10;
			if (nFind % 8 == 0)
			{
				position.a = 0;
				position.b += 0x10;
			}
		}

		/*AT delay*/
		//STRdown 16,240 AGLdown 144,240
		//STRup 0,240 AGLup 128,240
		//dwX = 0x70;
		if ((0x8 & lpBattleInf->UnderAttackFlags) != 0)
		//if(1)
		{
			dwY = position.b;

			position.a = 0x70;
			position.b = dwY+0x8;
			ab.a = 16;
			ab.b = 240+0x8;

			sprintf(szBuffer, "AT");
			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x72;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

			g_bDisplayBattleIcoEx = true;
			//battleIcoRec.a = 16;
			//battleIcoRec.b = 8;

			__asm
			{
				LEA ECX,position;
				MOV EAX,addrDisplayStatusResist4
				MOV EAX,DWORD PTR DS:[EAX];
				push 0x0;
				LEA EDX,ab;
				push ECX;
				push EDX;
				push EAX;
				MOV ECX,dwThis;
				call lpfnDisplayBattleIco;
			}
		}
#if CONSOLE_DEBUG
		QueryPerformanceCounter(&lStopCounter);
		PrintConsoleW(L"Elapsed time: %lf ms\n", (lStopCounter.QuadPart - lStartCounter.QuadPart) * 1000.0 / lFrequency.QuadPart);
#endif
	}

	ASM void ed6DisplayStatusPatch()
	{
		__asm
		{
			cmp DWORD PTR DS:[ESI+0x1C38],0x14;
			jne L01;
			mov DWORD PTR DS:[ESI+0x1C38],0xE; //行间距 仿p bug？ [ESI+15] 02757E6D
L01:
			//mov eax,0x52B1E0;
			call addrDisplayStatusPatch2;
            mov edx, ebx;
            sub edx, 0x2370;
            mov ecx, esi;
			call ed6DisplayStatus;
			//push 0x44AC36;
			//ret;
			jmp addrDisplayStatusPatch1;
		}
	}

	void FASTCALL ed6DisplayItemDrop(ULONG dwThis, ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		if (g_bShowExtraInfo == false)
		{
			return;
		}
		//DWORD	dwThis;
		DWORD	dwY = 0x16;

		//ED6_CHARACTER_BATTLE_INF*	lpBattleInf;
		ED6_DROP_ITEM* lpDropItem;

		//__asm mov dwThis,esi;
		//__asm mov lpBattleInf,ebx;

		char szBuffer[0x120];

		int i, j, stealMin, steal[2];
		stealMin = 100;

		/*偷窃 位置*/
		for (i = 0; i < 2; i++)
		{
			if (lpBattleInf->DropProbability[i] == 0 || lpBattleInf->DropIndex[i] == 0xFFFF)
			{
				continue;
			}
;
			lpDropItem = (ED6_DROP_ITEM*)getItemDrop(lpBattleInf->DropIndex[i]);
			if (lpDropItem == NULL)
			{
				continue;
			}

			for (j = 0; j < 8; j++)
			{
				if (lpDropItem->item[j] == 0 || lpDropItem->item[j] == 0xFFFF || lpDropItem->probability[j] == 0)
				{
					continue;
				}
				if (lpDropItem->probability[j] <= stealMin)
				{
					stealMin = lpDropItem->probability[j];
					steal[0] = i;
					steal[1] = j;
				}
			}
		}

		sprintf(szBuffer, "Item:");
		__asm
		{
			push 0x190;
			push 0xB;
			lea eax, szBuffer;
			push eax;
			push dwY;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
		}
		dwY+=0xC;

		for (i = 0; i < 2; i++)
		{
			if (lpBattleInf->DropIndex[i] == 0 || lpBattleInf->DropIndex[i] == 0xFFFF)
			{
				continue;
			}

			//dwY+=0xC;
			//sprintf(szBuffer, "[%3d%%]Item %d:Group%d", lpBattleInf->DropProbability[i], i+1, lpBattleInf->DropIndex[i]);
			sprintf(szBuffer, "[%3d%%]Group%d", lpBattleInf->DropProbability[i], lpBattleInf->DropIndex[i]);
			__asm
			{
				push 0x190;
				push 0xB;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
			dwY+=0xC;

			lpDropItem = (ED6_DROP_ITEM*)getItemDrop(lpBattleInf->DropIndex[i]);
			if (lpDropItem == NULL)
			{
				continue;
			}
			for (j = 0; j < 8; j++)
			{
				if (lpDropItem->item[j] == 0 || lpDropItem->item[j] == 0xFFFF || lpDropItem->probability[j] == 0)
				{
					continue;
				}

				//sprintf(szBuffer, "[%3d%%]%d", lpDropItem->probability[j], lpDropItem->item[j]);
				sprintf(szBuffer, "[%3d%%]%s", lpDropItem->probability[j], getItemName(lpDropItem->item[j]));
				
				if (steal[0] == i && steal[1] == j)
				{
					__asm
					{
						push 0x190;
						push 0x6;
						lea eax, szBuffer;
						push eax;
						push dwY;
						push 0x0;
						MOV ECX,dwThis;
						call lpfnDisplayText;
					}
				}
				else
				{
					__asm
					{
						push 0x190;
						push 0x0;
						lea eax, szBuffer;
						push eax;
						push dwY;
						push 0x0;
						MOV ECX,dwThis;
						call lpfnDisplayText;
					}
				}
				dwY+=0xC;
			}
			if (i == 0)
			{
				dwY+=0x8;
			}
		}

		/*显示分界线*/
		__asm
		{
			push 0x0;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
			push 0x190;
			push 0x8;
			push addrDisplayStatusLine;
			push dwY;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
			push 0x1;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
		}
		dwY += 0xA;

		sprintf(szBuffer, "Craft Named:");
		__asm
		{
			push 0x190;
			push 0xB;
			lea eax, szBuffer;
			push eax;
			push dwY;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
		}
		dwY += 0xC;
		for (i = 0; i < 16; i++)
		{
			if (lpBattleInf->CraftIntro[i].Name[0] == 0 || *(USHORT*)(lpBattleInf->CraftIntro[i].Name) == 0x20)
			{
				continue;
			}
			sprintf(szBuffer, "%X:%s", i, lpBattleInf->CraftIntro[i].Name);
			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
			dwY += 0xC;
		}

		/*显示分界线*/
		__asm
		{
			push 0x0;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
			push 0x190;
			push 0x8;
			push addrDisplayStatusLine;
			push dwY;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
			push 0x1;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
		}
		dwY += 0xA;

		sprintf(szBuffer, "Misc:");
		__asm
		{
			push 0x190;
			push 0xB;
			lea eax, szBuffer;
			push eax;
			push dwY;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
		}
		dwY += 0xC;

		for (i = 0; i < 4; i++)
		{
			switch (i)
			{
			case 0:
				sprintf(szBuffer, "BattleIndex:0x%X", *addrBattleNowIndex);
				addrBattleNowIndex;
				break;
			case 1:
				sprintf(szBuffer, "DT%02X:%s", lpBattleInf->MSFileIndex.DatNo, //lpBattleInf->MSFileIndex.Index,
						getFileInDirInf(&lpBattleInf->MSFileIndex)->fileName);
				break;
			case 2:
				sprintf(szBuffer, "DT%02X:%s", lpBattleInf->ASFileIndex.DatNo, //lpBattleInf->MSFileIndex.Index,
						getFileInDirInf(&lpBattleInf->ASFileIndex)->fileName);
				break;
			case 3:
				sprintf(szBuffer, "DT%02X:%s", lpBattleInf->SYFileIndex.DatNo, //lpBattleInf->MSFileIndex.Index,
						getFileInDirInf(&lpBattleInf->SYFileIndex)->fileName);
				break;
			}

			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
			dwY += 0xC;
		}


	}

	ASM void ed6DisplayGetParOld()
	{
		ASM_DUMMY_AUTO();
	}

	ASM void ed6DisplayGetPar()
	{
		__asm
		{
			mov eax,addrDisplayGetPar2;
			cmp DWORD PTR DS:[EAX],0x1;
			jne L02;
			mov ecx,edx;
			call getSoldierAddr;
			mov ax,[EAX]ED6_CHARACTER_BATTLE_INF.wUseMagic
			cmp ax, 0x5E;
			je L01;
			cmp ax, 0x5F;
			je L01;
			cmp ax, 0x41;
			je L01;
L02:
			mov parShowExtraInfo,0;
			jmp ed6DisplayGetParOld;
L01:
			mov parShowExtraInfo,1;
			jmp ed6DisplayGetParOld;
		}
	}

	ASM void ed6DisplayResetWidthOld()
	{
		ASM_DUMMY_AUTO();
	}

	ASM void ed6DisplayResetWidth()
	{
		#pragma warning (disable: 4414)
		__asm
		{
			cmp parShowExtraInfo,1;
			jne ed6DisplayResetWidthOld;
			add dword ptr[ESP+8], 0x9;
			jmp ed6DisplayResetWidthOld;
		}
		#pragma warning (default: 4414)
	}

	ASM void ed6DisplayBattleSepith()
	{
		__asm
		{
			cmp ebp, 0x1E0;
			jle L01;
			mov ebp, 0x1E0;
L01:
			MOV ECX,0x21C;
			retn;
		}
	}

	ASM void ed6DisplayItemDropPatch()
	{
		__asm
		{
/*			cmp DWORD PTR DS:[ESI+0x60B0],0x1;
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
			cmp parShowExtraInfo,1;
			je L01;
			push 0x190;
			jmp addrDisplayItemDropPatch1;
L01:
            mov edx, ebx;
            mov ecx, esi;
            call ed6DisplayItemDrop;
			mov parShowExtraInfo,0;
			jmp addrDisplayStatusPatch1;
		}
	}

	void __cdecl ed6ChangeEnemyStatus(UINT SoldierNo, ED6_STATUS* pStatusSum, ED6_STATUS* pStatusBasic)
	{
		if (nDifficulty != 0 && !IsSoldierParty(SoldierNo))
		{
			ED6_CHARACTER_BATTLE_INF* lpBattleInf = (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0 + SoldierNo;

			if (lpBattleInf->StatusSum.HPMax == lpBattleInf->StatusSum.HP)
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				lpBattleInf->StatusSum.HP = lpBattleInf->StatusSum.HPMax;
			}
			else
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				SaturateConvert(&lpBattleInf->StatusSum.HP, (INT64)lpBattleInf->StatusSum.HP * statusRateUserDefined.HP / 100);
			}
			SaturateConvert(&lpBattleInf->StatusSum.STR, (INT64)lpBattleInf->StatusSum.STR * statusRateUserDefined.STR / 100);
			SaturateConvert(&lpBattleInf->StatusSum.DEF, (INT64)lpBattleInf->StatusSum.DEF * statusRateUserDefined.DEF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ATS, (INT64)lpBattleInf->StatusSum.ATS * statusRateUserDefined.ATS / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ADF, (INT64)lpBattleInf->StatusSum.ADF * statusRateUserDefined.ADF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.SPD, (INT64)lpBattleInf->StatusSum.SPD * statusRateUserDefined.SPD / 100);
			SaturateConvertEx(&lpBattleInf->StatusSum.DEX, (INT64)lpBattleInf->StatusSum.DEX * statusRateUserDefined.DEX / 100, (SHORT)0xCCC);
			SaturateConvertEx(&lpBattleInf->StatusSum.AGL, (INT64)lpBattleInf->StatusSum.AGL * statusRateUserDefined.AGL / 100, (SHORT)0xCCC);
			SaturateConvert(&lpBattleInf->StatusSum.MOV, (INT64)lpBattleInf->StatusSum.MOV * statusRateUserDefined.MOV / 100);
			if (statusRateUserDefined.ResistAbnormalCondition)
			{
				lpBattleInf->Resistance |= conditionAbnormal;
			}
			if (statusRateUserDefined.ResistAbilityDown)
			{
				lpBattleInf->Resistance |= conditionDown;
				lpBattleInf->UnderAttackFlags |= 0x8;
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
	void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, ULONG color)
	{
		ASM_DUMMY_AUTO();
	}
	void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, ULONG color)
	{
		if (nShowConditionAT == 1 || nShowConditionAT == 3)
		{
			if (AT > 99)
			{
				if (nShowConditionAT == 1) return;
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
}

namespace NED62
{
	pSprintf sprintf = (pSprintf) 0x53A93F;
	pGetAddr getItemInf = (pGetAddr) 0x4C63A0;

	LPVOID	lpfnDisplayText = (LPVOID)0x0048C3A0;
	LPVOID	lpfnFtoL = (LPVOID)0x0053A8DC;
	LPVOID	lpfnResetRightBoard = (LPVOID)0x48A7A0;
	LPVOID	lpfnDisplayBattleIco = (LPVOID)0x48CE60;

	ULONG	addrDisplaySkipCondition0 = 0x004412B7;	// patch at
	//ULONG	addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
	ULONG	addrDisplaySkipCondition1 = 0x004412BC;
	ULONG	addrDisplaySkipCondition2 = 0x00441350; // ret2

	ULONG	addrDisplayStatusLine = 0x55C1DC;
	ULONG	addrDisplayStatusResist1 = 0x418610;
	ULONG	addrDisplayStatusResist2 = 0x558E9C;
	ULONG	addrDisplayStatusResist3 = 0x558F9C;
	ULONG	addrDisplayStatusResist4 = 0x63ED14;

	ULONG	addrDisplayStatusPatch0 = 0x4418D0;
	//ULONG	addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
	ULONG	addrDisplayStatusPatch1 = 0x04418D5;
	ULONG	addrDisplayStatusPatch2 = 0x0048C410; // call

	ULONG	addrDisplayBattleIcoEx0 = 0x0048CEAA;
	DWORD	dwYStart = 0x110;

	ULONG	addrSoldierNo0 = 0x60EF38;
	ULONG	addrChangeEnemyStatusPatch0 = 0x00443592;
	//ULONG	addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
	ULONG	addrChangeEnemyStatusPatch1 = 0x00443597;
	ULONG	addrChangeEnemyStatusPatch2 = 0x004CE170; // call

    PSIZE   resolution = (PSIZE)0x005643F8; // 分辨率

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

	RESISTANCE __stdcall ed6GetResistance(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		UINT addr;
		RESISTANCE resist = lpBattleInf->Resistance;
		for (int i = 3; i < 5; i++)
		{
			if (lpBattleInf->Equip[i] == 0)
			{
				continue;
			}
			addr = getItemInf(lpBattleInf->Equip[i]);

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
			}
			__asm or resist,EBX
		}
		return resist;
	}

	void FASTCALL ed6DisplayStatus(ULONG dwThis, ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		if (g_bShowExtraInfo == false)
		{
			return;
		}
		//DWORD	dwThis;
		DWORD	dwY = dwYStart + 0xA;

		//ED6_CHARACTER_BATTLE_INF*	lpBattleInf;

		//__asm mov dwThis,esi;
		//__asm mov eax, dword ptr[ebp];
		//__asm sub eax,0x2358
		//__asm mov lpBattleInf,eax;

		char szBuffer[0x120];
		static char *szStatus[] = { "Level", " STR", " DEF", " ATS", " ADF", " SPD", " DEX", " AGL", " MOV" };

		/*显示分界线*/
		__asm
		{
			push 0x0;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
			push 0x190;
			push 0x8;
			push addrDisplayStatusLine;
			push dwYStart;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
			push 0x1;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
		}

		/*物品掉落概率*/
		if (lpBattleInf->DropIndex1 !=0 && lpBattleInf->DropIndex1 !=0xFFFF)
		{
			sprintf(szBuffer, "%3d%%", lpBattleInf->DropProbability1);
			__asm
			{
				push 0x190;
				push 0x6;
				lea eax, szBuffer;
				push eax;
				push 0xBA;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
		}
		if (lpBattleInf->DropIndex2 !=0 && lpBattleInf->DropIndex2 !=0xFFFF)
		{
			sprintf(szBuffer, "%3d%%", lpBattleInf->DropProbability2);
			__asm
			{
				push 0x190;
				push 0x6;
				lea eax, szBuffer;
				push eax;
				push 0xC8;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
		}

		int i;
		//char *p;
		for (i = 0; i < countof(szStatus); i++, dwY+=0xC)
		{
			sprintf(szBuffer, "%s", szStatus[i]);
			__asm
			{
				push 0x190;
				push 0xB;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

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

			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

		}

		/*显示抗性*/
		structAB position, ab;
		position.a = 0;
		position.b = dwY;

		ULONG_PTR	j;
		int nFind = 0;
		ULONG	condition;
		RESISTANCE resist = ed6GetResistance(lpBattleInf);
		//RESISTANCE resist = lpBattleInf->Resistance;
		for (i = 0, j = addrDisplayStatusResist2; j < addrDisplayStatusResist3; i++, j+=0x8)
		{
			__asm
			{
				push i;
				//mov eax,0x418610;
				call addrDisplayStatusResist1;
				add esp,4;
				mov condition,eax;
			}

			if (condition == 0)
			{
				continue;
			}
			else
			{
				nFind++;
			}

			if ((condition & resist) != 0)
			{
				__asm
				{
					MOV EDI, j;
					FLD DWORD PTR DS:[EDI-0x4];
					call lpfnFtoL;
					MOV ab.a,EAX;

					FLD DWORD PTR DS:[EDI];
					call lpfnFtoL;
					MOV ab.b,EAX;

					LEA ECX,position;
					MOV EAX,addrDisplayStatusResist4
					MOV EAX,DWORD PTR DS:[EAX];
					push 0x0;
					LEA EDX,ab;
					push ECX;
					push EDX;
					push EAX;
					MOV ECX,dwThis;
					call lpfnDisplayBattleIco;
				}
			}

			position.a+=0x10;
			if (nFind % 8 == 0)
			{
				position.a = 0;
				position.b += 0x10;
			}
		}

		/*AT delay*/
		//STRdown 16,240 AGLdown 144,240
		//STRup 0,240 AGLup 128,240
		//dwX = 0x70;
		if ((0x8 & lpBattleInf->UnderAttackFlags) != 0)
		//if(1)
		{
			dwY = position.b;

			position.a = 0x70;
			position.b = dwY+0x8;
			ab.a = 16;
			ab.b = 240+0x8;

			sprintf(szBuffer, "AT");
			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x72;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

			g_bDisplayBattleIcoEx = true;

			__asm
			{
				LEA ECX,position;
				MOV EAX,addrDisplayStatusResist4
				MOV EAX,DWORD PTR DS:[EAX];
				push 0x0;
				LEA EDX,ab;
				push ECX;
				push EDX;
				push EAX;
				MOV ECX,dwThis;
				call lpfnDisplayBattleIco;
			}
		}
	}

	ASM void ed6DisplayStatusPatch()
	{
		__asm
		{
			//mov eax,0x0048C410;
			call addrDisplayStatusPatch2;
            mov edx, ebp;
            sub edx, 0x2358;
            mov ecx, esi;
			call ed6DisplayStatus;
			//push 0x04418D5;
			jmp addrDisplayStatusPatch1;
		}
	}

	void __cdecl ed6ChangeEnemyStatus(UINT SoldierNo, ED6_STATUS* pStatusSum, ED6_STATUS* pStatusBasic)
	{
		if (nDifficulty != 0 && !IsSoldierParty(SoldierNo))
		{
			ED6_CHARACTER_BATTLE_INF* lpBattleInf = (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0 + SoldierNo;

			if (lpBattleInf->StatusSum.HPMax == lpBattleInf->StatusSum.HP)
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				lpBattleInf->StatusSum.HP = lpBattleInf->StatusSum.HPMax;
			}
			else
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				SaturateConvert(&lpBattleInf->StatusSum.HP, (INT64)lpBattleInf->StatusSum.HP * statusRateUserDefined.HP / 100);
			}
			SaturateConvert(&lpBattleInf->StatusSum.STR, (INT64)lpBattleInf->StatusSum.STR * statusRateUserDefined.STR / 100);
			SaturateConvert(&lpBattleInf->StatusSum.DEF, (INT64)lpBattleInf->StatusSum.DEF * statusRateUserDefined.DEF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ATS, (INT64)lpBattleInf->StatusSum.ATS * statusRateUserDefined.ATS / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ADF, (INT64)lpBattleInf->StatusSum.ADF * statusRateUserDefined.ADF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.SPD, (INT64)lpBattleInf->StatusSum.SPD * statusRateUserDefined.SPD / 100);
			SaturateConvertEx(&lpBattleInf->StatusSum.DEX, (INT64)lpBattleInf->StatusSum.DEX * statusRateUserDefined.DEX / 100, (SHORT)0xCCC);
			SaturateConvertEx(&lpBattleInf->StatusSum.AGL, (INT64)lpBattleInf->StatusSum.AGL * statusRateUserDefined.AGL / 100, (SHORT)0xCCC);
			SaturateConvert(&lpBattleInf->StatusSum.MOV, (INT64)lpBattleInf->StatusSum.MOV * statusRateUserDefined.MOV / 100);
			if (statusRateUserDefined.ResistAbnormalCondition)
			{
				lpBattleInf->Resistance |= conditionAbnormal;
			}
			if (statusRateUserDefined.ResistAbilityDown)
			{
				lpBattleInf->Resistance |= conditionDown;
				lpBattleInf->UnderAttackFlags |= 0x8;
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

	void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
	{
		ASM_DUMMY_AUTO();
	}
	void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
	{
		if (nShowConditionAT == 1 || nShowConditionAT == 3)
		{
			if (AT > 99)
			{
				if (nShowConditionAT == 1) return;
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
}

namespace NED61
{
	pSprintf sprintf = (pSprintf) 0x004FC9FF;
	pGetAddr getItemInf = (pGetAddr) 0x4A84F0;

	LPVOID	lpfnDisplayText = (LPVOID)0x00474DC0;
	LPVOID	lpfnFtoL = (LPVOID)0x004FC99C;
	LPVOID	lpfnResetRightBoard = (LPVOID)0x00473260;
	LPVOID	lpfnDisplayBattleIco = (LPVOID)0x00475880;

	ULONG	addrDisplaySkipCondition0 = 0x0043411C;	// patch at
	//ULONG	addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
	ULONG	addrDisplaySkipCondition1 = 0x00434121;
	ULONG	addrDisplaySkipCondition2 = 0x004341B8; // ret2

	ULONG	addrDisplayStatusLine = 0x51A294;
	ULONG	addrDisplayStatusResist1 = 0x413220;
	ULONG	addrDisplayStatusResist2 = 0x517BCC;
	ULONG	addrDisplayStatusResist3 = 0x517CBC;
	ULONG	addrDisplayStatusResist4 = 0x5CB918;

	ULONG	addrDisplayStatusPatch0 = 0x434737;
	//ULONG	addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
	ULONG	addrDisplayStatusPatch1 = 0x0043473C;
	ULONG	addrDisplayStatusPatch2 = 0x00474E30; // call

	ULONG	addrDisplayBattleIcoEx0 = 0x004758CA;
	DWORD	dwYStart = 0x112;

	ULONG	addrSoldierNo0 = 0x5A58D0;
	ULONG	addrChangeEnemyStatusPatch0 = 0x00435036;
	//ULONG	addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
	ULONG	addrChangeEnemyStatusPatch1 = 0x0043503B;
	ULONG	addrChangeEnemyStatusPatch2 = 0x004AE7E0; // call

    PSIZE   resolution = (PSIZE)0x005204DC; // 分辨率

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

	RESISTANCE __stdcall ed6GetResistance(ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		UINT addr;
		RESISTANCE resist = lpBattleInf->Resistance;
		for (int i = 3; i < 5; i++)
		{
			if (lpBattleInf->Equip[i] == 0)
			{
				continue;
			}
			addr = getItemInf(lpBattleInf->Equip[i]);

			if (*(byte*)(addr+5) != 1)
			{
				continue;
			}

			__asm xor ebx,ebx;
			switch (*(byte*)(addr+6))
			{
			case 1:
				__asm or  BL,0x1
				break;
			case 2:
				__asm or  BL,0x2
				break;
			case 3:
				__asm or  BL,0x4
				break;
			case 4:
				__asm or  BL,0x8
				break;
			case 5:
				__asm or  BL,0x10
				break;
			case 6:
				__asm or  BL,0x20
				break;
			case 7:
				__asm or  BL,0x40
				break;
			case 8:
				__asm or  BL,0x80
				break;
			case 9:
				__asm or  BH,0x1
				break;
			case 10:
				__asm or  BH,0x2
				break;
			case 11:
				__asm or  BL,0x6
				break;
			case 12:
				__asm or  EBX,0x108
				break;
			case 13:
				__asm or  EBX,0x3FF
				break;
			}
			__asm or resist,EBX
		}
		return resist;
	}

	void FASTCALL ed6DisplayStatus(ULONG dwThis, ED6_CHARACTER_BATTLE_INF* lpBattleInf)
	{
		if (g_bShowExtraInfo == false)
		{
			return;
		}
		//DWORD	dwThis;
		DWORD	dwY = dwYStart + 0xA;

		//ED6_CHARACTER_BATTLE_INF*	lpBattleInf;

		//__asm mov dwThis,esi;
		//__asm sub ebx,0x22A8
		//__asm mov lpBattleInf,ebx;

		char szBuffer[0x120];
		static char *szStatus[] = { "Level", " STR", " DEF", " ATS", " ADF", " SPD", " DEX", " AGL", " MOV" };

		/*显示分界线*/
		__asm
		{
			push 0x0;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
			push 0x190;
			push 0x8;
			push addrDisplayStatusLine;
			push dwYStart;
			push 0x0;
			MOV ECX,dwThis;
			call lpfnDisplayText;
			push 0x1;
			MOV ECX,dwThis;
			call lpfnResetRightBoard;
		}

		/*物品掉落概率*/
		if (lpBattleInf->DropIndex1 !=0 && lpBattleInf->DropIndex1 !=0xFFFF)
		{
			sprintf(szBuffer, "%3d%%", lpBattleInf->DropProbability1);
			__asm
			{
				push 0x190;
				push 0x6;
				lea eax, szBuffer;
				push eax;
				push 0xBA;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
		}
		if (lpBattleInf->DropIndex2 !=0 && lpBattleInf->DropIndex2 !=0xFFFF)
		{
			sprintf(szBuffer, "%3d%%", lpBattleInf->DropProbability2);
			__asm
			{
				push 0x190;
				push 0x6;
				lea eax, szBuffer;
				push eax;
				push 0xC8;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}
		}

		int i;
		//char *p;
		for (i = 0; i < countof(szStatus); i++, dwY+=0xC)
		{
			sprintf(szBuffer, "%s", szStatus[i]);
			__asm
			{
				push 0x190;
				push 0xB;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

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

			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x0;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

		}

		/*显示抗性*/
		structAB position, ab;
		position.a = 0;
		position.b = dwY;

		ULONG_PTR	j;
		int nFind = 0;
		ULONG	condition;
		RESISTANCE resist = ed6GetResistance(lpBattleInf);
		//RESISTANCE resist = lpBattleInf->Resistance;
		for (i = 0, j = addrDisplayStatusResist2; j < addrDisplayStatusResist3; i++, j+=0x8)
		{
			__asm
			{
				push i;
				//mov eax,0x413220;
				call addrDisplayStatusResist1;
				add esp,4;
				mov condition,eax;
			}

			if (condition == 0)
			{
				continue;
			}
			else
			{
				nFind++;
			}

			if ((condition & resist) != 0)
			{
				__asm
				{
					MOV EDI, j;
					FLD DWORD PTR DS:[EDI-0x4];
					call lpfnFtoL;
					MOV ab.a,EAX;

					FLD DWORD PTR DS:[EDI];
					call lpfnFtoL;
					MOV ab.b,EAX;

					LEA ECX,position;
					mov eax,addrDisplayStatusResist4
					MOV EAX,DWORD PTR DS:[eax];
					//push 0x0;
					LEA EDX,ab;
					push ECX;
					push EDX;
					push EAX;
					MOV ECX,dwThis;
					call lpfnDisplayBattleIco;
				}
			}

			position.a+=0x10;
			if (nFind % 8 == 0)
			{
				position.a = 0;
				position.b += 0x10;
			}
		}

		/*AT delay*/
		//STRdown 16,240 AGLdown 144,240
		//STRup 0,240 AGLup 128,240
		//dwX = 0x70;
		if ((0x8 & lpBattleInf->UnderAttackFlags) != 0)
		//if(1)
		{
			dwY = position.b;

			position.a = 0x70;
			position.b = dwY+0x8;
			ab.a = 16;
			ab.b = 240+0x8;

			sprintf(szBuffer, "AT");
			__asm
			{
				push 0x190;
				push 0x0;
				lea eax, szBuffer;
				push eax;
				push dwY;
				push 0x72;
				MOV ECX,dwThis;
				call lpfnDisplayText;
			}

			g_bDisplayBattleIcoEx = true;

			__asm
			{
				LEA ECX,position;
				mov eax,addrDisplayStatusResist4
				MOV EAX,DWORD PTR DS:[eax];
				//push 0x0;
				LEA EDX,ab;
				push ECX;
				push EDX;
				push EAX;
				MOV ECX,dwThis;
				call lpfnDisplayBattleIco;
			}
		}
	}

	ASM void ed6DisplayStatusPatch()
	{
		__asm
		{
			//mov eax,0x00474E30;
			call addrDisplayStatusPatch2;
            mov edx, ebx;
            sub edx, 0x22A8;
            mov ecx, esi;
			call ed6DisplayStatus;
			//push 0x0043473C;
			jmp addrDisplayStatusPatch1;
		}
	}

	void __cdecl ed6ChangeEnemyStatus(UINT SoldierNo, ED6_STATUS* pStatusSum, ED6_STATUS* pStatusBasic)
	{
		if (nDifficulty != 0 && !IsSoldierParty(SoldierNo))
		{
			ED6_CHARACTER_BATTLE_INF* lpBattleInf = (ED6_CHARACTER_BATTLE_INF*)addrSoldierNo0 + SoldierNo;

			if (lpBattleInf->StatusSum.HPMax == lpBattleInf->StatusSum.HP)
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				lpBattleInf->StatusSum.HP = lpBattleInf->StatusSum.HPMax;
			}
			else
			{
				SaturateConvert(&lpBattleInf->StatusSum.HPMax, (INT64)lpBattleInf->StatusSum.HPMax * statusRateUserDefined.HP / 100);
				SaturateConvert(&lpBattleInf->StatusSum.HP, (INT64)lpBattleInf->StatusSum.HP * statusRateUserDefined.HP / 100);
			}
			SaturateConvert(&lpBattleInf->StatusSum.STR, (INT64)lpBattleInf->StatusSum.STR * statusRateUserDefined.STR / 100);
			SaturateConvert(&lpBattleInf->StatusSum.DEF, (INT64)lpBattleInf->StatusSum.DEF * statusRateUserDefined.DEF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ATS, (INT64)lpBattleInf->StatusSum.ATS * statusRateUserDefined.ATS / 100);
			SaturateConvert(&lpBattleInf->StatusSum.ADF, (INT64)lpBattleInf->StatusSum.ADF * statusRateUserDefined.ADF / 100);
			SaturateConvert(&lpBattleInf->StatusSum.SPD, (INT64)lpBattleInf->StatusSum.SPD * statusRateUserDefined.SPD / 100);
			SaturateConvertEx(&lpBattleInf->StatusSum.DEX, (INT64)lpBattleInf->StatusSum.DEX * statusRateUserDefined.DEX / 100, (SHORT)0xCCC);
			SaturateConvertEx(&lpBattleInf->StatusSum.AGL, (INT64)lpBattleInf->StatusSum.AGL * statusRateUserDefined.AGL / 100, (SHORT)0xCCC);
			SaturateConvert(&lpBattleInf->StatusSum.MOV, (INT64)lpBattleInf->StatusSum.MOV * statusRateUserDefined.MOV / 100);
			if (statusRateUserDefined.ResistAbnormalCondition)
			{
				lpBattleInf->Resistance |= conditionAbnormal;
			}
			if (statusRateUserDefined.ResistAbilityDown)
			{
				lpBattleInf->Resistance |= conditionDown;
				lpBattleInf->UnderAttackFlags |= 0x8;
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

	void __cdecl ed6ShowConditionAtOld(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
	{
		ASM_DUMMY_AUTO();
	}
	void __cdecl ed6ShowConditionAtNew(ULONG AT, float x, float y, float width, float height, ULONG a6, ULONG a7, ULONG color)
	{
		if (nShowConditionAT == 1 || nShowConditionAT == 3)
		{
			if (AT > 99)
			{
				if (nShowConditionAT == 1) return;
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
    char szConfigExPath[] = ".\\configEx.ini";
    nDifficulty = NINI::GetPrivateProfileIntA("Battle", "Difficulty", 0, szConfigExPath);
    if (nDifficulty != 0 && nDifficulty != 1)
    {
        nDifficulty = 0;
    }
    if (nDifficulty == 1)
    {
        statusRateUserDefined.HP = NINI::GetPrivateProfileIntA("Battle", "HP", 100, szConfigExPath);
        statusRateUserDefined.STR = NINI::GetPrivateProfileIntA("Battle", "STR", 100, szConfigExPath);
        statusRateUserDefined.DEF = NINI::GetPrivateProfileIntA("Battle", "DEF", 100, szConfigExPath);
        statusRateUserDefined.ATS = NINI::GetPrivateProfileIntA("Battle", "ATS", 100, szConfigExPath);
        statusRateUserDefined.ADF = NINI::GetPrivateProfileIntA("Battle", "ADF", 100, szConfigExPath);
        statusRateUserDefined.SPD = NINI::GetPrivateProfileIntA("Battle", "SPD", 100, szConfigExPath);
        statusRateUserDefined.DEX = NINI::GetPrivateProfileIntA("Battle", "DEX", 100, szConfigExPath);
        statusRateUserDefined.AGL = NINI::GetPrivateProfileIntA("Battle", "AGL", 100, szConfigExPath);
        statusRateUserDefined.MOV = NINI::GetPrivateProfileIntA("Battle", "MOV", 100, szConfigExPath);
        statusRateUserDefined.ResistAbnormalCondition = NINI::GetPrivateProfileBoolA("Battle", "ResistAbnormalCondition", false, szConfigExPath);
        statusRateUserDefined.ResistAbilityDown = NINI::GetPrivateProfileBoolA("Battle", "ResistAbilityDown", false, szConfigExPath);

    }

    nSepithUpLimit = NINI::GetPrivateProfileIntA("Battle", "SepithUpLimit", 0, szConfigExPath);
    SaturateConvertEx(&nSepithUpLimit, nSepithUpLimit, 9999, 0);

    nShowAT = NINI::GetPrivateProfileIntA("Battle", "ShowAT", 1, szConfigExPath);
    if (nShowAT < 0 || nShowAT > 2)	nShowAT = 1;

    nShowConditionAT = NINI::GetPrivateProfileIntA("Battle", "ShowConditionAT", 1, szConfigExPath);
    if (nShowConditionAT != 0 && nShowConditionAT != 3 && nShowConditionAT != 5)	nShowConditionAT = 1;

    nConditionATColor = NINI::GetPrivateProfileIntA("Battle", "ConditionATColor", FLAG_ON(g_GameVersion, ed63min) ? -1 : 0xFF00FF00, szConfigExPath);

#if CONSOLE_DEBUG
    PrintConsoleW(L"%s: 0x%X\r\n", L"ConditionATColor", nConditionATColor);
#endif
}

void Init()
{
#if 1

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
#endif
	//AllocConsole();
	//HMODULE hModuleKernel32=GetModuleHandle("kernel32.dll"); 
	//FARPROC pFunOutputDebugStringA = GetProcAddress(hModuleKernel32, "OutputDebugStringA");
	//PrintConsoleW(L"%x %x\r\n", pFunOutputDebugStringA, PrintDebugStringA);
	//PrintConsoleW(L"%x %x\r\n", hModule, hModuleKernel32);
	//Nt_PatchMemory(NULL, 0, fApiHook, countof(fApiHook), hModule);
//#if D3D8_VERSION
#if 0
	extern HMODULE hModuleSelf;
	WCHAR wszFileNameSelf[MAX_PATH];
	Nt_GetModuleFileName(hModuleSelf, wszFileNameSelf, MAX_PATH);
	if (_wcsicmp(findnamew(wszFileNameSelf), L"d3d8.dll") == 0)
	{
		HMODULE hModuleKernel32 = Nt_GetModuleHandle(L"kernel32.dll");
		MEMORY_FUNCTION_PATCH f_global1[] =
		{
			INLINE_HOOK(Nt_GetProcAddress(hModuleKernel32, "GetModuleHandleW"), GetModuleHandleWNew, GetModuleHandleWOld),
			INLINE_HOOK(Nt_GetProcAddress(hModuleKernel32, "GetModuleHandleA"), GetModuleHandleANew, GetModuleHandleAOld),
		};
		Nt_PatchMemory(NULL, 0, f_global1, countof(f_global1), hModule);
	}
#endif

#if 0
	if (1)
	{
		MEMORY_FUNCTION_PATCH f_global[] =
		{
			//PATCH_FUNCTION(JUMP, 0, 0x04A6F9, ed6DisplaySkipCondition, 0),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
			INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"user32.dll"), "PeekMessageA"), PeekMessageANew, PeekMessageAOld),
		};
		Nt_PatchMemory(NULL, 0, f_global, countof(f_global), hModule);
		return;

	}
#endif

#endif //折叠

	if (gameVersion == ed63cn7)
	{
		using namespace NED63;
		if (nSepithUpLimit == 0) nSepithUpLimit = 300;

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x0043CF5F -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x0041C652 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x1B73A0),	// Exp %4d->%5d
			PATCH_MEMORY(0xFC,		1, 0x04A9B7),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0x00,		1, 0x04A9D3),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0xEB,      1, 0x006185),	// party display extra inf
			//	PATCH_MEMORY(0xACE9,	4, 0x04A6E2),	// not display Condition
			PATCH_MEMORY(0x00,      1, 0x04A986),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x04A53F),	// height
			PATCH_MEMORY(0x30,      1, 0x04A547),	// height2
			PATCH_MEMORY(0x00,      1, 0x0061BE),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408E97 -0x00400000),
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00406F98 -0x00400000),
			PATCH_MEMORY(0x5A33F0,	4, 0x0041C672 -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY(0x5A33F0,	4, 0x0041C67F -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x0041C643 -0x00400000),	// 是否显状态AT

			PATCH_MEMORY(0x01,      1, 0x004A64D3 -0x00400000),	// LG_Font2
			//PATCH_MEMORY(0xEB,      1, 0x004A64D4 -0x00400000),	// LG_Font1
			//PATCH_MEMORY(0xEB,      1, 0x004A65A6 -0x00400000),	// LG_Font4
			//PATCH_MEMORY(0xEB,      1, 0x004A65EC -0x00400000),	// LG_Font5
			//PATCH_MEMORY(0xEB,      1, 0x004A6630 -0x00400000),	// LG_Font6
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			//PATCH_FUNCTION(JUMP, 0, 0x04A6F9, ed6DisplaySkipCondition, 0),
			//PATCH_FUNCTION(CALL, 0, 0x12BCFA, NED63::ed6DisplayBattleIcoEx, 1),
			//PATCH_FUNCTION(JUMP, 0, 0x04AC31, ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,	ed6DisplayItemDropPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,			ed6DisplayGetPar, 0, ed6DisplayGetParOld),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, 0x00446DDF,	ed6DisplayBattleSepith, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048D310,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed63jp7)
	{
		using namespace NED63;
		if (nSepithUpLimit == 0) nSepithUpLimit = 300;

		NED63::sprintf = (pSprintf) 0x0058C7AF;
		getItemInf = (pGetAddr) 0x0049F750;

		lpfnDisplayText = (LPVOID)0x00528B10;
		lpfnFtoL = (LPVOID)0x0058C750;
		lpfnResetRightBoard = (LPVOID)0x00526F20;
		lpfnDisplayBattleIco = (LPVOID)0x00529640;

		addrDisplaySkipCondition0 = 0x0044A0B9;	// patch at
		addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
		addrDisplaySkipCondition2 = 0x0044A152; // ret2

		addrDisplayStatusLine = 0x5B18E8;
		addrDisplayStatusResist1 = 0x41ADC0;
		addrDisplayStatusResist2 = 0x5AD1F4;
		addrDisplayStatusResist3 = 0x5AD2FC;
		addrDisplayStatusResist4 = 0x69E750;

		addrDisplayStatusPatch0 = 0x0044A5F2;
		addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
		addrDisplayStatusPatch2 = 0x00528B80; // call

		addrDisplayBattleIcoEx0 = 0x0052968A;
		dwYStart = 0x112;

		addrDisplayItemDropPatch0 = 0x0044A04E;
		addrDisplayItemDropPatch1 = addrDisplayItemDropPatch0 + 5;
		addrDisplayGetPar0 = 0x004061D0;
		//ULONG	addrDisplayGetPar1 = addrDisplayGetPar0 + 5;
		addrDisplayGetPar2 = 0x275E048; // 情报？
		addrDisplayResetWidth0 = 0x00449F16;

		addrSoldierNo0 = 0x66E780;
		addrLPDir0 = (ULONG*)0x2CAAFD0;
		addrBattleNowIndex = (ULONG*)0x71D900;
		getItemDrop = (pGetAddr) 0x004A0320;
		getItemName = (pGetAddr) 0x0049F850;

		addrChangeEnemyStatusPatch0 = 0x0044C72F;
		addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
		addrChangeEnemyStatusPatch2 = 0x004A36A0; // call

        resolution = (PSIZE)0x005B86C0; // 分辨率

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x0043CA4F -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x0041C3A2 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x1B186C),	// Exp %4d->%5d
			PATCH_MEMORY(0xD4,		1, 0x04A377),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0x00,		1, 0x04A393),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0xEB,      1, 0x006195),	// party display extra inf
			//	PATCH_MEMORY(0xACE9,	4, 0x04A6E2),	// not display Condition
			PATCH_MEMORY(0x00,      1, 0x04A346),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x049EFF),	// height
			PATCH_MEMORY(0x30,      1, 0x049F07),	// height2
			PATCH_MEMORY(0x00,      1, 0x0061CE),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408EA7 -0x00400000),
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00406FA8 -0x00400000),
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x0041C393 -0x00400000),	// 是否显状态AT
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,	ed6DisplayItemDropPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,			ed6DisplayGetPar, 0, ed6DisplayGetParOld),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,		ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, 0x0044677F,	ed6DisplayBattleSepith, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048C710,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed63jp1002) // 老版jp 及 仿p
	{
		using namespace NED63;
		if (nSepithUpLimit == 0) nSepithUpLimit = 300;

		NED63::sprintf = (pSprintf) 0x0058BEEF;
		getItemInf = (pGetAddr) 0x0049F520;

		lpfnDisplayText = (LPVOID)0x00528A30;
		lpfnFtoL = (LPVOID)0x0058BE90;
		lpfnResetRightBoard = (LPVOID)0x00526D60;
		lpfnDisplayBattleIco = (LPVOID)0x00529560;

		addrDisplaySkipCondition0 = 0x0044A039;	// patch at
		addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
		addrDisplaySkipCondition2 = 0x0044A0D2; // ret2

		addrDisplayStatusLine = 0x5B18E8;
		addrDisplayStatusResist1 = 0x0041ADC0;
		addrDisplayStatusResist2 = 0x5AD1F4;
		addrDisplayStatusResist3 = 0x5AD2FC;
		addrDisplayStatusResist4 = 0x69E610;

		addrDisplayStatusPatch0 = 0x0044A572;
		addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
		addrDisplayStatusPatch2 = 0x00528AA0; // call

		addrDisplayBattleIcoEx0 = 0x005295AA;
		dwYStart = 0x112;

		addrDisplayItemDropPatch0 = 0x00449FCE;
		addrDisplayItemDropPatch1 = addrDisplayItemDropPatch0 + 5;
		addrDisplayGetPar0 = 0x004061D0;
		//ULONG	addrDisplayGetPar1 = addrDisplayGetPar0 + 5;
		addrDisplayGetPar2 = 0x275DF08; // 情报？
		addrDisplayResetWidth0 = 0x00449EE6;

		addrSoldierNo0 = 0x66E640;
		addrLPDir0 = (ULONG*)0x2CA9538;
		addrBattleNowIndex = (ULONG*)0x71D7C0;
		getItemDrop = (pGetAddr) 0x004A00F0;
		getItemName = (pGetAddr) 0x0049F620;

		addrChangeEnemyStatusPatch0 = 0x0044C6AF;
		addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
		addrChangeEnemyStatusPatch2 = 0x004A35B0; // call

        resolution = (PSIZE)0x005B8644; // 分辨率

		unsigned char p0044A548[9] = { 0x6A, 0x01, 0x8B, 0xCE, 0xE8, 0x0F, 0xC8, 0x0D, 0x00 };
		unsigned char p00528A34[5] = { 0xE8, 0xD7, 0xFD, 0xFF, 0xFF };
		unsigned char p0043CA3F[6] = { 0xE9, 0x2E, 0x01, 0x00, 0x00, 0x90 };

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (*(UINT*)0x005B8654 == 0x59977089 && *(UINT*)0x004A0C34 == 0x5B863C) //日版 windows名称
		{
			MEMORY_FUNCTION_PATCH f1[] =
			{
				PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
				PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,		ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only
			};
			Nt_PatchMemory(NULL, 0, f1, countof(f1), hModule);
		}

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x0043CA3F -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x0041C3A2 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(p0044A548,	9, 0x04A548),	// 仿p恢复
			//PATCH_MEMORY(0xB4,		1, 0x04A041),	// 仿p恢复 condition
			PATCH_MEMORY(p00528A34,	5, 0x128A34),	// 仿p恢复 display + 0xF8
			PATCH_MEMORY(0x00,		1, 0x04A3C5),	// 仿p恢复 Exp -6
			//PATCH_MEMORY(p0043CA3F,	6, 0x03CA3F),	// AT 显示减少

			PATCH_MEMORY(0x643525,	4, 0x1B186C),	// Exp %4d->%5d
			PATCH_MEMORY(0xD4,		1, 0x04A2F7),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0x00,		1, 0x04A313),	// CP %3d->%6d, use HP's
			PATCH_MEMORY(0xEB,      1, 0x006195),	// party display extra inf
			//	PATCH_MEMORY(0xACE9,	4, 0x04A6E2),	// not display Condition
			PATCH_MEMORY(0x74,      2, 0x04A2C5),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x049ECF),	// height
			PATCH_MEMORY(0x30,      1, 0x049ED7),	// height2
			PATCH_MEMORY(0x00,      1, 0x0061CE),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408EA7 -0x00400000),
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00406FA8 -0x00400000),
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x0041C393 -0x00400000),	// 是否显状态AT
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			//PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayItemDropPatch0,	ed6DisplayItemDropPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayGetPar0,			ed6DisplayGetPar, 0, ed6DisplayGetParOld),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, 0x0044677F,	ed6DisplayBattleSepith, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x0048C4B0,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayResetWidth0,		ed6DisplayResetWidth, 0, ed6DisplayResetWidthOld), // jp ver only
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed62cn7)
	{
		using namespace NED62;
		if (nSepithUpLimit == 0) nSepithUpLimit = 70;

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x00433DC9 -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x00419A11 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x15C168),	// Exp %4d->%5d
			PATCH_MEMORY(0xEB,      1, 0x005F65),	// party display extra inf
			PATCH_MEMORY(0x00,      1, 0x04155B),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x041156),	// height
			PATCH_MEMORY(0x00,      1, 0x005F96),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408A97 -0x00400000),
			PATCH_MEMORY(0x54A330,	4, 0x00419A36 -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY(0x54A330,	4, 0x00419A43 -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x004199F9 -0x00400000),	// 是否显状态AT

			PATCH_MEMORY(0x01,      1, 0x004CBA86 -0x00400000),	// LG_Font2
			//PATCH_MEMORY(0xEB,      1, 0x004CBA87 -0x00400000),	// LG_Font1
			//PATCH_MEMORY(0xEB,      1, 0x004CBB59 -0x00400000),	// LG_Font4
			//PATCH_MEMORY(0xEB,      1, 0x004CBB9F -0x00400000),	// LG_Font5
			//PATCH_MEMORY(0xEB,      1, 0x004CBBE3 -0x00400000),	// LG_Font6
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			//PATCH_FUNCTION(JUMP, 0, 0x0412B7, ed6DisplaySkipCondition, 0),
			//PATCH_FUNCTION(CALL, 0, 0x08CEAA, NED63::ed6DisplayBattleIcoEx, 1),
			//PATCH_FUNCTION(JUMP, 0, 0x0418D0, ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9FD0,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed62jp7)
	{
		using namespace NED62;
		if (nSepithUpLimit == 0) nSepithUpLimit = 70;

		NED62::sprintf = (pSprintf) 0x005389AF;
		getItemInf = (pGetAddr) 0x004C5EF0;

		lpfnDisplayText = (LPVOID)0x0048BDA0;
		lpfnFtoL = (LPVOID)0x00538950;
		lpfnResetRightBoard = (LPVOID)0x0048A170;
		lpfnDisplayBattleIco = (LPVOID)0x0048C860;

		addrDisplaySkipCondition0 = 0x00440E77;	// patch at
		addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
		addrDisplaySkipCondition2 = 0x00440F10; // ret2

		addrDisplayStatusLine = 0x55B6C4;
		addrDisplayStatusResist1 = 0x418410;
		addrDisplayStatusResist2 = 0x557FC4;
		addrDisplayStatusResist3 = 0x5580C4;
		addrDisplayStatusResist4 = 0x63EBB0;

		addrDisplayStatusPatch0 = 0x00441499;
		addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;	
		addrDisplayStatusPatch2 = 0x0048BE10; // call

		addrDisplayBattleIcoEx0 = 0x0048C8AA;
		dwYStart = 0x112;

		addrSoldierNo0 = 0x60EDD8;
		addrChangeEnemyStatusPatch0 = 0x004431C2;
		addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
		addrChangeEnemyStatusPatch2 = 0x004CCF00; // call

        resolution = (PSIZE)0x00563CB0; // 分辨率

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x00433A19 -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x00419811 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x15B644),	// Exp %4d->%5d
			PATCH_MEMORY(0xEB,      1, 0x005F65),	// party display extra inf
			PATCH_MEMORY(0x00,      1, 0x04111B),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x040D16),	// height
			PATCH_MEMORY(0x00,      1, 0x005F96),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408A97 -0x00400000),
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x004197F9 -0x00400000),	// 是否显状态AT

		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9970,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed62jp1020) // jp1020 及 整合补丁
	{
		using namespace NED62;
		if (nSepithUpLimit == 0) nSepithUpLimit = 70;

		NED62::sprintf = (pSprintf) 0x00537EEF;
		getItemInf = (pGetAddr) 0x004C59F0;

		lpfnDisplayText = (LPVOID)0x0048B7D0;
		lpfnFtoL = (LPVOID)0x00537E8C;
		lpfnResetRightBoard = (LPVOID)0x00489C70;
		lpfnDisplayBattleIco = (LPVOID)0x0048C290;

		addrDisplaySkipCondition0 = 0x00440DC7;	// patch at
		addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
		addrDisplaySkipCondition2 = 0x00440E60; // ret2

		addrDisplayStatusLine = 0x55A6C4;
		addrDisplayStatusResist1 = 0x418420;
		addrDisplayStatusResist2 = 0x556FC4;
		addrDisplayStatusResist3 = 0x5570C4;
		addrDisplayStatusResist4 = 0x63DA70;

		addrDisplayStatusPatch0 = 0x004413E9;
		addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;	
		addrDisplayStatusPatch2 = 0x0048B840; // call

		addrDisplayBattleIcoEx0 = 0x0048C2DA;
		dwYStart = 0x112;

		addrSoldierNo0 = 0x60DC98;
		addrChangeEnemyStatusPatch0 = 0x00443112;
		addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
		addrChangeEnemyStatusPatch2 = 0x004CCB30; // call

        resolution = (PSIZE)0x00562BF4; // 分辨率

		unsigned char p004339F9[6] = { 0xE9, 0xF2, 0x00, 0x00, 0x00, 0x90 };

		unsigned char p004CBDCE[11] = { 0x6A, 0x0A, 0xFF, 0x15, 0x24, 0x90, 0x54, 0x00, 0x32, 0xC0, 0xC3 };
		unsigned char p004CDEE2[10] = { 0xE8, 0x4D, 0xA2, 0x07, 0x00, 0xEB, 0x3D, 0x90, 0x90, 0x90 };
		unsigned char p00548120[35] = {
			0xE8, 0x0B, 0xEA, 0xF7, 0xFF, 0x85, 0xC0, 0x75, 0x0A, 0x60, 0x6A, 0x01, 0xFF, 0x15, 0x24, 0x90,
			0x54, 0x00, 0x61, 0xC3, 0xE8, 0xF7, 0xE9, 0xF7, 0xFF, 0xE8, 0x42, 0x3C, 0xF8, 0xFF, 0x84, 0xC0,
			0x74, 0xF2, 0xC3};

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (*(UINT*)0x00562C04 == 0x59977089 && *(UINT*)0x004C6F01 == 0x562BEC) //日版 windows名称
		{
			MEMORY_FUNCTION_PATCH f1[] =
			{
				PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0), // jp ver only
			};
			Nt_PatchMemory(NULL, 0, f1, countof(f1), hModule);
		}

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	6, 0x004339F9 -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if (*(UINT*)0x004413BC == 0x02CFE5DB) // 理之补丁 at调整为一列
		{
			if (nShowConditionAT != 0)
			{
				__asm OR BYTE PTR DS:[0x2EDF77C],0x1;
			}
			MEMORY_PATCH p1[] =
			{
				//PATCH_MEMORY(p004339F9,	6, 0x0339F9),	// AT 显示减少
				PATCH_MEMORY(0x00077201,4, 0x004D0F1B -0x00400000),	// CPUtest
				PATCH_MEMORY(p004CDEE2,	sizeof(p004CDEE2), 0x004CDEE2 -0x00400000), // CPU手册
				PATCH_MEMORY(p00548120,	sizeof(p00548120), 0x00548120 -0x00400000), // new Code
			};
			MEMORY_FUNCTION_PATCH f1[] =
			{
				PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			};
			Nt_PatchMemory(p1, countof(p1), f1, countof(f1),  hModule);
			return;
		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x00419821 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			//PATCH_MEMORY(0x43,		1, 0x15A68C),	// 整合恢复 condition
			//PATCH_MEMORY(0x22,		1, 0x040E13),	// 整合恢复 condition

			PATCH_MEMORY(0x643525,	4, 0x15A644),	// Exp %4d->%5d
			PATCH_MEMORY(0xEB,      1, 0x005F65),	// party display extra inf
			PATCH_MEMORY(0x74,      2, 0x04106A),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x040C66),	// height
			PATCH_MEMORY(0x00,      1, 0x005F96),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00408A97 -0x00400000),
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x00419809 -0x00400000),	// 是否显状态AT

			//PATCH_MEMORY(p004339F9,	6, 0x0339F9),	// AT 显示减少
			PATCH_MEMORY(0x36,		1, 0x004C6B54 -0x00400000),	// CPU恢复
			PATCH_MEMORY(0x00145FD2,4, 0x0040215E -0x00400000),	// CPU战斗
			PATCH_MEMORY(0x00077201,4, 0x004D0F1B -0x00400000),	// CPUtest
			PATCH_MEMORY(p004CBDCE,	sizeof(p004CBDCE), 0x004CBDCE -0x00400000), // sleep@no active
			PATCH_MEMORY(p004CDEE2,	sizeof(p004CDEE2), 0x004CDEE2 -0x00400000), // CPU手册
			PATCH_MEMORY(p00548120,	sizeof(p00548120), 0x00548120 -0x00400000), // new Code
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			//PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0), //整合放右上角了
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x004A9530,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			//	INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed61cn7)
	{
		using namespace NED61;
		if (nSepithUpLimit == 0) nSepithUpLimit = 70;

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	8, 0x004286C3 -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x00414525 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x11A214),	// Exp %4d->%5d
			PATCH_MEMORY(0xEB,      1, 0x0054A1),	// party display extra inf
			PATCH_MEMORY(0x00,      1, 0x0343C2),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x033F76),	// height
			PATCH_MEMORY(0x00,      1, 0x0054D2),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00407D07 -0x00400000),
			PATCH_MEMORY(0x50C2FC,	4, 0x0041454A -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY(0x50C2FC,	4, 0x00414557 -0x00400000),	// 状态AT调整恢复
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x0041450D -0x00400000),	// 是否显状态AT

			//PATCH_MEMORY(0x01,      1, 0x004CBA86 -0x00400000),	// LG_Font2
			//PATCH_MEMORY(0xEB,      1, 0x004AC469 -0x00400000),	// LG_Font1
			//PATCH_MEMORY(0xEB,      1, 0x004AC4F4 -0x00400000),	// LG_Font4
			//PATCH_MEMORY(0xEB,      1, 0x004AC51F -0x00400000),	// LG_Font5
			//PATCH_MEMORY(0xEB,      1, 0x004AC548 -0x00400000),	// LG_Font6
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			//PATCH_FUNCTION(JUMP, 0, 0x03411C, ed6DisplaySkipCondition, 0),
			//PATCH_FUNCTION(CALL, 0, 0x0758CA, NED63::ed6DisplayBattleIcoEx, 1),
			//PATCH_FUNCTION(JUMP, 0, 0x034737, ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x00490A50,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			// INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
	else if (gameVersion == ed61jp7)
	{
		using namespace NED61;
		if (nSepithUpLimit == 0) nSepithUpLimit = 70;

		NED61::sprintf = (pSprintf) 0x004FBA2F;
		getItemInf = (pGetAddr) 0x004A84A0;

		lpfnDisplayText = (LPVOID)0x00474C00;
		lpfnFtoL = (LPVOID)0x004FB9D0;
		lpfnResetRightBoard = (LPVOID)0x00473010;
		lpfnDisplayBattleIco = (LPVOID)0x004756C0;

		addrDisplaySkipCondition0 = 0x00433D9C;	// patch at
		addrDisplaySkipCondition1 = addrDisplaySkipCondition0 + 5; // ret1
		addrDisplaySkipCondition2 = 0x00433E38; // ret2

		addrDisplayStatusLine = 0x51A714;
		addrDisplayStatusResist1 = 0x004130B0;
		addrDisplayStatusResist2 = 0x517C7C;
		addrDisplayStatusResist3 = 0x517D6C;
		addrDisplayStatusResist4 = 0x5CE3B8;

		addrDisplayStatusPatch0 = 0x004343C0;
		addrDisplayStatusPatch1 = addrDisplayStatusPatch0 + 5;
		addrDisplayStatusPatch2 = 0x00474C70; // call

		addrDisplayBattleIcoEx0 = 0x0047570A;
		dwYStart = 0x112;

		addrSoldierNo0 = 0x5A8370;
		addrChangeEnemyStatusPatch0 = 0x00434C56;
		addrChangeEnemyStatusPatch1 = addrChangeEnemyStatusPatch0 + 5;
		addrChangeEnemyStatusPatch2 = 0x004AE440; // call

        resolution = (PSIZE)0x00520F04; // 分辨率

		if (*(unsigned char*)addrChangeEnemyStatusPatch0 != 0xE8)	return;	//0xE8 call 0xE9 jump; 防止重复补丁

		if (nShowAT != 0)	// 显AT
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
				PATCH_MEMORY(pMemPatchShowAtData,	8, 0x004284D3 -0x00400000),	// AT 显示减少恢复
			};
			Nt_PatchMemory(memPatchShowAT, countof(memPatchShowAT), NULL, 0, hModule);

		}

		if ((nShowConditionAT == 1 || nShowConditionAT == 3) && resolution->cx >= 800)	// 状态AT 调整到一行
		{
			MEMORY_PATCH p[] =
			{
				PATCH_MEMORY(0,	4, 0x004143B5 -0x00400000),	// 状态AT 调整到一行
			};
			Nt_PatchMemory(p, countof(p), NULL, 0, hModule);
		}

		MEMORY_PATCH p[] =
		{
			PATCH_MEMORY(0x643525,	4, 0x11A694),	// Exp %4d->%5d
			PATCH_MEMORY(0xEB,      1, 0x0054A1),	// party display extra inf
			PATCH_MEMORY(0x00,      1, 0x034042),	// display cp 
			PATCH_MEMORY(0x30,      1, 0x033C46),	// height
			PATCH_MEMORY(0x00,      1, 0x0054D2),	// up board
			PATCH_MEMORY(nSepithUpLimit,	4, 0x00407D17 -0x00400000),
			PATCH_MEMORY((ULONG_PTR)&nShowConditionAT,	4, 0x0041439D -0x00400000),	// 是否显状态AT
		};

		MEMORY_FUNCTION_PATCH f[] =
		{
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplaySkipCondition0,	ed6DisplaySkipCondition, 0),
			PATCH_FUNCTION(CALL, NOT_RVA, addrDisplayBattleIcoEx0,		NED63::ed6DisplayBattleIcoEx, 1),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrDisplayStatusPatch0,		ed6DisplayStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, addrChangeEnemyStatusPatch0,	ed6ChangeEnemyStatusPatch, 0),
			PATCH_FUNCTION(JUMP, NOT_RVA, 0x00490930,	ed6ShowConditionAtNew, 5, ed6ShowConditionAtOld),
			// INLINE_HOOK(Nt_GetProcAddress(Nt_GetModuleHandle(L"kernel32.dll"), "OutputDebugStringA"), PrintDebugStringA, NULL),
		};
		Nt_PatchMemory(p, countof(p), f, countof(f), hModule);
	}
}