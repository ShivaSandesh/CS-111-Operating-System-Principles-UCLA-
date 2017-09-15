/*NAME: Anchal Goyanka, FNU Shiva Sandesh
  EMAIL: anchal.goyanka12344@gmail.com, sandesh.shiva362@gmail.com
  ID: 111-111-111, 111-111-1111*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <errno.h>
#include <time.h>
#include "ext2_fs.h"
#define EXIT_SUCCESS 0
#define EXIT_BAD_ARG 1
#define EXIT_ERROR 2
int image_Fd;
int total_groups;
struct ext2_super_block sb0;
int block_size;
int inode_size;
void process_Superblock() {
  if (pread(image_Fd, &sb0, sizeof(struct ext2_super_block), 1024 ) == -1) {
    fprintf(stderr, "Error While reading superblock from the file");
    exit(EXIT_ERROR);
  }
  int non_rev_inode; //inode number that can be used for regular files
  block_size = 1024 << sb0.s_log_block_size;
  pread(image_Fd, &inode_size, 2, 1024 + 88);
  pread(image_Fd, &non_rev_inode, 4, 1024 + 84);
  //  pread(image_Fd, &sb0, sizeof(ext2_super_block), 1024);
  printf("%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK", sb0.s_blocks_count, sb0.s_inodes_count, block_size, inode_size, sb0.s_blocks_per_group, sb0.s_inodes_per_group, non_rev_inode);
}
void process_BlockBitmap() {
  unsigned char *block_bitmap_array;
  block_bitmap_array = malloc(block_size);  /* allocate memory for the bitmap */
  int g;
  struct ext2_group_desc gd0;
  //for each group
  for (g = 0; g < total_groups; g++) {
    int head_group_descriptor = ((2048 / block_size) * block_size) + g * sizeof(struct ext2_group_desc);
    if ( pread(image_Fd, &gd0, sizeof(struct ext2_group_desc), head_group_descriptor) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int head_block_bitmap = gd0.bg_block_bitmap * block_size;
    if ( pread(image_Fd, block_bitmap_array, block_size, head_block_bitmap) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int i;
    int no_bytes_inbitmap = sb0.s_blocks_per_group / 8;
    for (i = 0; i < no_bytes_inbitmap ; i++) {
      unsigned char c = block_bitmap_array[i];
      int k;
      for (k = 0; k < 8; k++) {
	int bit = ((c >> k) & 1);
	if (bit == 0) {
	  int inode_number = i * 8 + (k + 1);
	  printf("%s,%d\n", "BFREE", inode_number);
	}
      }
    }
    int blocks_left = sb0.s_blocks_per_group % 8;
    if (blocks_left != 0) {
      unsigned char c = block_bitmap_array[i];
      int k;
      for (k = 0; k < blocks_left; k++) {
	int bit = ((c >> k) & 1);
	if (bit == 0) {
	  int block_number = i * 8 + (k + 1);
	  printf("%s,%d\n", "BFREE", block_number);
	}
      }
    }
  }
  free(block_bitmap_array);
}
void process_InodeBitmap () {
  unsigned char *inode_bitmap_array;
  inode_bitmap_array = malloc(block_size);  /* allocate memory for the bitmap */
  int g;
  struct ext2_group_desc gd0;
  //for each group
  for (g = 0; g < total_groups; g++) {
    int head_group_descriptor = ((2048 / block_size) * block_size) + g * sizeof(struct ext2_group_desc);
    if (block_size > 2048) {head_group_descriptor = (1 * block_size) + g * sizeof(struct ext2_group_desc);}
    if ( pread(image_Fd, &gd0, sizeof(struct ext2_group_desc), head_group_descriptor) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int head_inode_bitmap = gd0.bg_inode_bitmap * block_size;
    if ( pread(image_Fd, inode_bitmap_array, block_size, head_inode_bitmap) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int i;
    int no_bytes_in_ibitmap = sb0.s_inodes_per_group / 8;
    for (i = 0; i < no_bytes_in_ibitmap ; i++) {
      unsigned char c = inode_bitmap_array[i];
      int k;
      for (k = 0; k < 8; k++) {
	int bit = ((c >> k) & 1);
	if (bit == 0) {
	  int inode_number = i * 8 + (k + 1);
	  printf("%s,%d\n", "IFREE", inode_number);
	}
      }
    }

    int inodes_left = sb0.s_inodes_per_group % 8;
    if (inodes_left != 0) {
      unsigned char c = inode_bitmap_array[i];
      int k;
      for (k = 0; k < inodes_left; k++) {
	int bit = ((c >> k) & 1);
	if (bit == 0) {
	  int inode_number = i * 8 + (k + 1);
	  printf("%s,%d\n", "IFREE", inode_number);
	}
      }
    }
  }
}
void print_DirEntry (int block_no, int indd, int gr) {
  int logical_offset = 0;
  int head_dir_entry = block_no * block_size;//head of dir entry within the block, starting from 0
  int head_block_end = head_dir_entry + block_size - 1;//points to end of block
  //reading list of directoties in a char array
  struct ext2_dir_entry *entry0;
  unsigned char entire_block[block_size];
  if ( pread(image_Fd, entire_block, block_size, head_dir_entry) == -1) {
    fprintf(stderr, "Error While reading directory entry from the file");
    exit(EXIT_ERROR);
  }
  entry0 = (struct ext2_dir_entry *) entire_block;
  while ((head_dir_entry != head_block_end) && (head_dir_entry < head_block_end)) {
    int parent_inode = indd + gr * sb0.s_inodes_per_group;
    int current_inode = entry0->inode;
    int name_len = entry0->name_len;
    int entry_len = entry0->rec_len;
    char fname[name_len + 3]; int j; //file name to be printed
    fname[0] = '\'';
    for (j = 1; j <= name_len; j++) {
      fname[j] = entry0->name[j - 1];
    }
    fname[name_len + 1] = '\'';
    fname[name_len + 2] = '\0';
    if (entry0->rec_len < entry_len) {
      fprintf(stderr, "File is corrupted, rec len is greater than entry len\n");
      break;
    }
    if (entry0->rec_len == 0) {
      break;
    }
    if (current_inode != 0) {
      printf("%s,%d,%d,%d,%d,%d,%s\n", "DIRENT", parent_inode, logical_offset, current_inode, entry_len, name_len, fname);
    }
    head_dir_entry = head_dir_entry + entry0->rec_len;
    logical_offset += entry_len;
    entry0 = (void*) entry0 + entry0->rec_len;
  }
}
//Group Descriptor, Dirctory Entry, Indirect
void process_GD_DE_IP() {
  struct ext2_group_desc gd0;
  total_groups = ((sb0.s_blocks_count - 1) / sb0.s_blocks_per_group) + 1;
  int size_block_descriptor = total_groups * sizeof(struct ext2_group_desc);
  int g;
  //for each group
  for (g = 0; g < total_groups; g++) {
    int head_group_descriptor = ((2048 / block_size) * block_size) + g * sizeof(struct ext2_group_desc);
    if (block_size > 2048) {head_group_descriptor = (1 * block_size) + g * sizeof(struct ext2_group_desc);}
    if ( pread(image_Fd, &gd0, sizeof(struct ext2_group_desc), head_group_descriptor) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int blocks_in_this_group;
    int inodes_in_this_group;
    int b = ((sb0.s_blocks_count) - (g * sb0.s_blocks_per_group));
    int iblocks_per_group = sb0.s_blocks_per_group;
    //finding blocks in this group
    if (b < iblocks_per_group) {
      blocks_in_this_group = b;
    }
    else {
      blocks_in_this_group = sb0.s_blocks_per_group;
    }
    //finding inodes in this group
    int a = blocks_in_this_group - 2; //2 to account for bitmaps
    int inode_blocks_per_group;
    if (((inode_size * sb0.s_inodes_per_group) % block_size) != 0) {
      inode_blocks_per_group = ((inode_size * sb0.s_inodes_per_group) / block_size) + 1;
    }
    else {
      inode_blocks_per_group = (inode_size * sb0.s_inodes_per_group) / block_size;
    }
    if (a > inode_blocks_per_group) {
      inodes_in_this_group = sb0.s_inodes_per_group;
    }
    else {
      inodes_in_this_group = (a * block_size) / inode_size;
    }

    printf("%s,%d,%d,%d,%d,%d,%d,%d,%d\n", "GROUP", g, blocks_in_this_group, inodes_in_this_group, gd0.bg_free_blocks_count, gd0.bg_free_inodes_count, gd0.bg_block_bitmap, gd0.bg_inode_bitmap, gd0.bg_inode_table);
    /****Directory Entry ****/
    int head_inode_table = gd0.bg_inode_table * block_size;
    int ind;
    //
    struct ext2_inode inode_arr[inodes_in_this_group];
    if ( pread(image_Fd, inode_arr, inode_size * inodes_in_this_group, head_inode_table) == -1) {
      fprintf(stderr, "Error While reading inode structure from the file");
      exit(EXIT_ERROR);
    }
    //for each inode
    for (ind = 1; ind <= inodes_in_this_group; ind++) {
      struct ext2_inode *inode0 = &inode_arr[ind - 1];
      if ((inode0->i_links_count > 0) && ((inode0->i_mode & 0x4000) || (inode0->i_mode & 0x8000))) {
	int isDir = 0;
	if (inode0->i_mode & 0x4000) isDir = 1;
	int p;
	int flag = 0;
	for (p = 0; p < 15; p++) {
	  if (flag) break;
	  if ((p < 12) && isDir) { //direct pointers
	    //read dir entry in a struct, process and print
	    if (inode0->i_block[p] == 0) break;
	    print_DirEntry(inode0->i_block[p], ind, g);
	  }
	  //first indirect pointer
	  if (p == 12) {
	    int h; int head_indirect = inode0->i_block[p] * block_size;
	    int direct_block_no;
	    __u32 direct_block_data[block_size / 4];
	    pread(image_Fd, direct_block_data, block_size, head_indirect);
	    for (h = 0; h < block_size / 4; h++) {
	      direct_block_no = direct_block_data[h];
	      //pread(image_Fd, &direct_block_no, 4, head_indirect);
	      head_indirect += 4;
	      if ((direct_block_no == 0) && isDir) {
		flag = 1 ;
		break;
	      }
	      else if (isDir) print_DirEntry(direct_block_no, ind, g);
	      if (direct_block_no != 0) {
		printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 1, 12 + h, inode0->i_block[p], direct_block_no);
	      }
	    }
	  }
	  //second indirect pointer
	  if (p == 13) {
	    int hh; int head_indirect1 = inode0->i_block[p] * block_size;
	    int indirect_block_no1;
	    int flag1 = 0;
	    __u32 indirect_block_no1_arr[block_size / 4];
	    pread(image_Fd, indirect_block_no1_arr, block_size, head_indirect1);
	    for ( hh = 0; hh < block_size / 4; hh++) {
	      if (flag1) break;
	      indirect_block_no1 = indirect_block_no1_arr[hh];
	      head_indirect1 += 4;
	      if ((indirect_block_no1 == 0) && isDir) {
		flag = 1;
		break;
	      }
	      if (indirect_block_no1 != 0) {
		int offs = 12 + (block_size / 4) + (hh * (block_size / 4));
		printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 2, offs, inode0->i_block[p], indirect_block_no1);
	      }
	      int h; int head_indirect = indirect_block_no1 * block_size;
	      int direct_block_no;
	      __u32 direct_block_no_arr[block_size / 4];
	      pread(image_Fd, direct_block_no_arr, block_size, head_indirect);
	      for (h = 0; h < block_size / 4; h++) {
		head_indirect += 4;
		direct_block_no = direct_block_no_arr[h];
		if ((direct_block_no == 0) && isDir) {
		  flag = 1;
		  flag1 = 1;
		  break;
		}
		else if (isDir) print_DirEntry(direct_block_no, ind, g);
		if (direct_block_no != 0) {
		  int offs = 12 + (block_size / 4) + (hh * (block_size / 4)) + h;
		  printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 1, offs, indirect_block_no1, direct_block_no);
		}
	      }
	    }
	  }
	  //triple indirect block
	  if (p == 14) {
	    int hhh; int head_indirect2 = inode0->i_block[p] * block_size;
	    int indirect_block_no2;
	    int flag2 = 0;
	    __u32 indirect_block_no2_arr[block_size / 4];
	    pread(image_Fd, indirect_block_no2_arr, block_size, head_indirect2);
	    for (hhh = 0; hhh < block_size / 4; hhh++) {
	      if (flag2) break;
	      head_indirect2 += 4;
	      indirect_block_no2 = indirect_block_no2_arr[hhh];
	      if ((indirect_block_no2 == 0) && isDir) {
		flag = 1;
		break;
	      }
	      if (indirect_block_no2 != 0) {
		int offs = 12 + (block_size / 4) + ((block_size / 4) * (block_size / 4)) + hhh * ((block_size / 4) * (block_size / 4));
		printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 3, offs, inode0->i_block[p], indirect_block_no2);
	      }

	      int hh; int head_indirect1 = indirect_block_no2 * block_size;
	      int indirect_block_no1;
	      int flag1 = 0;
	      __u32 indirect_block_no1_arr[block_size / 4];
	      pread(image_Fd, indirect_block_no1_arr, block_size, head_indirect1);
	      for ( hh = 0; hh < block_size / 4; hh++) {
		if (flag1) break;
		indirect_block_no1 = indirect_block_no1_arr[hh];
		head_indirect1 += 4;
		if ((indirect_block_no1 == 0) && isDir) {
		  flag = 1;
		  flag2 = 1;
		  break;
		}
		if (indirect_block_no1 != 0) {
		  int offs = 12 + (block_size / 4) + ((block_size / 4) * (block_size / 4)) + hhh * ((block_size / 4) * (block_size / 4)) + hh * (block_size / 4);
		  printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 2, offs, indirect_block_no2, indirect_block_no1);
		}
		int h; int head_indirect = indirect_block_no1 * block_size;
		int direct_block_no;
		__u32 direct_block_no_arr[block_size / 4];
		pread(image_Fd, direct_block_no_arr, block_size, head_indirect);
		for (h = 0; h < block_size / 4; h++) {
		  direct_block_no = direct_block_no_arr[h];
		  head_indirect += 4;
		  if ((direct_block_no == 0) && isDir) {
		    flag = 1;
		    flag1 = 1;
		    flag2 = 1;
		    break;
		  }
		  else if (isDir) print_DirEntry(direct_block_no, ind, g);
		  if (direct_block_no != 0) {
		    int offs = 12 + (block_size / 4) + ((block_size / 4) * (block_size / 4)) + hhh * ((block_size / 4) * (block_size / 4)) + hh * (block_size / 4) + h;
		    printf("%s,%d,%d,%d,%d,%d\n", "INDIRECT", ind, 1, offs, indirect_block_no1, direct_block_no);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
char* time_Format(unsigned int sec) {
  time_t t = sec;
  struct tm *info = gmtime(&t);
  char* buffer = malloc(20);
  strftime(buffer, 20, "%m/%d/%y %H:%M:%S", info);
  return buffer;
}
void process_InodeSummary() {
  int g, j;
  struct ext2_group_desc gd0;
  for (g = 0; g < total_groups; g++) {
    int head_group_descriptor = ((2048 / block_size) * block_size) + g * sizeof(struct ext2_group_desc);
    if ( pread(image_Fd, &gd0, sizeof(struct ext2_group_desc), head_group_descriptor) == -1) {
      fprintf(stderr, "Error While reading group descriptor from the file");
      exit(EXIT_ERROR);
    }
    int offset = block_size * gd0.bg_inode_table;
    int size_inode = sizeof(struct ext2_inode);
    for (j = 0; j < (int) sb0.s_inodes_per_group; j++) {
      struct ext2_inode tocheck;
      if ((pread(image_Fd, &tocheck, size_inode, offset)) == -1) {
	fprintf(stderr, "Error While reading inode table from the file");
	exit(EXIT_ERROR);
      }
      __u16 mode = tocheck.i_mode;
      //printf("%s \n",tocheck.i_mode);
      char ftype = '?';
      int flag = 0;
      if (mode != 0) {
	if (S_ISREG(mode)) ftype = 'f';
	if (S_ISLNK(mode)) ftype = 's';
	if (S_ISDIR(mode)) ftype = 'd';
	int nblocks = tocheck.i_blocks / (2 << (block_size - 1));
	if (tocheck.i_links_count <= 0 ) continue;
	mode = mode & 0x0FFF;
	char* c_time = time_Format((unsigned int)tocheck.i_ctime);
	char* m_time = time_Format((unsigned int)tocheck.i_mtime);
	char* a_time = time_Format((unsigned int)tocheck.i_atime);
	printf("%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", "INODE", j + 1, ftype, mode, tocheck.i_uid, tocheck.i_gid, tocheck.i_links_count, c_time, m_time, a_time, tocheck.i_size, nblocks);
	int v = 0;
	if (ftype == 's') { printf(",%d\n", tocheck.i_block[0]);}
	else {
	  for (v = 0; v < 15; v++)
	    printf(",%d", tocheck.i_block[v]);
	  printf("\n");
	}
      }
      offset = offset + sizeof(struct ext2_inode);
    }
  }
}
int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Too many/little arguments applied to main \n");
    exit(EXIT_BAD_ARG);
  }
  image_Fd = open(argv[1], O_RDONLY);

  //STEP 1: Superblock summary
  process_Superblock();
  // STEP 2: Group summary
  // STEP 6: Directory entries
  // STEP 7: Indirect block references
  // in group descriptor
  process_GD_DE_IP();
  // STEP 3: Free block entries
  process_BlockBitmap();
  // STEP 4: Free I-node entries
  process_InodeBitmap();
  // STEP 5: I-node summary
  process_InodeSummary();
  exit(EXIT_SUCCESS);
}
