//Lithio (The developer's pseudonym)
//May 11, 2022

#include <stdlib.h>

//32 bit rotations for unsigned variables
#define lrotate(A, B) (((A << B) & 0xffffffff) + (A >> (32 - B)))
#define rrotate(A, B) (((A << (32 - B)) & 0xffffffff) + (A >> B))

int SHA256_hash(const void *in, int len, void *out){
	unsigned char *message;
	unsigned char *digest;
	int i, j;
	int message_len;
	
	size_t H[8] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	
	size_t s[2];
	size_t a, b, c, d, e, f, g, h;
	size_t t1, t2, ch, maj;
	
	static size_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
	
	
	size_t w[64];
	
	if(!in || !out){
		return -1;
	}
	
	//initialize message
	message_len = (len + 1 + 8 + 64 - 1) / 64 * 64 - 8;
	message = (unsigned char*) malloc(message_len + 8);
	
	for(i = 0; i < len; ++i){
		message[i] = ((unsigned char*)(in))[i];
	}
	message[i++] = 0x80;
	for(; i < message_len; ++i){
		message[i] = 0x00;
	}
	//big endian
	len <<= 3;
	for(j = 0; j < 4; ++j){
		message[i++] = 0x00;
	}
	for(j = 0; j < 4; ++j){
		message[i++] = (len >> (24 - j*8)) & 0xff;
	}
	
	for(i = 0; i < (message_len + 8) / 64; ++i){
		for(j = 0; j < 16; ++j){
			w[j] =  message[i*64 + j*4 + 3] << 0;
			w[j] += message[i*64 + j*4 + 2] << 8;
			w[j] += message[i*64 + j*4 + 1] << 16;
			w[j] += message[i*64 + j*4 + 0] << 24;
		}
		for(; j < 64; ++j){
			s[0] = rrotate(w[j-15],7) ^ rrotate(w[j-15],18) ^ w[j-15]>>3;
			s[1] = rrotate(w[j-2],17) ^ rrotate(w[j-2], 19) ^ w[j-2]>>10;
			w[j] = (w[j-16] + s[0] + w[j-7] + s[1]) & 0xffffffff;
		}
		a = H[0];
		b = H[1];
		c = H[2];
		d = H[3];
		e = H[4];
		f = H[5];
		g = H[6];
		h = H[7];
		
		for(j = 0; j < 64; ++j){
			s[1] = rrotate(e,6) ^ rrotate(e,11) ^ rrotate(e,25);
			ch = (e & f) ^ ((~e) & g);
			t1 = (h + s[1] + ch + K[j] + w[j]) & 0xffffffff;
			s[0] = rrotate(a,2) ^ rrotate(a,13) ^ rrotate(a,22);
			maj = (a & b) ^ (a & c) ^ (b & c);
			t2 = (s[0] + maj) & 0xffffffff;
			
			h = g;
			g = f;
			f = e;
			e = (d + t1) & 0xffffffff;
			d = c;
			c = b;
			b = a;
			a = (t1 + t2) & 0xffffffff;
		}
		H[0] = (H[0] + a) & 0xffffffff;
		H[1] = (H[1] + b) & 0xffffffff;
		H[2] = (H[2] + c) & 0xffffffff;
		H[3] = (H[3] + d) & 0xffffffff;
		H[4] = (H[4] + e) & 0xffffffff;
		H[5] = (H[5] + f) & 0xffffffff;
		H[6] = (H[6] + g) & 0xffffffff;
		H[7] = (H[7] + h) & 0xffffffff;
	}
	
	free(message);
	
	digest = (unsigned char*) out;
	for(i = 0; i < 8; ++i){
		digest[i*4 + 0] = H[i] >> 24;
		digest[i*4 + 1] = H[i] >> 16;
		digest[i*4 + 2] = H[i] >> 8;
		digest[i*4 + 3] = H[i] >> 0;
	}
	
	return 0;
}
