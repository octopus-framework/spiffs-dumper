#include "sys/dirent.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <filesystem>

#include <dirent.h>

#include <esp_spiffs.h>
#include <mbedtls/base64.h>

bool init_spiffs() {
    esp_vfs_spiffs_conf_t spiffs_conf = {
      .base_path = "/data",
      .partition_label = "data",
      .max_files = 30,
      .format_if_mount_failed = true
    };

    esp_err_t err = esp_vfs_spiffs_register(&spiffs_conf);

    if (err != ESP_OK) {
        printf("error on register : %d; %s\n", err, esp_err_to_name(err));
        return false;
    }

    err = esp_spiffs_check(spiffs_conf.partition_label);

    if (err != ESP_OK) {
        printf("error on check : %d\n", err);
        return false;
    }

    return true;
}

extern "C" void dump_spiffs() {
    std::cout << "salut from c++" << std::endl;

    // if (!init_spiffs()) {
    //     std::cout << "Could not init spiffs. Ending." << std::endl;
    //     return;
    // }
    //
    // DIR* dir = opendir("/data");
    // if (dir != nullptr) {
    //     while (true) {
    //         struct dirent* de = readdir(dir);
    //         if (!de) {
    //             break;
    //         }
    //         printf("Found file: %s\n", de->d_name);
    //     }
    // } else {
    //     std::cout << "could not open dir" << std::endl;
    // }
    //
    // closedir(dir);
    // 
    // return;
 //
 //    const std::filesystem::path sandbox{"/data/sandbox"};
 //    std::ofstream{sandbox/"test"/"file1.txt"};
 //    std::ofstream{sandbox/"test2"/"test3"/"file2.txt"};
 // 
 //    std::cout << "directory_iterator:\n";
 //    // directory_iterator can be iterated using a range-for loop
 //    for (auto const& dir_entry : std::filesystem::directory_iterator{sandbox}) 
 //        std::cout << dir_entry.path() << '\n';
 // 
 //    std::cout << "\ndirectory_iterator as a range:\n";
 //    // directory_iterator behaves as a range in other ways, too
 //    std::ranges::for_each(
 //        std::filesystem::directory_iterator{sandbox},
 //        [](const auto& dir_entry) { std::cout << dir_entry << '\n'; });
 // 
 //    std::cout << "\nrecursive_directory_iterator:\n";
 //    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator{sandbox}) 
 //        std::cout << dir_entry << '\n';
 
    // delete the sandbox dir and all contents within it, including subdirs
}

#define BLOCK_SIZE 0x100
#define B64_BLOCK_SIZE 0x160

extern "C" void dump_file(const char* dir, const char* path) {
    std::ifstream f;
    char block[BLOCK_SIZE];
    char b64Block[B64_BLOCK_SIZE];

    std::filesystem::path fullPath = dir;
    fullPath /= path;

    f.open(fullPath, std::ios::binary | std::ios::in);
    // std::cout << fullPath.c_str() << '\0' << size << '\0';
    std::cout << fullPath.c_str();
    std::cout << "@@FILEBEG@@";
    f.seekg(0);
    while (!f.eof() && !f.fail()) {
        f.read(block, BLOCK_SIZE);
        auto count = f.gcount();
        if (count > 0) {
            size_t b64Count;
            mbedtls_base64_encode((unsigned char *) b64Block, B64_BLOCK_SIZE, &b64Count, (unsigned char *) block, count);

            std::cout << std::string(b64Block, b64Count);
            std::cout << "@@BLOCKEND@@";
        }
    }

    std::cout << "@@FILEEND@@";
}
