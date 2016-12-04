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
#include "sock_server.h"
#include "cmds.h"
#include "liste_proc.h"

#define TBUF 256    
#define MAXCL 2 
/* nombre max de clients */    
#define MAX_ARG 16  

ListeProc liste_proc; 

void notice_termination(int pid, int status) {
	if (WIFEXITED(status)) {
		printf("Fils %d terminé avec sortie %d\n", pid, WEXITSTATUS(status));
		supprimer_proc(pid, &liste_proc);
	}
	if (WIFSIGNALED(status)) {
		printf("Processus %d -> signal émis: %d\n", pid, WTERMSIG(status));
		supprimer_proc(pid, &liste_proc);
	}
	if (WIFSTOPPED(status)) {
		printf("Processus %d stoppé avec le code %d\n", pid, WSTOPSIG(status));
		modifier_etat_proc(pid, SUSPENDU, liste_proc);
	}
	if (WIFCONTINUED(status)) {
		printf("Processus %d repris avec le code %d\n", pid, WSTOPSIG(status));
		modifier_etat_proc(pid, ACTIF, liste_proc);
	}

}
void trtsig(int sig) {
	int pid, status, serrno;
	serrno = errno;
	while(1) {
		pid = waitpid(WAIT_ANY, &status, WUNTRACED| WCONTINUED | WNOHANG);
		if (pid<=0)
			break;
		notice_termination(pid, status);
	}
	errno = serrno;
}

/* traitement d'une commande */ 
void traiter_cmd(int argc_cmd, char *argv_cmd[]) {
	int pid;

	  if (argc_cmd) { /* commande non vide*/
	if(!strcasecmp(argv_cmd[0], "exit")) {
		  /* strcasecmp compare 2 chaines sans tenir compte de la casse
		   * et renvoie 0 si elle sont egales */

		  /* tout fermer et sortie proprement */
		detruire_liste(liste_proc);
		exit(0);
	}
	if(!strcasecmp(argv_cmd[0], "rlp")) {
		  /* afficher la liste des processus */
		printf("J'affiche les processus en cours \n");
		afficher_liste_proc(liste_proc);

	}
	else if(!strcasecmp(argv_cmd[0], "kill")) {
		  /* COmmande kill entrée */
		if (!strcasecmp(argv_cmd[1], "-9")) {
		  	/* tuer un processus */
			printf("Je tue un processus en cours \n");
			kill(atoi(argv_cmd[2]), 9);
		}

		else if (!strcasecmp(argv_cmd[1], "-18")) {
		  	/* relancer un processus en cours */
			printf("Je relance un processus en cours\n");
			kill(atoi(argv_cmd[2]), 18);
			modifier_etat_proc(atoi(argv_cmd[2]), ACTIF, liste_proc);
		}

		else if (!strcasecmp(argv_cmd[1], "-20")) {
		  	/* suspendre un processus en cours */
			printf("Je suspend un processus en cours\n");
			kill(atoi(argv_cmd[2]), 20);
			modifier_etat_proc(atoi(argv_cmd[2]), SUSPENDU, liste_proc);
		}

	}
	else {
		printf("Je vais executer une commande externe \n");
		/* On execute la commande externe dans un processus fils, pour ne pas perdre la main
		   sur le programme */
		pid = fork();
		if (pid == 0) {
			execvp(argv_cmd[0], &argv_cmd[0]);
			perror("Erreur d'execution");
			exit(2);
		}
		else {
			inserer_proc_en_queue(pid, ACTIF, &liste_proc, *argv_cmd);
			signal(SIGCHLD, trtsig);
		}
	}
}
}
int main(int argc, char *argv[]) {

	/*pr et pw pour les communications dediees avec chaque client*/
	int pr[MAXCL][2], pw[MAXCL][2];
	/*tableaux des descripteurs de lecture et d'ecriture */
	int tdr[MAXCL], tdw[MAXCL];
	/* tableau des entrées connectées. */
	int connecte[MAXCL];
	int i, nump, pid, nblu, nbcl;
	char buf[TBUF],mess[TBUF];
	/* Ces variables suivantes vont comtenir le nombre d'argumants et la commande
	   après traitement via les fonctions du module cmds */
	int argc_cmd;
	char *argv_cmd[MAX_ARG];
	/* On sauvegarde les sortie standard et d'erreur, pour les remettre entre chaque execution de commande. */
	int saved_stdout=dup(1), saved_stderr=dup(2);

	if (argc!=2) {
		printf("Il faut un numero de port \n");
		exit(1);
	}
	nump=atoi(argv[1]);
	nbcl=0;
	/* on cree les pipes necessaires pour les echanges avec sock_server */
	while (nbcl<MAXCL && (pipe(pr[nbcl])!=-1) && (pipe(pw[nbcl])!=-1)) {
		nbcl++;
	}
	if (nbcl<1) {
		printf("Probleme de creation de pipes\n");
		exit(2);
	}
	printf("nb max clients = %d\n",nbcl);
	pid = fork(); 
	if (pid==-1) {
		perror("fork");
		exit(4);
	}

	if (pid== 0) { /* Fils */
		for (i=0; i<nbcl; i++) {
		    /*on ferme les extrremites non utilisees et on initialise les tableaux des descripteurs de communication avec sock_server*/
			close(pr[i][0]);
			close(pw[i][1]);
			tdr[i]=pr[i][1];    
			tdw[i]=pw[i][0];  
		}
		sock_server(nump, tdw, tdr, nbcl);
	  	   /* sock_server sans retour*/
	}
		else { /*Pere*/
	/* On ferme les extrémités inutiles. On initialise afin d'etre non bloquant pour les entrées clavier. */
	for (i=0; i<nbcl; i++) {
		close(pr[i][1]);
		close(pw[i][0]);
		connecte[i]=0;
		fcntl(pr[i][0], F_SETFL, fcntl(pr[i][0], F_GETFL, NULL)|O_NONBLOCK);
	}
	while(1) {
		for (i=0; i<nbcl; i++) {
		      /* on lit ce qui arrive sur la connexion numero i. On réinitialise le buffer a 
		         chaque tour.*/
			bzero(buf, sizeof(buf));
			nblu=read(pr[i][0],buf, TBUF);
			if (nblu>0) {
			  /* si connexion non fermee */
				if (strncmp(buf,"Fermee",6)) {
				/* si c'est le premier message de connexion */
					if (!connecte[i]) {
						connecte[i]=1;
					}
				/* on confirme la reception du message */
					else {
						/* On change les sortie standard et d'erreur vers le pipe de communication serveur-client. */ 
						dup2(pw[i][1],1);
						dup2(pw[i][1],2);
						/* On décompose les arguments avant le traitement via 'traiter_cmd'. */
						argc_cmd=get_args(MAX_ARG, argv_cmd, buf);
						if (argc_cmd > 0) {
							traiter_cmd(argc_cmd, argv_cmd);
							free_args(argc_cmd, argv_cmd);
		   		  			/* libere l'espace memoire reserve par get_args pour stocker les differents arguments*/
							/*On récupère les sortie standard par défaut. */
							dup2(saved_stdout,1);
							dup2(saved_stderr,1);
						}
					}
				}
			  /* connexion fermee */
				else {
					connecte[i]=0;
					sprintf(mess,"Connexion %d fermee\n ",i);
					write(1,mess,strlen(mess));
				}
			}
		}
	}
}
close(saved_stderr);
close(saved_stdout);
return 0;
}	

