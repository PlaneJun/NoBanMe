#include <zlib.h>
#include<string>
#include"compression.h"

namespace Zip {

	Data compress(std::string str)
	{
		uLongf compressedSize = compressBound(str.length());
		unsigned char* destBuffer = new unsigned char[compressedSize];
		const char* source_ = str.c_str();
		const unsigned char* source = reinterpret_cast<const unsigned char*>(source_);
		auto errorCode = ::compress(destBuffer, &compressedSize, source, str.length());
		if (errorCode != Z_OK)
			throw errorCode;
		return Data{ compressedSize, (char*)destBuffer };
	}

	Data uncompress(const void* input_, int inputSize_, unsigned int expectedSize)
	{
		const Bytef* input = (const Bytef*)input_;
		uLongf expectedSize_ = expectedSize;

		unsigned char* output = new unsigned char[expectedSize];
		memset(output, 0, expectedSize);
		int errorCode = ::uncompress(output, &expectedSize_,
			input, inputSize_);
		if (errorCode != Z_OK)
			throw errorCode;
		return Data{ expectedSize, (char*)output };
	}
};