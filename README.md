# Spiffs dumper

Plug and play tool to dump the flash content stored with SPIFFS on ESP-32.

## Usage

This tool needs *esp-idf* and *Python* to be installed. Any version of Python should work, no external package is required.

Be warned that this tool will upload a program into the connected esp32 and erase any existing. You will need to reupload any pre-existing program afterwards.

### Setup partition

- Copy your csv file containing the partition table (usually `partitions.csv`). Set `CONFIG_PARTITION_TABLE_CUSTOM` to `y` using menu config. You can also do it manually in the `sdkconfig` file :

```bash
#
# Partition Table
#
# CONFIG_PARTITION_TABLE_SINGLE_APP is not set
# CONFIG_PARTITION_TABLE_SINGLE_APP_LARGE is not set
# CONFIG_PARTITION_TABLE_TWO_OTA is not set
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
CONFIG_PARTITION_TABLE_OFFSET=0x8000
CONFIG_PARTITION_TABLE_MD5=y
# end of Partition Table
```

- In  (`CMakeLists.txt`)[CMakeLists.txt], set the name of the partition you want to dump (default is `"data"`) :
```cmake
add_compile_definitions(PARITION_NAME="data")
```
Only one partition at a time can be dumped right now. This is planned to change in the future (react on the corresponding issue if this is a feature you would need). 

### Launch

Make sure that esp-idf's `export.sh` is sourced and source the `run` file :

```bash
> source /path/to/export.sh
> source run
```

This will dump the content of the spiffs partition to ./out.


## Troubleshooting

The `dump_reader.py` will consume the ESP32 output, thus you wont be able to see errors that could be printed by the ESP32. If this is the case, you may want to troubleshoot by launching `idf.py monitor` instead of `source run` to check the eventual errors.

### Spiffs setup error

This error might come from :
- The `PARTITION_NAME` macro definition in the CMake file is incorrect
- The partition table from the stored data is not the same as the partition table of this project.

See the (Setup-partition)[#Setup-partition] section to fix this issie.

## How does it work

The tool basically uploads a program that will go through every file in the partition, prints their path and content, encoded in base64. On another hand, the dump_reader.py script will read the output, decode the base64 and rewrite the files into the ./out folder.
For convenience sake, the files' contents are separated in blocks that are separately encoded in base64.
The program uses the chars `@@` as separators in order to pass instructions to the python script.

## Contribution

Any help would be appreciated, you may open issues and pull requests.
