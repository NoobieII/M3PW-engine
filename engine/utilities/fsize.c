//Lithio (The developer's pseudonym)
//May 22, 2022

#include <stdio.h>
#include <stdlib.h>

//filesize in c, portable, using binary search

//not meant to be efficient, note that this is limited to 2GB files or smaller
int fsize(const char *filename){
	int result;
	unsigned int size = 1;
	int jump;
	FILE *f;
	char c;
	
	f = fopen(filename, "rb");
	if(!f){
		return -1;
	}
	
	// find upper bound
	do{
		fseek(f, size - 1, SEEK_SET);
		result = fread(&c, 1, 1, f);
		size <<= 1;
	} while(result == 1);
	
	size >>= 2;
	
	if(size == 0){
		fclose(f);
		return 0;
	}
	
	fseek(f, size - 1, SEEK_SET);
	result = fread(&c, 1, 1, f);
	if(result == 1 && fread(&c, 1, 1, f) != 1){
		return size;
	}
	
	size <<= 1;
	
	//begin binary search between size and size/2
	jump = size >> 1;
	
	do{
		fseek(f, size - 1, SEEK_SET);
		result = fread(&c, 1, 1, f);
		if(result == 1 && fread(&c, 1, 1, f) != 1){
			break;
		}
		
		jump >>= 1;
		if(result == 1){
			size += jump;
		}
		else{
			size -= jump;
		}
	} while(jump > 0);
	
	fclose(f);
	return size;
}

char *read_from_file(const char *filename){
	int filesize;
	FILE *file;
	char *data;
	
	filesize = fsize(filename);
	if(filesize == -1){
		return NULL;
	}
	data = (char*) malloc(filesize + 1);
	data[filesize] = '\0';
	file = fopen(filename, "rb");
	fread(data, 1, filesize, file);
	fclose(file);
	
	return data;
}

