# LitleFS Forensics Tool

A command-line forensic tool for analyzing and inspecting LittleFS filesystem images. Useful for embedded system development, debugging, and recovery tasks. 

This README explains how to set up and use the tool. 

## Prerequisites

- Python 3.7 or newer
- Git (optional, for cloning the repository)
- A LittleFS image file (e.g., `image.img`)
- A C compiler (e.g., gcc)


## Acknowledgments

This project uses portions of [LittleFS](https://github.com/littlefs-project/littlefs) by Arm Limited,  
which is licensed under the BSD-3-Clause License. See `lfs.c`, `lfs.h`, `lfs_util.c`, and `lfs_util.h`.

BSD-3-Clause License © 2017 Arm Limited.

For license details, see `littlefs_LICENSE`.


## Installation

### 1. Clone the repository

You can either clone this project or download it as a ZIP: https://github.com/hellesvaasand/littlefs-forensics-tool

```bash
git clone https://github.com/hellesvaasand/littlefs-forensics-tool.git
cd littlefs-forensics-tool
```

### 2. Create a virtual environment

I recommend using a virtual environment to isolate dependencies. 

#### On Windows (PowerShell):
```bash
python -m venv venv
.\venv\Scripts\Activate.ps1
```

#### On Windows (CMD):
```bash
python -m venv venv
venv\Scripts\activate.bat
```

#### On Linux/MacOS:
```bash
python3 -m venv venv
source venv/bin/activate
```

Make sure you use the correct interpreter (the one for the virtual environment). 

### 3. Install dependencies 

Navigate to the littlefs-forensics-tool folder and install dependencies:

```bash
pip install -r requirements.txt
```

## Usage

Once the tool is installed, the tool can be run from the command line.

### LittleFS Images
The tool can be used with existing images of a LittleFS filesystem (realistic usage for the tool), but the tool can also be used to create LittleFS images. This is useful for testing cases.

#### Using Existing Images
LittleFS images do **not** contain internal metadata about `block size`, `block count`, `read size` or `prog size`. These parameters must be provided manually, and they **must exactly match** the original configuration used when the image was created.

If these values are incorrect, the tool will fail to mount or interpret the filesystem properly. Thus, to be able to use existing images, the user must know the values of `block size` and `block count`.

Sources you can use to find this info:
- Embedded firmware source code
- Device datasheets
- Image generation scripts or documentation

#### How to Create Images
The example below shows how to create a LittleFS image containing three files where one of them is in a nested directory. 

```bash
mkdir test_data

echo "Hello from file 1" > test_data/file1.txt
echo "Hey from file 2" > test_data/file2.txt
mkdir test_data/nested
echo "Hi from file 3! Inside nested directory" > test_data/nested/file3.txt

littlefs-python create test_data test.img --block-size 4096 --block-count 16
```
 
### CLI Features
There are three possible features: 

| Feature      | Command Flag | Description                          |
|--------------|--------------|--------------------------------------|
| List files   | `--list`     | Recursively print files/directories  |
| Print layout | `--struct`   | Show filesystem structure and blocks |
| Recover files| `--recover`  | Try to recover deleted files         |

#### Default Values

The CLI commands for the features are given below. As previously stated, it is vital to provide the block_size, block_count read_size and prog_size of the filesystem image. If these are not provided, the tool will use some default values, which may or may not be correct. If not correct, the tool will not be able to mount the image, at least not correctly. 

| Option       | Default Value |
|--------------|---------------|
| block_size   | 16            |
| block_count  | 4096          |
| read_size    | 16            |
| prog_size    | 16            |


#### Compiling Files
Before using the tool, make sure the compiled binaries (`littlefs_list`, `littlefs_struct`, `littlefs_recover`) are either in the root project directory or accessible from your system's PATH, since the Python CLI uses them as subprocesses.


```bash
gcc littlefs_list.c lfs.c lfs_util.c -o littlefs_list
gcc littlefs_struct.c lfs.c lfs_util.c -o littlefs_struct
gcc littlefs_recover.c lfs.c lfs_util.c -o littlefs_recover
```


#### --list
The --list feature lists files and directories. 

```bash
python3 main.py <image_file> --list [--block-size <block_size>] [--block-count <block_count>] [--read-size <read_size>] [--prog-size <prog_size>]
```

#### --struct
The --struct feature allows the program to print: superblock information, filesystem configuration, files and directories, block usage summary, and raw hex dump of blocks. With the --dump-blocks option, the user can specify the number of blocks to be dumped (dump_size). Without the option, the default is 8 blocks. 

```bash
python3 main.py <image_file> --struct [--block-size <block_size>] [--block-count <block_count>] [--read-size <read_size>] [--prog-size <prog_size>] [--dump-blocks <dump_size>]
```

#### --recover
The --recover feature tries to recover deleted files, if possible. 

```bash
python3 main.py <image_file> --recover [--block-size <block_size>] [--block-count <block_count>] [--read-size <read_size>] [--prog-size <prog_size>]
```

### Troubleshooting
- `Failed to mount filesystem`: Double-check that your block size and block count are correct.
- `Segmentation fault`: Check that your image file is valid and matches the provided parameters.