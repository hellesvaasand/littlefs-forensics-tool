#include "lfs.h"
#include "lfs_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define CACHE_SIZE 512
#define LOOKAHEAD_SIZE 16

uint8_t *image = NULL;
int block_size = 4096;
int block_count = 16;
int read_size = 16; 
int prog_size = 16;
int dump_size = 8;
bool *block_usage = NULL;
typedef uint32_t lfs_tag_t;

int user_read(const struct lfs_config *c, lfs_block_t block,
              lfs_off_t off, void *buffer, lfs_size_t size) {
    memcpy(buffer, &image[block * c->block_size + off], size);
    return 0;
}

int user_prog(const struct lfs_config *c, lfs_block_t block,
              lfs_off_t off, const void *buffer, lfs_size_t size) {
    return LFS_ERR_IO;
}

int user_erase(const struct lfs_config *c, lfs_block_t block) {
    return LFS_ERR_IO;
}

int user_sync(const struct lfs_config *c) {
    return 0;
}

void mark_used_blocks(int block_size, int block_count) {
    for (int i = 0; i < block_count; i++) {
        bool is_blank = true;
        for (int j = 0; j < 16; j++) {
            if (image[i * block_size + j] != 0xFF) {
                is_blank = false;
                break;
            }
        }
        block_usage[i] = !is_blank;
    }
}

void print_block_usage(int block_count) {
    printf("\nBlock Usage Summary:\n");
    printf("  Used blocks: ");
    for (int i = 0; i < block_count; i++) {
        if (block_usage[i]) {
            printf("%d ", i);
        }
    }
    printf("\n  Free blocks: ");
    for (int i = 0; i < block_count; i++) {
        if (!block_usage[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");
}


void decode_block_header(uint8_t *block_data) {
    lfs_tag_t tag = *(lfs_tag_t *)block_data;
    uint8_t type = tag & 0x3F;

    switch (type) {
        case 0x01:
            printf("File entry");
            break;
        case 0x02:
            printf("Directory entry");
            break;
        case 0x05:
        case 0x06:
            printf("Superblock");
            break;
        default:
            printf("Unknown type");
            break;
    }
}


void dump_blocks(int block_size, int block_count) {
    printf("\nDumping first %d blocks:\n", block_count);

    for (int i = 0; i < block_count; i++) {
        printf("  Block %d: ", i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", image[i * block_size + j]);
        }
        printf("  -->  ");
        decode_block_header(&image[i * block_size]);
        
        printf("...\n");
    }
    printf("\n");
}


void traverse_directory(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    lfs_dir_t dir;

    if (lfs_dir_open(lfs, &dir, path) < 0) {
        printf("[!] Failed to open directory: %s\n", path);
        return;
    }

    printf("Directory: %s\n", path);

    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.name);

        if (info.type == LFS_TYPE_REG) {
            printf("  FILE: %s (Size: %lu)\n", full_path, (unsigned long)info.size);
        } else if (info.type == LFS_TYPE_DIR) {
            printf("  DIR: %s\n", full_path);
            traverse_directory(lfs, full_path);
        }
    }
    lfs_dir_close(lfs, &dir);
}

// Utility to read 32-bit little-endian values safely
uint32_t read_le32(const uint8_t *ptr) {
    return ((uint32_t)ptr[0]) |
           ((uint32_t)ptr[1] << 8) |
           ((uint32_t)ptr[2] << 16) |
           ((uint32_t)ptr[3] << 24);
}

void print_superblock_info(uint8_t *image, int block_size) {
    printf("Superblock information:\n");

    for (int block = 0; block <= 1; block++) {
        const uint8_t *block_data = image + block * block_size;
        uint32_t raw_tag = read_le32(block_data);
        uint8_t tag_type = raw_tag & 0x3F;

        if (tag_type == 0x05 || tag_type == 0x06) {
            printf("  Superblock tag detected in block %d\n", block);
            printf("  Raw tag: 0x%08X\n", raw_tag);
            printf("  Tag type: 0x%02X (Superblock)\n", tag_type);
            return;
        }
    }

    printf("  [!] No valid superblock tag found in block 0 or 1.\n");
}


int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <image_file> [block_size] [block_count] [read_size] [prog_size] [dump_blocks]\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    if (argc >= 3) {
        block_size = atoi(argv[2]);
    }
    if (argc >= 4) {
        block_count = atoi(argv[3]);
    }
    if (argc >= 5) {
        read_size = atoi(argv[4]);
    }
    if (argc >= 6) {
        prog_size = atoi(argv[5]);
    }

    if (argc >= 7) {
        dump_size = atoi(argv[6]);
        if (dump_size <= 0) {
            fprintf(stderr, "[!] Invalid dump block count.\n");
            return 1;
        }
    }

    if (block_size <= 0 || block_count <= 0) {
        fprintf(stderr, "[!] Invalid block size or block count.\n");
        return 1;
    }


    if (dump_size > block_count) {
        printf("[!] The filesystem has only %d blocks, but %d were requested for dump.\n", block_count, dump_size);
        printf("    Proceeding to dump %d blocks instead.\n", block_count);
        dump_size = block_count;
    }

    FILE *f = fopen(image_path, "rb");
    if (!f) {
        fprintf(stderr, "[!] Failed to open image file: %s\n", image_path);
        return 1;
    }

    size_t image_size = block_size * block_count;
    image = malloc(image_size);
    fread(image, 1, image_size, f);
    fclose(f);

    block_usage = malloc(sizeof(bool) * block_count);
    memset(block_usage, 0, sizeof(bool) * block_count);

    struct lfs_config cfg = {
        .read  = user_read,
        .prog  = user_prog,
        .erase = user_erase,
        .sync  = user_sync,

        .read_size = read_size,
        .prog_size = prog_size,
        .block_size = block_size,
        .block_count = block_count,
        .cache_size = CACHE_SIZE,
        .lookahead_size = LOOKAHEAD_SIZE,
        .block_cycles = -1
    };

    printf("\n");
    print_superblock_info(image, block_size);

    printf("\n");
    printf("Filesystem configuration:\n");
    printf("  Block size: %d\n", cfg.block_size);
    printf("  Block count: %d\n", cfg.block_count);
    printf("  Read size: %d\n", cfg.read_size);
    printf("  Prog size: %d\n", cfg.prog_size);
    printf("\n");

    lfs_t lfs;
    if (lfs_mount(&lfs, &cfg) != 0) {
        fprintf(stderr, "[!] Failed to mount filesystem\n");
        free(image);
        free(block_usage);
        return 1;
    }

    traverse_directory(&lfs, "/");

    mark_used_blocks(block_size, block_count);
    print_block_usage(block_count);
    dump_blocks(block_size, dump_size);

    lfs_unmount(&lfs);
    free(image);
    free(block_usage);
    return 0;
}

// // Compile with: gcc littlefs_struct.c lfs.c lfs_util.c -o littlefs_struct
// // Usage: ./littlefs_struct <image> <block_size> <block_count>