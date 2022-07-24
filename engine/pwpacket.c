
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwpacket.h"

//packet increase size is 2^N
#define N 5
#define M3PW_PACKET_INCREASE_SIZE (1 << N)

void packet_init(Packet *p){
	p->bytes_used = 0;
	p->read_length = 0;
	p->buffer_size = 0;
	p->buffer = NULL;
}

void packet_close(Packet *p){
	if(p->buffer){
		free(p->buffer);
	}
}

static void packet_increase_size(Packet *p, int n){
	char *tmp;
	//make n <= k * M3PW_PACKET_INCREASE_SIZE
	int size_increase = ((n + M3PW_PACKET_INCREASE_SIZE - 1) & (~(M3PW_PACKET_INCREASE_SIZE - 1))) << N;
	
	tmp = (char*) realloc(p->buffer, p->buffer_size + size_increase);
	if(!tmp){
		printf("packet_increase_size: error reallocating memory\n");
	}
	else{
		p->buffer = tmp;
		p->buffer_size += size_increase;
	}
}

void packet_write_8(Packet *p, int n){
	if(p->bytes_used + 1 > p->buffer_size){
		packet_increase_size(p, 1);
	}
	p->buffer[p->bytes_used++] = n;
}
	
void packet_write_16(Packet *p, int n){
	if(p->bytes_used + 2 > p->buffer_size){
		packet_increase_size(p, 2);
	}
	p->buffer[p->bytes_used++] = n & 0xff;
	p->buffer[p->bytes_used++] = (n >> 8) & 0xff;
}

void packet_write_32(Packet *p, int n){
	if(p->bytes_used + 4 > p->buffer_size){
		packet_increase_size(p, 4);
	}
	p->buffer[p->bytes_used++] = n & 0xff;
	p->buffer[p->bytes_used++] = (n >> 8) & 0xff;
	p->buffer[p->bytes_used++] = (n >> 16) & 0xff;
	p->buffer[p->bytes_used++] = (n >> 24) & 0xff;
}

void packet_write_f(Packet *p, float f){
	if(p->bytes_used + 4 > p->buffer_size){
		packet_increase_size(p, 4);
	}
	*(float*)(p->buffer + p->bytes_used) = f;
	p->bytes_used += 4;
}

void packet_write_str(Packet *p, const char *str){
	packet_write_16(p, strlen(str) + 1);
	packet_write_bytes(p, str, strlen(str) + 1);
}

void packet_write_bytes(Packet *p, const void *ptr, int n){
	if(p->bytes_used + n > p->buffer_size){
		packet_increase_size(p, n);
	}
	memcpy(p->buffer + p->bytes_used, ptr, n);
	p->bytes_used += n;
}

void packet_test(Packet *p, const char *message){
	packet_write_16(p, 32766);
	packet_write_str(p, message);
}
