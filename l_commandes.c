#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include "cmds.h"
#include "liste_proc.h"

#define TBUF 256    
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

int main (int argc, char *argv[]) {

  int argc_cmd;
  char *argv_cmd[MAX_ARG];
  int nblu;
  char buf[TBUF];
  char invite[TBUF];

  /* Pour illustrer la manipulation de chaines */

  strcpy(invite,"Entrez une commande");
  /* strcpy copie le second argument dans le premier (chaines) */
	
  strcat(invite," >");
  /* Pour illustrer le strcat qui concatene deux chaines */

  /* on peut utiliser 
   sprintf(invite,"Entrez une commande >");
   */

  while(1) {
  	write(1,invite,strlen(invite));
	/* write : niveau en dessous de celui du printf
	 * on indique le descripteur sur lequel on ecrit, la chaine a ecrire et sa longuer (obtenue avec strlen */
	
	bzero(buf, sizeof(buf)); /* vider buf avant le read */
	nblu=read(0, buf, sizeof(buf));
	/* lecture au clavier. Memes remarques que pour le write / printf */
	if (nblu>0) {
	  argc_cmd=get_args(MAX_ARG, argv_cmd, buf);
	  if (argc_cmd > 0) {
	    traiter_cmd(argc_cmd, argv_cmd);
	    sleep(1); /* laisser un peu de temps entre deux commandes */
	    free_args(argc_cmd, argv_cmd);
	    /* libere l'espace memoire reserve par get_args pour stocker les differents arguments*/
	  }
	}

  }
  return 0;
}
