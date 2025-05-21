#include "lfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CACHE_SIZE 512
#define LOOKAHEAD_SIZE 16

uint8_t *image = NULL;
int block_size = 4096;
int block_count = 16;
int read_size = 16;
int prog_size = 16;

int user_read(const struct lfs_config *c, lfs_block_t block,
              lfs_off_t off, void *buffer, lfs_size_t size) {
    memcpy(buffer, &image[block * c->block_size + off], size);
    return 0;
}

int user_prog(const struct lfs_config *c, lfs_block_t block,
              lfs_off_t off, const void *buffer, lfs_size_t size) {
    return LFS_ERR_IO; // Read-only mode
}

int user_erase(const struct lfs_config *c, lfs_block_t block) {
    return LFS_ERR_IO; // Read-only mode
}

int user_sync(const struct lfs_config *c) {
    return 0;
}

void walk_dir(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    lfs_dir_t dir;

    if (lfs_dir_open(lfs, &dir, path) < 0) {
        printf("[!] Failed to open directory: %s\n", path);
        return;
    }

    printf("DIR: %s\n", path);
    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.name);


        if (info.type == LFS_TYPE_REG) {
            printf("   FILE: %s\n", full_path);
        } else if (info.type == LFS_TYPE_DIR) {
            walk_dir(lfs, full_path);
        }
    }

    lfs_dir_close(lfs, &dir);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <image_file> [block_size] [block_count] [read_size] [prog_size]\n", argv[0]);

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

    if (block_size <= 0 || block_count <= 0) {
        fprintf(stderr, "[!] Invalid block size or block count.\n");
        return 1;
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

    lfs_t lfs;
    if (lfs_mount(&lfs, &cfg) != 0) {
        fprintf(stderr, "[!] Failed to mount filesystem\n");
        free(image);
        return 1;
    }

    walk_dir(&lfs, "/");

    lfs_unmount(&lfs);
    free(image);
    return 0;
}
