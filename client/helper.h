#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
/*

    Ce fichier contient les fonctions outils

*/
#define HUB 200
#define CHANNEL 201


// Définir des couleurs pour les messages
#define ANSI_COLOR_RED     "\x1b[91m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"



// Définir des macros pour les messages d'erreur avec début et fin de couleur
#define ERROR(message)    ANSI_COLOR_RED "\n[ERROR] " message ANSI_COLOR_RESET
#define LOG(type,message)      ANSI_COLOR_YELLOW "["type"] " message ANSI_COLOR_RESET


// On declare ses fonctions, pour simplifier le code, pas besoins de reverifier
// à chaque fois si l'allocution a bien fonctionné

// ---------------------------ALLOCATIONS--------------------------------------
/*
    Fonction pour allouer dynamiquement un entier
    @param valeur: la valeur de l'entier
    @return un pointeur vers l'entier alloué
*/
int* allouer_entier_dynamique(int valeur);

/*
    Fonction pour allouer dynamiquement une chaîne de caractères
    @param taille: la taille de la chaîne de caractères
    @return un pointeur vers la chaîne de caractères allouée
*/
char* allouer_chaine_dynamique(int taille);

char **allouer_liste_dynamique(int taille);
void free_liste_dynamique(char **liste, int taille);

/*
    Reproduit la fonction getline de la librairie stdio.h mais retoure la chaîne de caractères sans le retour à la ligne
    @param size: un pointeur vers la taille de la chaîne de caractères
    
*/
char* get_line(size_t *size);



#endif // HELPER_H