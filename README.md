#  Setup & Usage

This README explains how to set up and use the CLI forensics tool for the LittleFS filesystem.

## Prerequisites

- Python 3.7 or newer
- Git (optional, for cloning the repository)
- A LittleFS image file (e.g., `image.img`)

---

## Installation

### 1. Clone the repository

You can either clone this project or download it as a ZIP: LINK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

```bash
git clone https://github.com/your-username/littlefs-forensics-tool.git
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

#### How to Use Existing Images 
Using existing may be more challenging, since the tool requires some metadata to work; block size and block count (number of blocks). These values are not stored in the image itself and must match exactly for the tools to work properly. Hence, it is required that the user has some knowledge of the provided filesystem. This may not be the best solution, but the best I could do for this project. 

These values may be available from device firmware or datasheets, embedded firmware source code, or from image creation scripts used in the original development. 

When the block size and block count is known, the tool can be used as intended. 

### CLI Commands

There are three possible features: 
- Listing files and directories: --list
- Printing data-structure information: --struct
- Recovering deleted files: --recover

The commands for the features are given below. As previously stated, it is vital to provide the block size and block count of the filesystem image. 

#### Compiling Files
Before using the feature, the files must be compiled.

```bash
gcc littlefs_list.c lfs.c lfs_util.c -o littlefs_list
gcc littlefs_struct.c lfs.c lfs_util.c -o littlefs_struct
gcc littlefs_recover.c lfs.c lfs_util.c -o littlefs_recover
```


#### --list
The --list feature lists files and directories. 

```bash
python main.py <image_file> --list --block-size <block_size> --block-count <block_count>
```

#### --struct
The --struct feature allows the program to print: filesystem configuration, files and directories, block usage summary, and raw hex dump of blocks. With the --dump-blocks option, the user can specify the number of blocks to be dumped (dump_size). Without the option, the default is 8 blocks. 

```bash
python main.py <image_file> --struct --block-size <block_size> --block-count <block_count>
python main.py <image_file> --struct --block-size <block_size> --block-count <block_count> --dump-blocks <dump_size>
```

#### --recover
The --recover feature tries to recover deleted files, if possible. 

```bash
python main.py <image_file> --recover --block-size <block_size> --block-count <block_count>
```
