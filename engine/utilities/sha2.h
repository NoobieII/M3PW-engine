//Lithio (The developer's pseudonym)
//May 11, 2022

//SHA-256 Hashing Algorithm

#ifndef M3PW_SHA2_H
#define M3PW_SHA2_H

//returns 0 if successful, -1 if not... out should point to 16 bytes
int SHA256_hash(const void *in, int len, void *out);

#endif
