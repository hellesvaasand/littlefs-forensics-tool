
block_index = 5  # pick a block marked as free
block_size = 4096
message = b"This is manually injected deleted file content!\n"

with open("test2.img", "r+b") as f:
    f.seek(block_index * block_size)
    f.write(message.ljust(block_size, b'\xFF'))  # fill the rest with 0xFF
