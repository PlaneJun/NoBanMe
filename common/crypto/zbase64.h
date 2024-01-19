#pragma once

#include <string.h>

namespace zbase64
{
	int base64_encode(const unsigned char* src, int src_bytes, char* out);
	int base64_decode(const char* in, int in_bytes, unsigned char* out);
}

