#pragma once
#include <string>
#include <stdint.h>
#include <Windows.h>

class RegBrowserWidget
{
public:
	static inline const struct {
		std::string text;
		std::string stext;
		HKEY hKey;
	}Keys[]{
		{ "HKEY_CLASSES_ROOT", "HKCR",HKEY_CLASSES_ROOT},
		{ "HKEY_CURRENT_USER","HKCU", HKEY_CURRENT_USER},
		{ "HKEY_LOCAL_MACHINE","HKLM", HKEY_LOCAL_MACHINE},
		{ "HKEY_USERS","HKU", HKEY_USERS},
		{ "HKEY_CURRENT_CONFIG","HKCC", HKEY_CURRENT_CONFIG},
		{ "HKEY_PERFORMANCE_DATA","", HKEY_PERFORMANCE_DATA},
		{ "HKEY_PERFORMANCE_TEXT","", HKEY_PERFORMANCE_TEXT},
		{ "HKEY_PERFORMANCE_NLSTEXT","", HKEY_PERFORMANCE_NLSTEXT },
		{ "HKEY_CURRENT_USER_LOCAL_SETTINGS","", HKEY_CURRENT_USER_LOCAL_SETTINGS },
	};

public:
	void OnPaint();

private:
	DWORD GetSubKeyCount(HKEY hKey, DWORD* values=0, FILETIME* ft= nullptr);
};