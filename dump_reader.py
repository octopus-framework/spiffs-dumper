import sys
import os
import base64

if len(sys.argv) < 3:
    print("Usage : python dump_reader.py [<dumpfile>|stdin] <outdir>")
    exit(1)

if sys.argv[1] != "stdin":
    dump = open(sys.argv[1], "rb")
else:
    dump = sys.stdin.buffer

outdir = sys.argv[2]

line = dump.readline()
while len(line) > 0 and line.strip() != b"@@DUMPBEGIN@@":
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

# def readUntilStop(file):
#     ret = b""
#     c = file.read(1)
#     while c != b'\0':
#         if len(c) == 0:
#             raise RuntimeError("Could not find next \\0 char")
#         
#         ret += c
#         if ret == b'@@DUMPEND@@':
#             print("End of dump detected")
#             if sys.argv[1] == "stdin":
#                 print("Press Ctrl+] to end")
#             exit(0)
#
#         c = file.read(1)
#
#     return ret

# def readUntilPattern(file, pattern):
#     ret = b""
#     c = file.read(1)
#     ret += c
#     while ret[-len(pattern):] != pattern:
#         if len(c) == 0:
#             raise RuntimeError(f"Could not find pattern {pattern}")
#
#         c = file.read(1)
#         ret += c
#
#     return ret[:-len(pattern)]
#
# try:
#     readUntilSep(dump)
# except Exception as err:
#     raise RuntimeError("While getting first @@") from err
#
while True:
    try:
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
    
    while True:
        content = readUntilSep(dump)

        token = readUntilSep(dump)
        if token == b"BLOCKEND":
            f.write(base64.decodebytes(content))
        elif token == b"FILEEND":
            break
        else:
            print(f"Warning : unknown token '{token}' !!")


    # more_content = readUntilPattern(dump, b'@@FILEEND@@')
    # if len(more_content) != 0:
    #     print(f"Warning : {len(more_content)} were detected before the next '@@FILEEND@@' token. There may be some garbage data.")
    #     f.write(more_content)
    
    f.close()
