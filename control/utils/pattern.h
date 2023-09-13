#pragma once
#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

class CFindPatternEx {
private:

	PBYTE Buffer;
	ULONG64 dwFindPattern(ULONG64 dwAddress, DWORD dwLength, const char* szPattern)
	{
		const char* pat = szPattern;
		ULONG64 firstMatch = NULL;
		for (ULONG64 pCur = dwAddress; pCur < (dwAddress + dwLength); pCur++)
		{
			if (!*pat) return firstMatch;
			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
				if (!firstMatch) firstMatch = pCur;
				if (!pat[2]) return firstMatch;
				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
				else pat += 2;
			}
			else {
				pat = szPattern;
				firstMatch = 0;
			}
		}
		return NULL;
	}

	bool FindPatternSimplified(ULONG64 dwAddress, DWORD dwLength, const char* szPattern) {
		//DWORD result = dwFindPattern(dwAddress, dwAddress + dwLength, szPattern);
		//PBYTE bresult = (PBYTE)result;
		//bresult += Offset;
		return dwFindPattern(dwAddress, dwAddress + dwLength, szPattern) ? true : false;
	}

	DWORD getFileSize(std::string path)
	{
		HANDLE hFile = CreateFileA(
			path.c_str(), //PEÎÄ¼þÃû
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("Create File Failed.\n");
			return -1;
		}

		DWORD size = GetFileSize(hFile, NULL);

		CloseHandle(hFile);
		return size;
	}

public:
	CFindPatternEx(HMODULE T_hModule) {
		Buffer = (PBYTE)T_hModule;
	}


	bool FindPatternEx(std::string szPattern) {

		char filename[1024]{};
		GetModuleFileNameA((HMODULE)Buffer, filename, 1024);
		return FindPatternSimplified((ULONG64)Buffer, getFileSize(filename), szPattern.c_str());
	}
	ULONG64 FindPattern(std::string szPattern) {
		char filename[1024]{};
		GetModuleFileNameA((HMODULE)Buffer, filename, 1024);
		return dwFindPattern((ULONG64)Buffer, getFileSize(filename), szPattern.c_str());
	}
};