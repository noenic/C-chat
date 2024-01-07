#define _GNU_SOURCE // pour eviter les errors de précompilation
#include "helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>






/*
    Fonction pour allouer dynamiquement un entier
    @param valeur: la valeur de l'entier
    @return un pointeur vers l'entier alloué
*/
int* allouer_entier_dynamique(int valeur) {
    int* entier = (int*)malloc(sizeof(int));
    *entier = valeur;
    if (entier == NULL) {
        perror(ERROR("Erreur lors de l'allocation dynamique pour un entier"));
        exit(EXIT_FAILURE);
    }
    return entier;
}




/*
    Fonction pour allouer dynamiquement une chaîne de caractères
    @param taille: la taille de la chaîne de caractères
    @return un pointeur vers la chaîne de caractères allouée
*/
char* allouer_chaine_dynamique(int taille) {
    char* chaine = (char*)malloc(taille * sizeof(char));
    if (chaine == NULL) {
        perror(ERROR("Erreur lors de l'allocation dynamique pour une chaîne"));
        exit(EXIT_FAILURE);
    }
    return chaine;
}


char **allouer_liste_dynamique(int taille) {
    char **liste = (char**)malloc(taille * sizeof(char*));
    if (liste == NULL) {
        perror(ERROR("Erreur lors de l'allocation dynamique pour une liste"));
        exit(EXIT_FAILURE);
    }
    return liste;
}

void free_liste_dynamique(char **liste, int taille) {
    for(int i = 0; i < taille; i++){
        free(liste[i]);
    }
    free(liste);
}


/*
    Reproduit la fonction getline de la librairie stdio.h mais retoure la chaîne de caractères sans le retour à la ligne
    @param size: un pointeur vers la taille de la chaîne de caractères
    
*/
char* get_line(size_t *size) {
    char *line = NULL;
    size_t read = getline(&line, size, stdin);

    if (read != -1) {
        // Si la ligne lue contient un caractère de nouvelle ligne à la fin, le supprimer
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        return line;
    } else {
        perror("Erreur lors de la lecture de la ligne");
        return NULL;
    }
}
