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
#include <errno.h>
void print_file_info(char *path, FILE *output_file);

//functia care scrie datele in fisierele de statistica, apeleaza scriptul si gaseste nr de propozitii corecte
void process_file_content(char *file_path, FILE *output_file, char *character) {
  int pipefd[2];
  pid_t pid;

  if (pipe(pipefd) == -1) {
    perror("Error creating pipe");
    exit(EXIT_FAILURE);
  }

  pid = fork();

  if (pid == -1) {
    perror("Error creating process for file content processing");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    // procesul copil
    close(pipefd[0]); // inchidem capatul de citire al pipe-ului

    // redirectam STDOUT catre pipe
    dup2(pipefd[1], STDOUT_FILENO);
    int file_fd = open(file_path, O_RDONLY);//deschidem fisierul pentru a redirecta continutul lui spre script
    dup2(file_fd, STDIN_FILENO); //redirectam STDIN catre fisierul deschis anterior
    close(file_fd);
    //executam scriptul cu argumentul cerut
    execlp("bash","bash","script.sh", character, (char*)NULL);
    if(errno != 0)
      {
	fprintf(stderr, "fara eroare\n");
      }
    close(pipefd[1]);
    //daca intervin probleme la executarea scriptului
    perror("Error executing script");
    exit(EXIT_FAILURE);
  } else {
    // procesul parinte
    close(pipefd[1]); //inchidem capatul de scriere al pipe-ului
    print_file_info(file_path, output_file); //generam fisierele de statistica si scriem in ele
    // citim continutul pipe-ului
    char buffer[1024];
    int bytesRead;
    int totalPropositions = 0;

    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
      totalPropositions += atoi(buffer); //acumulam numarul de propozitii cu ceea ce avem in buffer
    }

    close(pipefd[0]); //inchidem capatul de citire al pipe-ului

    int status;
    waitpid(pid, &status, 0); //asteptam terminarea procesului copil

    if (WIFEXITED(status)) {
      printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", pid, WEXITSTATUS(status));
    } else {
      printf("Procesul cu pid-ul %d nu s-a încheiat normal\n", pid);
    }

    printf("Au fost identificate în total %d propoziții corecte care conțin caracterul %s\n", totalPropositions, character); //afisam numarul de propozitii corecte
  }
}

void convert_to_grayscale(char *bmp_file_path) { //functia de conversie in tonuri de gri
  FILE *bmp_file = fopen(bmp_file_path, "rb"); //deschidem fisierul .bmp in binar

  if (bmp_file == NULL) {
    perror("Error opening BMP file");
    exit(EXIT_FAILURE);
  }

  int fd = open(bmp_file_path, O_RDWR, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH); 

  fseek(bmp_file, 18, SEEK_SET); //inaintam cu cursorul pana la al 18lea byte, unde sunt informatii despre width si height
  int width, height, dataOffset;
  fread(&width, sizeof(int), 1, bmp_file);
  fread(&height, sizeof(int), 1, bmp_file);
  //avem nevoie de 3 pozitii de cursor
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
    //citim pe rand pixelii aferenti culorilor
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

    int gray_value = (int)(0.299 * red + 0.587 * green + 0.114 * blue); //calculam valoarea noilor pixeli
    red = gray_value;
    green = gray_value;
    blue = gray_value;

    //modificam poza cu noii pixeli calculati
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

  fclose(bmp_file);
}

void print_file_info(char *path, FILE *output_file) { //aceasta functie scrie in fisierele aferente, informatiile statistica
  struct stat file_stat;
    
  if (lstat(path, &file_stat) == -1) {
    perror("Error getting file information");
    exit(EXIT_FAILURE);
  }
    
  fprintf(output_file, "nume fisier: %s\n", path);
  if (S_ISREG(file_stat.st_mode)) { //printam informatiile cerute pentru regular files
    fprintf(output_file, "dimensiune: %ld\n", file_stat.st_size);
    fprintf(output_file, "identificatorul utilizatorului: %d\n", file_stat.st_uid);
    fprintf(output_file, "timpul ultimei modificari: %s", ctime(&file_stat.st_mtime));
    fprintf(output_file, "contorul de legaturi: %ld\n", file_stat.st_nlink);
  } else if (S_ISDIR(file_stat.st_mode)) { //printam informatiile cerute pentru directoare
    fprintf(output_file, "nume director: %s\n", path);
  } else if (S_ISLNK(file_stat.st_mode)) { //verificam legaturile simbolice
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
  //verificam drepturile de acces
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

void process_directory(char *dir_path, char *output_dir, char *character) { //functie ce parcurge directorul de intrare
  DIR *dir;
  struct dirent *entry;

  dir = opendir(dir_path);
  if (dir == NULL) {
    perror("Error opening directory");
    exit(EXIT_FAILURE);
  }

  while ((entry = readdir(dir)) != NULL) { //citim fiecare fisier din director
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      char entry_path[1024];
      snprintf(entry_path, sizeof(entry_path), "%s/%s", dir_path, entry->d_name); //aici stocam adresa absoluta a fisierului curent din director

      char output_filename[1024];
      snprintf(output_filename, sizeof(output_filename), "%s/%s_statistica.txt", output_dir, entry->d_name); //noul fisier in care se vor scrie statisticile

      if (strstr(entry->d_name, ".bmp") != NULL) {//daca este un .bmp file
	pid_t bmp_child_pid = fork(); //cream un proces nou

	if (bmp_child_pid == -1) {
	  perror("Error creating BMP conversion process");
	  exit(EXIT_FAILURE);
	}

	if (bmp_child_pid == 0) { //procesul copil
	  convert_to_grayscale(entry_path); //modificam poza -> o facem in tonuri de gri
	  exit(EXIT_SUCCESS);
	} else { //procesul parinte
	  int bmp_status;
	  waitpid(bmp_child_pid, &bmp_status, 0);

	  if (WIFEXITED(bmp_status)) { //verificam statusul procesului copil
	    printf("Procesul de conversie cu PID-ul %d s-a încheiat cu codul %d\n", bmp_child_pid, WEXITSTATUS(bmp_status));
	  } else {
	    printf("Procesul de conversie cu PID-ul %d nu s-a încheiat normal\n", bmp_child_pid);
	  }
	}
      } else { //daca nu e fisier .bmp
	FILE *output_file = fopen(output_filename, "w"); //cream fisierul in care vom scrie statisticile
	if (output_file == NULL) {
	  perror("Error opening output file");
	  exit(EXIT_FAILURE);
	}
	process_file_content(entry_path,output_file, character); //apelam functia de scriere si executare a scriptului
	fclose(output_file); //inchidem fisierul de statistica corespunzator intarii curente in director
      }

    }
  }

  closedir(dir);
}

int main(int argc, char *argv[]) {
  if (argc != 4) { //verificam numarul corect de argumente
    fprintf(stderr, "Usage: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char output_dir[1024]; //cream directorul de output
  snprintf(output_dir, sizeof(output_dir), "%s", argv[2]);

  if (mkdir(output_dir, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
    perror("Error creating output directory");
    exit(EXIT_FAILURE);
  }

  process_directory(argv[1], output_dir, argv[3]); //apelam functia de procesare a directoului

  return 0;
}
