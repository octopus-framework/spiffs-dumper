#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <esp_spiffs.h>


bool init_spiffs() {
    esp_vfs_spiffs_conf_t spiffs_conf = {
      .base_path = "/data",
      .partition_label = "data",
      .max_files = 5,
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

extern void dump_spiffs();
extern void dump_file(const char* dir, const char* path);

void listdir(const char *name, int indent)
{
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
            printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2);
        } else {
            // File here
            // printf("%*s- %s\n", indent, "", entry->d_name);
            dump_file("/data/", entry->d_name); 
        }
    }
    closedir(dir);
}

void app_main(void)
{
    if (!init_spiffs()) {
        printf("Could not init spiffs\n");
    }

    dump_spiffs();
    
    printf("@@DUMPBEGIN@@\n");
    listdir("/data/", 0);
    printf("@@DUMPEND@@\n");

    return;
}
