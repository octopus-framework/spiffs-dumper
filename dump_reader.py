import sys
import os
import base64

# Input dump
if len(sys.argv) < 3:
    print("Usage : python dump_reader.py [<dumpfile>|stdin] <outdir>")
    exit(1)

if sys.argv[1] != "stdin":
    dump = open(sys.argv[1], "rb")
else:
    dump = sys.stdin.buffer

outdir = sys.argv[2]

# Detect begin or errors
line = dump.readline()
while len(line) > 0 and line.strip() != b"@@DUMPBEGIN@@":
    if (line.strip() == b"@@SPIFFSERROR@@"):
        print("Error while setting up spiffs ! Did you check the partition tables configuration (https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/partition-tables.html)?")
        exit(1)

    line = dump.readline()

if len(line) == 0:
    print("Could not find beginning marker '@@DUMPBEGIN@@'")
    exit(1)

def readUntilSep(file):
    ret = b""
    while ret[-2:] != b"@@":
        c = file.read(1)

        if len(c) == 0:
            raise RuntimeError("Could not find next @@")
        
        ret += c

    return ret[:-2]

while True:
    try:
        # Get next file path
        file_path = readUntilSep(dump).decode()
        if len(file_path) == 0:
            print("End reached")
            if sys.argv[1] == "stdin":
                print("Press Ctrl+] to end")

            end = readUntilSep(dump)
            if end != b"DUMPEND":
                print("Did not find @@DUMPEND@@ ?")
                exit(1)
            exit(0)

        if file_path[0] == '/':
            file_path = "." + file_path
        file_path = os.path.relpath(os.path.realpath(os.path.join(outdir, file_path)), ".")

    except Exception as e:
        raise RuntimeError("While getting file path") from e

    try:
        beg = readUntilSep(dump)
        if beg != b"FILEBEG":
            raise RuntimeError(f"Did not find FILEBEG, got {beg}")
    except Exception as e:
        raise RuntimeError(f"While getting FILE_BEGIN") from e

    print(f"Writing {file_path}")

    os.makedirs(os.path.dirname(file_path), exist_ok=True)
    f = open(file_path, "wb")
    
    # Read, decode and write each blocks
    while True:
        content = readUntilSep(dump)

        token = readUntilSep(dump)
        if token == b"BLOCKEND":
            f.write(base64.decodebytes(content))
        elif token == b"FILEEND":
            break
        else:
            print(f"Warning : unknown token '{token}' !!")

    f.close()
