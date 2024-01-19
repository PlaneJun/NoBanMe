#pragma once
struct Data
{
	long unsigned int length;
	char* buffer;
};


namespace Zip
{
	Data compress(std::string str);
	Data uncompress(const void* input_, int inputSize_, unsigned int expectedSize);
};

