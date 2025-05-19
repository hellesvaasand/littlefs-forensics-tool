// // Enhanced littlefs_recover.c: prints all used blocks from files in filesystem
// #include "lfs.h"
// #include "lfs_util.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <string.h>
// #include <stdbool.h>
// #include <ctype.h>

// #define READ_SIZE 16
// #define PROG_SIZE 16
// #define CACHE_SIZE 512
// #define LOOKAHEAD_SIZE 16

// uint8_t *image = NULL;
// bool *block_usage = NULL;
// int block_size;
// int total_blocks;

// int user_read(const struct lfs_config *c, lfs_block_t block,
//               lfs_off_t off, void *buffer, lfs_size_t size) {
//     if (block < total_blocks) {
//         block_usage[block] = true;
//     }
//     memcpy(buffer, &image[block * c->block_size + off], size);
//     return 0;
// }

// int user_prog(const struct lfs_config *c, lfs_block_t block,
//               lfs_off_t off, const void *buffer, lfs_size_t size) {
//     return LFS_ERR_IO;
// }

// int user_erase(const struct lfs_config *c, lfs_block_t block) {
//     return LFS_ERR_IO;
// }

// int user_sync(const struct lfs_config *c) {
//     return 0;
// }

// void walk_and_mark(lfs_t *lfs, const char *path) {
//     struct lfs_info info;
//     lfs_dir_t dir;

//     if (lfs_dir_open(lfs, &dir, path) < 0) {
//         fprintf(stderr, "[!] Failed to open directory: %s\n", path);
//         return;
//     }

//     if (dir.m.pair[0] < total_blocks) {
//         block_usage[dir.m.pair[0]] = true;
//     }

//     while (lfs_dir_read(lfs, &dir, &info) > 0) {
//         if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) continue;

//         char full_path[512];
//         snprintf(full_path, sizeof(full_path), "%s/%s", path, info.name);

//         if (info.type == LFS_TYPE_REG) {
//             lfs_file_t file;
//             if (lfs_file_open(lfs, &file, full_path, LFS_O_RDONLY) == 0) {
//                 uint8_t buffer[64];
//                 while (lfs_file_read(lfs, &file, buffer, sizeof(buffer)) > 0);
//                 lfs_file_close(lfs, &file);
//             }
//         } else if (info.type == LFS_TYPE_DIR) {
//             walk_and_mark(lfs, full_path);
//         }
//     }

//     lfs_dir_close(lfs, &dir);
// }

// void dump_block_data(int block_index, const uint8_t *block_data, int block_size) {
//     printf("\nOrphaned block %d:\n", block_index);

//     bool printable = true;
//     for (int i = 0; i < block_size; i++) {
//         if (!isprint(block_data[i]) && block_data[i] != '\n' && block_data[i] != '\r') {
//             printable = false;
//             break;
//         }
//     }

//     if (printable) {
//         printf("ASCII content:\n");
//         fwrite(block_data, 1, block_size, stdout);
//         printf("\n");
//     } else {
//         printf("Hex dump (first 64 bytes):\n");
//         for (int i = 0; i < 64 && i < block_size; i++) {
//             printf("%02X ", block_data[i]);
//         }
//         printf("...\n");
//     }
// }

// int main(int argc, char **argv) {
//     if (argc < 4) {
//         fprintf(stderr, "Usage: %s <image_file> <block_size> <block_count>\n", argv[0]);
//         return 1;
//     }

//     const char *image_path = argv[1];
//     block_size = atoi(argv[2]);
//     total_blocks = atoi(argv[3]);

//     FILE *f = fopen(image_path, "rb");
//     if (!f) {
//         fprintf(stderr, "[!] Failed to open image file: %s\n", image_path);
//         return 1;
//     }

//     image = malloc(block_size * total_blocks);
//     fread(image, 1, block_size * total_blocks, f);
//     fclose(f);

//     block_usage = calloc(total_blocks, sizeof(bool));

//     struct lfs_config cfg = {
//         .read  = user_read,
//         .prog  = user_prog,
//         .erase = user_erase,
//         .sync  = user_sync,

//         .read_size = READ_SIZE,
//         .prog_size = PROG_SIZE,
//         .block_size = block_size,
//         .block_count = total_blocks,
//         .cache_size = CACHE_SIZE,
//         .lookahead_size = LOOKAHEAD_SIZE,
//         .block_cycles = -1
//     };

//     lfs_t lfs;
//     if (lfs_mount(&lfs, &cfg) == 0) {
//         walk_and_mark(&lfs, "/");
//         lfs_unmount(&lfs);
//     } else {
//         fprintf(stderr, "[!] Failed to mount image.\n");
//     }

//     printf("\nFiles in the filesystem use the following blocks:\n");
//     for (int i = 0; i < total_blocks; i++) {
//         if (block_usage[i]) {
//             printf("%d ", i);
//         }
//     }
//     printf("\n");

//     printf("\nOrphaned Block Scan:\n");
//     int orphaned = 0;
//     for (int i = 0; i < total_blocks; i++) {
//         if (block_usage[i]) continue;

//         bool blank = true;
//         for (int j = 0; j < block_size; j++) {
//             if (image[i * block_size + j] != 0xFF) {
//                 blank = false;
//                 orphaned++;
//                 break;
//             }
//         }
//         if (!blank) {
//             dump_block_data(i, &image[i * block_size], block_size);
//         }
        
//     }
//     if (orphaned == 0) {
//             printf("There are no orphaned blocks.\n");
//         }

//     free(image);
//     free(block_usage);
//     return 0;
// }



// Enhanced littlefs_recover.c: prints all used blocks and saves orphaned blocks to files
#include "lfs.h"
#include "lfs_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define READ_SIZE 16
#define PROG_SIZE 16
#define CACHE_SIZE 512
#define LOOKAHEAD_SIZE 16

uint8_t *image = NULL;
bool *block_usage = NULL;
int block_size;
int total_blocks;

int user_read(const struct lfs_config *c, lfs_block_t block,
              lfs_off_t off, void *buffer, lfs_size_t size) {
    if (block < total_blocks) {
        block_usage[block] = true;
    }
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

void walk_and_mark(lfs_t *lfs, const char *path) {
    struct lfs_info info;
    lfs_dir_t dir;

    if (lfs_dir_open(lfs, &dir, path) < 0) {
        fprintf(stderr, "[!] Failed to open directory: %s\n", path);
        return;
    }

    if (dir.m.pair[0] < total_blocks) {
        block_usage[dir.m.pair[0]] = true;
    }

    while (lfs_dir_read(lfs, &dir, &info) > 0) {
        if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.name);

        if (info.type == LFS_TYPE_REG) {
            lfs_file_t file;
            if (lfs_file_open(lfs, &file, full_path, LFS_O_RDONLY) == 0) {
                uint8_t buffer[64];
                while (lfs_file_read(lfs, &file, buffer, sizeof(buffer)) > 0);
                lfs_file_close(lfs, &file);
            }
        } else if (info.type == LFS_TYPE_DIR) {
            walk_and_mark(lfs, full_path);
        }
    }

    lfs_dir_close(lfs, &dir);
}

void dump_block_to_file(int block_index, const uint8_t *block_data, int block_size) {
    char filename[64];
    snprintf(filename, sizeof(filename), "recovered_blocks/block_%d.bin", block_index);

    FILE *f = fopen(filename, "wb");
    if (f) {
        fwrite(block_data, 1, block_size, f);
        fclose(f);
        printf("\nSaved block %d to %s\n", block_index, filename);
    } else {
        fprintf(stderr, "[!] Failed to write %s\n", filename);
    }
}

void dump_block_data(int block_index, const uint8_t *block_data, int block_size) {
    printf("\nOrphaned block %d:\n", block_index);

    bool printable = true;
    for (int i = 0; i < block_size; i++) {
        if (!isprint(block_data[i]) && block_data[i] != '\n' && block_data[i] != '\r') {
            printable = false;
            break;
        }
    }

    if (printable) {
        printf("ASCII content:\n");
        fwrite(block_data, 1, block_size, stdout);
        printf("\n");
    } else {
        printf("Hex dump (first 64 bytes):\n");
        for (int i = 0; i < 64 && i < block_size; i++) {
            printf("%02X ", block_data[i]);
        }
        printf("...\n");
    }

    dump_block_to_file(block_index, block_data, block_size);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <image_file> <block_size> <block_count>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    block_size = atoi(argv[2]);
    total_blocks = atoi(argv[3]);

    FILE *f = fopen(image_path, "rb");
    if (!f) {
        fprintf(stderr, "[!] Failed to open image file: %s\n", image_path);
        return 1;
    }

    image = malloc(block_size * total_blocks);
    fread(image, 1, block_size * total_blocks, f);
    fclose(f);

    block_usage = calloc(total_blocks, sizeof(bool));

    struct lfs_config cfg = {
        .read  = user_read,
        .prog  = user_prog,
        .erase = user_erase,
        .sync  = user_sync,

        .read_size = READ_SIZE,
        .prog_size = PROG_SIZE,
        .block_size = block_size,
        .block_count = total_blocks,
        .cache_size = CACHE_SIZE,
        .lookahead_size = LOOKAHEAD_SIZE,
        .block_cycles = -1
    };

    mkdir("recovered_blocks", 0755);

    lfs_t lfs;
    if (lfs_mount(&lfs, &cfg) == 0) {
        walk_and_mark(&lfs, "/");
        lfs_unmount(&lfs);
    } else {
        fprintf(stderr, "[!] Failed to mount image.\n");
    }

    printf("\nThe files in the filesystem use the following blocks:\n");
    for (int i = 0; i < total_blocks; i++) {
        if (block_usage[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");

    printf("\nOrphaned Block Scan:\n");
    for (int i = 0; i < total_blocks; i++) {
        if (block_usage[i]) continue;

        bool blank = true;
        for (int j = 0; j < block_size; j++) {
            if (image[i * block_size + j] != 0xFF) {
                blank = false;
                break;
            }
        }
        if (!blank) {
            dump_block_data(i, &image[i * block_size], block_size);
        }
    }

    free(image);
    free(block_usage);
    return 0;
}
