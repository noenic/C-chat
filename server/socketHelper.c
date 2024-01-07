#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socketHelper.h"

void ajouterSocket(int **tableau, int *tailleTableau, int socket) {
    (*tailleTableau)++;
    int *nouveauTableau = (int *)realloc(*tableau, *tailleTableau * sizeof(int));

    if (nouveauTableau == NULL) {
        fprintf(stderr, "Échec de la réallocation mémoire\n");
        free(*tableau);
        exit(1);
    } else {
        *tableau = nouveauTableau;
    }

    (*tableau)[*tailleTableau - 1] = socket;
}

void supprimerSocket(int **tableau, int *tailleTableau, int socket) {
    int index = -1;

    // Recherche de l'index de la socket
    for (int i = 0; i < *tailleTableau; i++) {
        if ((*tableau)[i] == socket) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        // Déplacer les éléments suivants vers l'avant
        for (int i = index; i < *tailleTableau - 1; i++) {
            (*tableau)[i] = (*tableau)[i + 1];
        }

        // Réduire la taille du tableau
        (*tailleTableau)--;
        int *nouveauTableau = (int *)realloc(*tableau, *tailleTableau * sizeof(int));

        if (*tailleTableau > 0 && nouveauTableau == NULL) {
            fprintf(stderr, "Échec de la réallocation mémoire\n");
            free(*tableau);
            exit(1);
        } else {
            *tableau = nouveauTableau;
        }
    } else {
        fprintf(stderr, "Socket non trouvée dans le tableau\n");
    }
}

void fermerToutesLesSockets(int *tableau, int tailleTableau) {
    for (int i = 0; i < tailleTableau; i++) {
        close(tableau[i]); 
    }
}
