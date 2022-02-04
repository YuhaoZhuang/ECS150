#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define BLOCK_SIZE 4096 
#define FAT_SIZE 2048
#define FAT_EOC 0XFFFF
#define FD_MAX 32

typedef unsigned int uint;

typedef struct __attribute__((__packed__)) Super_Block {
	char signature[8];
	uint16_t total; 
	uint16_t root_index;
	uint16_t start_index; 
	uint16_t data_count; 
	uint8_t fat_count;
	uint8_t padding[4079];
}Super_Block;

typedef struct __attribute__((__packed__)) FAT_Block {
	uint16_t entries[FAT_SIZE];
}FAT_Block;

typedef struct __attribute__((__packed__)) Root {
	char filename[FS_FILENAME_LEN];
	uint32_t size;
	uint16_t index_first;
	uint8_t padding[10];
}Root;

typedef struct __attribute__((__packed__)) Root_List {
	Root rl[FS_FILE_MAX_COUNT];
}Root_List;

typedef struct __attribute__((__packed__)) FAT_List {
	FAT_Block **fl;
}FAT_List;

typedef struct __attribute__((__packed__)) file_des {
	uint file_index;
	uint offset; 
	uint open; 
}file_des;

typedef struct __attribute__((__packed__)) fd_list {
	file_des fl[FD_MAX];
	uint count;
}fd_list;

/* global super block declared */
Super_Block *super;
Root_List *root;
FAT_List *fat; 
fd_list *fd_l; 

int signature_compare(){
	char str[8] = {'E', 'C', 'S', '1', '5', '0', 'F', 'S'};
	int str_cmp = 0; 
	for (int i = 0; i < 8; i ++){
		if (super->signature[i] != str[i]){
			str_cmp += 1;
		}
	}
	return str_cmp;
}

uint FAT_block_finder(uint block){
	uint count = 0;
	while (block > FAT_SIZE-1){
		block -= FAT_SIZE;
		count += 1;
	}
	return count;
}

uint FAT_index_finder(uint index){
	uint result = index % FAT_SIZE; 
	return result;
}


int check_filename(const char *filename){
	int file_length = strlen(filename);
	// Error if filename invalid, NULL considered
	if (file_length >= FS_FILENAME_LEN || file_length <= 0){
		return -1;
	}

	// Error if filename is not null terminated 
	if (filename[file_length] != '\0') {
		return -1;
	}
	return 0; 
}

int fd_check(int fd){
	// Error if fd is out of bounds 
	if (fd >= FD_MAX || fd < 0){
		return -1; 
	}
	// Error if fd is not open 
	if (fd_l->fl[fd].open == 0){
		return -1; 
	} 
	return 0;
}

// find correct offset
uint offset_finder(uint index){
	/*
	while(index > (BLOCK_SIZE-1)){
		index -= (BLOCK_SIZE-1); 
	}
	*/
	uint result = index % BLOCK_SIZE;
	return result;
}

uint find_data_index(int fd){
	uint root_index = fd_l->fl[fd].file_index;
	uint index = root->rl[root_index].index_first;
	uint offset = fd_l->fl[fd].offset;
	//printf("this is offset %u\n", offset);
	uint fat_i = 0; 
	uint fat_index = 0; 
	while(offset > (BLOCK_SIZE-1)){
		offset -= BLOCK_SIZE; 
		fat_i = FAT_block_finder(index);
		fat_index = FAT_index_finder(index);
		index = fat->fl[fat_i]->entries[fat_index];
	}
	//index += super->root_index;
	//index += 1; 
	return index;
}

uint find_fat_i(int fd){
	uint root_index = fd_l->fl[fd].file_index;
	uint index = root->rl[root_index].index_first;
	uint offset = fd_l->fl[fd].offset;
	//printf("this is offset %u\n", offset);
	uint fat_i = 0; 
	uint fat_index = 0; 
	while(offset > (BLOCK_SIZE-1)){
		offset -= BLOCK_SIZE; 
		fat_i = FAT_block_finder(index);
		fat_index = FAT_index_finder(index);
		index = fat->fl[fat_i]->entries[fat_index];
	}
	//index += super->root_index;
	//index += 1; 
	return fat_i;
}

uint find_fat_index(int fd){
	uint root_index = fd_l->fl[fd].file_index;
	uint index = root->rl[root_index].index_first;
	uint offset = fd_l->fl[fd].offset;
	//printf("this is offset %u\n", offset);
	uint fat_i = 0; 
	uint fat_index = 0; 
	while(offset > (BLOCK_SIZE-1)){
		offset -= BLOCK_SIZE; 
		fat_i = FAT_block_finder(index);
		fat_index = FAT_index_finder(index);
		index = fat->fl[fat_i]->entries[fat_index];
	}
	//index += super->root_index;
	//index += 1; 
	return fat_index;
}

uint find_root_index(const char *filename){
	uint index = 0;
	uint index_exist = 0; 
	for (uint i = 0; i < FS_FILE_MAX_COUNT; i++){
		uint x = strcmp(root->rl[i].filename, filename);
		if (x == 0){
			index_exist += 1;
			break;
		} else {
			index += 1;
		}
	}
	
	if (index_exist == 0){
		return FS_FILE_MAX_COUNT+1; 
	} else {
		return index;
	}
}

uint find_next_FAT(){
	uint index = 0; 
	uint set = 0; 
	uint count = 0; 
	for (uint i = 0; i < super->fat_count; i++){
		for (uint y = 0; y < FAT_SIZE; y++){
			if (fat->fl[i]->entries[y] == 0){	
				index = count;
				set += 1; 
				break; 
			}
			count += 1;
		}
		if (set == 1){
			break; 
		}
	}
	// if larger than allowed size or no empty entries left
	if (index >= super->data_count || index == 0){
		return 0; 
	} else {
		return index; 
	}
}

int fs_mount(const char *diskname)
{
	int block = block_disk_open(diskname);
	
	if (block != 0){
		return -1;
	}

	// allocate memory for pointers 
	super = (Super_Block*)malloc(sizeof(Super_Block));
	root = (Root_List*)malloc(sizeof(Root_List));
	fat = (FAT_List*)malloc(sizeof(FAT_List));
	fd_l = (fd_list*)malloc(sizeof(fd_list));
	// write block
	
	block_read(0, super);
	block_read(super->root_index, root);
	
	fat->fl = (FAT_Block**)malloc(super->fat_count * sizeof(FAT_Block*));
	
	for (uint i = 1; i <= super->fat_count; i++){
		fat->fl[i-1] = (FAT_Block*)malloc(sizeof(FAT_Block));
		block_read(i, fat->fl[i-1]);
	}
	
	
	if (signature_compare() != 0){
		return -1;
	}
	int total_b = 2 + super->fat_count + super->data_count;
	if (total_b != block_disk_count()){
		return -1;
	}
	if (total_b != super->total){
		return -1; 
	}
	if ((super->fat_count+1) != super->root_index){
		return -1;
	}
	if ((super->fat_count+2) != super->start_index){
		return -1;
	}
	
	return 0;
}

int fs_umount(void)
{
	block_write(0, super);
	block_write(super->root_index, root);
	for (uint i = 1; i <= super->fat_count; i++){
		block_write(i, fat->fl[i-1]);
	}
	free(super);
	free(root);
	free(fat);

	for (uint i = 0; i < FS_OPEN_MAX_COUNT; i++){
		if (fd_l->fl[i].open != 0){
			return -1; 
		}
	}
	free(fd_l);
	int block = block_disk_close();	
	if (block != 0){
		return -1;
	}

	return 0;
}

int fs_info(void)
{

	if (block_disk_count() == -1){
		return -1; 
	}

	uint blk_count = 2 + super->fat_count + super->data_count;
	uint fat_free = 0;
	uint root_used = 0;
	uint count = 0; 
	uint set = 0; 
	for (uint i = 0; i < super->fat_count; i++){
		for (uint y = 0; y < FAT_SIZE; y++){
			if (fat->fl[i]->entries[y] == 0){
				fat_free += 1;
			}
			count += 1;
			if (count >= super->data_count){
				set += 1;
				break; 
			}
		}
		if (set == 1){
			break;
		}
	}

	for (uint i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (root->rl[i].filename[0] != '\0'){
			root_used += 1;
		}
	}

	printf("FS Info:\n");
	printf("total_blk_count=%u\n", blk_count);
	printf("fat_blk_count=%u\n", super->fat_count);
	printf("rdir_blk=%u\n", super->fat_count + 1);
	printf("data_blk=%u\n", super->fat_count + 2);
	printf("data_blk_count=%u\n", super->data_count);
	printf("fat_free_ratio=%u/%u\n", fat_free, super->data_count);
	printf("rdir_free_ratio=%u/%u\n", 128 - root_used, 128);
	//printf("This is the length: %d\n", strlen(root->rl[0].filename));
	return 0;
}

int fs_create(const char *filename)
{
	if (check_filename(filename) != 0){
		return -1; 
	}
	
	// check if file name already exist
	for (uint i = 0; i < FS_FILE_MAX_COUNT; i++){
		int x = strcmp(root->rl[i].filename, filename);
		if (x == 0){
			return -1;
		}
	}

	// get the first empty file in root directory
	uint root_exist = 0;
	for (uint i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (root->rl[i].filename[0] == '\0'){
			strcpy(root->rl[i].filename, filename);
			root->rl[i].size = 0;
			root->rl[i].index_first = FAT_EOC;
			root_exist += 1;
			//printf("this is create: %d\n", i);
			break;
		}
	}

	//Error if no more space left in root directory
	if (root_exist == 0 ){
		return -1;
	}

	return 0;
}

int fs_delete(const char *filename)
{
	if (check_filename(filename) != 0){
		return -1; 
	}

	// find index
	uint root_current = find_root_index(filename);
	if (root_current == FS_FILE_MAX_COUNT+1){
		return -1; 
	}

	// FAT deletion , includes fat index conversion
	uint index = root->rl[root_current].index_first;
	//printf("This is the delete: %d\n", root_current);
	
	if (index != FAT_EOC){
		uint fat_i = 0; 
		uint fat_index = 0;
		while (index != FAT_EOC){
			fat_i = FAT_block_finder(index);
			fat_index = FAT_index_finder(index);
			index = fat->fl[fat_i]->entries[fat_index];
			fat->fl[fat_i]->entries[fat_index] = 0;
		}
	}
	// Root deletion 
	root->rl[root_current].filename[0] = '\0';
	//root->rl[root_current].size = 0;
	//root->rl[root_current].index_first = FAT_EOC;
	
	return 0;
}

int fs_ls(void)
{
	if (block_disk_count() == -1){
		return -1; 
	}
	printf("FS Ls:\n");
	for (uint i = 0; i < FS_FILE_MAX_COUNT; i++){
		if (root->rl[i].filename[0] != '\0'){
			printf("file: %s, ",root->rl[i].filename);
			printf("size: %u, ", root->rl[i].size);
			printf("data_blk: %u\n", root->rl[i].index_first);
		}
	}
	return 0;
}

int fs_open(const char *filename)
{
	if (check_filename(filename) != 0){
		return -1; 
	}

	// Maximum number of file descriptors opened already
	if (fd_l->count == FD_MAX){
		return -1; 
	}

	// find available root entry 
	uint index = find_root_index(filename);
	if (index == FS_FILE_MAX_COUNT+1){
		return -1;
	}

	// keep track of the fd integer
	uint fd_index = 0;
	// find available fd and set file descriptor
	for(uint i = 0; i < FD_MAX; i++){
		if (fd_l->fl[i].open == 0){
			fd_l->fl[i].file_index = index;
			fd_l->fl[i].offset = 0;
			fd_l->fl[i].open = 1;
			fd_l->count += 1;  
			fd_index = i; 
			break;
		}
	}
	//printf("This is fd %u\n", fd_index);
	return fd_index;
}

int fs_close(int fd)
{
	// if file descriptor number is out of bounds
	
	if (fd_check(fd) == -1){
		return -1;
	}
	
	// double check first line
	fd_l->fl[fd].file_index = 0;
	fd_l->fl[fd].offset = 0;
	fd_l->fl[fd].open = 0;
	fd_l->count -= 1; 
	
	return 0;
}

int fs_stat(int fd)
{
	// if file descriptor number is out of bounds
	if (fd_check(fd) == -1){
		return -1;
	}

	uint index = fd_l->fl[fd].file_index; 
	return root->rl[index].size;
}

int fs_lseek(int fd, size_t offset)
{
	// if file descriptor number is out of bounds
	if (fd_check(fd) == -1){
		return -1;
	}

	// if file offset is larger than size of file
	uint index = fd_l->fl[fd].file_index; 
	if (offset > root->rl[index].size){
		return -1; 
	}

	// set offset
	fd_l->fl[fd].offset = offset;
	return 0;
}

int fs_write(int fd, void *buf, size_t count)
{
	if (fd_check(fd) == -1){
		return -1;
	}

	uint buf_count = 0; 
	uint root_index = fd_l->fl[fd].file_index;
	uint index = root->rl[root_index].index_first;

	uint data_index = 0; 
	uint offset = 0; 

	uint fat_i = 0; 
	uint fat_index = 0;

	// empty file 
	if (index == FAT_EOC){
		index = find_next_FAT();
		if (index == 0){
			return buf_count; 
		} else {
			root->rl[root_index].index_first = index;
			fat_i = FAT_block_finder(index);
			fat_index = FAT_index_finder(index);
			fat->fl[fat_i]->entries[fat_index] = FAT_EOC;
		}
	}
	index = find_data_index(fd);
	fat_i = find_fat_i(fd);
	fat_index = find_fat_index(fd);
	while (index != 0 && buf_count < count){
		if (index == FAT_EOC && buf_count < count){
			index = find_next_FAT();
			if (index == 0){
				return buf_count; 
			} else {
				fat->fl[fat_i]->entries[fat_index] = index;
				fat_i = FAT_block_finder(index);
				fat_index = FAT_index_finder(index);
				fat->fl[fat_i]->entries[fat_index] = FAT_EOC;
			}
		}
		void *block = (void*)malloc(BLOCK_SIZE);
		data_index = index + super->root_index + 1;
		//printf("this is data index %u\n", data_index);
		block_read(data_index , block);
		offset = offset_finder(fd_l->fl[fd].offset);
		for (uint i = offset; i < BLOCK_SIZE; i++){
			if(buf_count < count ){
				memcpy(block+i, buf+buf_count, 1);
				buf_count += 1; 
				fd_l->fl[fd].offset += 1;

				if (fd_l->fl[fd].offset > 
				root->rl[root_index].size){
					root->rl[root_index].size += 1; 	
				}
			} else {
				break;
			}
		}
		block_write(data_index, block);
		free(block);
		fat_i = FAT_block_finder(index);
		fat_index = FAT_index_finder(index);
		index = fat->fl[fat_i]->entries[fat_index];
	}
	return buf_count;
}

int fs_read(int fd, void *buf, size_t count)
{
	if (fd_check(fd) == -1){
		return -1;
	}
	// keeps track of how many bytes written into buffer
	uint buf_count = 0; 
	
	// keeps tracks of FAT index 
	uint index = root->rl[fd_l->fl[fd].file_index].index_first;
	uint size = root->rl[fd_l->fl[fd].file_index].size;

	uint fat_i = 0; 
	uint fat_index = 0;

	// which data block reading from
	uint data_index = 0;
	// keeps track of offset in each block
	uint offset = 0;  
	
	// for each data block of the file
	while (index != FAT_EOC && buf_count < count &&
	fd_l->fl[fd].offset < size){
		// allocating memory and read data block
		void *block = (void*)malloc(BLOCK_SIZE);
		// calculating which data block to read from
		//data_index = index + super->root_index +1;
		data_index = find_data_index(fd)+super->root_index + 1;
		block_read(data_index, block);

		offset = offset_finder(fd_l->fl[fd].offset);
		for (uint i = offset; i < BLOCK_SIZE; i++){
			if (buf_count < count && fd_l->fl[fd].offset < size){
				memcpy(buf+buf_count, block+i, 1);
				buf_count += 1;
				fd_l->fl[fd].offset += 1;
			} else {
				break;
			}
		}
		fat_i = FAT_block_finder(index);
		fat_index = FAT_index_finder(index);
		index = fat->fl[fat_i]->entries[fat_index];
		free(block);
	}
	
	return buf_count;
	
}
