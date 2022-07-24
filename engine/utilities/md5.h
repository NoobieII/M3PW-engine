//Lithio (the developer's pseudonym)
//May 10, 2022

//MD5 Algorithm

#ifndef M3PW_MD5_H
#define M3PW_MD5_H

//return 0 if successful, -1 if unsuccessful - out should point to 16 bytes
int MD5_hash(void *in, int len, void *out);

#endif
