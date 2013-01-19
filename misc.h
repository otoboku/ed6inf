#ifndef _misc_H_456967E2_6FDC_4227_B022_8DB0EA5350DE
#define _misc_H_456967E2_6FDC_4227_B022_8DB0EA5350DE

#include <windows.h>


template<class T1> T1 TypeMax(T1 t1)
{
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
	long long qwTemp = (long long)t2;
	return SaturateConvert(t1, qwTemp);
}

template<class T1> T1 SaturateConvert(T1* t1, double t2)
{
	long long qwTemp = (long long)t2;
	return SaturateConvert(t1, qwTemp);
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

namespace NINI
{
	double WINAPI GetPrivateProfileDoubleA(LPCSTR lpAppName, LPCSTR lpKeyName, double fDefault, LPCSTR lpFileName)
	{
		char szTemp[100];
		DWORD dwCopied = GetPrivateProfileStringA(lpAppName, lpKeyName, NULL, szTemp, sizeof(szTemp), lpFileName);
		if (dwCopied == 0)
		{
			return fDefault;
		}
		else
		{
			return atof(szTemp);
		}
	}

	INT WINAPI GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
	{
		char szTemp[100];
		DWORD dwCopied = GetPrivateProfileStringA(lpAppName, lpKeyName, "", szTemp, sizeof(szTemp), lpFileName);
		if (dwCopied == 0)
		{
			return nDefault;
		}
		else
		{
			return strtol(szTemp, NULL, 10);
		}
	}

	bool WINAPI GetPrivateProfileBoolA(LPCSTR lpAppName, LPCSTR lpKeyName, bool bDefault, LPCSTR lpFileName)
	{
		return GetPrivateProfileIntA(lpAppName, lpKeyName, bDefault, lpFileName) != 0;
	/*	char szTemp[100];
		DWORD dwCopied = GetPrivateProfileStringA(lpAppName, lpKeyName, "", szTemp, sizeof(szTemp), lpFileName);
		if (dwCopied == 0)
		{
			return bDefault;
		}
		else
		{
			return strtol(szTemp, NULL, 0) != 0;
		}*/
	}
}

ULONG __stdcall PrintDebugStringA(PCHAR lpString)
{
	//HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL    Result;
	ULONG   Length;

	Length = lstrlen(lpString);
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
#endif