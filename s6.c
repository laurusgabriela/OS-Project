#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usage: ./%s <fisier_intrare>", argv[0]);
    exit(EXIT_FAILURE);
  }

  int input_file, output_file;
  struct stat file_info;
  char statistics[528];

  if (access(argv[1], F_OK) == -1) {
    fprintf(stderr, "Eroare: Fișierul %s nu există.\n", argv[1]);
    exit(EXIT_FAILURE);
  }


  input_file = open(argv[1], O_RDONLY);
  if (input_file == -1) {
    perror("Eroare la deschiderea fișierului de intrare");
    exit(EXIT_FAILURE);
  }
 

  if (fstat(input_file, &file_info) == -1) {
    perror("Eroare la citirea informațiilor despre fișier");
    close(input_file);
    exit(EXIT_FAILURE);
  }


  snprintf(statistics, sizeof(statistics),
	   "nume fisier: %s\ninaltime: 1920\nlungime: 1280\ndimensiune: %lld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %scontorul de legaturi: %lu\ndrepturi de acces user: RWX\ndrepturi de acces grup: R--\ndrepturi de acces altii: ---\n",
	   argv[1], (long long)file_info.st_size, file_info.st_uid, ctime(&file_info.st_mtime), file_info.st_nlink);


  output_file = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (output_file == -1) {
    perror("Eroare la deschiderea fișierului de ieșire");
    close(input_file);
    exit(EXIT_FAILURE);
  }


  if (write(output_file, statistics, strlen(statistics)) == -1) {
    perror("Eroare la scrierea în fișierul de ieșire");
  }


  close(input_file);
  close(output_file);

  return 0;
}
