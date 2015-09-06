#pragma comment(linker,"/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker,"/SECTION:indexmoe,ERW /MERGE:.text=indexmoe")
//#pragma comment(lib,"E:\\WinDDK\\7600.16385.1\\lib\\win7\\i386\\ntdllp.lib")
//#pragma comment(lib,"ntdllpC.lib")

#define USE_NT_VER      1
#define CONSOLE_DEBUG   0

#include "ed6inf.h"
//#include "my_commsrc.h"
#include "MyLibrary.cpp"

bool bIsInit = false;

#if 0
#if D3D8_VERSION

#pragma comment(linker,"/ENTRY:DllMain")
#pragma comment(linker, "/EXPORT:Direct3DCreate8=__Direct3DCreate8,@4")

FARPROC	pDirect3DCreate8;

bool Load()
{
//	if (Nt_IsPathExists(L".\\dsound.dll") && Nt_IsPathExists(L".\\d3d8.dll"))
//	{
//		MessageBoxW(NULL, L"不要同时使用dsound.dll和d3d8.dll！", L"d3d8.dll", MB_ICONSTOP);
//		//return false;
//	}

	ULONG           Length;
	NTSTATUS        Status;
	HMODULE         hModule;
	WCHAR           szPath[MAX_NTPATH];
	UNICODE_STRING  DllPath;
	
	Length = Nt_GetSystemDirectory(szPath, countof(szPath));
	
	StrCopyW(szPath + Length, L"d3d8.dll");
	DllPath.Buffer = szPath;
	DllPath.Length = (Length + CONST_STRLEN(L"d3d8.dll")) * sizeof(WCHAR);
	DllPath.MaximumLength = DllPath.Length;
	
	Status = LdrLoadDll(NULL, 0, &DllPath, (PVOID *)&hModule);
	if (!NT_SUCCESS(Status))
		return FALSE;
	
	LdrAddRefDll(GET_MODULE_HANDLE_EX_FLAG_PIN, hModule);
	
	pDirect3DCreate8 = Nt_GetProcAddress(hModule, "Direct3DCreate8");
	if (pDirect3DCreate8 == NULL)
		return FALSE;
	return TRUE;
}

extern "C" __declspec(naked) void __cdecl _Direct3DCreate8(void)
{
	if (pDirect3DCreate8 == NULL)
	{
		if (!Load())
		{
			__asm xor eax,eax;
			__asm retn;
		}
	}
	if (!bIsInit)
	{
		Init();
		bIsInit = true;
	}
	__asm JMP pDirect3DCreate8;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG Reason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
	
    switch (Reason)
    {
	case DLL_PROCESS_ATTACH:
		LdrDisableThreadCalloutsForDll(hInstance);
		MyLib_Initialize();
		//Load();

#if CONSOLE_DEBUG
		AllocConsole();
#endif
		break;
		
	case DLL_PROCESS_DETACH:
		MyLib_UnInitialize();
		break;
    }
	
    return TRUE;
}

#elif DSOUND_VERSION

#pragma comment(linker,"/ENTRY:DllMain")
#pragma comment(linker, "/EXPORT:DirectSoundCreate=__DirectSoundCreate,@1")

FARPROC	pDirectSoundCreate;

bool Load()
{
//	if (Nt_IsPathExists(L".\\dsound.dll") && Nt_IsPathExists(L".\\d3d8.dll"))
//	{
//		MessageBoxW(NULL, L"不要同时使用dsound.dll和d3d8.dll！", L"dsound.dll", MB_ICONSTOP);
//		//return false;
//	}

	ULONG           Length;
	NTSTATUS        Status;
	HMODULE         hModule;
	WCHAR           szPath[MAX_NTPATH];
	UNICODE_STRING  DllPath;
	
	Length = Nt_GetSystemDirectory(szPath, countof(szPath));
	
	StrCopyW(szPath + Length, L"dsound.dll");
	DllPath.Buffer = szPath;
	DllPath.Length = (Length + CONST_STRLEN(L"dsound.dll")) * sizeof(WCHAR);
	DllPath.MaximumLength = DllPath.Length;
	
	Status = LdrLoadDll(NULL, 0, &DllPath, (PVOID *)&hModule);
	if (!NT_SUCCESS(Status))
		return FALSE;
	
	LdrAddRefDll(GET_MODULE_HANDLE_EX_FLAG_PIN, hModule);
	
	//pDirectSoundCreate = Nt_GetProcAddress(hModule, "DirectSoundCreate");
	pDirectSoundCreate = Nt_GetProcAddress(hModule, (LPCSTR)1);
	if (pDirectSoundCreate == NULL)
		return FALSE;
	return TRUE;
}

extern "C" __declspec(naked) void __cdecl _DirectSoundCreate(void)
{
	if (pDirectSoundCreate == NULL)
	{
		if (!Load())
		{
			__asm xor eax,eax;
			__asm retn;
		}
	}
	if (!bIsInit)
	{
		Init();
		bIsInit = true;
	}
	__asm JMP pDirectSoundCreate;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG Reason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
	
    switch (Reason)
    {
	case DLL_PROCESS_ATTACH:
		LdrDisableThreadCalloutsForDll(hInstance);
		MyLib_Initialize();
		//Load();

#if CONSOLE_DEBUG
		AllocConsole();
#endif
		break;
		
	case DLL_PROCESS_DETACH:
		MyLib_UnInitialize();
		break;
    }
	
    return TRUE;
}
#else
#endif
#endif

#pragma comment(linker,"/ENTRY:DllMain")

#if ED6EX_VERSION
#pragma comment(linker, "/EXPORT:DirectInput8Create=dinput8.DirectInput8Create")
//#pragma comment(linker, "/EXPORT:Direct3DCreate8=d3d8.Direct3DCreate8")


#elif D3D8_VERSION
#pragma comment(linker, "/EXPORT:Direct3DCreate8=system32\\d3d8.Direct3DCreate8")

#elif DINPUT8_VERSION
#pragma comment(linker, "/EXPORT:DirectInput8Create=system32/dinput8.DirectInput8Create")

#endif

bool Load()
{
	if (Nt_IsPathExists(L".\\dsound.dll") || Nt_IsPathExists(L".\\d3d8.dll"))
	{
		//MessageBoxW(NULL, L"不要和dsound.dll、d3d8.dll同时使用！", L"ed6Ex.dll", MB_ICONSTOP);
		//max(2,3);
		//AllocConsole();
		//PrintConsoleW(L"不要和dsound.dll、d3d8.dll同时使用！\r\n");
		//PrintDebugStringW(L"不要和dsound.dll、d3d8.dll同时使用！\r\n");
		//__asm MOV EAX,DWORD PTR DS:[0];
		//__asm int 3;
		return false;
	}
	return true;
}

HMODULE hModuleSelf;

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG Reason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
	hModuleSelf = hInstance;

    switch (Reason)
    {
	case DLL_PROCESS_ATTACH:
		LdrDisableThreadCalloutsForDll(hInstance);
		MyLib_Initialize();

#if CONSOLE_DEBUG
		AllocConsole();
#endif
	
		/*
		if(Load())
		{
			//MessageBoxW(NULL, L"load original d3d dll failed!", L"error!", MB_ICONSTOP);
			//MessageBoxW(NULL, L"load original system dll failed!\r\n加载系统dll失败！", L"error!", MB_ICONSTOP);
			Init();
		}*/
		//Load();

		if (!bIsInit)
		{
			Init();
			bIsInit = true;
		}
		break;
		
	case DLL_PROCESS_DETACH:
		MyLib_UnInitialize();
		break;
    }
	
    return TRUE;
}
//#endif