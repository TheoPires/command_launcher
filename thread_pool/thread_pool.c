#include "thread_pool.h"

threads *thread_ini() {
	th = malloc(sizeof(th));
	if (th == NULL) {
		perror("malloc thread ini");
		return NULL;
	}
	th->count = 0;
	th->max_thread = MAX_THREAD;
	return th;
}

int thread_create(threads *th,char *commande) {
	pthread_t ptr;
	if (th->count < th->max_thread) {
		th->count += 1;
	} else {
		return -2;
	}
	if (pthread_create(&ptr, NULL, split_func, commande) != 0) {
		perror("pthread_create");
		return EXIT_FAILURE;
	}
	return FUN_SUCCESS;
}

void *split_func(void *commande) {
	char cmd[strlen(commande)];
	char *com = (char*)commande;
	
	size_t l = (size_t)com[0] * sizeof(char);
	char *pid = malloc(l);
	if (pid == NULL) {
		perror("malloc pid");
		return NULL;
	}
	strcpy(cmd, (char *)commande);
	
	int i;
	for (i = 1; i <= cmd[0] - '0'; i++) {
		pid[i - 1] = cmd[i];
	}

	//	Split la commande des options et des arguments de la commande
	while(isdigit(cmd[i])) {
		i += 1;
	}

	char command[100];
	char *option[100];
	char *args[100];
	int k = 0;
	//	Commande sans option ni arguments
	while (cmd[i] != ' ' &&  i < (int)strlen(cmd)) {
		command[k] = cmd[i];
		k += 1;
		i += 1;
	}

	//	Integre la commande aux arguments
	int oi = 0;
	int ai = 0;
	if (strncmp(command,"/bin/",5) == 0) {
		option[oi] = malloc(strlen(command) * sizeof(char) + 1);
		if (option[oi] == NULL) {
			perror("malloc option[oi]");
			return NULL;
		}
		strcpy(option[oi],command + 5);
	} else {
		option[oi] = malloc(strlen(command) * sizeof(char) + 1);
		if (option[oi] == NULL) {
			perror("malloc option[oi]");
			return NULL;
		}
		strcpy(option[oi],command);
	}
	oi += 1;
	//	Split les options et les arguments de la requete
	while (i < (int)strlen(cmd)) {
		//	Les options de la commande
		if (cmd[i] == '-') {
			char buf[100];
			size_t y = 0;
			while (cmd[i] != ' ') {
				buf[y] = cmd[i];
				y += 1;
				i += 1;
			}
			buf[y] = '\0';
			option[oi] = malloc(strlen(buf) * sizeof(char) + 1);
			if (option[oi] == NULL) {
				perror("malloc option[oi]");
				return NULL;
			}
			strcpy(option[oi], buf);
			oi += 1;
		}
		//	Les arguments de la commandes
		if (cmd[i] != '-' && cmd[i] != ' ' && cmd[i-1] == ' ' ) {
			char buf2[100];
			size_t y = 0;
			while (cmd[i] != ' ') {
				buf2[y] = cmd[i];
				y += 1;
				i += 1;
			}
			buf2[y] = '\0';
			args[ai] = malloc(strlen(buf2) * sizeof(char) + 1);
			if (args[ai] == NULL) {
				perror("malloc args[ai]");
				return NULL;
			}
			strcpy(args[ai], buf2);
			ai += 1;
		}
		i+=1;
	}
	//	Fusionne les options et les arguments
	for (int p = 0; p < ai ; p++) {
		option[oi] = malloc(strlen(args[p]) * sizeof(char) + 1);
		if (option[oi] == NULL) {
			perror("malloc option[oi]");
			return NULL;
		}
		strcpy(option[oi],args[p]);
		free(args[p]);
		oi += 1;
	}
	option[oi] = NULL;
	if (fork_thread(pid, command,option) == -1) {
		printf("erreur d'ouverture de fork thread'");
		return NULL;
	}
	free(commande);
	return NULL;
}

int fork_thread(char *pid, char *cmd, char *args[]) {
	char tube_client[strlen(TUBE_CLIENT) + strlen(pid) + 1];
	int fd_client;
	//	Creation du nom du tube avec le pid du client
	sprintf(tube_client,"%s%s",TUBE_CLIENT,pid);
	//	Ouverture du tube client
	if ((fd_client = open(tube_client, O_WRONLY)) == -1) {
		perror("thread pool fork thread: Impossible d'ouvrir le tube du client");
		dispose_req(pid, args);
		return FUN_FAILURE;
	}
	//	Duplication
	switch (fork()) {
		case -1:
			perror("fork_thread: fork");
			dispose_req(pid, args);
			return FUN_FAILURE;
		case 0:
			//	Redirection des sorties
			if (dup2(fd_client, STDOUT_FILENO) == -1) {
				perror("fork_thread: dup2 stdout");
				dispose_req(pid, args);
				return FUN_FAILURE;
			}
			if (dup2(fd_client, STDERR_FILENO) == -1) {
				perror("fork_thread: dup2 stderr");
				dispose_req(pid, args);
				return FUN_FAILURE;
			}
			//	Execution de la commande
			execv(cmd, args);
			perror("execv");
			exit(EXIT_FAILURE);
			break;
		default:
			//	Attente du fils pour fermer le tube
			if (wait(NULL) == -1) {
				dispose_req(pid, args);
				return FUN_FAILURE;
			}
			th->count -= 1;
			dispose_req(pid, args);
			if (close(fd_client) == -1) {
				dispose_req(pid, args);
				return FUN_FAILURE;
			}
			break;
	}
	return FUN_SUCCESS;
}

void dispose_req(char *pid, char *args[]) {
	free(pid);
	int i = 0;
	while (args[i] != NULL) {
		free(args[i]);
		i += 1;
	}
}