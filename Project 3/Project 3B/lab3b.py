#!usr/bin/python

# NAME: ANCHAL GOYNKA, FNU SHIVA SANDESH
# EMAIL: anchal.goyanka12344@gmail.com, sandesh.shiva362@gmail.com
# ID: 111111111, 1111111111
# Project 3B
import re
import sys

arg_name = " "
tot_Blocks = inodes_PerGroup = blocks_PerGroup = tot_Inodes = first_NonRevI = 0
group_lst = []
bfree_lst = []
ifree_lst = []
inode_lst = []
dirent_lst = []
indirect_lst = []
ifree_map = []
bfree_map = []
iused_map = []
inode_dict = {}
indirect_dict  = {}
group_dict = {}
reserved_list = []
block_size =1
inode_size = 1
dirent_dict = {}
allocated = []
unallocated= []
#keys as block nos between end of inodes and start of next grp
#values as 1 or 0 for present on bitmap or not, list of each time block is refrenced [inode no, offset]
usable_blocks = {}
# Method to extract all the values into the list #
def parse_file():
    with open(sys.argv[1]) as input_file:    
        for line in input_file:
            line = line.rstrip('\n')
            if line.startswith('SUPERBLOCK,'):
                 line = line.split(",")
                 global tot_Blocks 
                 global inode_PerGroup
                 global blocks_PerGroup
                 global tot_Inodes
                 global first_NonRevI
                 global inode_size
                 global block_size
                 tot_Blocks = int(line[1])
                 tot_Inodes = (int)(line[2])
                 blocks_PerGroup = int (line[5])
                 inode_PerGroup = int (line[6])
                 first_NonRevI = int (line[7])
                 block_size = int(line[3])
                 inode_size = int(line[4])

            elif line.startswith('GROUP,'):
                #group_lst.append(line)
                line = line.split(",")
                #key = group no
                #values = blocks in this grp, inode in this grp, block bitmap block no,
                #inode bitmap block no, inode table block no
                group_dict[int(line[1])] = [int(x) for x in line[2:4] + line[6:9]]
                
            elif line.startswith('BFREE,'):
                bfree_lst.append(line)
                line = line.split(",")
                bfree_map.append((int)(line[1]))
                
            elif line.startswith('IFREE,'):
                ifree_lst.append(line)
                line = line.split(",")
                ifree_map.append((int)(line[1]))

            elif line.startswith('INODE,'):
                inode_lst.append(line)
                line = line.split(",")
                iused_map.append((int)(line[1]))
                #all 15 pointer corresponding to direct and indirect pointers
                if(line[2] != 's'):
                    inode_dict[(int)(line[1])] = [int(x) for x in line[12:]]

            elif line.startswith('DIRENT,'):
                dirent_lst.append(line)
                line = line.split(",")
                #key is inode no of entry, value is parent inode, name
                if int(line[1]) not in dirent_dict.keys():
                    dirent_dict[int(line[1])] = [[int(line[3]), line[6]]]
                else:
                    dirent_dict[int(line[1])].append([int(line[3]), line[6]])

            elif line.startswith('INDIRECT,'):
                line = line.split(",")
                #indirect_lst.append(line)
                #key is block no that is pointed to
                #values: inode no, level indirection, offset, parent block no
                indirect_dict[int(line[5])] = [int(x) for x in line[1:5]]
            
            else:
                print("Unidentified word in begining of line")
                sys.exit(1)
    return

def finds_reserved_blocks():
    global reserved_list
    #appeds superblock, grp descriptor , bitmaps to reserverd block
    inode_starts = group_dict[0][4]
    j =0
    while j < inode_starts:
        reserved_list.append(j)
        j+=1
    for g in range(len(group_dict.keys())):
        inode_blocks_in_this_group =int((group_dict[g][1] * inode_size)/block_size)
        if (group_dict[g][1] * inode_size)%block_size !=0:
            inode_blocks_in_this_group +=1
        #adding both bitmap blocks to reserverd blocks of each group
        reserved_list.append(group_dict[g][3])
        reserved_list.append(group_dict[g][2])
        i =0
        while(i < inode_blocks_in_this_group):
            #adding all inode blocks to reserverd blocks of each group
            reserved_list.append(group_dict[g][4]+i)
            i+=1
    reserved_set = set(reserved_list)
    reserved_list = list(reserved_set)

def finds_usable_blocks():
    for i in range(tot_Blocks):
        if i not in reserved_list:
            if i in bfree_map:
                usable_blocks[i] = [1]
            else:
                usable_blocks[i] = [0]

def blocks_validity():
    for i in indirect_dict.keys():
        off = indirect_dict[i][2]
        if off <12 + block_size/4:
            level = " INDIRECT"
        elif off < 12 + block_size/4 + (block_size/4*block_size/4):
            level = " DOUBLE INDIRECT"
        else:
            level = " TRIPPLE INDIRECT"
        if i< 0 or i > tot_Blocks -1:
            print(("INVALID%s BLOCK " + str(i) + " IN INODE " + str(indirect_dict[i][0]) + " AT OFFSET "+ str(indirect_dict[i][2]))%level)
        elif i in reserved_list:
            print(("RESERVED%s BLOCK " + str(i) + " IN INODE " + str(indirect_dict[i][0]) + " AT OFFSET "+ str(indirect_dict[i][2]))%level)
        else:
            usable_blocks[i].append([indirect_dict[i][0],indirect_dict[i][2]])
    for i in inode_dict.keys():
        for j in range(15):
            if j <12:
                level = ""
            if j == 12:
                level = " INDIRECT"
            if j == 13:
                level = " DOUBLE INDIRECT"
            if j == 14:
                level = " TRIPPLE INDIRECT"
            b = inode_dict[i][j]
            offset = find_offset(j)
            if b < 0 or b > tot_Blocks -1:
                print(("INVALID%s BLOCK " + str(b) + " IN INODE " + str(i) + " AT OFFSET "+ str(offset))%level)
            ##############!= 0 or not
            elif b in reserved_list and b!=0: #checking for every block no in inode, even if 0, not in use
                print(("RESERVED%s BLOCK " + str(b) + " IN INODE " + str(i) + " AT OFFSET "+ str(offset))%level)
            elif b!=0:
                usable_blocks[b].append([i,offset])
    for b in usable_blocks.keys():
        temp = usable_blocks[b]
        if len(temp) == 1 and temp[0] == 0:#no refernces and not in free list
            print("UNREFERENCED "+ "BLOCK " + str(b))
        elif len(temp) == 2 and temp[0] == 1:
            print("ALLOCATED BLOCK " + str(b) + " ON FREELIST")
        elif len(temp) >2:
            if temp[0] == 1:
                print("ALLOCATED BLOCK " + str(b) + " ON FREELIST")
            for k in temp[1:]:
                off = k[1]
                if off < 12:
                    level = ""
                elif off < 12 + block_size/4:
                    level = " INDIRECT"
                elif off < 12 + block_size/4 + (block_size/4*block_size/4):
                    level = " DOUBLE INDIRECT"
                else:
                    level = " TRIPPLE INDIRECT"
                print(("DUPLICATE%s BLOCK "+ str(b) + " IN INODE " + str(k[0]) + " AT OFFSET " + str(k[1]))%level)

def find_offset(jj):
    global block_size
    if jj < 12:
        offs= jj
    elif jj == 12:
        offs = 12
    elif jj == 13:
        offs = 12 + block_size/4 #32 bit block no
    elif jj == 14:
        offs = 12 + block_size/4 + (block_size/4)*(block_size/4)
    return offs

def directory_consistency():
    for d in dirent_dict.keys():
        temp = dirent_dict[d]
        for j in range(len(dirent_dict[d])):
            if temp[j][0] <1 or temp[j][0] >tot_Inodes:
                print("DIRECTORY INODE " + str(d) + " NAME " + temp[j][1] + " INVALID INODE " + str(temp[j][0]))
            elif temp[j][0] not in iused_map or temp[j][0] in unallocated:
                print("DIRECTORY INODE " + str(d) + " NAME " + temp[j][1] + " UNALLOCATED INODE " + str(temp[j][0]))
            if temp[j][1] == "'.'":
                if temp[j][0] != d:
                    print("DIRECTORY INODE " + str(d) + " NAME " + temp[j][1] + " LINK TO INODE " + str(temp[j][0])+" SHOULD BE " + str(d))
            if temp[j][1] == "'..'":
                if d == 2:
                    if temp[j][0] != d:
                        print("DIRECTORY INODE " + str(d) + " NAME " + temp[j][1] + " LINK TO INODE " + str(temp[j][0])+" SHOULD BE " + str(d))
                else:
                    for h in dirent_dict.keys():
                        for k in dirent_dict[h]:
                            if k[0] == d and h != d:
                                if temp[j][0] != h:
                                    print("DIRECTORY INODE " + str(d) + " NAME " + temp[j][1] + " LINK TO INODE " + str(temp[j][0])+" SHOULD BE " + str(h))

# This method checks if any of the inodes after the first non reserved inode 
# is not in either the free or the allocated list.
def inode_comparator():
    # check for all the case where an inode greater than or equal to first 
    # non-reserved inode is not beonging to a either free or used inode map
    for i in range (first_NonRevI, tot_Inodes):
        if( i not in ifree_map and i not in iused_map):
            print("UNALLOCATED INODE " + str(i) + " NOT ON FREELIST")
        #elif( i in ifree_map and i in iused_map):
         #   print("ALLOCATED INODE " + str(i) + " ON FREELIST")
    
    # check if the type of each inode is either 's' or 'd' or 'f' or '?'
    # in nutshell it is non-zero
    correct_type = ['s', 'f', 'd', '?']
    for entry in inode_lst:
        entry = entry.split(",")
        if(str(entry[2]) not in correct_type):
            if(int(entry[2]) == 0):
                unallocated.append(int(entry[1]))

        if(str(entry[2]) in correct_type):
                allocated.append(int(entry[1]))
    
    for i in unallocated:
        if(i not in ifree_map):
           print("UNALLOCATED INODE " + str(i) + " NOT ON FREELIST")
    
    for i in allocated:
        if(i in ifree_map):
            print("ALLOCATED INODE " + str(i) + " ON FREELIST")

    # check for the unallocated inodes i.e directory entries are pointing
    
    return

# This method will compare the link count 
def linkcount_comparator():
    for entry in inode_lst:
        entry = entry.split(",")
        # get the inode number and link count
        # visit all the directory, check if no. matches to actual refrences
        inum = int(entry[1])
        lcount = int(entry[6])
        count = 0
        for j in dirent_lst:
            j = j.split(",")
            curr_dirref = int(j[3])
            if(curr_dirref == inum):
                count = count + 1;
        if(count != lcount):
            print("INODE " + str(inum) + " HAS " + str(count) + " LINKS BUT LINKCOUNT IS " + str(lcount))
    corupt_map = [18]
        # <------- UNREFERENCED INODE LINKCOUNT CHECKING ------------->
#    for entry in ifree_map:
 #       inum = int(entry)
  #      lcount = 0
   #     count = 0
    #    for j in dirent_lst:
     #       j = j.split(",")
      #      curr_dirref = int(j[3])
       #     if((curr_dirref == inum) and curr_dirref not in corupt_map):
        #        count = count + 1;
        #if(count != lcount):
         #   print("INODE " + str(inum) + " HAS " + str(count) + " LINKS BUT LINKCOUNT IS " + str(lcount))
    return

# Main function
def main():
    total = len(sys.argv) 
    # Get the arguments list 
    cmdargs = str(sys.argv) 
    # Print it
    if total < 2:
        sys.stderr.write(" ERROR: Input `.csv` file is missing.")
        sys.stderr.write(" Corect Usage: python lab3b.py Name_OF_FILE.")
        sys.exit(1)
    if total > 2:
        sys.stderr.write(" ERROR: Too many arguments to program.")
        sys.stderr.write(" Corect Usage: python lab3b.py Name_OF_FILE.")
        sys.exit(1)

    arg_name = sys.argv[1]
    try:
        csv_fp = open(arg_name) 
    except IOError:
        sys.stderr.write("Could not open file!!")
        sys.exit(1)

    parse_file()
    inode_comparator()
    finds_reserved_blocks()
    finds_usable_blocks()
    blocks_validity()
    directory_consistency()
    linkcount_comparator()
if __name__ == "__main__":
    main()
lab3b.py
Open with Drive Notepad
Displaying lab3b.py.