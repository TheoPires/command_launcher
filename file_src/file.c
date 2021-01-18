#include "file.h"

//  main de test :

/*int main(void) {
  int shm_fd = shm_open(NOM_SHM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(EXIT_SUCCESS);
  }

  if (shm_unlink(NOM_SHM) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(shm_fd, TAILLE_SHM) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }
  
  char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED) {
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  
  file *file_p = create_shm_file(shm_ptr);
  char s_stdin[100];
  fgets(s_stdin, 100, stdin);
  printf("s_stdin : %s\n", s_stdin);
  printf("Affichage de la longueur de strlen(stdin) : %ld\n", strlen(s_stdin));
  char *req;
  enfiler(s_stdin, file_p);
  req = defiler(file_p);
  printf("Requete : %s\n", req);
  destroy_semaphore(file_p);
  return EXIT_SUCCESS;
}*/

file *create_shm_file(char* shm_ptr) {
  file *file_p;
  file_p = (file*) shm_ptr;

  // Initialisation des variables
  if (sem_init(&file_p->mutex, 1, 1) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&file_p->vide, 1, N) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&file_p->plein, 1, 0) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  file_p->tete = 0;
  file_p->queue = 0;
  return file_p;
}

void destroy_semaphore(file *file_p) {
  if (sem_destroy(&file_p->mutex) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&file_p->plein) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  if (sem_destroy(&file_p->vide) == -1) {
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
}

void enfiler(char *command, file* file_p) {
  if (sem_wait(&file_p->vide) == -1) {
    perror("producteur : sem_wait(vide)");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&file_p->mutex) == -1) {
    perror("producteur : sem_wait(mutex)");
    exit(EXIT_FAILURE);
  }

  if (strcpy(&file_p->buffer[file_p->tete], command) == NULL) {
    fprintf(stderr, "Probleme strcpy dans la shm");
    exit(EXIT_FAILURE);
  } 

  //MODIFICATION DU POINTEUR DE TETE DE LA TAILLE DES DONNEES ENTREE
  file_p->tete = (file_p->tete + (int)strlen(command)) % N;
  if (sem_post(&file_p->mutex) == -1) {
    perror("enfiler sem_post(mutex)");
    exit(EXIT_FAILURE);
  }
  if (sem_post(&file_p->plein) == -1) {
    perror("enfiler sem_post(plein)");
    exit(EXIT_FAILURE);
  }
}

char *defiler(file* file_p) {
  if (sem_wait(&file_p->plein) == -1) {
    perror("defiler : sem_wait(plein)");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&file_p->mutex) == -1) {
    perror("defiler : sem_wait(mutex)");
    exit(EXIT_FAILURE);
  }

  // Recuperation de la longueur du Pid
  char *lengthOfPid = malloc(6 * sizeof(char));
  if (strncpy(lengthOfPid, &file_p->buffer[file_p->queue], 1) == NULL) {
    fprintf(stderr, "defiler strncpy pid problem");
    exit(EXIT_FAILURE);
  }
  int lengthPid = atoi(lengthOfPid);
  free(lengthOfPid);

  // Recuperation longueur de la requete
  int lengthRequest = atoi(&file_p->buffer[file_p->queue] + lengthPid + 1); //+1 octet car stockage longueur pi
  int taille_lengthRequest = 0;
  if (lengthRequest <= 9) {
    taille_lengthRequest = 1;
  } else if (lengthRequest <= 99) {
    taille_lengthRequest = 2;
  } else {
    taille_lengthRequest = 3;
  }
  
  //Recuperation de la requete
  char *request  = malloc((size_t)lengthRequest);
  if (strcpy(request, &file_p->buffer[file_p->queue]) == NULL) {
    fprintf(stderr, "problem pour la rÃ©cupÃ©ration de la requÃªte");
    exit(EXIT_FAILURE);
  }

  // Deplacement du pointeur de queue de la file de la taille de la requete
  int decalage = lengthPid + lengthRequest + 1 + taille_lengthRequest;
  file_p->queue = (file_p->queue + decalage) % N;  

  if (sem_post(&file_p->vide) == -1) {
    perror("defiler : sem_wait(vide)");
    exit(EXIT_FAILURE);
  }
  if (sem_post(&file_p->mutex) == -1) {
    perror("defiler : sem_wait(mutex)");
    exit(EXIT_FAILURE);
  }
  return request;
}
