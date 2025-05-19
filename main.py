# main.py
import argparse
from fs_analyzer import list_files, print_structures, recover_deleted


def main():
    parser = argparse.ArgumentParser(description="LittleFS Forensics Tool")
    parser.add_argument("image", help="Path to the LittleFS image file")
    parser.add_argument("--list", action="store_true", help="List files and directories")
    parser.add_argument("--struct", action="store_true", help="Print filesystem structures")
    parser.add_argument("--block-size", type=int, default=4096, help="Block size used in the image (default: 4096)")
    parser.add_argument("--block-count", type=int, default=16, help="Number of blocks in the image (default: 16)")
    parser.add_argument("--dump-blocks", type=int, default=None, help="Specify number of blocks to dump in --struct mode (default: 8, specify fewer if filesystem is smaller)")
    parser.add_argument("--recover", action="store_true", help="Attempt to recover deleted files")


    args = parser.parse_args()

    if args.list:
        list_files(args.image, args.block_size, args.block_count)

    if args.struct:
        print_structures(args.image, args.block_size, args.block_count, args.dump_blocks)

    if args.recover:
        recover_deleted(args.image, args.block_size, args.block_count)


if __name__ == "__main__":
    main()