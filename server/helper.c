#include "helper.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "channels.h"






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
    Fonction pour allouer dynamiquement un size_t
    @param valeur: la valeur de l'entier
    @return un pointeur vers l'entier alloué
*/
size_t* allouer_size_t_dynamique(int valeur) {
    size_t* entier = (size_t*)malloc(sizeof(size_t));
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


/*
    Fonction pour réallouer dynamiquement une chaîne de caractères
    @param chaine: la chaîne de caractères à réallouer
    @param taille: la nouvelle taille de la chaîne de caractères
    @return un pointeur vers la chaîne de caractères réallouée
*/
char* reallocation_dynamique(char* chaine, int taille) {
    char* chaine_reallouee = (char*)realloc(chaine, taille * sizeof(char));
    if (chaine_reallouee == NULL) {
        perror(ERROR("Erreur lors de la réallocation dynamique pour une chaîne"));
        exit(EXIT_FAILURE);
    }
    return chaine_reallouee;
}
