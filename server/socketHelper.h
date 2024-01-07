#ifndef SOCKETHELPER_H
#define SOCKETHELPER_H

/*
    * Fonction permetant d'ajouter une socket à un tableau de sockets
    * @note l'espacement mémoire est augmenté avec realloc
    * @param int **tableau : le tableau de sockets
    * @param int *tailleTableau : la taille du tableau de sockets
    * @param int socket : la socket à ajouter
*/
void ajouterSocket(int **tableau, int *tailleTableau, int socket);

/*
    * Fonction permetant de supprimer une socket d'un tableau de sockets
    * @note l'espacement mémoire est réduit avec realloc
    * @note la socket n'est pas fermée
    * @param int **tableau : le tableau de sockets
    * @param int *tailleTableau : la taille du tableau de sockets
    * @param int socket : la socket à supprimer
*/
void supprimerSocket(int **tableau, int *tailleTableau, int socket);

/*
    * Fonction permetant de fermer toutes les sockets d'un tableau de sockets
    * @note la mémoire du tableau n'est pas libérée
    * @param int *tableau : le tableau de sockets
    * @param int tailleTableau : la taille du tableau de sockets
*/
void fermerToutesLesSockets(int *tableau, int tailleTableau);

#endif // SOCKETHELPER_H
