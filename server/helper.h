#ifndef HELPER_H
#define HELPER_H
#include "channels.h"
#include "client.h"


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




// On définit une structure pour les arguments des threads (La liste des clients,la listes des sockets des clients et la structure channel)
typedef struct thread_args{
    struct listeClients* channel_client_list;
    struct channel* channel;
    struct pollfd* list_clients_socks;
}thread_args;


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
    Fonction pour allouer dynamiquement un size_t
    @param valeur: la valeur de l'entier
    @return un pointeur vers l'entier alloué
*/ 
size_t* allouer_size_t_dynamique(int valeur);




/*
    Fonction pour allouer dynamiquement une chaîne de caractères
    @param taille: la taille de la chaîne de caractères
    @return un pointeur vers la chaîne de caractères allouée
*/
char* allouer_chaine_dynamique(int taille);


// ---------------------------REALLOCATIONS------------------------------------

/*
    Fonction pour réallouer dynamiquement une chaîne de caractères
    @param chaine: la chaîne de caractères à réallouer
    @param taille: la nouvelle taille de la chaîne de caractères
    @return un pointeur vers la chaîne de caractères réallouée
*/

char* reallocation_dynamique(char* chaine, int taille);









char** allouer_tableau_chaines_dynamique(int nombre_de_chaines);
void liberer_tableau_chaines(char** tableau, int nombre_de_chaines);




#endif // HELPER_H