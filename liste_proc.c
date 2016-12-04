#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "liste_proc.h"

struct fiche_proc{
  int pid;
  State etat;
  char* name;
  struct fiche_proc* suivant;
};

/* fonction appartient : 
  -- indique si un processus de n° pid appartient à la liste
  -- paramètres : 
        int pid : n° du nouveau processus (D)
        ListeProc liste_proc : liste de processus (D)
  -- résultat :
        int : pseudo booléen indiquant l'appartenance
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
int appartient(int pid, ListeProc liste_proc){

  while(liste_proc != NULL && liste_proc -> pid != pid) {
    liste_proc = liste_proc -> suivant;
  }

  return (liste_proc != NULL);
}

/* fonction afficher_liste_proc : 
  -- affiche la liste des processus
  -- paramètres : 
        liste_proc : liste de processus (D)
  -- résultat :
        aucun
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
void afficher_liste_proc(ListeProc liste_proc) {

  ListeProc paux;
  paux = liste_proc;
  char* state = malloc(sizeof(char*));
  if (paux == NULL) {
    printf("Aucun processus\n");
  }
  while(paux!=NULL) {
    printf("Nom = %s\n", paux -> name);
    printf("PID = %d\n", paux -> pid);
    if (paux->etat) {
      strcpy(state, "ACTIF");
    }
    else {
      strcpy(state, "SUSPENDU");
    }
    printf("Statut = %s\n----------------\n",state);
    paux = paux -> suivant;
  }
}

/* fonction inserer_proc_en_tete : 
  -- insère un nouveau processus en tête de la liste de processus
  -- paramètres : 
        int pid : n° du nouveau processus (D)
        State etat : état du nouveau processus (D)
        ListeProc liste_proc : liste de processus (D/R)
  -- résultat :
        int : 0 OK, 1 KO
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
int inserer_proc_en_queue(int pid, State etat, ListeProc *liste_proc, char* name) {
  ListeProc nouvelem;
  ListeProc paux;
  nouvelem = malloc(sizeof(struct fiche_proc));
  
  char* realname;
  realname = malloc(sizeof(char*));
  strcpy(realname, name);

  if (nouvelem == NULL){
    return 1;
  }

  nouvelem -> pid = pid;
  nouvelem -> etat = etat;
  nouvelem -> name = realname;
  nouvelem -> suivant = NULL;

  if (*liste_proc == NULL) {
    *liste_proc = nouvelem;
  }
  else {
    paux = *liste_proc;
    while (paux->suivant != NULL) {
      paux = paux->suivant;
    }
    paux->suivant = nouvelem;
  }

  return 0;
}

/* fonction modifier_etat_proc : 
  -- change l'état du processus de n° pid
  -- paramètres : 
        int pid : n° du processus (D)
        State etat : nouvel état du processus (D)
        ListeProc liste_proc : liste de processus (D) 
                 [attention : la structure de liste n'est pas modifiée]
  -- résultat :
        int : 0 OK, 1 KO (processus non présent par exemple)
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
int modifier_etat_proc(int pid, State etat, ListeProc liste_proc){
  ListeProc paux;
  paux = liste_proc;

  if (liste_proc == NULL) {
    return 1;
  }

  while (paux != NULL && paux->pid != pid) {
    paux = paux -> suivant;
  }

  if (paux == NULL) {
    return 1;
  }
  else {
    paux -> etat = etat;
    return 0;
  }
}

/* fonction supprimer_proc : 
  -- supprimer le processus de n° pid de la structure de liste
  -- paramètres : 
        int pid : n° du processus (D)
        ListeProc liste_proc : liste de processus (D) 
               [attention : la structure de liste est modifiée]
  -- résultat :
        int : 0 OK, 1 KO (processus non présent par exemple)
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
int supprimer_proc(int pid, ListeProc *liste_proc) {

  ListeProc pauxcourant;
  ListeProc pauxpre;

  pauxcourant = *liste_proc;
  pauxpre = NULL;

  if (pauxcourant == NULL) {
    /* échec */
    return 1;
  }
  else if (pauxcourant->pid == pid) {
    *liste_proc = pauxcourant->suivant;
    /* succès */
    return 0;
  }
  else {
    while (pauxcourant != NULL) {
      if (pauxcourant->pid == pid){
        pauxpre->suivant = pauxcourant->suivant;
        /* succès */
        return 0;
      }
      else {
        pauxpre = pauxcourant;
        pauxcourant = pauxcourant->suivant;
      }
    }
    /* échec */
    return 1 ;
  }
}

/* fonction lire_num_tete : 
  -- renvoie le PID de la tête de liste.
  -- paramètres : 
        ListeProc liste_proc : liste de processus (D) 
               [attention : la structure de liste n'est pas modifiée]
  -- résultat :
        int : pid de la tête ou -1 si erreur
  -- pré : aucune
  -- post : aucune 
  --------------------------------------------------*/
int lire_num_tete(ListeProc liste_proc) {
  if (liste_proc == NULL) {
    return -1;
  }
  else {
    return liste_proc -> pid;
  }
}

State lire_etat_proc(ListeProc liste_proc, int pid) {
  ListeProc paux;
  paux = liste_proc;

  while (paux != NULL && paux->pid != pid) {
    paux = paux -> suivant;
  }
  return paux -> etat;
}

int detruire_liste(ListeProc liste_proc) {
  ListeProc temp;

  temp = liste_proc;

  if (liste_proc == NULL) {
    return 0;
  }
  while (liste_proc -> suivant != NULL) {
    temp = liste_proc -> suivant;
    free(liste_proc);
    liste_proc = temp;
  }

  if (liste_proc == NULL) {
    return 0;
  }
  else {
    return 1;
  }
}