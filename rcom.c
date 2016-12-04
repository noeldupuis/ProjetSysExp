#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	int pid, TBUF=64,nblu;
	char  buf[TBUF];
    char invite[TBUF];

    /* On initialise le pipe de communication entre le père et le fils */
	int pppf[2];
	if (pipe(pppf)==-1) {
		perror("Pipe error");
		exit(1);
	}

	if ((pid=fork())==-1) {
		perror("Fork error");
		exit(1);
	}

	if (pid == 0) {
		close(pppf[1]);
		/* On considère le pipe comme entrée standard. sc_client reçoit alors ses arguments depuis
		   le processus père */
		dup2(pppf[0],0);
		/* On lance sc_client sur le port et la machine serveur*/
		execlp("sc_client","sc_client", argv[1], argv[2], NULL);
		perror("Erreur srvcom");
		exit(1);
	}
	else {
		close(pppf[0]);
		int fdwrite;

		if ((fdwrite=open("client.log", O_WRONLY | O_CREAT, 0666)) == -1) {
				perror("Erreur d'ouverture de client.log chez le père");
		}

		while(1) {
			sleep(1);

			/* On invite l'utilisateur a entrer une commande */
			sprintf(invite,"\nEntrez une commande >");
			write(1,invite,strlen(invite));

			/* On réinitialise le buffer, pour avoir une entrée propre. */
			bzero(buf, sizeof(buf));
			nblu=read(0, buf, TBUF);
			/* Cas où quelque chose est entré au clavier */
			if (nblu> 0) {
				if (write(fdwrite, buf, strlen(buf))== -1) {
					perror("Erreur d'écriture de client.log chez le père.");
				}
				/* Cas où on a entré 'exit': il faut envoyer un signal de fin au serveur pour qu'il
				   nous déconnecte, puis on stoppe le fils sc_client, et enfin on termine le programme */
				if(!strncmp(buf, "exit", 4)) {
					write(pppf[1], "Fermee", 6);
					kill(pid, SIGSTOP);
					close(fdwrite);
					exit(0);
				}
				/* Sinon on envoie la commande a sc_client via le pipe, sans aucun traitement. Tout sera fait
				   par le serveur ensuite. */
				else{					
					write(pppf[1], buf, nblu);
				}
			}
		}
	}
	return 0;
}