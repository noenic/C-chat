#include <stdlib.h>
#include <unistd.h>
#include "client.h"
#include <string.h>
#include "helper.h"







struct listeClients *initListeClients(){
    struct listeClients *listeClients = malloc(sizeof(struct listeClients));
    listeClients->nbClients = 0;
    listeClients->clients = malloc(sizeof(int));
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
    struct client *client = malloc(sizeof(struct client));
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
    printf("Le nom du client est %s\n", client->name);
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
void addClientTolist(struct client *client, struct listeClients *listeClients){
    printf("L'adresse memoire du client est %p\n", client);
    listeClients->nbClients++;
    listeClients->clients = realloc(listeClients->clients, listeClients->nbClients * sizeof(int));
    listeClients->clients[listeClients->nbClients - 1] = &client;
}


/*
    * Supprime un client de la liste des clients
    * @param client Le client à supprimer
    * @param listeClients La liste des clients
    * @note Va réduire la taille de la liste des clients dynamiquement et detruires le client supprimé
*/
void removeClientFromList(struct client *client, struct listeClients *listeClients) {
    int i;
    for (i = 0; i < listeClients->nbClients; i++) {
        if (listeClients->clients[i].socket == client->socket) {
            // Libérer la mémoire du champ 'name' du client à supprimer
            free(listeClients->clients[i].name);

            listeClients->nbClients--;

            // Déplacement du dernier élément à la position de l'élément à supprimer
            listeClients->clients[i] = listeClients->clients[listeClients->nbClients];

            // Réallocation de la mémoire pour réduire la taille du tableau
            listeClients->clients = realloc(listeClients->clients, listeClients->nbClients * sizeof(struct client));
            break;
        }
    }
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
        if(strcmp(listeClients->clients[i].name, name) == 0){
            return &listeClients->clients[i];
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
    // return NULL;
    int i;
    for(i = 0; i < listeClients->nbClients; i++){
        if(listeClients->clients[i].socket == socket){
            return &listeClients->clients[i];
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
    * @note Si channel est NULL, le message sera envoyé à tous les clients
*/
void sendToAllClientsInChannel(char *author, char *message,struct listeClients *listeClients){
    // [Auteur] : message
    char *msg = malloc(sizeof(char) * (strlen(author) + strlen(message) + 4));
    sprintf(msg, "[%s] : %s", author, message);
    int i;
    size_t size = strlen(msg);

    //On n'enverra pas le message à l'auteur
    for(i = 0; i < listeClients->nbClients; i++){
        if(strcmp(listeClients->clients[i].name, author) != 0){
            // On envoie la taille du message et ensuite le message
            write(listeClients->clients[i].socket, &size, sizeof(size_t));
            write(listeClients->clients[i].socket, msg, strlen(msg));
        }
    }
    free(msg);


}


/*
    * Déconnecte tous les clients
    * @param listeClients La liste des clients
    * @note Va fermer tous les sockets des clients et les supprimer de la liste des clients et les détruire
*/
void disconnet_all_clients(struct listeClients *listeClients){
    int i;
    for(i = 0; i < listeClients->nbClients; i++){
        printf(LOG("EXIT", "Deconnexion du client [%s]\n"), listeClients->clients[i].name);
        close(listeClients->clients[i].socket);
        removeClientFromList(&listeClients->clients[i], listeClients);
    }
}






