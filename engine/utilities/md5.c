//Lithio (the developer's pseudonym)
//May 10, 2022

//MD5 Algorithm

#include <stdlib.h>

int MD5_hash(void *in, int len, void *out){
	static unsigned char s[64] = {
		7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
		5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
		4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
		6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 };
	
	static unsigned int k[64] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };
	
	//A, B, C, D
	size_t buffer[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
	
	//this is the message, padded
	unsigned char *message;
	int message_len;
	int i, j;
	size_t A, B, C, D, g;
	size_t F;
	unsigned char *M;
	char *digest;
	
	if(!in || !out){
		return -1;
	}
	
	message_len = (len + 8 + 1 + 64 - 1) / 64 * 64 - 8;
	
	message = (unsigned char*) malloc(message_len + 8);
	for(i = 0; i < len; ++i){
		message[i] = ((unsigned char*)(in))[i];
	}
	message[i++] = 0x80;
	for(; i < message_len; ++i){
		message[i] = 0x00;
	}
	
	//length is now in terms of bits
	len <<= 3;
	
	message[i++] = len >> 0;
	message[i++] = len >> 8;
	message[i++] = len >> 16;
	message[i++] = len >> 24;
	
	message[i++] = 0;
	message[i++] = 0;
	message[i++] = 0;
	message[i++] = 0;
	
	for(i = 0; i < (message_len + 8) / 64; ++i){
		M = (unsigned char*)(message + i*64);
		A = buffer[0];
		B = buffer[1];
		C = buffer[2];
		D = buffer[3];
		
		for(j = 0; j < 64; ++j){
			if(j < 16){
				F = (B & C) | ((~B) & D);
				g = j;
			}
			else if(j < 32){
				F = (D & B) | ((~D) & C);
				g = (5 * j + 1) & 0x0f;
			}
			else if(j < 48){
				F = B ^ C ^ D;
				g = (3 * j + 5) & 0x0f;
			}
			else{
				F = C ^ (B | (~D));
				g = (7 * j) & 0x0f;
			}
			
			//F = F + A + k[j] + (M[g*4+0]<<24)+(M[g*4+1]<<16)+(M[g*4+2]<<8)+(M[g*4+3]<<0);
			F = F + A + k[j] + (M[g*4+3]<<24)+(M[g*4+2]<<16)+(M[g*4+1]<<8)+(M[g*4+0]<<0);
			F &= 0xffffffff;
			A = D;
			D = C;
			C = B;
			B = B + ((F << s[j]) & 0xffffffff) + (F >> (32 - s[j]));
		}
		buffer[0] += A;
		buffer[1] += B;
		buffer[2] += C;
		buffer[3] += D;
	}
	free(message);
	
	digest = (char*) out;
	//convert from small endian
	for(i = 0; i < 4; ++i){
		digest[i*4+0] = (buffer[i] & 0xff);
		digest[i*4+1] = (buffer[i] & 0xff00) >> 8;
		digest[i*4+2] = (buffer[i] & 0xff0000) >> 16;
		digest[i*4+3] = (buffer[i] & 0xff000000) >> 24;
	}
	
	return 0;
}
