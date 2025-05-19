#include "lfs.h"
#include "lfs_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define READ_SIZE 16
#define PROG_SIZE 16
#define CACHE_SIZE 512
#define LOOKAHEAD_SIZE 16

uint8_t *image = NULL;
bool *block_usage = NULL;

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

void mark_used_blocks_by_content(int block_size, int block_count) {
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

void print_block_summary(int block_count) {
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

void dump_blocks(int block_size, int block_count) {
    printf("\nDumping first %d blocks:\n", block_count);
    for (int i = 0; i < block_count; i++) {
        printf("  Block %d: ", i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", image[i * block_size + j]);
        }
        printf("...\n");
    }
    printf("\n");
}

void walk_dir(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    lfs_dir_t dir;

    if (lfs_dir_open(lfs, &dir, path) < 0) {
        printf("[!] Failed to open directory: %s\n", path);
        return;
    }

    printf("Filesystem contents:\n");
    printf("DIR: %s\n", path);

    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.name);

        if (info.type == LFS_TYPE_REG) {
            printf("  FILE: %s (Size: %lu)\n", full_path, (unsigned long)info.size);
        } else if (info.type == LFS_TYPE_DIR) {
            walk_dir(lfs, full_path);
        }
    }
    lfs_dir_close(lfs, &dir);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <image_file> <block_size> <block_count> [dump_blocks]\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    int block_size = atoi(argv[2]);
    int block_count = atoi(argv[3]);

    if (block_size <= 0 || block_count <= 0) {
        fprintf(stderr, "[!] Invalid block size or block count.\n");
        return 1;
    }

    // Set default dump count
    int dump_block_count = 8;
    if (argc >= 5) {
        dump_block_count = atoi(argv[4]);
        if (dump_block_count <= 0) {
            fprintf(stderr, "[!] Invalid dump block count.\n");
            return 1;
        }
    }

    if (dump_block_count > block_count) {
        printf("[!] The filesystem has only %d blocks, but %d were requested for dump.\n", block_count, dump_block_count);
        printf("    Proceeding to dump %d blocks instead.\n", block_count);
        dump_block_count = block_count;
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

        .read_size = READ_SIZE,
        .prog_size = PROG_SIZE,
        .block_size = block_size,
        .block_count = block_count,
        .cache_size = CACHE_SIZE,
        .lookahead_size = LOOKAHEAD_SIZE,
        .block_cycles = -1
    };

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

    walk_dir(&lfs, "/");

    mark_used_blocks_by_content(block_size, block_count);
    print_block_summary(block_count);
    dump_blocks(block_size, dump_block_count);

    lfs_unmount(&lfs);
    free(image);
    free(block_usage);
    return 0;
}

// Compile with: gcc littlefs_struct.c lfs.c lfs_util.c -o littlefs_struct
// Usage: ./littlefs_struct <image> <block_size> <block_count>
