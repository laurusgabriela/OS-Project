#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void print_file_info(char *path, FILE *output_file) {
    struct stat file_stat;

    if (lstat(path, &file_stat) == -1) {
        perror("Error getting file information");
        exit(EXIT_FAILURE);
    }

    fprintf(output_file, "nume fisier: %s\n", path);

    if (S_ISREG(file_stat.st_mode)) {
        fprintf(output_file, "dimensiune: %ld\n", file_stat.st_size);

        if (strstr(path, ".bmp") != NULL) {
            FILE *bmp_file = fopen(path, "rb");
            if (bmp_file != NULL) {
                fseek(bmp_file, 18, SEEK_SET);
                int width, height;
                fread(&width, sizeof(int), 1, bmp_file);
                fread(&height, sizeof(int), 1, bmp_file);
                fclose(bmp_file);

                fprintf(output_file, "inaltime: %d\n", height);
                fprintf(output_file, "lungime: %d\n", width);
            }
        }

        fprintf(output_file, "identificatorul utilizatorului: %d\n", file_stat.st_uid);
        fprintf(output_file, "timpul ultimei modificari: %s", ctime(&file_stat.st_mtime));
        fprintf(output_file, "contorul de legaturi: %ld\n", file_stat.st_nlink);
    } else if (S_ISDIR(file_stat.st_mode)) {
        fprintf(output_file, "nume director: %s\n", path);
    } else if (S_ISLNK(file_stat.st_mode)) {
        char target_path[256];
        ssize_t target_size = readlink(path, target_path, sizeof(target_path) - 1);

        if (target_size == -1) {
            perror("Error reading symbolic link");
            exit(EXIT_FAILURE);
        }

        target_path[target_size] = '\0';

        fprintf(output_file, "nume legatura: %s\n", path);
        fprintf(output_file, "dimensiune: %ld\n", target_size);
        fprintf(output_file, "dimensiune fisier: %ld\n", file_stat.st_size);
    }

    fprintf(output_file, "drepturi de acces user: %c%c%c\n",
           file_stat.st_mode & S_IRUSR ? 'R' : '-',
           file_stat.st_mode & S_IWUSR ? 'W' : '-',
           file_stat.st_mode & S_IXUSR ? 'X' : '-');
    fprintf(output_file, "drepturi de acces grup: %c%c%c\n",
           file_stat.st_mode & S_IRGRP ? 'R' : '-',
           file_stat.st_mode & S_IWGRP ? 'W' : '-',
           file_stat.st_mode & S_IXGRP ? 'X' : '-');
    fprintf(output_file, "drepturi de acces altii: %c%c%c\n",
           file_stat.st_mode & S_IROTH ? 'R' : '-',
           file_stat.st_mode & S_IWOTH ? 'W' : '-',
           file_stat.st_mode & S_IXOTH ? 'X' : '-');
}

void process_directory(char *dir_path, FILE *output_file) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char entry_path[256];
            snprintf(entry_path, sizeof(entry_path), "%s/%s", dir_path, entry->d_name);
            print_file_info(entry_path, output_file);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fisier_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char output_filename[] = "statistica.txt";
    FILE *output_file = fopen(output_filename, "w");

    if (output_file == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    process_directory(argv[1], output_file);

    fclose(output_file);

    return 0;
}
