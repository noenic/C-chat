#ifndef CLIENT_H
#define CLIENT_H
#include "channels.h"
/*

    Ce fichier contient les fonctions permettant de gérer les clients
        - Par le bias de la structure client
        - Par le bias de la structure listeClients  (qui contient un tableau de clients)

*/



typedef struct listeClients{
    int nbClients;
    // On fait un tableau de pointeur
    int* clients;
} listeClients;


typedef struct client {
    char *name;
    int socket;
    struct channel *channel;
} client;

/*
    * Initialise la liste des clients
    * @return La liste des clients
*/
listeClients *initListeClients();

/*
    * Créé un client
    * @param name Le nom du client
    * @param socket La socket du client
    * @return Le client
*/
struct client *client_create(int socket);


/*
    * Ajoute un nom à un client
    * @param client Le client
    * @param name Le nom à ajouter
*/
void addNameToClient(struct client *client, char *name);

/*
    * Ajoute un channel à un client
    * @param client Le client
    * @param channel Le channel à ajouter
*/
void addChannelToClient(struct client *client, struct channel *channel);




/*
    * Deconnecte le client en fermant son socket et en le supprimant de la liste des clients et en le détruisant
    * @param c Le client à déconnecter
    * @param listeClients La liste des clients
*/
void client_disconnect(struct client *c, struct listeClients *listeClients);


/*
    * Ajoute un client à la liste des clients
    * @param client Le client à ajouter
    * @param listeClients La liste des clients
    * @note Va augmenter la taille de la liste des clients dynamiquement
*/
void addClientTolist(struct client *client, struct listeClients *listeClients);

/*
    * Supprime un client de la liste des clients
    * @param client Le client à supprimer
    * @param listeClients La liste des clients
*/
void removeClientFromList(struct client *client, struct listeClients *listeClients);

/*
    * Récupère un client par son nom
    * @param name Le nom du client
    * @param listeClients La liste des clients
    * @return Le client
*/
struct client *getClientByName(char *name, struct listeClients *listeClients);

/*
    * Récupère un client par son socket
    * @param socket Le socket du client
    * @param listeClients La liste des clients
    * @return Le client
*/
struct client *getClientBySocket(int socket, struct listeClients *listeClients);

/*
    * Envoie un message à tous les clients d'un channel
    * @param author Le nom de l'auteur du message
    * @param message Le message à envoyer
    * @param channel Le channel dans lequel envoyer le message
    * @param listeClients La liste des clients
    * @note Le message sera envoyé à tous les clients du channel sauf l'auteur
*/
void sendToAllClientsInChannel(char *author, char *message,struct listeClients *listeClients);






/*
    * Déconnecte tous les clients
    * @param listeClients La liste des clients
    * @note Va fermer tous les sockets des clients et les supprimer de la liste des clients et les détruire
*/
void disconnet_all_clients(struct listeClients *listeClients);





















#endif // CLIENT_H