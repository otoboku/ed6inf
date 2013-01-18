#pragma comment(linker,"/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker,"/SECTION:indexmoe,ERW /MERGE:.text=indexmoe")
//#pragma comment(lib,"E:\\WinDDK\\7600.16385.1\\lib\\win7\\i386\\ntdllp.lib")
#pragma comment(lib,"ntdllpC.lib")

//#pragma comment(lib,"libcmt.lib")
//#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
//#pragma comment(linker, "/NODEFAULTLIB:msvcrt.lib")
//#pragma comment(lib,"E:\\program3\\lib\\test.lib")
//#pragma comment(lib,"D:\\svn\\lib\\msvcrt.lib")
//#pragma comment(lib,"D:\\svn\\lib\\msvcrt_lib.lib")
//#pragma comment(lib,"D:\\svn\\lib\\BufferOverflowU.lib")

#define USE_NT_VER      1
#define CONSOLE_DEBUG   0

#include "ed6inf.h"
#include "my_commsrc.h"

bool bIsInit = false;

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

#pragma comment(linker,"/ENTRY:DllMain")

#if ED6EX_VERSION
#pragma comment(linker, "/EXPORT:Direct3DCreate8=d3d8.Direct3DCreate8")

#elif VERSION_VERSION
#pragma comment(linker, "/EXPORT:GetFileVersionInfoA=system32\\version.GetFileVersionInfoA,@1")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=system32\\version.GetFileVersionInfoByHandle,@2")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=system32\\version.GetFileVersionInfoExW,@3")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=system32\\version.GetFileVersionInfoSizeA,@4")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=system32\\version.GetFileVersionInfoSizeExW,@5")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=system32\\version.GetFileVersionInfoSizeW,@6")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoW=system32\\version.GetFileVersionInfoW,@7")
#pragma comment(linker, "/EXPORT:VerFindFileA=system32\\version.VerFindFileA,@8")
#pragma comment(linker, "/EXPORT:VerFindFileW=system32\\version.VerFindFileW,@9")
#pragma comment(linker, "/EXPORT:VerInstallFileA=system32\\version.VerInstallFileA,@10")
#pragma comment(linker, "/EXPORT:VerInstallFileW=system32\\version.VerInstallFileW,@11")
#pragma comment(linker, "/EXPORT:VerLanguageNameA=system32\\version.VerLanguageNameA,@12")
#pragma comment(linker, "/EXPORT:VerLanguageNameW=system32\\version.VerLanguageNameW,@13")
#pragma comment(linker, "/EXPORT:VerQueryValueA=system32\\version.VerQueryValueA,@14")
#pragma comment(linker, "/EXPORT:VerQueryValueW=system32\\version.VerQueryValueW,@15")

#elif DINPUT8_VERSION
#pragma comment(linker, "/EXPORT:DirectInput8Create=system32\\dinput8.DirectInput8Create,@1")
#pragma comment(linker, "/EXPORT:DllCanUnloadNow=system32\\dinput8.DllCanUnloadNow,@2")
#pragma comment(linker, "/EXPORT:DllGetClassObject=system32\\dinput8.DllGetClassObject,@3")
#pragma comment(linker, "/EXPORT:DllRegisterServer=system32\\dinput8.DllRegisterServer,@4")
#pragma comment(linker, "/EXPORT:DllUnregisterServer=system32\\dinput8.DllUnregisterServer,@5")

#endif

bool Load()
{
	if (Nt_IsPathExists(L".\\dsound.dll") || Nt_IsPathExists(L".\\d3d8.dll"))
	{
		//MessageBoxW(NULL, L"不要和dsound.dll、d3d8.dll同时使用！", L"ed6Ex.dll", MB_ICONSTOP);
		//max(2,3);
		AllocConsole();
		//PrintConsoleW(L"不要和dsound.dll、d3d8.dll同时使用！\r\n");
		PrintDebugStringW(L"不要和dsound.dll、d3d8.dll同时使用！\r\n");
		//__asm MOV EAX,DWORD PTR DS:[0];
		//__asm int 3;
		return false;
	}
	return true;
}


BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG Reason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
	
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
#endif