#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <esp_spiffs.h>
#include <mbedtls/base64.h>

#ifndef PARITION_NAME
#define PARITION_NAME "data"
#endif

bool init_spiffs() {
    esp_vfs_spiffs_conf_t spiffs_conf = {
      .base_path = "/" PARITION_NAME,
      .partition_label = PARITION_NAME,
      .max_files = 5,
      .format_if_mount_failed = false
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

#define BLOCK_SIZE 0x100
#define B64_BLOCK_SIZE 0x160

void dump_file(const char* path) {
    char block[BLOCK_SIZE];
    char b64Block[B64_BLOCK_SIZE];

    FILE *fptr;
    fptr = fopen(path, "r");
    if (fptr == NULL)
        return;

    printf("%s", path);
    printf("@@FILEBEG@@");
    while (!feof(fptr) && !ferror(fptr)) {
        size_t count = fread(block, sizeof(char), BLOCK_SIZE, fptr);

        if (count > 0) {
            size_t b64Count;
            mbedtls_base64_encode((unsigned char *) b64Block, B64_BLOCK_SIZE, &b64Count, (unsigned char *) block, count);

            printf("%*s", b64Count, b64Block);
            printf("@@BLOCKEND@@");
        }
    }

    printf("@@FILEEND@@");
}

void listdir(const char *name) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            listdir(path);
        } else {
            char path[1024];
            snprintf(path, sizeof(path), "/" PARITION_NAME "/%s", entry->d_name);
            dump_file(path);
        }
    }
    closedir(dir);
}

void app_main(void) {
    if (!init_spiffs()) {
        printf("Could not init spiffs. Did you check the partition configuration ? Partition name is set to '" PARITION_NAME "'\n");
        printf("@@SPIFFSERROR@@\n");
        return;
    }

    printf("@@DUMPBEGIN@@\n");
    listdir("/" PARITION_NAME "/");
    printf("@@DUMPEND@@\n");

    return;
}
