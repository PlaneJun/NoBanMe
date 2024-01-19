#pragma once 
#include <vector>
#include <Windows.h>
#include <stdint.h>
class PEFile
{
public:
	PEFile();

	bool load_from_pid(uint32_t pid,const wchar_t* module_name);

	bool load_from_file(const char* path);

	bool load_from_memory(uint8_t* data,size_t len,bool mapper =false,uint64_t new_base=NULL);

	std::vector<uint8_t>& get_data();
	
	uint32_t get_number_of_sections() const { return number_of_sections_; }

	uint32_t get_size_of_option_header() const { return size_of_option_header_; }

	uint32_t get_size_of_code() const { return size_of_code_; }

	uint32_t get_size_of_initialized_data() const { return size_of_initialized_data_; }

	uint32_t get_size_of_uninitialized_data() const { return size_of_uninitialized_data_; }

	uint32_t get_entry_point() const { return entry_point_; }

	uint32_t get_base_of_code() const { return base_of_code_; }

	uint32_t get_base_of_data() const { return base_of_data_; }

	uint64_t get_image_base() const { return image_base_; }

	uint32_t get_segtion_alignment() const { return segtion_alignment_; }

	uint32_t get_file_alignment() const { return file_alignment_; }

	uint32_t get_size_of_image() const { return size_of_image_; }

	uint32_t get_size_of_headers() const { return size_of_headers_; }

	IMAGE_DATA_DIRECTORY get_data_directory(uint8_t index) const { return data_directory_[index]; }

	std::vector<PIMAGE_SECTION_HEADER> get_section_headers();

	bool save_to_file(const char* path);
private:
	bool arch64_;
	std::vector<uint8_t> data_;
	uint32_t number_of_sections_;
	uint32_t size_of_option_header_;
	uint32_t size_of_code_;
	uint32_t size_of_initialized_data_;
	uint32_t size_of_uninitialized_data_;
	uint32_t entry_point_;
	uint32_t base_of_code_;
	uint32_t base_of_data_;
	uint64_t image_base_;
	uint32_t segtion_alignment_;
	uint32_t file_alignment_;
	uint32_t size_of_image_;
	uint32_t size_of_headers_;
	IMAGE_DATA_DIRECTORY data_directory_[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};