
#if ED6EX_VERSION

#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/EXPORT:Direct3DCreate8=d3d8.Direct3DCreate8")
#pragma comment(linker, "/EXPORT:DirectInput8Create=dinput8.DirectInput8Create")

#else // dinput8.dll d3d9.dll

#include <d3d8.h>

#pragma comment(linker, "/EXPORT:Direct3DCreate8=_Arianrhod_Direct3DCreate8@4")
#pragma comment(linker, "/EXPORT:DirectInput8Create=_Arianrhod_DirectInput8Create@20")

NoInline BOOL IsCodeDecompressed()
{
    SEH_TRY
    {
        static ULONG Signature[] =
    {
        0xe9003ffa, 0x0024ca08, 0x0e8a03e9, 0x78cee900,
        0xf9e90025, 0xe900276e, 0x00225f64, 0x0c354fe9,
        0x54dae900, 0xb5e90025, 0xe900033e, 0x004ebe90
    };

    return RtlCompareMemory((PVOID)0x672020, Signature, sizeof(Signature)) == sizeof(Signature);
    }
        SEH_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
}

//EXTC IDirect3D9* STDCALL Arianrhod_Direct3DCreate9(UINT SDKVersion)
EXTC IDirect3D8* STDCALL Arianrhod_Direct3DCreate8(UINT SDKVersion)
{
    static TYPE_OF(Arianrhod_Direct3DCreate8) *Direct3DCreate8;

    if (Direct3DCreate8 == nullptr)
    {
        ULONG           Length;
        NTSTATUS        Status;
        PVOID           hModule;
        WCHAR           szPath[MAX_NTPATH];

        Length = Nt_GetSystemDirectory(szPath, countof(szPath));
        CopyStruct(szPath + Length, L"d3d8.dll", sizeof(L"d3d8.dll"));

        hModule = Ldr::LoadDll(szPath);
        if (hModule == nullptr)
            return nullptr;

        LdrAddRefDll(GET_MODULE_HANDLE_EX_FLAG_PIN, hModule);

        *(PVOID *)&Direct3DCreate8 = GetRoutineAddress(hModule, "Direct3DCreate8");
        if (Direct3DCreate8 == nullptr)
            return nullptr;
    }

#if ENABLE_LOG
    {
        NtFileDisk f;
        f.Create(L"log.txt");
    }
#endif

    WriteLog(L"%08X\n", _ReturnAddress());

    static BOOL Hooked = FALSE;

    if (!Hooked)
    {
        DllMain(&__ImageBase, DLL_PROCESS_ATTACH, 0);
        Hooked = TRUE;
    }

    return Direct3DCreate8(SDKVersion);
}

EXTC
HRESULT
STDCALL
Arianrhod_DirectInput8Create(
    HINSTANCE   hinst,
    ULONG       Version,
    REFIID      riidltf,
    PVOID*      ppvOut,
    LPUNKNOWN   punkOuter
)
{
    static TYPE_OF(Arianrhod_DirectInput8Create) *DirectInput8Create;

    if (DirectInput8Create == nullptr)
    {
        ULONG       Length;
        NTSTATUS    Status;
        PVOID       hModule;
        WCHAR       szPath[MAX_NTPATH];

        Length = Nt_GetSystemDirectory(szPath, countof(szPath));
        CopyStruct(szPath + Length, L"dinput8.dll", sizeof(L"dinput8.dll"));

        hModule = Ldr::LoadDll(szPath);
        if (hModule == nullptr)
            return E_FAIL;

        LdrAddRefDll(GET_MODULE_HANDLE_EX_FLAG_PIN, hModule);

        *(PVOID *)&DirectInput8Create = GetRoutineAddress(hModule, "DirectInput8Create");
        if (DirectInput8Create == nullptr)
            return E_FAIL;
    }

#if ENABLE_LOG
    {
        NtFileDisk f;
        f.Create(L"log.txt");
    }
#endif

    WriteLog(L"%08X\n", _ReturnAddress());

    static BOOL Hooked = FALSE;

    if (!Hooked)
    {
        DllMain(&__ImageBase, DLL_PROCESS_ATTACH, 0);
        Hooked = TRUE;
    }

    return DirectInput8Create(hinst, Version, riidltf, ppvOut, punkOuter);
}

#endif
