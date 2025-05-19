from littlefs import LittleFS
import subprocess
import platform

def list_files(image_path, block_size, block_count):
    print("");
    print(f"Listing files in: {image_path}")
    print("")

    # Use correct executable name depending on platform
    list_tool = "littlefs_list.exe" if platform.system() == "Windows" else "./littlefs_list"

    try:
        result = subprocess.run(
            [list_tool, image_path, str(block_size), str(block_count)],
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)

    except FileNotFoundError:
        print(f"[!] Could not find 'littlefs_list'. Did you compile littlefs_list.c?")
    except subprocess.CalledProcessError as e:
        print(f"[!] Error running 'littlefs_list': {e.stderr}")
        

def print_structures(image_path, block_size, block_count, dump_blocks=None):
    print(f"Printing data-structure information from: {image_path}")

    if dump_blocks is None:
        dump_blocks = 8

    # Don't try to dump more blocks than exist
    if dump_blocks > block_count:
        print(f"[!] The filesystem has only {block_count} blocks, but you requested {dump_blocks}.")
        print(f"    Proceeding to dump {block_count} blocks instead.")
        dump_blocks = block_count

    try:
        result = subprocess.run(
            ["./littlefs_struct", image_path, str(block_size), str(block_count), str(dump_blocks)],
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)

    except FileNotFoundError:
        print("[!] 'littlefs_struct' binary not found.")
    except subprocess.CalledProcessError as e:
        print(f"[!] Error: {e.stderr}")


def recover_deleted(image_path, block_size, block_count):
    print(f"Recovering deleted data from: {image_path}")

    try:
        result = subprocess.run(
            ["./littlefs_recover", image_path, str(block_size), str(block_count)],
            capture_output=True,
            text=True,
            check=True
        )
        print(result.stdout)

    except subprocess.CalledProcessError as e:
        print(f"[!] Error recovering deleted data: {e.stderr}")
    except FileNotFoundError:
        print("[!] 'littlefs_recover' tool is missing or not compiled.")

