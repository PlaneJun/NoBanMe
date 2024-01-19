#include <Windows.h>
#include "pefile.h"
#include "../mem/memstub.h"

PEFile::PEFile()
{
	number_of_sections_ = 0;
	size_of_option_header_ = 0;
	size_of_code_ = 0;
	size_of_initialized_data_ = 0;
	size_of_uninitialized_data_ = 0;
	entry_point_ = 0;
	base_of_code_ = 0;
	base_of_data_ = 0;
	image_base_ = 0;
	segtion_alignment_ = 0;
	file_alignment_ = 0;
	size_of_image_ = 0;
	size_of_headers_ = 0;
}

bool PEFile::load_from_pid(uint32_t pid, const wchar_t* module_name)
{
	uint64_t hBase = (uint64_t)MemStub::GetProcessModuleHandle(pid, module_name);
	if (hBase<=0)
	{
		return false;
	}
	BOOL isWow64 = false;
	MemStub::IsWow64(pid, &isWow64);

	IMAGE_DOS_HEADER dos{};
	MemStub::ReadMemory(pid, hBase, (uintptr_t)&dos, sizeof(IMAGE_DOS_HEADER));
	uint64_t lpNtHeaders = hBase + dos.e_lfanew;
	uint32_t SizeOfImage = 0;

	//区分32和64位
	if (isWow64)
	{
		IMAGE_NT_HEADERS32 nt32{};
		MemStub::ReadMemory(pid, lpNtHeaders, (uintptr_t)&nt32, sizeof(IMAGE_NT_HEADERS32));
		SizeOfImage = nt32.OptionalHeader.SizeOfImage;
	}
	else
	{
		IMAGE_NT_HEADERS64 nt64{};
		MemStub::ReadMemory(pid, lpNtHeaders, (uintptr_t)&nt64, sizeof(IMAGE_NT_HEADERS64));
		SizeOfImage = nt64.OptionalHeader.SizeOfImage;
	}

	std::vector<uint8_t> data{};
	data.resize(SizeOfImage);
	MemStub::ReadMemory(pid, hBase, (uintptr_t)data.data(), SizeOfImage);

	return load_from_memory(data.data(), data.size());
}

bool PEFile::load_from_file(const char* path)
{
	//读取文件
	HANDLE hFile= CreateFileA(path,GENERIC_READ, FILE_SHARE_READ,NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	DWORD dwFileSize = GetFileSize(hFile,NULL);
	std::vector<uint8_t> data{};
	data.resize(dwFileSize);
	ReadFile(hFile, data.data(), dwFileSize,NULL,NULL);
	CloseHandle(hFile);
	
	return load_from_memory(data.data(), data.size(),true,0x7FF70AE40000);
}

bool PEFile::load_from_memory(uint8_t* data, size_t len, bool mapper, uint64_t new_base)
{
	if (len <= 0)
	{
		return false;
	}

	PIMAGE_DOS_HEADER pDos = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
	if (pDos->e_magic != 'ZM')
	{
		return false;
	}

	uint32_t magic_offset = pDos->e_lfanew + sizeof(IMAGE_FILE_HEADER) + 4;
	arch64_ = *reinterpret_cast<PWORD>(&data[magic_offset]) == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
	if (arch64_)
	{
		PIMAGE_NT_HEADERS64 pNt64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(&data[pDos->e_lfanew]);
		number_of_sections_ = pNt64->FileHeader.NumberOfSections;
		size_of_option_header_ = pNt64->FileHeader.SizeOfOptionalHeader;
		size_of_code_ = pNt64->OptionalHeader.SizeOfCode;
		size_of_initialized_data_ = pNt64->OptionalHeader.SizeOfInitializedData;
		size_of_uninitialized_data_ = pNt64->OptionalHeader.SizeOfUninitializedData;
		entry_point_ = pNt64->OptionalHeader.AddressOfEntryPoint;
		base_of_code_ = pNt64->OptionalHeader.BaseOfCode;
		base_of_data_ = 0;
		image_base_ = pNt64->OptionalHeader.ImageBase;
		segtion_alignment_ = pNt64->OptionalHeader.SectionAlignment;
		file_alignment_ = pNt64->OptionalHeader.FileAlignment;
		size_of_image_ = pNt64->OptionalHeader.SizeOfImage;
		size_of_headers_ = pNt64->OptionalHeader.SizeOfHeaders;
		RtlCopyMemory(data_directory_, pNt64->OptionalHeader.DataDirectory, IMAGE_NUMBEROF_DIRECTORY_ENTRIES * sizeof(IMAGE_DATA_DIRECTORY));
	}
	else
	{
		PIMAGE_NT_HEADERS32 pNt32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(&data[pDos->e_lfanew]);
		number_of_sections_ = pNt32->FileHeader.NumberOfSections;
		size_of_option_header_ = pNt32->FileHeader.SizeOfOptionalHeader;
		size_of_code_ = pNt32->OptionalHeader.SizeOfCode;
		size_of_initialized_data_ = pNt32->OptionalHeader.SizeOfInitializedData;
		size_of_uninitialized_data_ = pNt32->OptionalHeader.SizeOfUninitializedData;
		entry_point_ = pNt32->OptionalHeader.AddressOfEntryPoint;
		base_of_code_ = pNt32->OptionalHeader.BaseOfCode;
		base_of_data_ = pNt32->OptionalHeader.BaseOfData;
		image_base_ = pNt32->OptionalHeader.ImageBase;
		segtion_alignment_ = pNt32->OptionalHeader.SectionAlignment;
		file_alignment_ = pNt32->OptionalHeader.FileAlignment;
		size_of_image_ = pNt32->OptionalHeader.SizeOfImage;
		size_of_headers_ = pNt32->OptionalHeader.SizeOfHeaders;
		RtlCopyMemory(data_directory_, pNt32->OptionalHeader.DataDirectory, IMAGE_NUMBEROF_DIRECTORY_ENTRIES*sizeof(IMAGE_DATA_DIRECTORY));
	}

	if (!mapper || new_base <= 0)
	{
		data_.clear();
		data_.resize(len);
		RtlCopyMemory(data_.data(), data, len);
	}
	else // 映射
	{
		data_.clear();
		data_.resize(size_of_image_);

		// 拷贝头
		RtlCopyMemory(data_.data(), data, size_of_headers_+ number_of_sections_* sizeof(IMAGE_SECTION_HEADER));
		//拷贝每个节区
		PIMAGE_SECTION_HEADER SectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(data + pDos->e_lfanew + sizeof(IMAGE_FILE_HEADER) + size_of_option_header_ + 4);
		for (int i = 0; i < number_of_sections_; ++i, SectionHeader++)
		{
			RtlCopyMemory(&data_[SectionHeader->VirtualAddress], data+SectionHeader->PointerToRawData, SectionHeader->SizeOfRawData);
		}
		//重定向
		if (data_directory_[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress > 0 && data_directory_[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0)
		{
			uint64_t Delta = new_base - image_base_;
			uint8_t* pAddress = NULL;
			PIMAGE_BASE_RELOCATION pLoc = (PIMAGE_BASE_RELOCATION)(data_.data() + data_directory_[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
			while ((pLoc->VirtualAddress + pLoc->SizeOfBlock) != 0) //开始扫描重定位表
			{
				uint16_t* pLocData = reinterpret_cast<uint16_t*>((uint64_t)pLoc + sizeof(IMAGE_BASE_RELOCATION));
				int NumberOfReloc = (pLoc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);//计算需要修正的重定位项（地址）的数目
				for (int i = 0; i < NumberOfReloc; i++)
				{
					int type = (pLocData[i] & 0xF000) >> 12;
					if (type == IMAGE_REL_BASED_HIGHLOW || type == IMAGE_REL_BASED_DIR64) //这是一个需要修正的地址
					{
						pAddress = reinterpret_cast<uint8_t*>(data_.data() + pLoc->VirtualAddress + (pLocData[i] & 0x0FFF));
						if (arch64_)
						{
							*(uint64_t*)pAddress += Delta;
						}
						else
						{
							*(uint32_t*)pAddress += Delta;
						}
					}
				}
				pLoc = reinterpret_cast<PIMAGE_BASE_RELOCATION>((uint64_t)pLoc + pLoc->SizeOfBlock);
			}
		}
	}

	return true;
}


std::vector<uint8_t>& PEFile::get_data()
{
	return data_;
}

std::vector<PIMAGE_SECTION_HEADER> PEFile::get_section_headers()
{
	std::vector<PIMAGE_SECTION_HEADER> ret;
	PIMAGE_DOS_HEADER pDos = reinterpret_cast<PIMAGE_DOS_HEADER>(data_.data());
	PIMAGE_SECTION_HEADER SectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(data_.data()+pDos->e_lfanew + sizeof(IMAGE_FILE_HEADER)  + size_of_option_header_ + 4);
	for (int i = 0; i < number_of_sections_; i++, SectionHeader++)
	{
		ret.push_back(SectionHeader);
	}

	return ret;
}

bool PEFile::save_to_file(const char* path)
{
	HANDLE hFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != NULL)
	{
		return false;
	}
	WriteFile(hFile, data_.data(), data_.size(), NULL, NULL);
	CloseHandle(hFile);
	return true;
}