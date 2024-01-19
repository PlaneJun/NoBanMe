#pragma once
#include <string>
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Urlmon.lib")


class EasyPdb
{
private:
	//Thanks mambda
	//https://bitbucket.org/mambda/pdb-parser/src/master/
	struct PDBHeader7
	{
		char signature[0x20];
		int page_size;
		int allocation_table_pointer;
		int file_page_count;
		int root_stream_size;
		int reserved;
		int root_stream_page_number_list_number;
	};

	struct RootStream7
	{
		int num_streams;
		int stream_sizes[1]; //num_streams
	};

	struct GUID_StreamData
	{
		int ver;
		int date;
		int age;
		GUID guid;
	};

	struct PdbInfo
	{
		DWORD	Signature;
		GUID	Guid;
		DWORD	Age;
		char	PdbFileName[1];
	};

#define EZ_PDB_BASE_OF_DLL (DWORD64)0x10000000
public:

	typedef struct _EZPDB
	{
		HANDLE hProcess;
		HANDLE hPdbFile;
	}EZPDB, * PEZPDB;

public:


	//https://msdl.microsoft.com/download/symbols/
	std::string EzPdbDownload(IN std::string pePath, IN OPTIONAL std::string pdbDownloadPath = "./Symbol", IN OPTIONAL std::string symbolServer = "http://msdl.blackint3.com:88/download/symbols/");

	bool EzPdbLoad(IN std::string pdbPath, OUT PEZPDB Pdb);

	ULONG EzPdbGetRva(IN PEZPDB Pdb, IN std::string SymName);

	ULONG EzPdbGetStructPropertyOffset(IN PEZPDB Pdb, IN std::string StructName, IN std::wstring PropertyName);

	ULONG EzPdbGetStructSize(IN PEZPDB Pdb, IN std::string StructName);

	VOID EzPdbUnload(IN PEZPDB Pdb);

};

