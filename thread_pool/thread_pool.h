#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>

#define TUBE_CLIENT "mon_tube_"

#define FUN_SUCCESS 0
#define FUN_FAILURE -1

#define MAX_THREAD 5

typedef struct threads {
  size_t count;
  size_t max_thread;
}threads;

threads *th;

/**
 * Creation d'une structure generalisant les informations sur les threads
 * creer et utilise par notre programme.
 */
threads *thread_ini();

/**
 * Cree un thread en lui envoyant une commande.
 */
int thread_create(threads *th, char *commande);

/**
 * Fonction decoupant la commande donne en argument afin de trouver la commande 
 * Ã  execute les arguments/options.
 */
void *split_func(void *commande);

/**
 * fork_thread ouvre un tube en utilisant pid, se duplique afin d'effectuer
 * la commande et ses arguments.
 */
int fork_thread(char *pid, char *cmd, char *args[]);

/**
 * Libere la memoire du pointeur sur pids et le tableau args.
 */
void dispose_req(char *pid, char *args[]);
