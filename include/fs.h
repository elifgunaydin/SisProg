#ifndef FS_H
#define FS_H

#include <ctime>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>

#define DISK_NAME "disk.sim"
#define DISK_SIZE (1024 * 1024)
#define METADATA_SIZE 4096
#define MAX_FILES 48
#define FILENAME_MAX_LEN 32
#define BLOCK_SIZE 512

struct FileEntry {
    char filename[FILENAME_MAX_LEN];
    int size;
    int start_block;
    uint32_t created;
    bool used;
    char padding[3];
};

struct Metadata {
    int file_count;
    FileEntry entries[MAX_FILES];
    char reserved[METADATA_SIZE - sizeof(int) - sizeof(FileEntry) * MAX_FILES];
};

// File system interface
bool fs_format();
bool fs_load_metadata(Metadata &metadata);
bool fs_save_metadata(const Metadata &metadata);
bool fs_create(const std::string &filename);
bool fs_delete(const std::string &filename);
bool fs_write(const std::string &filename, const char *data, int size);
bool fs_read(const std::string &filename, int offset, int size, char *buffer);
void fs_ls();
bool fs_rename(const std::string &old_name, const std::string &new_name);
bool fs_exists(const std::string &filename);
int fs_size(const std::string &filename);
bool fs_append(const std::string &filename, const char *data, int size);
bool fs_truncate(const std::string &filename, int new_size);
bool fs_copy(const std::string &src_filename, const std::string &dest_filename);
bool fs_mv(const std::string &old_name, const std::string &new_name);
void fs_defragment();
void fs_check_integrity();
bool fs_backup(const std::string &backup_filename);
bool fs_restore(const std::string &backup_filename);
void fs_cat(const std::string &filename);
bool fs_diff(const std::string &file1, const std::string &file2);
void fs_log(const std::string &message);

#endif
