#pragma once
#include <stdint.h>

namespace md5
{
	char* get_md5_from_memory(char* buffer, size_t len, char* l_p_fileMd5);

	char* get_md5_from_file(const char* filePath);

	uint8_t* get_md5_in_hex(char* buffer, size_t len, uint8_t*out);

}

