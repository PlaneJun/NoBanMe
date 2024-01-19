#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

//x << y
#define RL(x, y) (((x) << (y)) | ((x) >> (32 - (y))))
//PP(aabbccdd)=ddccbbaa
#define PP(x) (x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)| (x>>24)

#define FF(a, b, c, d, x, s, ac) a = b + (RL((a + F(b,c,d) + x + ac),s))
#define GG(a, b, c, d, x, s, ac) a = b + (RL((a + G(b,c,d) + x + ac),s))
#define HH(a, b, c, d, x, s, ac) a = b + (RL((a + H(b,c,d) + x + ac),s))
#define II(a, b, c, d, x, s, ac) a = b + (RL((a + I(b,c,d) + x + ac),s))


static void Md5(void);

/*
i:		Temp variable
len:	The length of the file
flen:	The initial leng of a file represented by a 64-bits binary
*/
static unsigned A = 0;
static unsigned B = 0;
static unsigned C = 0;
static unsigned D = 0;
static unsigned a = 0;
static unsigned b = 0;
static unsigned c = 0;
static unsigned d = 0;
static unsigned i = 0;
static unsigned len = 0;
static unsigned flen[2] = { 0, 0 };
static unsigned x[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char* md5::get_md5_from_memory(char* buffer, size_t len,char* l_p_fileMd5)
{
	__try
	{
		A = 0x67452301;
		B = 0xefcdab89;
		C = 0x98badcfe;
		D = 0x10325476;

		flen[0] = (len % 0x20000000) * 8;
		//flen's unit is a bit.
		flen[1] = len / 0x20000000;

		char* cur = buffer;
		char* end = buffer + len;
		memset(x, 0, 64);
		//Read 16 sets of data in groups of 4 bytes.
		memcpy(&x, cur, 64);
		cur += 64;
		//Loop to end of file.
		for (i = 0; i < len / 64; i++) {
			Md5();
			memset(x, 0, 64);

			if (cur + 64 > end)
			{
				if (cur < end)
				{
					int over = end - cur;
					memcpy(&x, cur, over);
					cur += over;
				}
			}
			else
			{
				memcpy(&x, cur, 64);
				cur += 64;
			}
		}

		((char*)x)[len % 64] = 128;

		if (len % 64 > 55) {
			Md5();
			memset(x, 0, 64);

		}
		memcpy(x + 14, flen, 8);
		Md5();

		char Md5_1[10] = { 0 };
		char Md5_2[10] = { 0 };
		char Md5_3[10] = { 0 };
		char Md5_4[10] = { 0 };
		sprintf_s(Md5_1, sizeof(char) * 9, "%08x", PP(A));
		sprintf_s(Md5_2, sizeof(char) * 9, "%08x", PP(B));
		sprintf_s(Md5_3, sizeof(char) * 9, "%08x", PP(C));
		sprintf_s(Md5_4, sizeof(char) * 9, "%08x", PP(D));

		strcpy_s(l_p_fileMd5, strlen(Md5_1) + 1, Md5_1);
		strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_2) + 1, Md5_2);
		strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + 1, Md5_3);
		strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + 1, Md5_4);
	}
	__except (1)
	{
	}

	return l_p_fileMd5;
}

uint8_t* md5::get_md5_in_hex(char* buffer, size_t len, uint8_t* out)
{

		A = 0x67452301;
		B = 0xefcdab89;
		C = 0x98badcfe;
		D = 0x10325476;

		flen[0] = (len % 0x20000000) * 8;
		//flen's unit is a bit.
		flen[1] = len / 0x20000000;

		char* cur = buffer;
		char* end = buffer + len;
		memset(x, 0, 64);
		//Read 16 sets of data in groups of 4 bytes.
		memcpy(&x, cur, 64);
		cur += 64;
		//Loop to end of file.
		for (i = 0; i < len / 64; i++) {
			Md5();
			memset(x, 0, 64);

			if (cur + 64 > end)
			{
				if (cur < end)
				{
					int over = end - cur;
					memcpy(&x, cur, over);
					cur += over;
				}
			}
			else
			{
				memcpy(&x, cur, 64);
				cur += 64;
			}
		}

		((char*)x)[len % 64] = 128;

		if (len % 64 > 55) {
			Md5();
			memset(x, 0, 64);

		}

		memcpy(x + 14, flen, 8);
		Md5();

		uint32_t* md5hex = (uint32_t*)out;
		md5hex[0] = A;
		md5hex[1] = B;
		md5hex[2] = C;
		md5hex[3] = D;


		return out;

}

char* md5::get_md5_from_file(const char* filePath) 
{

	FILE* l_p_fp = NULL;
	fopen_s(&l_p_fp,filePath,"rb");
	if (!l_p_fp) {
		return NULL;
	}

	//The file pionter jumps to end of the file.
	fseek(l_p_fp, 0, SEEK_END);

	//Check file size.
	len = ftell(l_p_fp);

	if (len == -1) {
		fclose(l_p_fp);

		return NULL;
	}

	//The file pionter is reset to the file header.
	rewind(l_p_fp);

	A = 0x67452301;
	B = 0xefcdab89;
	C = 0x98badcfe;
	D = 0x10325476;

	flen[0] = (len % 0x20000000) * 8;
	//flen's unit is a bit.
	flen[1] = len / 0x20000000;

	memset(x, 0, 64);
	//Read 16 sets of data in groups of 4 bytes.
	fread(&x, 4, 16, l_p_fp);

	//Loop to end of file.
	for (i = 0; i < len / 64; i++) {
		Md5();
		memset(x, 0, 64);
		fread(&x, 4, 16, l_p_fp);
	}

	((char*)x)[len % 64] = 128;

	if (len % 64 > 55) {
		Md5();
		memset(x, 0, 64);
	}

	memcpy(x + 14, flen, 8);
	Md5();

	fclose(l_p_fp);
	static char l_p_fileMd5[33];
	memset(&l_p_fileMd5, 0, sizeof(l_p_fileMd5));
	char Md5_1[10] = { 0 };
	char Md5_2[10] = { 0 };
	char Md5_3[10] = { 0 };
	char Md5_4[10] = { 0 };

	sprintf_s(Md5_1, sizeof(char) * 9, "%08x", PP(A));
	sprintf_s(Md5_2, sizeof(char) * 9, "%08x", PP(B));
	sprintf_s(Md5_3, sizeof(char) * 9, "%08x", PP(C));
	sprintf_s(Md5_4, sizeof(char) * 9, "%08x", PP(D));

	strcpy_s(l_p_fileMd5, strlen(Md5_1) + 1, Md5_1);
	strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_2) + 1, Md5_2);
	strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + 1, Md5_3);
	strcat_s(l_p_fileMd5, strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + strlen(Md5_1) + 1, Md5_4);


	return l_p_fileMd5;
}



static void Md5(void) {
	a = A;
	b = B;
	c = C;
	d = D;

	/*Four rounds were performed for a total of 64 rounds.*/

	//Round 1
	FF(a, b, c, d, x[0], 7, 0xd76aa478);
	FF(d, a, b, c, x[1], 12, 0xe8c7b756);
	FF(c, d, a, b, x[2], 17, 0x242070db);
	FF(b, c, d, a, x[3], 22, 0xc1bdceee);
	FF(a, b, c, d, x[4], 7, 0xf57c0faf);
	FF(d, a, b, c, x[5], 12, 0x4787c62a);
	FF(c, d, a, b, x[6], 17, 0xa8304613);
	FF(b, c, d, a, x[7], 22, 0xfd469501);
	FF(a, b, c, d, x[8], 7, 0x698098d8);
	FF(d, a, b, c, x[9], 12, 0x8b44f7af);
	FF(c, d, a, b, x[10], 17, 0xffff5bb1);
	FF(b, c, d, a, x[11], 22, 0x895cd7be);
	FF(a, b, c, d, x[12], 7, 0x6b901122);
	FF(d, a, b, c, x[13], 12, 0xfd987193);
	FF(c, d, a, b, x[14], 17, 0xa679438e);
	FF(b, c, d, a, x[15], 22, 0x49b40821);

	//Round 2
	GG(a, b, c, d, x[1], 5, 0xf61e2562);
	GG(d, a, b, c, x[6], 9, 0xc040b340);
	GG(c, d, a, b, x[11], 14, 0x265e5a51);
	GG(b, c, d, a, x[0], 20, 0xe9b6c7aa);
	GG(a, b, c, d, x[5], 5, 0xd62f105d);
	GG(d, a, b, c, x[10], 9, 0x02441453);
	GG(c, d, a, b, x[15], 14, 0xd8a1e681);
	GG(b, c, d, a, x[4], 20, 0xe7d3fbc8);
	GG(a, b, c, d, x[9], 5, 0x21e1cde6);
	GG(d, a, b, c, x[14], 9, 0xc33707d6);
	GG(c, d, a, b, x[3], 14, 0xf4d50d87);
	GG(b, c, d, a, x[8], 20, 0x455a14ed);
	GG(a, b, c, d, x[13], 5, 0xa9e3e905);
	GG(d, a, b, c, x[2], 9, 0xfcefa3f8);
	GG(c, d, a, b, x[7], 14, 0x676f02d9);
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);

	//Round 3
	HH(a, b, c, d, x[5], 4, 0xfffa3942);
	HH(d, a, b, c, x[8], 11, 0x8771f681);
	HH(c, d, a, b, x[11], 16, 0x6d9d6122);
	HH(b, c, d, a, x[14], 23, 0xfde5380c);
	HH(a, b, c, d, x[1], 4, 0xa4beea44);
	HH(d, a, b, c, x[4], 11, 0x4bdecfa9);
	HH(c, d, a, b, x[7], 16, 0xf6bb4b60);
	HH(b, c, d, a, x[10], 23, 0xbebfbc70);
	HH(a, b, c, d, x[13], 4, 0x289b7ec6);
	HH(d, a, b, c, x[0], 11, 0xeaa127fa);
	HH(c, d, a, b, x[3], 16, 0xd4ef3085);
	HH(b, c, d, a, x[6], 23, 0x04881d05);
	HH(a, b, c, d, x[9], 4, 0xd9d4d039);
	HH(d, a, b, c, x[12], 11, 0xe6db99e5);
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8);
	HH(b, c, d, a, x[2], 23, 0xc4ac5665);

	//Round 4
	II(a, b, c, d, x[0], 6, 0xf4292244);
	II(d, a, b, c, x[7], 10, 0x432aff97);
	II(c, d, a, b, x[14], 15, 0xab9423a7);
	II(b, c, d, a, x[5], 21, 0xfc93a039);
	II(a, b, c, d, x[12], 6, 0x655b59c3);
	II(d, a, b, c, x[3], 10, 0x8f0ccc92);
	II(c, d, a, b, x[10], 15, 0xffeff47d);
	II(b, c, d, a, x[1], 21, 0x85845dd1);
	II(a, b, c, d, x[8], 6, 0x6fa87e4f);
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
	II(c, d, a, b, x[6], 15, 0xa3014314);
	II(b, c, d, a, x[13], 21, 0x4e0811a1);
	II(a, b, c, d, x[4], 6, 0xf7537e82);
	II(d, a, b, c, x[11], 10, 0xbd3af235);
	II(c, d, a, b, x[2], 15, 0x2ad7d2bb);
	II(b, c, d, a, x[9], 21, 0xeb86d391);

	A += a;
	B += b;
	C += c;
	D += d;
}

