#include "../include/fs.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <cstring>

using namespace std;

bool fs_format() {
    int fd = open(DISK_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) return false;

    char zero = 0;
    lseek(fd, DISK_SIZE - 1, SEEK_SET);
    write(fd, &zero, 1);

    Metadata metadata{};
    metadata.file_count = 0;
    memset(metadata.entries, 0, sizeof(metadata.entries));
    memset(metadata.reserved, 0, sizeof(metadata.reserved));

    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));
    close(fd);
    fs_log("FORMAT");
    return true;
}

bool fs_load_metadata(Metadata &metadata) {
    int fd = open(DISK_NAME, O_RDONLY);
    if (fd < 0) return false;
    lseek(fd, 0, SEEK_SET);
    read(fd, &metadata, sizeof(Metadata));
    close(fd);
    return true;
}

bool fs_save_metadata(const Metadata &metadata) {
    int fd = open(DISK_NAME, O_WRONLY);
    if (fd < 0) return false;
    lseek(fd, 0, SEEK_SET);
    write(fd, &metadata, sizeof(Metadata));
    close(fd);
    return true;
}

bool fs_create(const string &filename) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    if (filename.length() >= FILENAME_MAX_LEN) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        if (metadata.entries[i].used && filename == metadata.entries[i].filename)
            return false;
    }

    int index = -1;
    for (int i = 0; i < MAX_FILES; ++i) {
        if (!metadata.entries[i].used) {
            index = i;
            break;
        }
    }

    if (index == -1) return false;

    int used_blocks[MAX_FILES] = {0};
    for (int i = 0; i < MAX_FILES; ++i)
        if (metadata.entries[i].used)
            used_blocks[i] = metadata.entries[i].start_block;

    int start_block = (METADATA_SIZE / BLOCK_SIZE);
    while (find(begin(used_blocks), end(used_blocks), start_block) != end(used_blocks))
        start_block++;

    FileEntry &entry = metadata.entries[index];
    strcpy(entry.filename, filename.c_str());
    entry.size = 0;
    entry.start_block = start_block;
    entry.created = static_cast<uint32_t>(time(nullptr));
    entry.used = true;

    metadata.file_count++;
    fs_save_metadata(metadata);
    fs_log("CREATE " + filename);
    return true;
}

bool fs_delete(const string &filename) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;
    for (int i = 0; i < MAX_FILES; ++i) {
        if (metadata.entries[i].used && filename == metadata.entries[i].filename) {
            metadata.entries[i].used = false;
            metadata.file_count--;
            fs_save_metadata(metadata);
            fs_log("DELETE " + filename);
            return true;
        }
    }
    return false;
}

bool fs_write(const string &filename, const char *data, int size) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    int fd = open(DISK_NAME, O_RDWR);
    if (fd < 0) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        FileEntry &entry = metadata.entries[i];
        if (entry.used && filename == entry.filename) {
            off_t offset = METADATA_SIZE + entry.start_block * BLOCK_SIZE;
            lseek(fd, offset, SEEK_SET);
            write(fd, data, size);
            entry.size = size;
            fs_save_metadata(metadata);
            close(fd);
            fs_log("WRITE " + filename);
            return true;
        }
    }

    close(fd);
    return false;
}

bool fs_read(const string &filename, int offset, int size, char *buffer) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    int fd = open(DISK_NAME, O_RDONLY);
    if (fd < 0) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        FileEntry &entry = metadata.entries[i];
        if (entry.used && filename == entry.filename) {
            if (offset + size > entry.size) {
                close(fd);
                return false;
            }

            off_t read_offset = METADATA_SIZE + entry.start_block * BLOCK_SIZE + offset;
            lseek(fd, read_offset, SEEK_SET);
            read(fd, buffer, size);
            close(fd);
            fs_log("READ " + filename);
            return true;
        }
    }

    close(fd);
    return false;
}

void fs_ls() {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) {
        cerr << "Metadata okunamadı.\n";
        return;
    }

    cout << "Dosyalar:\n";
    for (int i = 0; i < MAX_FILES; ++i) {
        const FileEntry &entry = metadata.entries[i];
        if (entry.used) {
            cout << "- " << entry.filename << " (" << entry.size << " bytes)\n";
        }
    }

    fs_log("LS");
}

bool fs_exists(const string &filename) {
    if(filename.empty()) return false;

    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;
    for (int i = 0; i < MAX_FILES; ++i)
        if (metadata.entries[i].used && filename == metadata.entries[i].filename)
            return true;
    return false;
}

int fs_size(const string &filename) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return -1;
    for (int i = 0; i < MAX_FILES; ++i)
        if (metadata.entries[i].used && filename == metadata.entries[i].filename)
            return metadata.entries[i].size;
    return -1;
}

bool fs_append(const string &filename, const char *data, int size) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    int fd = open(DISK_NAME, O_RDWR);
    if (fd < 0) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        FileEntry &entry = metadata.entries[i];
        if (entry.used && filename == entry.filename) {
            off_t offset = METADATA_SIZE + entry.start_block * BLOCK_SIZE + entry.size;
            lseek(fd, offset, SEEK_SET);
            write(fd, data, size);
            entry.size += size;
            fs_save_metadata(metadata);
            close(fd);
            fs_log("APPEND " + filename);
            return true;
        }
    }

    close(fd);
    return false;
}

bool fs_rename(const string &old_name, const string &new_name) {
    if (!fs_exists(old_name)) return false;
    if (fs_exists(new_name)) return false;

    if (new_name.length() >= FILENAME_MAX_LEN) return false;

    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        if (metadata.entries[i].used && old_name == metadata.entries[i].filename) {
            strcpy(metadata.entries[i].filename, new_name.c_str());
            fs_save_metadata(metadata);
            fs_log("RENAME " + old_name + " " + new_name);
            return true;
        }
    }

    return false;
}

void fs_cat(const string &filename) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return;

    int fd = open(DISK_NAME, O_RDONLY);
    if (fd < 0) return;

    for (int i = 0; i < MAX_FILES; ++i) {
        const FileEntry &entry = metadata.entries[i];
        if (entry.used && filename == entry.filename) {
            char *buffer = new char[entry.size + 1];
            off_t offset = METADATA_SIZE + entry.start_block * BLOCK_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, buffer, entry.size);
            buffer[entry.size] = '\0';
            cout << buffer << "\n";
            delete[] buffer;
            fs_log("CAT " + filename);
            close(fd);
            return;
        }
    }

    close(fd);
    cerr << "Dosya bulunamadı.\n";
}

void fs_log(const string &message) {
    ofstream logFile("fs.log", ios::app);
    if (logFile.is_open()) {
        time_t now = time(nullptr);
        logFile << "[" << ctime(&now) << "] " << message << endl;
        logFile.close();
    }
}

bool fs_truncate(const string &filename, int new_size) {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return false;

    for (int i = 0; i < MAX_FILES; ++i) {
        FileEntry &entry = metadata.entries[i];
        if (entry.used && filename == entry.filename) {
            if (new_size >= entry.size) return false;
            entry.size = new_size;
            fs_save_metadata(metadata);
            fs_log("TRUNCATE " + filename);
            return true;
        }
    }

    return false;
}

bool fs_copy(const string &src_filename, const string &dest_filename) {
    if (!fs_exists(src_filename)) return false;
    if (fs_exists(dest_filename)) return false;

    int size = fs_size(src_filename);
    if (size <= 0) return false;

    char *buffer = new char[size];
    if (!fs_read(src_filename, 0, size, buffer)) {
        delete[] buffer;
        return false;
    }

    if (!fs_create(dest_filename)) {
        delete[] buffer;
        return false;
    }

    bool result = fs_write(dest_filename, buffer, size);
    delete[] buffer;
    fs_log("COPY " + src_filename + " to " + dest_filename);
    return result;
}

bool fs_mv(const string &old_name, const string &new_name) {
    return fs_rename(old_name, new_name);
}

bool fs_diff(const string &file1, const string &file2) {
    if (!fs_exists(file1)) return false;
    if (!fs_exists(file2)) return false;

    int size1 = fs_size(file1);
    int size2 = fs_size(file2);

    if (size1 != size2) return false;

    char *buf1 = new char[size1];
    char *buf2 = new char[size2];
    bool result = fs_read(file1, 0, size1, buf1) && fs_read(file2, 0, size2, buf2);

    bool same = result && (memcmp(buf1, buf2, size1) == 0);

    delete[] buf1;
    delete[] buf2;

    fs_log("DIFF " + file1 + " " + file2);
    return same;
}

bool fs_backup(const string &backup_filename) {
    int fd_src = open(DISK_NAME, O_RDONLY);
    int fd_dst = open(backup_filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd_src < 0 || fd_dst < 0) return false;

    char buffer[1024];
    int bytes;
    while ((bytes = read(fd_src, buffer, sizeof(buffer))) > 0)
        write(fd_dst, buffer, bytes);

    close(fd_src);
    close(fd_dst);
    fs_log("BACKUP to " + backup_filename);
    return true;
}

bool fs_restore(const string &backup_filename) {
    int fd_src = open(backup_filename.c_str(), O_RDONLY);
    if(fd_src <= 0) return false;
    
    int fd_dst = open(DISK_NAME, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if(fd_dst <= 0) return false;

    char buffer[1024];
    int bytes;
    while ((bytes = read(fd_src, buffer, sizeof(buffer))) > 0)
        write(fd_dst, buffer, bytes);

    close(fd_src);
    close(fd_dst);
    fs_log("RESTORE from " + backup_filename);
    return true;
}

void fs_defragment() {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) return;

    int current_block = METADATA_SIZE / BLOCK_SIZE;
    int fd = open(DISK_NAME, O_RDWR);

    for (int i = 0; i < MAX_FILES; ++i) {
        FileEntry &entry = metadata.entries[i];
        if (entry.used && entry.start_block > current_block) {
            char *buffer = new char[entry.size];
            lseek(fd, METADATA_SIZE + entry.start_block * BLOCK_SIZE, SEEK_SET);
            read(fd, buffer, entry.size);

            entry.start_block = current_block;
            lseek(fd, METADATA_SIZE + current_block * BLOCK_SIZE, SEEK_SET);
            write(fd, buffer, entry.size);

            delete[] buffer;
            current_block += (entry.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        } else if (entry.used) {
            current_block = max(current_block, entry.start_block + (entry.size + BLOCK_SIZE - 1) / BLOCK_SIZE);
        }
    }

    fs_save_metadata(metadata);
    close(fd);
    fs_log("DEFRAGMENT");
}

void fs_check_integrity() {
    Metadata metadata;
    if (!fs_load_metadata(metadata)) {
        cerr << "Metadata okunamadı.\n";
        return;
    }

    bool overlap = false;
    for (int i = 0; i < MAX_FILES; ++i) {
        for (int j = i + 1; j < MAX_FILES; ++j) {
            if (metadata.entries[i].used && metadata.entries[j].used) {
                int start1 = metadata.entries[i].start_block;
                int end1 = start1 + (metadata.entries[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE;

                int start2 = metadata.entries[j].start_block;
                int end2 = start2 + (metadata.entries[j].size + BLOCK_SIZE - 1) / BLOCK_SIZE;

                if (max(start1, start2) < min(end1, end2)) {
                    cerr << "Uyarı: '" << metadata.entries[i].filename
                         << "' ve '" << metadata.entries[j].filename << "' blok çakışması içeriyor.\n";
                    overlap = true;
                }
            }
        }
    }

    if (!overlap) cout << "Tüm dosyalar bütünlüğünü koruyor.\n";
    fs_log("CHECK_INTEGRITY");
}

