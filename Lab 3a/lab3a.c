/**
Name: Tian Ye
Email: tianyesh@gmail.com
ID: 704931660
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "ext2_fs.h"

/**
References:
	http://www.nongnu.org/ext2-doc/ext2.html
	http://cs.smith.edu/~nhowe/Teaching/csc262/oldlabs/ext2.html
	http://man7.org/linux/man-pages/man8/debugfs.8.html
	https://stackoverflow.com/questions/8266183/linking-with-gcc-and-lm-doesnt-define-ceil-on-ubuntu
*/

/**
Global Variables
*/
int fd;
uint32_t blocks_count;
uint32_t inodes_count;
uint32_t block_size;
uint32_t inode_size;
uint32_t blocks_per_group;
uint32_t inodes_per_group;
uint32_t first_ino;
uint32_t first_data_block;
struct ext2_super_block superblock;

/**
Block Processing Function:
*/
void free_block(uint32_t group, uint32_t block_bitmap) {
	char* bytes = (char*) malloc(block_size);
	pread(fd, bytes, block_size, 1024 + (block_bitmap - 1) * block_size);
	
	uint32_t index = first_data_block + group * blocks_per_group;
	uint32_t i, j;
	for(i = 0; i < block_size; i++) {
		char c = bytes[i];
		for(j = 0; j < 8; j++) {
			if(!(c & 1)) {
				fprintf(stdout, "BFREE,%u\n", index);
			}
			index++;
			c >>= 1;
		}
	}
	free(bytes);
}

/**
Inode Processing Function:
*/
void free_inode(uint32_t group, uint32_t inode_bitmap, uint32_t inode_table) {
	uint32_t num_bytes = inodes_per_group / 8;
	char* bytes = (char*) malloc(num_bytes);
	pread(fd, bytes, num_bytes, 1024 + (inode_bitmap - 1) * block_size);
	
	uint32_t first = group * inodes_per_group + 1;
	uint32_t current = first;
	uint32_t i, j, k, l, m, n;
	for(i = 0; i < num_bytes; i++) {
		char c = bytes[i];
		for(j = 0; j < 8; j++) {
			if(!(c & 1)) {
				fprintf(stdout, "IFREE,%u\n", current);
			}
			else {
				uint32_t index = current - first;
				struct ext2_inode inode;
				pread(fd, &inode, sizeof(inode), (1024 + (inode_table - 1) * block_size) + index * sizeof(inode));
				
				if(inode.i_mode && inode.i_links_count) {
					char filetype = '?';
					uint16_t file_type = ((inode.i_mode) >> 12) << 12;
					// File
					if(file_type == 0x8000) {
						filetype = 'f';
					}
					// Directory
					else if(file_type == 0x4000) {
						filetype = 'd';
					}
					// Symbolic Link
					else if(file_type == 0xa000) {
						filetype = 's';
					}
					
					char creation_time[20], modification_time[20], access_time[20];
					
					time_t c_time = inode.i_ctime;
					struct tm *ctime = gmtime(&c_time);
					strftime(creation_time, 18, "%m/%d/%y %H:%M:%S", ctime);
					time_t m_time = inode.i_mtime;
					struct tm *mtime = gmtime(&m_time);
					strftime(modification_time, 18, "%m/%d/%y %H:%M:%S", mtime);
					time_t a_time = inode.i_atime;
					struct tm *atime = gmtime(&a_time);
					strftime(access_time, 18, "%m/%d/%y %H:%M:%S", atime);
					
					// Initial output
					fprintf(stdout, "INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u", current, filetype, inode.i_mode & 0xFFF, inode.i_uid, inode.i_gid, inode.i_links_count, creation_time, modification_time, access_time, inode.i_size, 2 * (inode.i_blocks / (2 << superblock.s_log_block_size)));
					
					if(filetype == 'f' || filetype == 'd' || (filetype == 's' && inode.i_size > 60)) {
						for(k = 0; k < EXT2_N_BLOCKS; k++) {
							fprintf(stdout, ",%u", inode.i_block[k]);
						}
					}
					fprintf(stdout, "\n");
					
					// Directory entries
					for(k = 0; k < EXT2_NDIR_BLOCKS; k++) {
						if(inode.i_block[k] && filetype == 'd') {
							struct ext2_dir_entry dir_entry;

							for(l = 0; l < block_size; l += dir_entry.rec_len) {
								memset(dir_entry.name, 0, 256);
								pread(fd, &dir_entry, sizeof(dir_entry), (1024 + (inode.i_block[k] - 1) * block_size) + l);
								if(dir_entry.inode) {
									fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", current, l, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
								}
							}
						}
					}
					
					// Indirect block references
					if(inode.i_block[12]) {
						uint32_t *block_ptr = malloc(block_size);
						uint32_t num_ptrs = block_size / sizeof(uint32_t);
						pread(fd, block_ptr, block_size, 1024 + (inode.i_block[12] - 1) * block_size);
						
						for(k = 0; k < num_ptrs; k++) {
							if(block_ptr[k]) {
								// Directory processing
								if(filetype == 'd') {
									struct ext2_dir_entry dir_entry;
									for(l = 0; l < block_size; l += dir_entry.rec_len) {
										memset(dir_entry.name, 0, 256);
										pread(fd, &dir_entry, sizeof(dir_entry), (1024 + (block_ptr[k] - 1) * block_size) + l);
										if(dir_entry.inode) {
											fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", current, l, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
										}
									}
								}
								fprintf(stdout, "INDIRECT,%u,1,%u,%u,%u\n", current, k + 12, inode.i_block[12], block_ptr[k]);
							}
						}
						free(block_ptr);
					}
					
					// Double linked indirect block references
					if(inode.i_block[13]) {
						uint32_t *indirect_block_ptr = malloc(block_size);
						uint32_t num_ptrs = block_size / sizeof(uint32_t);
						pread(fd, indirect_block_ptr, block_size, 1024 + (inode.i_block[13] - 1) * block_size);
						
						for(k = 0; k < num_ptrs; k++) {
							if(indirect_block_ptr[k]) {
								fprintf(stdout, "INDIRECT,%u,2,%u,%u,%u\n", current, k + 12 + 256, inode.i_block[13], indirect_block_ptr[k]);
								
								// Search through block to find directory entires
								uint32_t *block_ptr = malloc(block_size);
								pread(fd, block_ptr, block_size, 1024 + (indirect_block_ptr[k] - 1) * block_size);
								for(l = 0; l < num_ptrs; l++) {
									if(block_ptr[l]) {
										if((filetype == 'd')) {
											struct ext2_dir_entry dir_entry;
											
											for(m = 0; m < block_size; m += dir_entry.rec_len) {
												memset(dir_entry.name, 0, 256);
												pread(fd, &dir_entry, sizeof(dir_entry), (1024 + (block_ptr[l] - 1) * block_size) + m);
												if(dir_entry.inode) {
													fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", current, m, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
												}
											}
										}
										fprintf(stdout, "INDIRECT,%u,1,%u,%u,%u\n", current, l + 12 + 256, indirect_block_ptr[k], block_ptr[l]);
									}
								}
								free(block_ptr);
							}
						}
						free(indirect_block_ptr);
					}
					
					// Triply linked indirect block references
					if(inode.i_block[14]) {
						uint32_t *double_indirect_block_ptr = malloc(block_size);
						uint32_t num_ptrs = block_size / sizeof(uint32_t);
						pread(fd, double_indirect_block_ptr, block_size, 1024 + (inode.i_block[14] - 1) * block_size);
						
						for(k = 0; k < num_ptrs; k++) {
							if(double_indirect_block_ptr[k]) {
								fprintf(stdout, "INDIRECT,%u,3,%u,%u,%u\n", current, k + 12 + 256 + 65536, inode.i_block[14], double_indirect_block_ptr[k]);
								
								uint32_t *indirect_block_ptr = malloc(block_size);
								pread(fd, indirect_block_ptr, block_size, 1024 + (double_indirect_block_ptr[k] - 1) * block_size);
								for(l = 0; l < num_ptrs; l++) {
									if(indirect_block_ptr[l]) {
										fprintf(stdout, "INDIRECT,%u,2,%u,%u,%u\n", current, l + 12 + 256 + 65536, double_indirect_block_ptr[k], indirect_block_ptr[l]);
										uint32_t *block_ptr = malloc(block_size);
										pread(fd, block_ptr, block_size, 1024 + (indirect_block_ptr[l] - 1) * block_size);
										
										for(m = 0; m < num_ptrs; m++) {
											if(block_ptr[m]) {
												if((filetype == 'd')) {
													struct ext2_dir_entry dir_entry;
													
													for(n = 0; n < block_size; n += dir_entry.rec_len) {
														memset(dir_entry.name, 0, 256);
														pread(fd, &dir_entry, sizeof(dir_entry), (1024 + (block_ptr[m] - 1) * block_size) + n);
														if(dir_entry.inode) {
															fprintf(stdout, "DIRENT,%u,%u,%u,%u,%u,'%s'\n", current, n, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
														}
													}
												}
												fprintf(stdout, "INDIRECT,%u,1,%u,%u,%u\n", current, m + 12 + 256 + 65536, indirect_block_ptr[l], block_ptr[m]);
											}
										}
										free(block_ptr);
									}
								}
								free(indirect_block_ptr);
							}
						}
						free(double_indirect_block_ptr);
					}
				}
			}
			current++;
			c >>= 1;
		}
	}
	free(bytes);
}

/**
Group Processing Function:
*/
void process_group(uint32_t group, uint32_t num_groups) {
	struct ext2_group_desc group_desc;
	
	uint32_t offset;
	if(block_size == 1024) {
		offset = 2;
	}
	else {
		offset = 1;
	}
	
	pread(fd, &group_desc, sizeof(group_desc), block_size * offset + group * sizeof(group_desc));
	
	uint32_t blocks_in_group = blocks_per_group;
	uint32_t inodes_in_group = inodes_per_group;
	
	if(group == num_groups - 1) {
		blocks_in_group = blocks_count - (blocks_per_group * (num_groups - 1));
		inodes_in_group = inodes_count - (blocks_per_group * (num_groups - 1));
	}
	
	fprintf(stdout, "GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", group, blocks_in_group, inodes_in_group, group_desc.bg_free_blocks_count, group_desc.bg_free_inodes_count, group_desc.bg_block_bitmap, group_desc.bg_inode_bitmap, group_desc.bg_inode_table);
	
	free_block(group, group_desc.bg_block_bitmap);
	free_inode(group, group_desc.bg_inode_bitmap, group_desc.bg_inode_table);
}

/**
Main Program Routine:
*/
int main(int argc, char* argv[]) {
	// Preprocessing
	if(argc != 2) {
		fprintf(stderr, "Error, incorrect number of arguments.\n");
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Error opening file.\n");
		exit(1);
	}
	
	pread(fd, &superblock, sizeof(superblock), 1024);
	blocks_count = superblock.s_blocks_count;
	inodes_count = superblock.s_inodes_count;
	block_size = EXT2_MIN_BLOCK_SIZE << superblock.s_log_block_size;
	inode_size = superblock.s_inode_size;
	blocks_per_group = superblock.s_blocks_per_group;
	inodes_per_group = superblock.s_inodes_per_group;
	first_ino = superblock.s_first_ino;
	first_data_block = superblock.s_first_data_block;
	fprintf(stdout, "SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", blocks_count, inodes_count, block_size, inode_size, blocks_per_group, inodes_per_group, first_ino);
	
	uint32_t groups = ceil((double) blocks_count / blocks_per_group);
	uint32_t i;
	for(i = 0; i < groups; i++) {
		process_group(i, groups);
	}
	
	return 0;
}