#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

// DÉFINITION DE LA STRUCTURE DE LA SHM
typedef struct file {
  sem_t mutex;
  sem_t vide;
  sem_t plein;
  int tete;      // Position d'ajout dans le tampon
  int queue;     // Position de suppression dans le tampon
  char buffer[]; // Le tampon contenant les données
}file;

#define N 500

// NOM DE LA SHM
#define NOM_SHM "/mon_shm"

//TAILLE DE LA SHM
#define TAILLE_SHM (sizeof(file) + N)

/**
 * Crée une shm avec une structure de file
 */
file *create_shm_file(char* shm_ptr);

/**
 * Destruction des sémaphores
 */
void destroy_semaphore(file *file_p);

/**
 * Ajoute en queue le message command dans la file.
 */
void enfiler(char *command, file* file_p);

/**
 * Retirer en tête les messages un à un.
 */
char *defiler(file* file_p);
