#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include  <sys/shm.h>
#include <file.h>

//#include "../file_src/file.h"

#define FUN_SUCCESS 0
#define FUN_FAILURE -1

#define PID_SIZE 6
#define FINISH "exit"

#define TUBE_NAME_LENGTH 128
#define TUBE_CLIENT "mon_tube_"
#define MSG "\nEntrez votre commande : "
#define MSG2 "\nRetour de votre commande :\n\n"

#define BUFFER_SIZE 128
#define TUBE_SIZE 750

//	Liberation des ressources allouées au tube du client
void dispose(char *tube);

int main(void) {
	//	Overture du SHM
	int shm_fd;
	if ((shm_fd = shm_open(NOM_SHM, O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
		perror("client : Impossible d'ouvrir le shm");
		exit(EXIT_FAILURE);
  }
	char *shm_ptr = mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shm_ptr == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
 	pid_t pid = getpid();
	
	// Creation du tube nommé du client
	char tube_client[TUBE_NAME_LENGTH];
	sprintf(tube_client, "%s%d", TUBE_CLIENT, pid);
	if (mkfifo(tube_client, S_IRUSR | S_IWUSR) == -1) {
		perror("client: Impossible de créer le tube du client");
		exit(EXIT_FAILURE);
	}
	
	char shm_buff[BUFFER_SIZE];  	// Buffer de SHM pour les commandes entrantes
	char pipe_buff[TUBE_SIZE];    // Buffer de tube de sortie
	char mypid[PID_SIZE];					// ex. 65367\0
	char* request;
	snprintf(mypid, PID_SIZE,"%d", pid);
	printf("Identifiant du client : %s\n", mypid);
	printf(MSG);
	int pipe_fd = 0;
	//	Recuperation des requetes envoyées par le client
	while (fgets(shm_buff, BUFFER_SIZE, stdin) != NULL
			&& strncmp(shm_buff, FINISH, strlen(FINISH)) != 0 ) {
		request = malloc(1 + strlen(mypid) + 3 + strlen(shm_buff) * sizeof(char));
		if (request == NULL) {
			perror("malloc request");
			dispose(tube_client);
			exit(EXIT_FAILURE);
		}
		shm_buff[strlen(shm_buff)] = '\0';
		//	Creation de la requete sous la forme suivante :
		//	taille du PID . PID . taille de la commande . commande
		sprintf(request, "%ld%s%ld%s", strlen(mypid), mypid, strlen(shm_buff) - 1, shm_buff);
		request[strlen(request) - 1] = '\0';
		enfiler(request, (file*)shm_ptr);
		if (strstr(shm_buff, "close_demon") != NULL) {
			free(request);
			dispose(tube_client);
			exit(EXIT_SUCCESS);
		}
		// Ouverture en lecture puis lecture dans le tube client
		pipe_fd = open(tube_client, O_RDONLY);
		if (pipe_fd == -1) {
			dispose(tube_client);
			free(request);
			exit(EXIT_FAILURE);
		}
		if ((read(pipe_fd, pipe_buff, TUBE_SIZE)) == -1) {
			perror("client : read");
			dispose(tube_client);
			free(request);
			exit(EXIT_FAILURE);
		}
		printf(MSG2);
		puts(pipe_buff);
		//	Re initialisation du buffer du tube
		memset(pipe_buff,0,strlen(pipe_buff));
		if (close(pipe_fd) == -1) {
			perror("client : closeTubeClient");
			dispose(tube_client);
			free(request);
			exit(EXIT_FAILURE);
		}
		free(request);
		printf(MSG);
	}
	dispose(tube_client);
	return EXIT_SUCCESS;
}

void dispose(char *tube) {
	if (unlink(tube) == -1) {
		perror("unlink");
		exit(EXIT_FAILURE);
	}
}
