#ifndef _misc_H_456967E2_6FDC_4227_B022_8DB0EA5350DE
#define _misc_H_456967E2_6FDC_4227_B022_8DB0EA5350DE

#include <windows.h>

#pragma warning (disable: 4127)

template<class T1> T1 TypeMax(T1 t1)
{
    UNREFERENCED_PARAMETER(t1);

	if ((T1)0xFFFFFFFFFFFFFFFF > 0)
	{
		return (T1)~T1(0);
	}
	else
	{
		return (T1)~(T1)(T1(1) << (sizeof(T1)*8 - 1));
	}
}

template<class T1> T1 TypeMin(T1 t1)
{
    UNREFERENCED_PARAMETER(t1);

	if ((T1)0xFFFFFFFFFFFFFFFF > 0)
	{
		return 0;
	}
	else
	{
		return (T1)(T1(1) << (sizeof(T1)*8 - 1));
	}
}

template<class T1, class T2> T1 SaturateConvert(T1 t1, T2 t2)
{
//#pragma warning (disable: 4307)
//#pragma warning (disable: 4293)
//#pragma warning (disable: 4308)

	if (sizeof(T1) > sizeof(T2))
	{
		t1 = (T1)t2;
		return t1;
	}
	else if (sizeof(T1) == sizeof(T2))
	{
		if ((T1)0xFFFFFFFFFFFFFFFF > 0 && (T2)0xFFFFFFFFFFFFFFFF < 0)
		{
			if (t2 < 0)
			{
				t1 = 0;
				return t1;
			}
			else
			{
				t1 = (T1)t2;
				return t1;
			}
		}
		else if ((T1)0xFFFFFFFFFFFFFFFF < 0 && (T2)0xFFFFFFFFFFFFFFFF > 0)
		{
			if ((T1)t2 < 0)
			{
				t1 = (T1)~(T1)(T1(1) << (sizeof(T1)*8 - 1));
				return t1;
			}
			else
			{
				t1 = (T1)t2;
				return t1;
			}
		}
		else
		{
			t1 = (T1)t2;
			return t1;
		}
	}
	else
	{
		T2 max, min;
		//max = (T2)TypeMax(t1);
		//min = (T2)TypeMin(t1);
		if ((T1)0xFFFFFFFFFFFFFFFF > 0)
		{
			max = (T2)(T1)~T1(0);
			min = 0;
		}
		else
		{
			max = (T2)(T1)~(T1)(T1(1) << (sizeof(T1)*8 - 1));
			min = (T2)(T1)(T1(1) << (sizeof(T1)*8 - 1));
		}
		//printf("%d %d\r\n", max, min);
		if ((T2)0xFFFFFFFFFFFFFFFF > 0)
		{
			if (t2 > max)
			{
				t1 = (T1)max;
			}
			else
			{
				t1 = (T1)t2;
			}
		}
		else
		{
			if (t2 > max)
			{
				t1 = (T1)max;
				//printf(">\r\n");
			}
			else if (t2 < min)
			{
				t1 = (T1)min;
				//printf("<\r\n");
			}
			else
			{
				t1 = (T1)t2;
			}
		}
		return t1;

	}

//#pragma warning (default: 4307)
//#pragma warning (default: 4293)
//#pragma warning (default: 4308)
}

template<class T1, class T2> T1 SaturateConvert(T1* t1, T2 t2)
{
	*t1 = SaturateConvert(*t1, t2);
	return *t1;
}

template<class T1> T1 SaturateConvert(T1 t1, double t2)
{
    if (t2 > (double)TypeMax(t1))
        t1 = TypeMax(t1);
    else if (t2 < (double)TypeMin(t1))
        t1 = TypeMin(t1);
    else
        t1 = (T1)t2;

    return t1;
	//long long qwTemp = (long long)t2;
	//return SaturateConvert(t1, qwTemp);
}

template<class T1> T1 SaturateConvert(T1* t1, double t2)
{
    *t1 = SaturateConvert(*t1, t2);
	return *t1;
	//long long qwTemp = (long long)t2;
	//return SaturateConvert(t1, qwTemp);
}

template<class T1> T1 SaturateConvert(T1 t1, float t2)
{
    if (t2 > (float)TypeMax(t1))
        t1 = TypeMax(t1);
    else if (t2 < (float)TypeMin(t1))
        t1 = TypeMin(t1);
    else
        t1 = (T1)t2;
    
    return t1;
}

template<class T1> T1 SaturateConvert(T1* t1, float t2)
{
    *t1 = SaturateConvert(*t1, t2);
    return *t1;
}

template<class T1, class T2> T1 SaturateConvertEx(T1 t1, T2 t2, T1 MAX = TypeMax(T1(0)), T1 MIN = TypeMin(T1(0)))
{
	t1 = SaturateConvert(t1, t2);
	t1 = t1 < MAX ? t1 : MAX;
	t1 = t1 > MIN ? t1 : MIN;
	return t1;
}

template<class T1, class T2> T1 SaturateConvertEx(T1* t1, T2 t2, T1 MAX = TypeMax(T1(0)), T1 MIN = TypeMin(T1(0)))
{
	*t1 = SaturateConvertEx(*t1, t2, MAX, MIN);
	return *t1;
}

/*
template<class T1> T1 SaturateConvert(T1 t1, float t2)
{
	long long qwTemp = (long long)t2;
	return SaturateConvert(t1, qwTemp);
}


template<class T1> T1 SaturateConvert(T1* t1, float t2)
{
	long long qwTemp = (long long)t2;
	return SaturateConvert(t1, qwTemp);
}
*/
#pragma warning (default: 4127)

namespace NINI
{
	double GetPrivateProfileDoubleA(LPCSTR lpAppName, LPCSTR lpKeyName, double fDefault, LPCSTR lpFileName)
	{
		char szTemp[0x100];
		DWORD dwCopied = GetPrivateProfileStringA(lpAppName, lpKeyName, NULL, szTemp, countof(szTemp), lpFileName);
		if (dwCopied == 0)
		{
			return fDefault;
		}
		else
		{
			return atof(szTemp);
		}
	}

	INT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
	{
		char szTemp[0x100];
		DWORD dwCopied = GetPrivateProfileStringA(lpAppName, lpKeyName, NULL, szTemp, countof(szTemp), lpFileName);
		if (dwCopied == 0)
		{
			return nDefault;
		}
		else
		{
			return strtol(szTemp, NULL, 10);
		}
	}

    INT GetPrivateProfileIntW( IN LPCWSTR lpAppName, IN LPCWSTR lpKeyName, IN INT nDefault, IN LPCWSTR lpFileName )
    {
        WCHAR szTemp[0x100];
        DWORD dwCopied = GetPrivateProfileStringW(lpAppName, lpKeyName, NULL, szTemp, countof(szTemp), lpFileName);
        if (dwCopied == 0)
        {
            return nDefault;
        }
        else
        {
            return wcstol(szTemp, NULL, 10);
		}
    }

	BOOL GetPrivateProfileBoolA(LPCSTR lpAppName, LPCSTR lpKeyName, BOOL bDefault, LPCSTR lpFileName)
	{
		switch (GetPrivateProfileIntA(lpAppName, lpKeyName, bDefault, lpFileName))
        {
        case 0:
            return FALSE;
        case 1:
            return TRUE;
        default:
            return bDefault;
        }
	}

    BOOL GetPrivateProfileBoolW( IN LPCWSTR lpAppName, IN LPCWSTR lpKeyName, IN BOOL bDefault, IN LPCWSTR lpFileName )
    {
        switch (GetPrivateProfileIntW(lpAppName, lpKeyName, bDefault, lpFileName))
        {
        case 0:
            return FALSE;
        case 1:
            return TRUE;
        default:
            return bDefault;     
        }
    }
}

BOOL Nt_IsPathExistsW(LPCWSTR pszPath)
{
	return Nt_GetFileAttributes(pszPath) != -1;
}

BOOL Nt_IsPathExistsA(LPCSTR pszPath)
{
	WCHAR	szwPath[MAX_PATH];
	Nt_AnsiToUnicode(szwPath, countof(szwPath), (PCHAR)pszPath, -1);
	return Nt_GetFileAttributes(szwPath) != -1;
}


ULONG PrintConsoleA(PCHAR pszFormat, ...)
{
    BOOL    Result;
    ULONG   Length;
    CHAR	Buffer[0xFF0 / 4];
	WCHAR	wBuffer[0xFF0 / 2];
    va_list pArg;
	
    va_start(pArg, pszFormat);
#pragma warning (disable: 4996)
    Length = _vsnprintf(Buffer, countof(Buffer) - 1, pszFormat, pArg);
#pragma warning (default: 4996)
    if (Length == -1)
        return Length;

	Nt_AnsiToUnicode(wBuffer, countof(wBuffer), (PCHAR)Buffer, -1);
	
    Result = WriteConsoleW(
		Nt_CurrentPeb()->ProcessParameters->StandardOutput,
		wBuffer,
		StrLengthW(wBuffer),//Length,
		&Length,
		NULL
		);
	
    return Result ? Length : 0;
}

ULONG __stdcall PrintDebugStringA(PCHAR lpString)
{
	//HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL    Result;
	ULONG   Length;

	Length = StrLengthA(lpString);
	if (Length == 0)
		return Length;

	Result = WriteConsoleA(
		//hstdout,
		Nt_CurrentPeb()->ProcessParameters->StandardOutput,
		lpString,
		Length,
		&Length,
		NULL
		);

	return Result ? Length : 0;
}

ULONG __stdcall PrintDebugStringW(PWCHAR lpString)
{
	//HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL    Result;
	ULONG   Length;

	//Length = lstrlenW(lpString);
	Length = StrLengthW(lpString);
	if (Length == 0)
		return Length;

	Result = WriteConsoleW(
		//hstdout,
		Nt_CurrentPeb()->ProcessParameters->StandardOutput,
		lpString,
		Length,
		&Length,
		NULL
		);

	return Result ? Length : 0;
}

namespace NFILE
{
    BOOL WriteFileW(
        IN  LPCWSTR         lpFileName,
        IN  LPVOID          lpBuffer,
        IN  DWORD           nNumberOfBytesToWrite/*  OPTIONAL,
        OUT LPDWORD         lpNumberOfBytesWritten OPTIONAL = NULL,
        IN  LPOVERLAPPED    lpOverlapped           OPTIONAL = NULL*/
        )
    {      
        if (lpFileName == NULL || lpBuffer == NULL)
            return FALSE;

        NtFileDisk file;
        BOOL bResult = FALSE;

        LOOP_ONCE
        {
            if (!NT_SUCCESS(file.Create(lpFileName)))
                break;
        
            //ULONG BOM = BOM_UTF16_LE;
            //file.Write(&BOM, 2);
            //file.Seek(0, SEEK_END);
            if (!NT_SUCCESS(file.Write(lpBuffer, nNumberOfBytesToWrite)))
                break;

            bResult = TRUE;
        }
        file.Close();
        return bResult;
    }
}
#endif