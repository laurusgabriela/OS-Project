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
#include <fcntl.h>
#include <sys/wait.h>

void convert_to_grayscale(char *bmp_file_path) {
    FILE *bmp_file = fopen(bmp_file_path, "rb");

    if (bmp_file == NULL) {
        perror("Error opening BMP file");
        exit(EXIT_FAILURE);
    }

    int fd = open(bmp_file_path, O_RDWR, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);

    fseek(bmp_file, 18, SEEK_SET);
    int width, height, dataOffset;
    fread(&width, sizeof(int), 1, bmp_file);
    fread(&height, sizeof(int), 1, bmp_file);

    if (lseek(fd, 0, SEEK_SET) < 0){
                        perror("Error to move the file cursor1\n");
                        exit(-1);
    }
    if (lseek(fd, 10, SEEK_CUR) < 0){
      perror("Error to move the file cursor1\n");
      exit(-1);
    }

    if(read(fd,&dataOffset,4) < 0){
      perror("Error to read from file!\n");
      exit(-1);
    }

    if(lseek(fd,4,SEEK_SET) < 0){
      perror("Error to move the file cursor2.\n");
      exit(-1);
    }
                
    if(lseek(fd,dataOffset,SEEK_SET) < 0){
      perror("Error to move the file cursor3.\n");
      exit(-1);
    }

    
    int red,green,blue;
    long long i;
    for (i = 0; i < width * height; i++) {

      if (read(fd, &red, 1) < 0){
	perror("error red.\n");
	exit(-1);
      }

      if (read(fd, &green, 1) < 0){
	perror("error green.\n");
	exit(-1);
      }

      if (read(fd, &blue, 1) < 0){
	perror("error blue.\n");
	exit(-1);
      }

      int gray_value = (int)(0.299 * red + 0.587 * green + 0.114 * blue);

      // Seteaza aceeasi valoare de gri pentru toate cele trei culori
      red = gray_value;
      green = gray_value;
      blue = gray_value;

      // Muta cursorul înapoi la poziția inițială și scrie noile valori
      if (lseek(fd, -3, SEEK_CUR) < 0) {
	perror("Error to move the file cursor4.\n");
	exit(-1);
      }

      if (write(fd, &red, 1) < 0) {
	perror("Error to write red value.\n");
	exit(-1);
      }

      if (write(fd, &green, 1) < 0) {
	perror("Error to write green value.\n");
	exit(-1);
      }

      if (write(fd, &blue, 1) < 0) {
	perror("Error to write blue value.\n");
	exit(-1);
      }
                    
    }

    /*  for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char pixel[3];
            fread(pixel, sizeof(unsigned char), 3, bmp_file);

            unsigned char grayscale = 0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2];

            fseek(bmp_file, -3, SEEK_CUR);
            for (int k = 0; k < 3; k++) {
                fwrite(&grayscale, sizeof(unsigned char), 1, bmp_file);
            }
        }
    }*/

    fclose(bmp_file);
}

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
            pid_t bmp_child_pid = fork();

            if (bmp_child_pid == -1) {
                perror("Error creating BMP conversion process");
                exit(EXIT_FAILURE);
            }

            if (bmp_child_pid == 0) {
                convert_to_grayscale(path);
                exit(EXIT_SUCCESS);
            } else {
                int bmp_status;
                waitpid(bmp_child_pid, &bmp_status, 0);

                if (WIFEXITED(bmp_status)) {
                    printf("Conversion process with PID %d exited with code %d\n", bmp_child_pid, WEXITSTATUS(bmp_status));
                } else {
                    printf("Conversion process with PID %d did not exit normally\n", bmp_child_pid);
                }
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

void process_directory(char *dir_path, char *output_dir) {
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

            char output_filename[256];
            snprintf(output_filename, sizeof(output_filename), "%s/%s_statistica.txt", output_dir, entry->d_name);

            FILE *output_file = fopen(output_filename, "w");
            if (output_file == NULL) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }

            print_file_info(entry_path, output_file);

            fclose(output_file);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char output_dir[256];
    snprintf(output_dir, sizeof(output_dir), "%s", argv[2]);

    if (mkdir(output_dir, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    process_directory(argv[1], output_dir);

    return 0;
}
