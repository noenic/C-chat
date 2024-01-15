#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../includes/client.h"
#include "../includes/helper.h"







struct listeClients *initListeClients(){
    listeClients *listeClients = malloc(sizeof(struct listeClients));
    if (pthread_mutex_init(&listeClients->listeClients_mutex, NULL) != 0) {
        perror("Erreur lors de l'initialisation du mutex de la liste des clients");
        return NULL;
    }
    listeClients->nbClients = 0;
    listeClients->clients = malloc(sizeof(int));
    if (listeClients->clients == NULL) {
        perror("Erreur lors de l'allocation dynamique pour la liste des clients");
        return NULL;
    }
    return listeClients;
}


/*
    * Crée un client
    * @param name Le nom du client
    * @param socket Le socket du client
    * @param channel Le channel du client
    * @return Le client créé
*/
struct client *client_create(int socket){
    client *client = malloc(sizeof(struct client));
    // par default le nom du client est son socket, il sera changé plus tard par le client
    client->name = malloc(sizeof(char) * sizeof(int)); 
    sprintf(client->name, "%d", socket);
    client->socket = socket;
    return client;

}


/*
    * Ajoute un nom à un client
    * @param client Le client
    * @param name Le nom à ajouter
*/
void addNameToClient(struct client *client, char *name){
    client->name = malloc(strlen(name) * sizeof(char));
    strcpy(client->name, name);
}


/*
    * Ajoute un channel à un client
    * @param client Le client
    * @param channel Le channel à ajouter
*/
void addChannelToClient(struct client *client, struct channel *channel){
    client->channel = channel;
}




/*
    * Deconnecte le client en fermant son socket et en le supprimant de la liste des clients et en le détruisant
    * @param c Le client à déconnecter
    * @param listeClients La liste des clients
*/
void client_disconnect(struct client *c, struct listeClients *listeClients){    
    close(c->socket);
    printf(LOG("CLIENT", "Le client [%s] n'est plus connecté\n"), c->name);
    removeClientFromList(c, listeClients);
}


/*
    * Ajoute un client à la liste des clients
    * @param client Le client à ajouter
    * @param listeClients La liste des clients
    * @note Va augmenter la taille de la liste des clients dynamiquement
*/
void addClientToList(const client* c,listeClients* listeClients) {
    // On ferme le mutex de la liste des clients
    pthread_mutex_lock(&listeClients->listeClients_mutex);
    // Augmente la taille du tableau de pointeurs
    listeClients->clients = realloc(listeClients->clients, (listeClients->nbClients + 1) * sizeof(struct client*));

    if (listeClients->clients == NULL) {
        perror("Erreur lors de la réallocation de mémoire pour la liste des clients");
        return;
    }

    // Alloue de la mémoire pour stocker un pointeur vers une structure client
    listeClients->clients[listeClients->nbClients] = malloc(sizeof(struct client*));

    if (listeClients->clients[listeClients->nbClients] == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour un client");
        return;
    }

    // Stocke le pointeur vers la structure client donnée en paramètre
    listeClients->clients[listeClients->nbClients] = (struct client*)c;
    listeClients->nbClients++;
    // On ouvre le mutex de la liste des clients
    pthread_mutex_unlock(&listeClients->listeClients_mutex);
}


/*
    * Supprime un client de la liste des clients
    * @param client Le client à supprimer
    * @param listeClients La liste des clients
    * @note Va réduire la taille de la liste des clients dynamiquement et detruires le client supprimé
*/
void removeClientFromList(struct client *client, struct listeClients *listeClients) {
    // On ferme le mutex de la liste des clients
    pthread_mutex_lock(&listeClients->listeClients_mutex);
    
    int i;
    for (i = 0; i < listeClients->nbClients; i++) {
        if (listeClients->clients[i]->socket == client->socket) {
            // Libérer la mémoire du champ 'name' du client à supprimer
            free(listeClients->clients[i]->name);


            listeClients->nbClients--;

            // Déplacement du dernier élément à la position de l'élément à supprimer
            listeClients->clients[i] = listeClients->clients[listeClients->nbClients];

            // Réallocation de la mémoire pour réduire la taille du tableau
            listeClients->clients = realloc(listeClients->clients, listeClients->nbClients * sizeof(struct client));
            break;
        }
    }
    // On ouvre le mutex de la liste des clients
    pthread_mutex_unlock(&listeClients->listeClients_mutex);
}





/*
    * Récupère un client par son nom
    * @param name Le nom du client
    * @param listeClients La liste des clients
    * @return Le client
*/
struct client *getClientByName(char *name, struct listeClients *listeClients){
    int i;
    for(i = 0; i < listeClients->nbClients; i++){
        if(strcmp(listeClients->clients[i]->name, name) == 0){
            return listeClients->clients[i];
        }
    }
    return NULL;
}


/*
    * Récupère un client par son socket
    * @param socket Le socket du client
    * @param listeClients La liste des clients
    * @return Le client
*/
struct client *getClientBySocket(int socket, struct listeClients *listeClients){
    int i;
    for(i = 0; i < listeClients->nbClients; i++){
        if(listeClients->clients[i]->socket == socket){
            return listeClients->clients[i];
        }
    }
    return NULL;
}


/*
    * Envoie un message à tous les clients d'un channel
    * @param author Le nom de l'auteur du message
    * @param message Le message à envoyer
    * @param channel Le channel dans lequel envoyer le message
    * @param listeClients La liste des clients
    * @note Le message sera envoyé à tous les clients du channel sauf l'auteur
    * @note On utilise utilise pas le type client car le serveur peut aussi envoyé des messages 
*/
void sendToAllClientsInChannel(char *author, char *message,struct listeClients *listeClients){
    // [Auteur] : message
    char *msg = malloc(sizeof(char) * (strlen(author) + strlen(message) + 4));
    sprintf(msg, "[%s] : %s", author, message);
    int i;
    size_t size = strlen(msg)+1;
    //On n'enverra pas le message à l'auteur
    for(i = 0; i < listeClients->nbClients; i++){
        if(strcmp(listeClients->clients[i]->name, author) != 0){
            // On envoie la taille du message et ensuite le message
            write(listeClients->clients[i]->socket, &size, sizeof(size_t));
            write(listeClients->clients[i]->socket, msg, size);
        }
    }
    free(msg);
}


void error_report(struct client *client){
    printf(LOG("CHANNEL", "La connexion avec le client [%s] a été perdue, on le déconnecte\n"), client->name);
    char *message = allouer_chaine_dynamique(strlen(client->name)+strlen("Le client [] s'est déconnecté")+1);
    snprintf(message, strlen(client->name)+strlen("Le client [] s'est déconnecté")+1, "Le client [%s] s'est déconnecté", client->name);
    client_disconnect(client, client->channel->listeClients);
    sendToAllClientsInChannel("SERVEUR", message, client->channel->listeClients);
    free(message); // On a plus besoin de cette variable donc on la libère
}


