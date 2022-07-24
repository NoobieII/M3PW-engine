//Lithio (The developer's pseudonym)
//June 28, 2022

//Packet for use on servers and clients

#ifndef M3PW_PACKET_H
#define M3PW_PACKET_H

typedef struct Packet{
	int bytes_used;
	int read_length;
	int buffer_size;
	char *buffer;
} Packet;

void packet_init(Packet *p);
void packet_close(Packet *p);
void packet_write_8(Packet *p, int n);
void packet_write_16(Packet *p, int n);
void packet_write_32(Packet *p, int n);
void packet_write_f(Packet *p, float f);
void packet_write_str(Packet *p, const char *str);
void packet_write_bytes(Packet *p, const void *ptr, int n);

void packet_test(Packet *p, const char *message);

#endif
