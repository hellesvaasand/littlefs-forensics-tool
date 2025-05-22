
block_index = 5  # Pick a block marked as free
block_size = 4096 # Must be adapted to the correct block size of the image you are using
message = b"This is manually injected deleted file content!\n" # Content to add 

with open("test2.img", "r+b") as f:
    f.seek(block_index * block_size)
    f.write(message.ljust(block_size, b'\xFF'))
    

# Usage: python3 inject_content.py