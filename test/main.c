#include <stdio.h>
#include <stdlib.h>
#include "string.h"


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


listeClients *initListeClients(){
    listeClients *listeClients = malloc(sizeof(struct listeClients));
    listeClients->nbClients = 0;
    listeClients->clients = malloc(sizeof(int));
    return listeClients;
}

listeClients *tab_client = NULL;

void addNameToClient(struct client *client, char *name);
void addNameToClient(struct client *client, char *name){
    client->name = malloc(strlen(name) * sizeof(char));
    strcpy(client->name, name);
}
void addClientTolist(listeClients *listeClients, const client *c);
void addClientTolist(listeClients *listeClients, const client *c) {
    int *newClients = realloc(listeClients->clients, (listeClients->nbClients + 1) * sizeof(client));
    // On crée newChannels pour éviter de perdre le pointeur 
    if (newClients == NULL) {
        perror("Erreur lors de la réallocation de mémoire pour la liste des clients");
        return;
    }
    listeClients->clients = newClients;
    // assignment to ‘int’ from ‘const client **’ makes integer from pointer without a cast [-Wint-conversion]
    listeClients->clients[listeClients->nbClients] = &c;
    listeClients->nbClients++;
}





int main(int argc, char *argv[])
{
    tab_client = initListeClients();
    printf("Hello world\n");
    // On créé un client
    client *c = malloc(sizeof(client));
    // On lui ajoute un nom
    addNameToClient(c, "toto");
    c->socket = 1;
    // On l'ajoute à la liste des clients
    addClientTolist(tab_client, c);

    for (int i = 0; i < tab_client->nbClients; i++)
    {
        printf("Le nom du client est %s\n", tab_client->clients[i]->name);
    }
    
}