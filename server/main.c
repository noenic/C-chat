#define _GNU_SOURCE // pour eviter les errors de précompilation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include "socketHelper.h"
#include "channels.h"
#include "client.h"
#include "helper.h"







struct listChannel *channel_list = NULL;
int nb_max_fd = 10;

listeClients *hub_client_list = NULL;
int hub_sock ;

int nbThreads = 0;
pthread_t server[100];

pthread_mutex_t lock; // Mutex pour protéger les variables globales


// Fonctions

void* handleHub(void *arg);
void* handleChannel(void *arg);
void* handleClients(void *arg);






void* handleHub(void *arg){
    struct client *client = (struct client *)arg;
    printf(LOG("THREAD", "Un thread a été créé pour gérer le client [%s]\n"), client->name);

    printf("\n");

    // On utilise un entier pour préciser au client qu'il communique avec le hub
    // 200 = HUB
    // 201 = CHANNEL
    int *serverType = allouer_entier_dynamique(HUB);
    write(client->socket, serverType, sizeof(int));
    free(serverType); // On a plus besoin de cette variable donc on la libère

    // On envoie la liste des channels au client
    write(client->socket, &channel_list->nb_channels, sizeof(int)); // On envoie le nombre de channels

    // On envois une chaine de caractère pour les channels en les séparant par un retour à la ligne
    char *channels = allouer_chaine_dynamique(1);
    channels[0] = '\0'; // Initialiser la chaîne vide

    for (int i = 0; i < channel_list->nb_channels; i++) {
        // Calcule la taille nécessaire pour le nom du canal et la virgule
        int size = snprintf(NULL, 0, "%s,", channel_list->channels[i].name);

        // Alloue la mémoire pour la chaîne de caractères avec la nouvelle taille
        channels = reallocation_dynamique(channels, strlen(channels) + size + 1);

        // Concaténe le nom du channel et un retour à la ligne
        snprintf(channels + strlen(channels), size + 1, "%s\n", channel_list->channels[i].name);
    }
    // On envoie la taille de la liste des channels
    int *size = allouer_entier_dynamique(strlen(channels));
    write(client->socket, size, sizeof(int));
    free(size); // On a plus besoin de cette variable donc on la libère

    // On envoie la liste des channels
    write(client->socket, channels, strlen(channels));
    free(channels); // On a plus besoin de cette variable donc on la libère

    // On attend que le client choisisse un channel
    char *choice = allouer_chaine_dynamique(100);
    read(client->socket, choice, 100);
    // Si on reçoit un message vide, on déconnecte le client
    if(strlen(choice) == 0){
        printf(LOG("HUB", "Le client [%s] a envoyé un message vide, on le déconnecte\n"), client->name);
        client_disconnect(client, hub_client_list);
        pthread_exit(NULL);
    }
    
    // On sanitise le nom du channel (seulement les lettres, les chiffres et les tirets)
    for(int i = 0; i < strlen(choice); i++){
        // On remplace les caractères spéciaux par des tirets, on
        if(!isalnum(choice[i])){
            choice[i] = '-';
        }
    }
    printf(LOG("HUB", "Le client [%s] a choisi le channel [%s]\n"), client->name, choice);
    // On cherche le channel dans la liste des channels
    channel *Searchedchannel = getChannelByName(channel_list, choice);
    if(Searchedchannel == NULL){
        printf(LOG("HUB", "Le channel [%s] n'existe pas, on le créé\n"), choice);


        // On l'ajoute à la liste des channels
        // On lock la variable globale pour éviter les problèmes
        Searchedchannel = create_channel(choice, 0);
        pthread_mutex_lock(&lock);
        add_channel(channel_list, Searchedchannel);
        pthread_mutex_unlock(&lock);
        // On parcoure la liste des channels pour lister tous les channels avec leurs ports

        // On créé un thread pour gérer le channel
        pthread_create(&server[nbThreads], NULL, handleChannel, Searchedchannel);
        nbThreads++;
    }
    while (Searchedchannel->port == 0)
    {
        sleep(0.5); // On attend que le port soit généré par le thread du channel
    }
    // On envoie le port du channel au client
    write(client->socket, &Searchedchannel->port, sizeof(int));


    printf(LOG("THREAD", "Le thread du client [%s] s'est terminé\n"), client->name);
    client_disconnect(client, hub_client_list);
    pthread_exit(NULL);
}






/*
    la fonction handleChannel fait la même chose que main mais lui est dédié à un channel,
    
*/
void* handleChannel(void *arg) {
    struct channel *channel = (struct channel *)arg;
    printf(LOG("THREAD", "Un thread a été créé pour gérer le channel [%s]\n"), channel->name);

    // Créez la socket du canal et configurez-la (comme dans votre code)
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror(ERROR("Impossible de créer la socket du canal"));
        exit(EXIT_FAILURE);
    }

    // Mettez la socket en mode non bloquant
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        perror(ERROR("Impossible de récupérer les drapeaux de la socket"));
        exit(EXIT_FAILURE);
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror(ERROR("Impossible de mettre la socket en mode non bloquant"));
        exit(EXIT_FAILURE);
    }

    // Initialisez la structure d'adresse pour bind
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(channel->port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        perror(ERROR("Impossible de bind la socket du canal"));
        exit(EXIT_FAILURE);
    }

    // Configurez la socket en mode écoute
    if (listen(sock, 100) == -1) {
        perror(ERROR("Impossible de mettre la socket du canal en écoute"));
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(addr);
    if (getsockname(sock, (struct sockaddr *)&addr, &len) == -1) {
        perror(ERROR("Impossible de récupérer le port de la socket du channel"));
        exit(-1);
    }

    channel->port = ntohs(addr.sin_port); // On récupère le port du channel
    channel->socket = sock; // On met la socket du channel dans la structure du channel
    printf(LOG("CHANNEL", "Le channel [%s] est sur le port [%d]\n"), channel->name, channel->port);

    // On ecoute sur le port et on créé un thread pour chaque client qui se connecte
    while (1) {
        int clientSocket = accept(sock, NULL, NULL);
        if (clientSocket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Pas de client en attente de connexion
                continue;
            }
            else {
                perror(ERROR("Impossible d'accepter le client"));
                exit(-1);
            }
        }
        // On créé l'object client
        struct client *current_client = client_create(clientSocket);
        // On envoie le type de serveur au client
        int *serverType = allouer_entier_dynamique(CHANNEL); //CHANNEL = 201
        write(current_client->socket, serverType, sizeof(int));
        free(serverType); // On a plus besoin de cette variable donc on la libère

        // On ajoute le channel au client et le client au channel 
        addChannelToClient(current_client, channel);
        addClientTolist(current_client, channel->listeClients);
        printf(LOG("CHANNEL", "Le client [%s] s'est connecté sur le channel [%s]\n"), current_client->name, channel->name);
        pthread_create(&server[nbThreads], NULL, handleClients, current_client);
        nbThreads++;
    }

}


void* handleClients(void *arg){
    struct client *client = (struct client *)arg;
    printf(LOG("THREAD", "ET C'EST PARTIt [%s]\n"), client->name);
    addNameToClient(client, "test");
    printf("2Le nom du client est %s\n", client->name);
    printf("Il il ya %d clients dans le channel\n", client->channel->listeClients->nbClients);

    while(1){

    }


}


void handleSIGINT(int signum) {
    // // Fermez la socket et déconnectez tous les clients du hub
    // printf("Fermerture des sockets du serveur\n");
    // close(hub_sock);
    // disconnet_all_clients(hub_client_list);

    // //Fermez les sockets et déconnectez tous les clients des channels
    // for(int i = 0; i < channel_list->nb_channels; i++){
    //     printf(LOG("EXIT", "Fermerture des sockets du channel [%s]\n"), channel_list->channels[i].name);
    //     for (int j = 0; j < channel_list->channels[i].listeClients->nbClients; j++) {
    //         close(channel_list->channels[i].listeClients->clients[j].socket);
    //     }
    // }

    
        

    exit(0);
}

/*

    La fonction main intialise le serveur et le met en écoute sur le port 8000 qui sera le port du hub, l'endroit où les clients se connecteront pour choisir un channel

*/
int main(int argc, char *argv[]){
    //initialise la liste des channels et on la remplie avec les channels existants
    channel_list = initListChannel();
    // On initialise la liste des clients du hub
    hub_client_list = initListeClients();
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


    // Ouverture d'un nouveau thread pour chaque channel
    for(int i = 0; i < channel_list->nb_channels; i++){
        pthread_create(&server[nbThreads], NULL, handleChannel, &channel_list->channels[i]);
        nbThreads++;
    }
    int hub_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(hub_sock == -1){
        perror(ERROR("Impossible de créer la socket"));
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // Si on a passé un argument, on utilise ce port, sinon on utilise le port 8000
    if(argc < 2){
        printf(LOG("HUB", "Pas de port spécifié, on utilise le port 8000\n"));
        addr.sin_port = htons(8000);
    }else{
        addr.sin_port = htons(atoi(argv[1]));
    }
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(hub_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
        perror(ERROR("Impossible de bind la socket"));
        exit(-1);
    }

    struct sigaction act;
    act.sa_handler = handleSIGINT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    if(listen(hub_sock, 10) == -1){
        perror(ERROR("Impossible de mettre la socket en écoute"));
        exit(-1);
    }

    while(1){
        int clientSocket = accept(hub_sock, NULL, NULL);
        if(clientSocket == -1){
            perror(ERROR("Impossible d'accepter le client"));
            exit(-1);
        }


         // On créé l'object client
        struct client *current_hub_client = client_create(clientSocket);
        addClientTolist(current_hub_client, hub_client_list);
        printf(LOG("HUB", "Le client [%s] s'est connecté sur le hub\n"), current_hub_client->name);
        pthread_create(&server[nbThreads], NULL, handleHub, current_hub_client);
        nbThreads++;
    }

    

}