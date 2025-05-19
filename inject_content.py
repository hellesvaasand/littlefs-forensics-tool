# from littlefs import LittleFS

# IMAGE_FILE = "test2.img"
# BLOCK_SIZE = 4096
# BLOCK_COUNT = 16
# READ_SIZE = 16
# PROG_SIZE = 16
# CACHE_SIZE = 512
# LOOKAHEAD_SIZE = 16

# FILE_TO_DELETE = "/to_be_deleted.txt"

# fs = LittleFS(
#     block_size=BLOCK_SIZE,
#     block_count=BLOCK_COUNT,
#     read_size=READ_SIZE,
#     prog_size=PROG_SIZE,
#     cache_size=CACHE_SIZE,
#     lookahead_size=LOOKAHEAD_SIZE
# )

# with open(IMAGE_FILE, "rb+") as f:
#     fs._block_device = f
#     fs.mount()

#     print("Listing files before deletion:")
#     for name in fs.listdir("/"):
#         print(f" - {name}")

#     print(f"\nDeleting file: {FILE_TO_DELETE}")
#     fs.remove(FILE_TO_DELETE)
#     fs.unmount()
#     print("File successfully deleted.")



# # Usage: python delete_file.py

        
        
# # Usage: python delete_file.py imagename.img /filename.txt



block_index = 5  # pick a block marked as free
block_size = 4096
message = b"This is manually injected deleted file content!\n"

with open("test2.img", "r+b") as f:
    f.seek(block_index * block_size)
    f.write(message.ljust(block_size, b'\xFF'))  # fill the rest with 0xFF
