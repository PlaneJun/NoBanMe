/****************zbase64.c****************/
#include "zbase64.h"
namespace zbase64 {
	int base64_encode(const unsigned char* src, int src_bytes, char* out)
	{
		const char EncodeTable[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		unsigned char Tmp[4] = { 0 };
		int LineLength = 0;
		int i = 0;
		for (int j = 0; j < (int)(src_bytes / 3); j++)
		{
			Tmp[1] = *src++;
			Tmp[2] = *src++;
			Tmp[3] = *src++;
			out[i++] = EncodeTable[Tmp[1] >> 2];
			out[i++] = EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
			out[i++] = EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
			out[i++] = EncodeTable[Tmp[3] & 0x3F];
			if (LineLength += 4, LineLength == 76)
			{
				out[i++] = '\r';
				out[i++] = '\n';
				LineLength = 0;
			}
		}
		int Mod = src_bytes % 3;
		if (Mod == 1)
		{
			Tmp[1] = *src++;
			out[i++] = EncodeTable[(Tmp[1] & 0xFC) >> 2];
			out[i++] = EncodeTable[((Tmp[1] & 0x03) << 4)];
			out[i++] = '=';
			out[i++] = '=';
		}
		else if (Mod == 2)
		{
			Tmp[1] = *src++;
			Tmp[2] = *src++;
			out[i++] = EncodeTable[(Tmp[1] & 0xFC) >> 2];
			out[i++] = EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
			out[i++] = EncodeTable[((Tmp[2] & 0x0F) << 2)];
			out[i++] = '=';
		}
		return i;
	}

	int base64_decode(const char* in, int input_len, unsigned char* out)
	{

		const char DecodeTable[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0,
			62, // '+'
			0, 0, 0,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60,
			61, // '0'-'9'
			0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
			14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
			25, // 'A'-'Z'
			0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
			38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
		};

		int nValue;
		int input_idx = 0;
		int i = 0;
		while (input_idx < input_len)
		{
			if (*in == '\r' || *in == '\n')
			{
				in++;
				input_idx++;
				continue;
			}

			nValue = DecodeTable[*in++] << 18;
			nValue += DecodeTable[*in++] << 12;
			out[i++] = (nValue & 0x00FF0000) >> 16;
			if (*in != '=')
			{
				nValue += DecodeTable[*in++] << 6;
				out[i++] = (nValue & 0x0000FF00) >> 8;
				if (*in != '=')
				{
					nValue += DecodeTable[*in++];
					out[i++] = nValue & 0x000000FF;
				}
			}
			input_idx += 4; //note that input_idx increases even when *in == '='
		}
		return i;
	}

};
