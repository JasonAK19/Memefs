#define FUSE_USE_VERSION 31
#define _GNU_SOURCE  // for S_IFDIR define error

#include <fuse.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

//according to project requirements
#define dir_SIZE 512
#define DIR_BLOCKS 253
#define MAX_FILES 224
#define BLOCK_SIZE 512

//for opening image file without hardcoding
#define MAX_PATH_LEN 1024
char image_path[MAX_PATH_LEN];

char *files[MAX_FILES];
int file_count = 0;

static int memefs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);

static int memefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags);

static int memefs_open(const char *path, struct fuse_file_info *fi);

static int memefs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

// Part 2
static int memefs_create(const char *path, mode_t mode, struct fuse_file_info *fi);

static int memefs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

static int memefs_truncate(const char *path, off_t offset, struct fuse_file_info *fi);

static int memefs_unlink(const char *path);

//dirctory structure
typedef struct {
    uint16_t file_permissions;
    uint16_t start_block;
    char filename[11]; // 8.3 format
    uint8_t unused;
    uint8_t atime[8]; //BCD timestamp
    uint8_t ctime[8]; //BCD timestamp
    uint8_t mtime[8]; //BCD timestamp
    uint8_t last_write[8]; //BCD timestap
    uint32_t file_size;
    uint16_t uid;
    uint16_t gid;
} dir_entry;

typedef struct memefs_superblock {
    char signature[16]; // Filesystem signature
    uint8_t cleanly_unmounted; // Flag for unmounted state
    uint8_t reseerved1[3]; // Reserved bytes
    uint32_t fs_version; // Filesystem version
    uint8_t fs_ctime[8]; // Creation timestamp in BCD format
    uint16_t main_fat; // Starting block for main FAT
    uint16_t main_fat_size; // Size of the main FAT
    uint16_t backup_fat; // Starting block for backup FAT
    uint16_t backup_fat_size; // Size of the backup FAT
    uint16_t directory_start; // Starting block for directory
    uint16_t directory_size; // Directory size in blocks
    uint16_t num_user_blocks; // Number of user data blocks
    uint16_t first_user_block; // First user data block
    char volume_label[16]; // Volume label
    uint8_t unused[448]; // Unused space for alignment
} __attribute__((packed)) memefs_superblock_t;

//helpers

static int find_file_entry(FILE *fs, const memefs_superblock_t *superblock, const char *filename, dir_entry *entry,
                           int *index) {
    // Iterate over directory entries and find the file
    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock->directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        fread(entry, sizeof(dir_entry), 1, fs);
        // Check if the filename matches
        char entry_name[12];
        memcpy(entry_name, entry->filename, 11);
        entry_name[11] = '\0';
        //found
        if (strcmp(entry_name, filename) == 0) {
            if (index != NULL) {
                *index = i;
            }
            return 1;
        }
    }
    return 0;
}

static int load_superblock(FILE *fs, memefs_superblock_t *superblock) {
    //read superblock
    fseek(fs, 0, SEEK_SET);
    if (fread(superblock, sizeof(memefs_superblock_t), 1, fs) != 1) {
        return -EIO;
    }
    return 0;
}

static int read_dir_entry(FILE *fs, memefs_superblock_t *superblock, int index, dir_entry *entry) {
    //read directory entry
    fseek(fs, (superblock->directory_start * BLOCK_SIZE) + (index * sizeof(dir_entry)), SEEK_SET);
    //check if read was successful
    if (fread(entry, sizeof(dir_entry), 1, fs) != 1) {
        perror("Error reading directory entry");
        return 0;
    }
    return 1;
}

static int find_dir_entry(FILE *fs, memefs_superblock_t *superblock, const char *filename, dir_entry *entry) {
    //iterate over directory entries and find the file
    for (int i = 0; i < MAX_FILES; i++) {
        if (read_dir_entry(fs, superblock, i, entry) && strcmp(entry->filename, filename) == 0 && entry->
            file_permissions != 0x0000) {
            return 1;
        }
    }
    return 0;
}

static int read_file_data(FILE *fs, memefs_superblock_t *superblock, dir_entry *entry, off_t offset, char *buf,
                          size_t size) {
    //calculate block offset
    off_t block_offset = (entry->start_block * BLOCK_SIZE) + offset;
    fseek(fs, block_offset, SEEK_SET);
    //check if read was successful
    if (fread(buf, 1, size, fs) != size) {
        perror("Error reading file data");
        return -1;
    }

    return 0;
}

//converts decimal number to bcd
static inline uint8_t pbcd(uint8_t num) {
    uint8_t a = num % 10, b = num / 10;
    return b <= 9 ? ((b << 4) | a) : 0xFF;
}

static void time_to_bcd(const time_t *t, uint8_t bcd_time[8]) {
    struct tm *tm_info = localtime(t);

    bcd_time[0] = 0x20;
    bcd_time[1] = pbcd((tm_info->tm_year + 1900) % 100);
    bcd_time[2] = pbcd(tm_info->tm_mon + 1);
    bcd_time[3] = pbcd(tm_info->tm_mday);
    bcd_time[4] = pbcd(tm_info->tm_hour);
    bcd_time[5] = pbcd(tm_info->tm_min);
    bcd_time[6] = pbcd(tm_info->tm_sec);
    bcd_time[7] = 0x00;
}

//made because of nonsensical timestamp issues (year was like 50812 or something) -jason
static uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static time_t bcd_to_time(const uint8_t bcd_time[8]) {
    struct tm tm_info = {0};
    tm_info.tm_year = (bcd_to_dec(bcd_time[1]) + 100);
    tm_info.tm_mon = bcd_to_dec(bcd_time[2]) - 1;
    tm_info.tm_mday = bcd_to_dec(bcd_time[3]);
    tm_info.tm_hour = bcd_to_dec(bcd_time[4]);
    tm_info.tm_min = bcd_to_dec(bcd_time[5]);
    tm_info.tm_sec = bcd_to_dec(bcd_time[6]);

    time_t time_value = timegm(&tm_info);
    if (time_value == -1) {
        perror("Error converting BCD to time");
    }
    return time_value;
}

//for truncating file
static int shrink(const char *path, off_t size, struct fuse_file_info *fi, FILE *fs, uint16_t *fat, dir_entry *entry,
                  struct memefs_superblock *superblock, int index) {
    int prev_size = entry->file_size;
    int prev_size_block = BLOCK_SIZE * prev_size;
    int new_size_in = BLOCK_SIZE * size;
    int change = prev_size_block - new_size_in;
    // inn add it woud just be abs of change I think ]
    // change / Block_size would work but there wouold be remainder so this wold just round up
    int block_change = (change + BLOCK_SIZE - 1) / BLOCK_SIZE;
    /*
    if(change % BLOCK_SIZE != 0) {
        block_change ++;
    }
    */
    // cut
    int start = entry->start_block;
    int temp = entry->start_block;
    int curr;
    int prev = -1;
    int freed = 0;
    int freefat = 0;
    /*?
     * blcok 2      1
     *   if avilabe free is less then block cange thernn throw no space weerror
     *c    if no free block then or no enough free space in the ifle systme  reutn nopspace error
     *   retunt minus ENOspace
     *   *?/  */
    // thsi si going through the fat chain until the end and looking for free block just in case
    //                looks throught the chain to see if
    while (temp != 0xFFFF) {
        curr = fat[temp];
        if (curr == 0x0000) {
            // unlink this
            freefat++;
        }
        temp = curr;
    }
    if (freefat < block_change) {
        // return error since we do not have any more free space
        return ENOSR;
        //
    }
    // restarts
    temp = entry->start_block;

    // freeing
    while (temp != 0xFFFF && block_change > 0) {
        prev = temp;
        curr = fat[temp];
        if (curr == 0x0000) {
            // unlink this
            if (prev != -1) {
                fat[prev] = 0x0000;
                freed++;
            }
            block_change--;
        }
        temp = curr;
    }

    fseek(fs, superblock->main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    entry->file_size = size;
    fseek(fs, BLOCK_SIZE + index * sizeof(dir_entry), SEEK_SET);
    fwrite(&entry, sizeof(dir_entry), 1, fs);

    superblock->num_user_blocks -= freed;
    fseek(fs, superblock->directory_start * BLOCK_SIZE, SEEK_SET);
    fwrite(&superblock, sizeof(memefs_superblock_t), 1, fs);

    return 0;
}

static int grow(const char *path, off_t size, struct fuse_file_info *fi, FILE *fs, uint16_t *fat, dir_entry *entry,
                struct memefs_superblock *superblock, int index) {
    // add files on top of the currnt one
    // ext
    int prev_size = entry->file_size;
    int prev_size_block = BLOCK_SIZE * prev_size;
    int new_size_in = BLOCK_SIZE * size;
    int change = abs((prev_size_block - new_size_in));
    // inn add it woud just be abs of change I think ]
    // change / Block_size would work but there wouold be remainder so this wold just round up
    int block_change = (change + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int start = entry->start_block;
    int temp = entry->start_block;
    int curr;
    int prev = -1;
    int freed = 0;
    int freefat = 0;

    // calc see
    while (temp != 0xFFFF && block_change > 0) {
        prev = temp;
        curr = fat[temp];
        if (curr == 0x0000) {
            // unlink this
            freefat++;
        }
        temp = curr;
    }
    if (freefat < block_change) {
        // return error since we do not have any more free space
        return ENOSR;
        //
    }
    // so find n number of fat  - 1 index start at 0
    int fat_index[block_change];
    int indx = 0;
    for (uint16_t i = superblock->first_user_block; i < superblock->first_user_block + superblock->num_user_blocks; i
         ++) {
        if (fat[i] == 0x0000) {
            fat_index[indx] = i;
            indx++;
        }
    }
    prev = -1;
    int hold;
    int curr_index = 0;
    while (block_change > 0) {
        while (temp != 0xFFFF) {
            // this shoudl keep track of prev
            prev = temp;
            // this will iterate throught the fat
            curr = fat[temp];
            temp = curr;
        }
        fat[temp] = 0x0000;
        fat[temp] = fat[curr_index];
        fat[curr_index] = 0xFFFF;
        block_change--;
        curr_index++;
        // go find unused free fat
        // assign it and repet
        // add it toward the end  no space error no more free space
        // we need to
    }

    fseek(fs, superblock->main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    entry->file_size = size;
    fseek(fs, BLOCK_SIZE + index * sizeof(dir_entry), SEEK_SET);
    fwrite(&entry, sizeof(dir_entry), 1, fs);

    superblock->num_user_blocks -= freed;
    fseek(fs, superblock->directory_start * BLOCK_SIZE, SEEK_SET);
    fwrite(&superblock, sizeof(memefs_superblock_t), 1, fs);

    return 0;
}


//main functions
//getattr is called when the file system is asked for the attributes of a file or directory
static int memefs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    printf("memefs_getattr called for path: %s\n", path);

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        // Root directory attributes
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    const char *filename = path + 1;
    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    if (!find_file_entry(fs, &superblock, filename, &entry, NULL)) {
        fclose(fs);
        return -ENOENT;
    }

    // Set file attributes
    stbuf->st_mode = S_IFREG | entry.file_permissions;
    stbuf->st_nlink = 1;
    stbuf->st_size = entry.file_size;
    stbuf->st_uid = entry.uid;
    stbuf->st_gid = entry.gid;
    /*
    stbuf->st_atime = *((time_t*)entry.atime);
    stbuf->st_mtime = *((time_t*)entry.mtime);
    stbuf->st_ctime = *((time_t*)entry.ctime);
    */
    stbuf->st_atime = bcd_to_time(entry.atime);
    stbuf->st_mtime = bcd_to_time(entry.mtime);
    stbuf->st_ctime = bcd_to_time(entry.ctime);


    fclose(fs);
    return 0;
}

//readdir is called when the file system is asked to list the contents of a directory
static int memefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi,
                          enum fuse_readdir_flags flags) {
    printf("memefs_readdir called for path: %s\n", path);

    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -EIO;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    // iterate over directory entries and add valid entries to the directory listing
    dir_entry entry;
    for (int i = 0; i < MAX_FILES; i++) {
        if (read_dir_entry(fs, &superblock, i, &entry) && entry.file_permissions != 0x0000) {
            filler(buf, entry.filename, NULL, 0, 0);
        }
    }

    fclose(fs);
    return 0;
}

//open is called when the file system is asked to open a file
static int memefs_open(const char *path, struct fuse_file_info *fi) {
    printf("memefs_open called for path: %s\n", path);

    //skips the /
    const char *filename = path + 1;
    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -EIO;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    if (!find_dir_entry(fs, &superblock, filename, &entry)) {
        fclose(fs);
        return -ENOENT;
    }

    fclose(fs);
    return 0;
}

//read is called when the file system is asked to read the contents of a file
static int memefs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("memefs_read called for path: %s\n", path);
    const char *filename = path + 1;
    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -EIO;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    if (!find_dir_entry(fs, &superblock, filename, &entry)) {
        fclose(fs);
        return -ENOENT;
    }
    // Check if the offset is within the file size
    if (offset >= entry.file_size) {
        fclose(fs);
        return 0;
    }

    if (offset + size > entry.file_size) {
        size = entry.file_size - offset;
    }

    if (read_file_data(fs, &superblock, &entry, offset, buf, size) != 0) {
        fclose(fs);
        return -EIO;
    }

    fclose(fs);
    return size;
}

static int memefs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    printf("memefs_create called for path: %s\n", path);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    // Validate the filename
    const char *filename = path + 1;
    if (strlen(filename) > 11 || strpbrk(filename, "!@#$%^&*(){}[]<>?/\\:;\"'")) {
        printf("Invalid filename: %s\n", filename);
        fclose(fs);
        return -EINVAL;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }
    // for duplicates
    dir_entry entry1;

    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        fread(&entry1, sizeof(dir_entry), 1, fs);
        if (strcmp(entry1.filename, filename) == 0) {
            return -EEXIST;
        }
    }

    // Load the FAT
    uint16_t fat[BLOCK_SIZE / sizeof(uint16_t)];
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fread(fat, sizeof(fat), 1, fs);

    // Find a free block
    uint16_t free_block = 0xFFFF;
    for (uint16_t i = superblock.first_user_block; i < superblock.first_user_block + superblock.num_user_blocks; i++) {
        if (fat[i] == 0x0000) {
            free_block = i;
            fat[i] = 0xFFFF;
            break;
        }
    }

    // Error handling for no free blocks
    if (free_block == 0xFFFF) {
        printf("No free blocks available\n");
        fclose(fs);
        return -ENOSPC;
    }

    // Zero out the block
    uint8_t zero_block[BLOCK_SIZE] = {0};
    fseek(fs, free_block * BLOCK_SIZE, SEEK_SET);
    fwrite(zero_block, BLOCK_SIZE, 1, fs);

    // Find a free directory entry
    dir_entry entry;
    int free_entry_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        fread(&entry, sizeof(dir_entry), 1, fs);

        if (entry.file_permissions == 0x0000) {
            free_entry_index = i;
            break;
        }
    }

    if (free_entry_index == -1) {
        printf("No free directory entries available\n");
        fclose(fs);
        return -ENOSPC;
    }

    // Initialize entry
    memset(&entry, 0, sizeof(dir_entry));
    snprintf(entry.filename, sizeof(entry.filename), "%s", filename);
    entry.file_permissions = mode | S_IFREG;
    entry.start_block = free_block;
    entry.file_size = 0;
    entry.uid = getuid();
    entry.gid = getgid();

    time_t current_time = time(NULL);
    time_to_bcd(&current_time, entry.last_write);

    fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (free_entry_index * sizeof(dir_entry)), SEEK_SET);
    fwrite(&entry, sizeof(dir_entry), 1, fs);

    // Update the FAT and superblock
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    superblock.num_user_blocks--;
    fseek(fs, 0, SEEK_SET);
    fwrite(&superblock, sizeof(memefs_superblock_t), 1, fs);

    fclose(fs);

    printf("File created successfully: %s\n", path);
    return 0;
}

static int memefs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("memefs_write called for path: %s, size: %zu, offset: %ld\n", path, size, offset);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    const char *filename = path + 1;
    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    int found = 0;
    int index = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        fread(&entry, sizeof(dir_entry), 1, fs);

        if (strcmp(entry.filename, filename) == 0) {
            found = 1;
            // assigning entry
            index = i;
            break;
        }
    }

    if (!found) {
        fclose(fs);
        return -ENOENT;
    }

    uint16_t fat[BLOCK_SIZE / sizeof(uint16_t)];
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fread(fat, sizeof(fat), 1, fs);
    //check if the offset is within the file size
    size_t end_offset = offset + size;
    size_t current_size = entry.file_size;
    size_t required_blocks = (end_offset + BLOCK_SIZE - 1) / BLOCK_SIZE;
    //extend the file
    if (end_offset > current_size) {
        uint16_t current_block = entry.start_block;
        while (fat[current_block] != 0xFFFF) {
            current_block = fat[current_block];
        }
        //check if there are enough free blocks
        size_t current_blocks = (current_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        size_t additional_blocks = required_blocks - current_blocks;
        for (size_t i = 0; i < additional_blocks; i++) {
            uint16_t free_block = 0xFFFF;
            for (uint16_t j = superblock.first_user_block; j < superblock.first_user_block + superblock.num_user_blocks;
                 j++) {
                if (fat[j] == 0x0000) {
                    free_block = j;
                    fat[j] = 0xFFFF;
                    break;
                }
            }

            if (free_block == 0xFFFF) {
                printf("No free blocks available during write\n");
                fclose(fs);
                return -ENOSPC;
            }

            fat[current_block] = free_block;
            current_block = free_block;
        }
    }

    uint16_t current_block = entry.start_block;
    size_t block_offset = offset / BLOCK_SIZE;
    size_t write_offset = offset % BLOCK_SIZE;
    //move to the block where the write should start
    for (size_t i = 0; i < block_offset; i++) {
        current_block = fat[current_block];
        if (current_block == 0xFFFF) {
            printf("Unexpected end of chain during write\n");
            fclose(fs);
            return -EIO;
        }
    }

    size_t bytes_written = 0;
    size_t remaining = size;
    //write the data
    while (remaining > 0) {
        size_t to_write = remaining > BLOCK_SIZE - write_offset ? BLOCK_SIZE - write_offset : remaining;

        fseek(fs, current_block * BLOCK_SIZE + write_offset, SEEK_SET);
        fwrite(buf + bytes_written, 1, to_write, fs);

        bytes_written += to_write;
        remaining -= to_write;
        write_offset = 0;

        if (remaining > 0) {
            current_block = fat[current_block];
            if (current_block == 0xFFFF) {
                printf("Unexpected end of chain during write\n");
                fclose(fs);
                return -EIO;
            }
        }
    }
    //update the file size if the end offset is greater than the current file size
    if (end_offset > entry.file_size) {
        entry.file_size = end_offset;
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (index) * sizeof(dir_entry), SEEK_SET);
        fwrite(&entry, sizeof(dir_entry), 1, fs);
        //update the last write timestamp
        time_t now = time(NULL);
        struct tm *tm_info = gmtime(&now);
        time_to_bcd(&now, entry.last_write);
    }

    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    fclose(fs);

    printf("Write completed: %zu bytes written\n", bytes_written);
    return bytes_written;
}


static int memefs_unlink(const char *path) {
    printf("memefs_unlink called for path: %s\n", path);
    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    uint16_t fat[BLOCK_SIZE / sizeof(uint16_t)];
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fread(fat, sizeof(fat), 1, fs);

    dir_entry entry;
    const char *filename = path + 1;
    int index;

    if (!find_file_entry(fs, &superblock, filename, &entry, &index)) {
        printf("File not found\n");
        fclose(fs);
        return -ENOENT;
    }

    // this point assuming we reasch where the entry is
    int temp = entry.start_block;
    /*
        tried this,but I think this would not work-krishna
    for (int i =start_fat;i< size_file;i++){

        fat[i]=0x0000 ;
        but its a chain wont work

    } */

    int freed = 0;
    while (temp != 0xFFFF) {
        if (temp < superblock.first_user_block || temp >= superblock.first_user_block + superblock.num_user_blocks) {
            printf("Invalid block number in FAT: %d\n", temp);
            fclose(fs);
            return -EIO;
        }
        int next = fat[temp];
        fat[temp] = 0x0000;
        temp = next;
        freed++;
    }

    //clear entry
    // may need to move this below
    memset(&entry, 0, sizeof(dir_entry));
    fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (index * sizeof(dir_entry)), SEEK_SET);
    // this migh be error
    if (fwrite(&entry, sizeof(dir_entry), 1, fs) != 1) {
        perror("Failed to update directory entry");
        fclose(fs);
        return -EIO;
    }

    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    superblock.num_user_blocks += freed;
    fseek(fs, 0, SEEK_SET);
    fwrite(&superblock, sizeof(memefs_superblock_t), 1, fs);

    fclose(fs);
    return 0;
}

static int memefs_truncate(const char *path, off_t size, struct fuse_file_info *fi) {
    printf("memefs_truncate called for path: %s\n", path);
    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    uint16_t fat[BLOCK_SIZE / sizeof(uint16_t)];
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fread(fat, sizeof(fat), 1, fs);

    dir_entry entry;
    int index;
    if (!find_file_entry(fs, &superblock, path + 1, &entry, &index)) {
        printf("File not found\n");
        fclose(fs);
        return -ENOENT;
    }

    int prev_size = entry.file_size;
    // Shrink or grow the file based on the new size
    if (prev_size > size) {
        return shrink(path, size, fi, fs, fat, &entry, &superblock, index);
    } else if (prev_size < size) {
        return grow(path, size, fi, fs, fat, &entry, &superblock, index);
    }

    // Update file size if no change in size
    entry.file_size = size;
    fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (index * sizeof(dir_entry)), SEEK_SET);
    fwrite(&entry, sizeof(dir_entry), 1, fs);

    fclose(fs);
    return 0;
}

//utimens is called when the file system is asked to update the timestamps of a file
static int memefs_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *fi) {
    printf("memefs_utimens called for path: %s\n", path);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    const char *filename = path + 1;
    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    int entry_index = -1;

    if (!find_file_entry(fs, &superblock, filename, &entry, &entry_index)) {
        fclose(fs);
        return -ENOENT;
    }

    // Update the timestamps
    time_to_bcd(&ts[0].tv_sec, entry.atime);
    time_to_bcd(&ts[1].tv_sec, entry.mtime);
    time_t current = time(NULL);
    time_to_bcd(&current, entry.ctime);
    time_to_bcd(&ts[1].tv_sec, entry.last_write);
    // Write the updated entry back to the directory
    fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (entry_index * sizeof(dir_entry)), SEEK_SET);
    if (fwrite(&entry, sizeof(dir_entry), 1, fs) != 1) {
        fclose(fs);
        return -EIO;
    }

    fclose(fs);
    return 0;
}

// I got an error because of this function, so I added it to the code -jason
static int memefs_flush(const char *path, struct fuse_file_info *fi) {
    (void) path;
    (void) fi;
    return 0;
}

static struct fuse_operations memefs_oper = {
    .getattr = memefs_getattr,
    .readdir = memefs_readdir,
    .open = memefs_open,
    .read = memefs_read,
    .create = memefs_create,
    .unlink = memefs_unlink,
    .write = memefs_write,
    .truncate = memefs_truncate,
    .utimens = memefs_utimens,
    .flush = memefs_flush,
};

int main(int argc, char *argv[]) {
    printf(" \n Radhe Radhe\n");

    printf("Argument count: %d\n", argc);

    const char *mountpoint = argv[1];
    char cwd[MAX_PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return 1;
    }
    snprintf(image_path, sizeof(image_path), "%s/memefs.img", cwd);

    printf("Mount point: %s\n", mountpoint);
    printf("Image file: %s\n", image_path);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return 1;
    }

    printf("Image file opened successfully!\n");
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    int ret = fuse_main(argc, argv, &memefs_oper, NULL);
    printf("fuse_main returned %d\n", ret);
    fclose(fs);
    return ret;
}
