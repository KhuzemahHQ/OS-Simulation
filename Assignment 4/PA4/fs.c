// Khuzemah Hassan Qazi (24100092)
// Qaboos Ali Khan (24100153)

#include "fs.h"
#include "disk.h"
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   128
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define BLOCK_SIZE 4096

struct fs_superblock 
{
	// Superblock structure 
	int magic; // File system magic number
	int nblocks; // Number of blocks in file system
	int ninodeblocks; // Number of blocks reserved for inodes 
	int ninodes; // Number of inodes in file system 
};

struct fs_inode 
{
	int isvalid; // Whether or not inode is valid 
	int size; // Size of file
	int direct[POINTERS_PER_INODE]; // Direct pointers 
	int indirect; // Indirect pointer 
};

union fs_block 
{
	struct fs_superblock super; // Superblock 
	struct fs_inode inode[INODES_PER_BLOCK]; // Inode block
	int pointers[POINTERS_PER_BLOCK]; // Pointer block 
	char data[BLOCK_SIZE]; // Data block
};

// DEFINE ALL YOUR HELPER FUNCTIONS HERE:
int *bitmap = NULL; //initialized when mount



// Debug file system -----------------------------------------------------------
void fs_debug()
{
	if (bitmap == NULL){
		return ;
	}

	union fs_block flsys; 
	char* flsys_ptr = &flsys;
	disk_read(0,flsys_ptr);

	if (flsys.super.magic == FS_MAGIC){
		printf("magic number is valid \n");
	}
	else{
		printf("magic number is invalid \n");
	}
	
	// Print bitmap if needed
    // printf("Bitmap: \n");      
    // for (int i = 0; i < disk_size(); i++) {     
    //     printf("%d ", bitmap[i]);     
    // }    
	// printf("\n");  

	printf("%d blocks on disk \n",flsys.super.nblocks);
	printf("%d blocks for inodes \n",flsys.super.ninodeblocks);
	printf("%d inodes total \n",flsys.super.ninodes);

	union fs_block ptr_block; 
	char* ptr_block_ptr = &ptr_block;


	// union fs_block pointed_block; 
	// char* pointed_block_ptr = &pointed_block;

	int inode_blocks = flsys.super.ninodeblocks;

	for (int h=0;h<inode_blocks; h++){

		disk_read(h+1,flsys_ptr);
		for (int i=0; i<INODES_PER_BLOCK; i++){
			if (flsys.inode[i].isvalid ==1){
				printf("Inode %d: \n",i+h*INODES_PER_BLOCK);
				printf("size: %d bytes \n",flsys.inode[i].size);
				printf("Direct blocks:");

				for (int j=0; j<POINTERS_PER_INODE; j++){
					if (flsys.inode[i].direct[j] != 0){
						printf(" %d",flsys.inode[i].direct[j]);
					}
				}

				printf("\n");
				if (flsys.inode[i].indirect != 0){
					
					printf("Indirect block: %d",flsys.inode[i].indirect);
					printf("\n");
					printf("Indirect data blocks: ");
					disk_read(flsys.inode[i].indirect,ptr_block_ptr);
					for (int k=0; k<POINTERS_PER_BLOCK; k++){
						if (ptr_block.pointers[k] != 0){
							printf(" %d",ptr_block.pointers[k]);
						}
					}
					printf("\n");
				}
			}
		}
	}
	
}

// Format file system ----------------------------------------------------------
int fs_format() 
{

	// Make sure disk has not been mounted already
	if (bitmap != NULL){
		return 0;
	}

	// Destroying any data in disk
	union fs_block flsys = {0};
	const char* flsys_ptr_temp = &flsys;
	disk_write(0,flsys_ptr_temp);

	int num_blocks = disk_size();

	int inode_blocks = num_blocks/10 + 1;

	// Clear inode table and thus removing any data previously present in that block
	for (int h=0; h < inode_blocks; h++){
		for (int i = 0; i < INODES_PER_BLOCK; i++){
			flsys.inode[i].isvalid = 0;
			for (int k=0; k<POINTERS_PER_INODE; k++){
				flsys.inode[i].direct[k] = 0;
			}
			flsys.inode[i].indirect = 0;
		}
		disk_write(h+1,flsys_ptr_temp);
	}
	

	// Writing superblock
	flsys.super.magic = FS_MAGIC;
	flsys.super.nblocks = num_blocks;
	flsys.super.ninodeblocks = inode_blocks;
	flsys.super.ninodes = INODES_PER_BLOCK*inode_blocks;

	const char* flsys_ptr = &flsys;
	disk_write(0,flsys_ptr);

	return 1;

}

// Mount file system -----------------------------------------------------------
int fs_mount() 
{
	// Making sure nothing has been mounted before
	if (bitmap != NULL){
		return 0;
	}

	union fs_block flsys; 
	char* flsys_ptr = &flsys;
	disk_read(0,flsys_ptr);

	if (flsys.super.magic != FS_MAGIC){
		return 0;
	}

	int fr_br = flsys.super.nblocks;
	bitmap = malloc(sizeof(int)*disk_size()); 
	for (int i=0; i<disk_size();i++){
		bitmap[i] = 0;
	}
	bitmap[0] = 1;

	int inode_blocks = flsys.super.ninodeblocks;

	// Making bitmap for all inodes

	union fs_block iblock;
	char* iblock_ptr = &iblock;

	union fs_block ind; 
	char* ind_ptr = &ind;

	for (int h=0;h < inode_blocks; h++){
		bitmap[h+1] = 1;

		disk_read(h+1,iblock_ptr);
		for (int i=0; i<INODES_PER_BLOCK; i++){
			for (int j=0; j<POINTERS_PER_INODE; j++){
				if (iblock.inode[i].direct[j] != 0){
					bitmap[iblock.inode[i].direct[j]] = 1;
				}
			}

			if (iblock.inode[i].indirect != 0){
				disk_read(iblock.inode[i].indirect,ind_ptr);
				for (int k=0; k<POINTERS_PER_BLOCK; k++){
					if (ind.pointers[k] != 0){
						if (ind.pointers[k] != 0){
							bitmap[ind.pointers[k]] = 1;
						}
					}
				}
			}
		}

	}

	return 1;
}

int fs_create() 
{

	// Making sure disk has been mounted
	if (bitmap == NULL){
		return -1;
	}

    // Finidng invalid inode to make valid
	union fs_block flsys; 
	char* flsys_ptr = &flsys;
	disk_read(0,flsys_ptr);
	int inode_blocks = flsys.super.ninodeblocks;

	for (int h=0; h < inode_blocks; h++){
		disk_read(h+1,flsys_ptr);
		for (int i = 0; i < INODES_PER_BLOCK; i++){
			if (flsys.inode[i].isvalid == 0){
				flsys.inode[i].isvalid = 1;
				flsys.inode[i].size = 0;
				disk_write(h+1,flsys_ptr);
				return i;
			}
		}
	}

}

// Remove inode ----------------------------------------------------------------
int fs_delete(int inumber) 
{
    // Making sure disk has been mounted
	if (bitmap == NULL){
		return 0;
	}

	// Finding correct inode block and respective inode number
	int iblock = 1;
	int temp_inumber = inumber;
	while (temp_inumber > INODES_PER_BLOCK){
		temp_inumber  = temp_inumber - INODES_PER_BLOCK;
		iblock++;
	}

	union fs_block flsys; 
	char* flsys_ptr = &flsys;
	disk_read(iblock,flsys_ptr);
	
	flsys.inode[temp_inumber].isvalid = 0;
	flsys.inode[temp_inumber].size = 0;

	// Clearing the bitmap for these freed direct blocks
	int temp;
	for (int k=0; k<POINTERS_PER_INODE; k++){
		temp = flsys.inode[temp_inumber].direct[k];
		if (temp != 0){
			bitmap[temp] = 0;
		}
		flsys.inode[temp_inumber].direct[k] = 0;
	}

	//  Clearing the bitmap for these freed indirectly connected blocks
	union fs_block ind_block = {0};
	const char* ind_block_temp = &ind_block;
	if (flsys.inode[temp_inumber].indirect != 0){
		disk_write(flsys.inode[temp_inumber].indirect,ind_block_temp);
		flsys.inode[temp_inumber].indirect = 0;
	}

	const char* flsys_ptr_temp = &flsys;
	disk_write(iblock,flsys_ptr_temp);

	return 1;

}

// Inode stat ------------------------------------------------------------------
int fs_getsize(int inumber) 
{
    // Your Code Here
	// Did not need to make this
}

// Read from inode -------------------------------------------------------------
int fs_read(int inumber, char *data, int length, int offset) 
{

	int intial_length = length;

	// Making sure disk has been mounted
	if (bitmap == NULL){
		return 0;
	}

	// Finding correct inode block and respective inode number
	int iblock = 1;
	int temp_inumber = inumber;
	while (temp_inumber > INODES_PER_BLOCK){
		temp_inumber  = temp_inumber - INODES_PER_BLOCK;
		iblock++;
	}

	// Loading correct inode block
	union fs_block inode_block; 
	char* inode_block_ptr = &inode_block;
	disk_read(iblock,inode_block_ptr);

	if (inode_block.inode[temp_inumber].isvalid == 0){
		return 0;
	}

	if (inode_block.inode[temp_inumber].size < offset){
		return 0;
	}

	// Finding correct start location for reading
	int temp_offset = offset;
	int bl = 0;
	while (temp_offset>BLOCK_SIZE){
		temp_offset = temp_offset - BLOCK_SIZE;
		bl ++;
	}

	// Temperary data block for reading from disk
	union fs_block flsys; 
	char* flsys_ptr = &flsys;
	char *str2 = calloc(length,1);

	int i ;

	// Direct blocks
	for (int gd=bl; gd<POINTERS_PER_INODE;gd++){

		if (inode_block.inode[temp_inumber].direct[gd] != 0){
			disk_read(inode_block.inode[temp_inumber].direct[gd],flsys.data);
		}

		if (length <= 0){
			break;
		}

		for (i=temp_offset; i<length; i++){ 
			if (i+temp_offset > BLOCK_SIZE){
				break;
			}

			str2[i+(gd*BLOCK_SIZE)] = flsys.data[i]; 
		}

		temp_offset = 0;
		length = length - i;

	}

	int l = 0;
    while (str2[l] != '\0') {
        l++; 
	}

	union fs_block pointer_block; 
	char* pointer_block_ptr = &pointer_block;

	// If length > 0, go into indirect blocks
	if (length > 0 && inode_block.inode[temp_inumber].indirect != 0){

		int pointer_block_number = inode_block.inode[temp_inumber].indirect;
		printf("inside indirect \n");
		disk_read(pointer_block_number,pointer_block_ptr);

		for (int f=0; f<POINTERS_PER_BLOCK; f++){
			if (pointer_block.pointers[f] != 0){
				disk_read(pointer_block.pointers[f],flsys.data);
				
				if (length <= 0){
					break;
				}

				for (i=0; i<length; i++){ 
					if (i+temp_offset > BLOCK_SIZE){
						break;
					}
					str2[l+i+(f*BLOCK_SIZE)] = flsys.data[i]; 
				}

				temp_offset = 0;
				length = length - i;
			}
		}
	}

	// Finding length
	l = 0;
	while (str2[l] != '\0') {
        l++; 
	}

	strcpy(data, str2);

	// Multiplying by 4 since a char had size of 4 bytes
	return l*4;

}

// Write to inode --------------------------------------------------------------
int fs_write(int inumber, const char *data, int length, int offset) 
{
    // Making sure disk has been mounted
	if (bitmap == NULL){
		return 0;
	}

	union fs_block superblock;
	char* super_block_ptr = &superblock;
	disk_read(0,super_block_ptr);
	if (inumber > superblock.super.ninodes){
		return 0;
	}

	// Finding correct inode block and respective inode number
	int iblock = 1;
	int temp_inumber = inumber;
	while (temp_inumber > INODES_PER_BLOCK){
		temp_inumber  = temp_inumber - INODES_PER_BLOCK;
		iblock++;
	}

	// Loading inode block
	int origin_length = length;
	union fs_block inode_block; 
	char* inode_block_ptr = &inode_block;
	disk_read(iblock,inode_block_ptr);

	if (inode_block.inode[temp_inumber].isvalid == 0){
		return 0;
	}
	char *str2 = calloc(BLOCK_SIZE/4,1);
	int i = 0 ;

	// Direct blocks
	for (int gd=0; gd<POINTERS_PER_INODE;gd++){

		if (length <= 0){
			break;
		}

		int free_block = -1;
		for (int j = superblock.super.ninodeblocks+1 ; j<disk_size(); j++){
			if(bitmap[j] == 0){
				free_block = j;
				bitmap[free_block] = 1;
				break;
			}
		}
		if (free_block == -1){
			return 0;
		}

		union fs_block dir_bl;
		char* dir_bl_ptr = &dir_bl;

		int y;
		for (y = 0; y< length; y++){
			if (y >= BLOCK_SIZE/4){
				break;
			}
			if (data[y+(i*BLOCK_SIZE)/4] != '\0'){
				dir_bl.data[y] = data[y+(i*BLOCK_SIZE)/4];
			}
			else{
				break;
			}	
		}

		inode_block.inode[temp_inumber].direct[gd] = free_block;
		disk_write(free_block,dir_bl_ptr);

		length = length - y;
		i ++;
	}
	disk_write(iblock,inode_block_ptr);

	// Indirect blocks
	if (length > 0 && i == POINTERS_PER_INODE){
		// Finding free block to become pointer block
		int free_block = -1;
		for (int j = 0; j<disk_size(); j++){
			if(bitmap[j]==0){
				free_block = j;
				break;
			}
		}
		if (free_block == -1){
			return 0;
		}

		inode_block.inode[temp_inumber].indirect = free_block;

		union fs_block pointer_block;
		char* pointer_block_ptr = &pointer_block;

		union fs_block dir_bl;
		char* dir_bl_ptr = &dir_bl;

		for (int f=0; f<POINTERS_PER_BLOCK; f++){
			// Finding free data block to point to
			int free_block_for_ptr = -1;
			for (int j = 0; j<disk_size(); j++){
				if(bitmap[j]==0){
					free_block_for_ptr = j;
					break;
				}
			}
			if (free_block_for_ptr == -1){
				return 0;
			}

			
			int y;
			for (y = 0; y < length; y++){
				if (y >= BLOCK_SIZE/4){
					break;
				}
				if (data[y+(i*BLOCK_SIZE)/4] != '\0'){
					dir_bl.data[y] = data[y+(i*BLOCK_SIZE)/4];
				}
				else{
					break;
				}
			}
			
			pointer_block.pointers[f] = free_block_for_ptr;
			// Writing to pointer block
			disk_write(free_block,pointer_block_ptr);

			// Writing to data block
			disk_write(free_block_for_ptr,dir_bl_ptr);
			length = length - y;
		}
	}

	// Updating inode block
	inode_block.inode[temp_inumber].size = origin_length - length;
	disk_write(iblock,inode_block_ptr);
	return origin_length - length;

}