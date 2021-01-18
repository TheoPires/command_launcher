#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <file.h>

//#include "../file_src/file.h"
#include "../thread_pool/thread_pool.h"

#define BUFFER_SIZE 20
#define FINISH "exit"

#define OPEN_MSG "Le serveur est ouvert\n"
#define CLOSE_MSG "\nLe serveur est fermé\n"

//  Recupere le pid contenu dans commande
int get_pid(char *commande);

//  Libere la memoire de th et ferme le SHM
void dispose(threads *th);

//  Supprime le SHM en cas d'erreur du demon et en recrée un
int restart();

int main(void) {
  printf(OPEN_MSG);
  //  Creation du SHM
  int shm_fd = shm_open(NOM_SHM, O_RDWR |O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    shm_fd = restart();
    if (shm_fd == -1) {
      return EXIT_FAILURE;
    }
  }
  if (ftruncate(shm_fd, (long int) 1000) == -1) {
    perror("ftruncate");
    return EXIT_FAILURE;
  }
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("mmap");
    dispose(th);
    exit(EXIT_FAILURE);
  }
  //  Initialisation de la structure des Threads et de la file
  file* file_p = create_shm_file(shm_ptr);
  threads *th = thread_ini();
  if (th == NULL) {
    if (shm_unlink(NOM_SHM) == -1) {
		  perror("shm_unlink");
		  exit(EXIT_FAILURE);
    }
    return EXIT_FAILURE;
  }
  int pid;
  char *req;
  int thc_return;
  //  Boucle de defilage
  while (1) {
    req = defiler(file_p);
    //  test commande fermeture demon
    if (strstr(req,"close_demon") != NULL) {
      break;
    }
    //  Creation des threads
    thc_return = thread_create(th, req);
    if (thc_return == -1) {
      dispose(th);
      return EXIT_FAILURE;
    } else if (thc_return == -2) {
      enfiler("0",file_p);
      //  Envois des signaux aux clients
      while ((pid = get_pid(defiler(file_p))) != 0) {
          kill((pid_t)pid, SIGUSR1);
      }
    }
  }
  dispose(th);
  printf(CLOSE_MSG);
  return EXIT_SUCCESS;
}

//  Méthodes outils
int get_pid(char *commande) {
  char cmd[strlen(commande)];
  char *com = (char*)commande;
  size_t l = (size_t)com[0];
  char *pid = malloc(l);
  strcpy(cmd, (char *)commande);
  int i;
  for (i = 1; i <= cmd[0] - '0'; i++) {
    pid[i - 1] = cmd[i];
    printf("%c", pid[i - 1]);
  }
  return atoi(pid);
}

void dispose(threads *th) {
  if (shm_unlink(NOM_SHM) == -1) {
		perror("shm_unlink");
		exit(EXIT_FAILURE);
  }
  free(th); 
}

int restart() {
  if (shm_unlink(NOM_SHM) == -1) {
	  perror("shm_unlink");
		exit(EXIT_FAILURE);
  }
  int shm_fd = shm_open(NOM_SHM, O_RDWR |O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    return FUN_FAILURE;
  }
  printf("redémarrage serveur...");
  printf("\n");
  return shm_fd;
}