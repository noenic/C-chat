#define _GNU_SOURCE // pour eviter les errors de précompilation
#include "../includes/helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>






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
    Reproduit la fonction getline de la librairie stdio.h mais retoure la chaîne de caractères sans le retour à la ligne
    @param size: un pointeur vers la taille de la chaîne de caractères
    @note La valeur de size est modifiée par la fonction et contient la taille de la chaîne de caractères
    
*/
char* get_line(size_t *size) {
    char *line = NULL;
    *size = getline(&line, size, stdin);
    if (*size != -1) {
        // Supprimer le caractère de nouvelle ligne à la fin de la ligne lue
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        // Sanitiser la ligne en ne conservant que les caractères imprimables
        size_t i, j = 0;
        for (i = 0; i < strlen(line); i++) {
            if (isprint(line[i])) {
                line[j++] = line[i];
            }
        }
        line[j] = '\0'; // Terminer la chaîne après la dernière caractère valide
        return line;
    } else {
        perror("Erreur lors de la lecture de la ligne");
        return NULL;
    }
}



/*
    Fonction qui vérifie si le message est une commande
    @note Les commandes commencent par un '/'
    @note L'existance de cette fonction permettra d'ajouter de futures commandes facilement
    @param message: le message à vérifier
    @return 0 si le message est une commande, 1 si c'est un message normal, 2 si c'est la commande /quit
*/
int is_not_command(char* message,int sock){
    int return_code = 1;
    if (message[0] == '/') {
        if (strcmp(message, "/quit") == 0) {
            printf("Déconnexion...\n");
            close(sock);
            return_code= 2;
        } 

        else {
            printf("Liste des commandes disponibles:\n");
            printf("- /quit: Quitter le channel\n");
            printf("- /help: Afficher la liste des commandes disponibles\n\n");
            return_code = 0;
        }
    }
    return return_code;
}