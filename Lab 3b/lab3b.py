'''
Name: Tian Ye
Email: tianyesh@gmail.com
UID: 704931660
'''
#!/usr/bin/python
import sys
import csv

# Key: Block Num
# Val: [Inode Num, Offset, Level]
block_dict = {}

# Key: Inode Num
# Val: Link Count
inode_dict_lnk_c = {}

# Key: Inode Num
# Val: Ref Link Count
inode_dict_ref_c = {}

# Key: Inode Num
# Val: Directory Name
inode_referenced = {}

# Key: Inode Num
# Val: Parent Inode
inode_dict_parnt = {}

# Key: Parent Inode Num
# Val: Inode Num
parnt_inode_dict = {}

# Set of free blocks and inodes
bfree = set()
ifree = set()

# Set of reserved block and inode numbers
reserved_blocks = set([0, 1, 2, 3, 4, 5, 6, 7, 64])
reserved_inodes = set([1, 3, 4, 5, 6, 7, 8, 9, 10])

# Flags and Counters
total_blocks = 0
total_inodes = 0
corrupted = False

# Input Checking
if len(sys.argv) != 2:
    sys.stderr.write("Error, incorrect number of arguments.\n")
    sys.exit(1)

# Open File and Parse
try:
    with open(sys.argv[1], 'rb') as input_file:
        reader = csv.reader(input_file, delimiter=',')
        for entry in reader:

            if entry[0] == "SUPERBLOCK":
                total_blocks = int(entry[1])
                total_inodes = int(entry[2])

            elif entry[0] == "BFREE":
                bfree.add(int(entry[1]))

            elif entry[0] == "IFREE":
                ifree.add(int(entry[1]))

            elif entry[0] == "INODE":
                inode_num = int(entry[1])
                inode_dict_lnk_c[inode_num] = int(entry[6])
                for i in range(12, 27):
                    block_num = int(entry[i])
                    # Unused Block Address
                    if block_num == 0:
                        continue

                    # Value Setting
                    if i == 24:
                        ind_str = "INDIRECT"
                        offset = 12
                        level = 1
                    elif i == 25:
                        ind_str = "DOUBLE INDIRECT"
                        offset = 268
                        level = 2
                    elif i == 26:
                        ind_str = "TRIPLE INDIRECT"
                        offset = 65804
                        level = 3
                    else:
                        ind_str = ""
                        offset = 0
                        level = 0

                    # Parsing and Checking
                    if block_num > total_blocks or block_num < 0:
                        if ind_str == "":
                            print "INVALID BLOCK {} IN INODE {} AT OFFSET {}".format(block_num, inode_num, offset)
                        else:
                            print "INVALID {} BLOCK {} IN INODE {} AT OFFSET {}".format(ind_str, block_num, inode_num, offset)
                        corrupted = True
                    elif block_num in reserved_blocks and block_num != 0:
                        if ind_str == "":
                            print "RESERVED BLOCK {} IN INODE {} AT OFFSET {}".format(block_num, inode_num, offset)
                        else:
                            print "RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}".format(ind_str, block_num, inode_num, offset)
                        corrupted = True
                    elif block_num not in block_dict:
                        # First Reference
                        block_dict[block_num] = [ [inode_num, offset, level] ]
                    else:
                        # Duplicate Reference
                        block_dict[block_num].append([inode_num, offset, level])

            elif entry[0] == "INDIRECT":
                inode_num = int(entry[1])
                block_num = int(entry[5])
                level = int(entry[2])
                
                # Level and Offset Setting
                if level == 1:
                    ind_str = "INDIRECT"
                    offset = 12
                elif level == 2:
                    ind_str = "DOUBLE INDIRECT"
                    offset = 268
                elif level == 3:
                    ind_str = "TRIPLE INDIRECT"
                    offset = 65804

                # Parsing and Checking
                if block_num > total_blocks or block_num < 0:
                    print "INVALID {} BLOCK {} IN INODE {} AT OFFSET {}".format(ind_str, block_num, inode_num, offset)
                    corrupted = True
                elif block_num in reserved_blocks:
                    print "RESERVED {} BLOCK {} IN INODE {} AT OFFSET {}".format(ind_str, block_num, inode_num, offset)
                    corrupted = True
                elif block_num not in block_dict:
                    # First Reference
                    block_dict[block_num] = [ [inode_num, offset, level] ]
                else:
                    # Duplicate Reference
                    block_dict[block_num].append([inode_num, offset, level])

            elif entry[0] == "DIRENT":
                parent_inode = int(entry[1])
                inode_num = int(entry[3])
                dir_name = entry[6]
                inode_referenced[inode_num] = dir_name

                # Update Reference Counter
                if inode_num not in inode_dict_ref_c:
                    inode_dict_ref_c[inode_num] = 1
                else:
                    inode_dict_ref_c[inode_num] = inode_dict_ref_c[inode_num] + 1
                
                if inode_num > total_inodes or inode_num < 1:
                    print "DIRECTORY INODE {} NAME {} INVALID INODE {}".format(parent_inode, dir_name, inode_num)
                    corrupted = True
                    continue

                if dir_name[0:3] == "'.'" and parent_inode != inode_num:
                    print "DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(parent_inode, inode_num, parent_inode)
                    corrupted = True
                elif dir_name[0:3] == "'.'":
                    continue
                elif dir_name[0:4] == "'..'":
                    parnt_inode_dict[parent_inode] = inode_num
                else:
                    inode_dict_parnt[inode_num] = parent_inode
except IOError:
    sys.stderr.write("Error, could not open file.\n")
    sys.exit(1)

# Check Free Block List for Unreferenced Blocks and Allocation Inconsistencies
for block in range(1, total_blocks + 1):
    if block not in bfree and block not in block_dict and block not in reserved_blocks:
        print "UNREFERENCED BLOCK {}".format(block)
        corrupted = True
    elif block in bfree and block in block_dict:
        print "ALLOCATED BLOCK {} ON FREELIST".format(block)
        corrupted = True

# Duplicate Block Entries
for block in block_dict:
    if len(block_dict[block]) > 1:
        corrupted = True
        for data in block_dict[block]:
            level = int(data[2])

            if level == 1:
                ind_str = "INDIRECT"
            elif level == 2:
                ind_str = "DOUBLE INDIRECT"
            elif level == 3:
                ind_str = "TRIPLE INDIRECT"

            if level == 0:
                print "DUPLICATE BLOCK {} IN INODE {} AT OFFSET {}".format(block, data[0], data[1])
            else:
                print "DUPLICATE {} BLOCK {} IN INODE {} AT OFFSET {}".format(ind_str, block, data[0], data[1])

# Check Allocated Inodes List for Inconsistencies
for inode in range(1, total_inodes + 1):
    if inode not in ifree and inode not in inode_dict_lnk_c and inode not in inode_dict_parnt and inode not in reserved_inodes:
        print "UNALLOCATED INODE {} NOT ON FREELIST".format(inode)
        corrupted = True
    elif inode in inode_dict_lnk_c and inode in ifree:
        print "ALLOCATED INODE {} ON FREELIST".format(inode)
        corrupted = True

# Unallocated Inodes
for inode in inode_referenced:
    if inode in ifree and inode in inode_dict_parnt:
        dir_name = inode_referenced[inode]
        print "DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(inode_dict_parnt[inode], dir_name, inode)
        corrupted = True

# Inconsistent Link Count
for inode in inode_dict_lnk_c:
    link_1 = 0
    link_2 = inode_dict_lnk_c[inode]
    if inode in inode_dict_ref_c:
        link_1 = inode_dict_ref_c[inode]
    
    if link_1 != link_2:
        print "INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode, link_1, link_2)
        corrupted = True

# Directory Links
for parent in parnt_inode_dict:
    if parent == 2 and parnt_inode_dict[parent] == 2:
        continue
    elif parent == 2:
        print "DIRECTORY INODE 2 NAME '..' LINK TO INODE {} SHOULD BE 2".format(parnt_inode_dict[parent])
        corrupted = True
    elif parent not in inode_dict_parnt:
        print "DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(parnt_inode_dict[parent], parent, parnt_inode_dict[parent])
        corrupted = True
    elif parnt_inode_dict[parent] != inode_dict_parnt[parent]:
        print "DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(parent, parent, inode_dict_parnt[parent])
        corrupted = True

if corrupted:
    sys.exit(2)
else:
    sys.exit(0)
