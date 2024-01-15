#define _GNU_SOURCE // pour eviter les erreurs de précompilation
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include "includes/channels.h"
#include "includes/client.h"
#include "includes/helper.h"





listChannel *channel_list = NULL;       // Liste des channels 
listeClients *hub_client_list = NULL;   // Liste des clients du hub
int hub_sock ;                          // Socket du hub

int nbThreads = 0;
pthread_t server[100];                  // On utilise que 100 threads pour le moment

pthread_mutex_t ChannelCreateLock; // Mutex pour eviter les problemes de creation de channel en meme temps


// Fonctions
void* handleHub(void *arg); 
void* handleChannel(void *arg);
void* handleClients(void *arg);





/*
    La fonction handleHub est dédié au hub, c'est ici que les clients se connectent pour choisir un channel
*/
void* handleHub(void *arg){
    struct client *client = (struct client *)arg;
    printf(LOG("THREAD", "Un thread a été créé pour gérer le client [%s] sur le HUB\n"), client->name);

    write(client->socket, &(int){HUB}, sizeof(int));

     // On envoie le nombre de channels au client
    write(client->socket, &channel_list->nb_channels, sizeof(int));

    // On envois une chaine de caractère pour les channels en les séparant par un retour à la ligne
    char *channels = allouer_chaine_dynamique(1);
    channels[0] = '\0'; // Initialiser la chaîne vide

    for (int i = 0; i < channel_list->nb_channels; i++) {
        // Calcule la taille nécessaire pour le nom du canal et la virgule
        int size = snprintf(NULL, 0, "%s,", channel_list->channels[i]->name);

        // Alloue la mémoire pour la chaîne de caractères avec la nouvelle taille
        channels = reallocation_dynamique(channels, strlen(channels) + size + 1);

        // Concaténe le nom du channel et un retour à la ligne
        snprintf(channels + strlen(channels), size + 1, "%s\n", channel_list->channels[i]->name);
    }
    // On envoie la taille de la chaine de caractère de liste des channels
    write(client->socket, &(int){strlen(channels)+1}, sizeof(int));
    // On envoie la liste des channels
    write(client->socket, channels, strlen(channels)+1);
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
        // On lock la variable globale pour éviter les problèmes de création de channel en même temps
        Searchedchannel = create_channel(choice, 0);
        pthread_mutex_lock(&ChannelCreateLock);
        add_channel(channel_list, Searchedchannel);
        pthread_mutex_unlock(&ChannelCreateLock);

        // On créé un thread pour gérer le channel
        pthread_create(&server[nbThreads], NULL, handleChannel, Searchedchannel);
        nbThreads++;
    }
    free(choice); // On a plus besoin de cette variable donc on la libère
    while (Searchedchannel->port == 0)
    {
        sleep(0.5); // On attend que le port soit généré par le thread du channel
    }
    // On envoie le port du channel au client
    write(client->socket, &Searchedchannel->port, sizeof(int));


    printf(LOG("THREAD", "Le thread du client [%s] sur le HUB s'est terminé\n"), client->name);
    client_disconnect(client, hub_client_list);
    pthread_exit(NULL);
}


/*
    la fonction handleChannel  est dédié à un channel, c'est ici que le channel est géré,
    on attend que des clients se connectent et on créé un thread pour chaque client qui se connecte
*/
void* handleChannel(void *arg) {
    struct channel *current_channel = (struct channel *)arg;
    printf(LOG("THREAD", "Un thread a été créé pour gérer le channel [%s]\n"), current_channel->name);

    // Crée la socket du channel 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror(ERROR("Impossible de créer la socket du canal"));
        exit(EXIT_FAILURE);
    }


    // Initialise la structure d'adresse pour bind
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(current_channel->port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        perror(ERROR("Impossible de bind la socket du canal"));
        exit(EXIT_FAILURE);
    }

    // Configure la socket en mode écoute
    if (listen(sock, 100) == -1) {
        perror(ERROR("Impossible de mettre la socket du canal en écoute"));
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(addr);
    if (getsockname(sock, (struct sockaddr *)&addr, &len) == -1) {
        perror(ERROR("Impossible de récupérer le port de la socket du channel"));
        exit(-1);
    }

    current_channel->port = ntohs(addr.sin_port); // On récupère le port du channel
    current_channel->socket = sock; // On met la socket du channel dans la structure du channel
    printf(LOG("CHANNEL", "Le channel [%s] est sur le port [%d]\n"), current_channel->name, current_channel->port);

    // On ecoute sur le port et on créé un thread pour chaque client qui se connecte
    while (1) {
        int clientSocket = accept(sock, NULL, NULL);
        if (clientSocket == -1) {
                perror(ERROR("Impossible d'accepter le client"));
                exit(EXIT_FAILURE);
            }
        // On créé l'object client
        struct client *current_client = client_create(clientSocket);
        // On envoie le type de serveur au client
        write(current_client->socket, &(int){CHANNEL}, sizeof(int));

        // On ajoute le channel au client et le client au channel 
        addChannelToClient(current_client, current_channel);
        addClientToList(current_client, current_channel->listeClients);
        printf(LOG("CHANNEL", "Le client [%s] s'est connecté sur le channel [%s]\n"), current_client->name, current_channel->name);
        pthread_create(&server[nbThreads], NULL, handleClients, current_client);
        nbThreads++;
    }

}

/*
    La fonction handleClients est dédié à un client, c'est ici que le client est géré,
    on attend que le client envoie un message et on l'envoie à tous les autres clients du channel
*/
void* handleClients(void *arg){
    struct client *client = (struct client *)arg;
    printf(LOG("THREAD", "Un thread a été créé pour gérer le client [%s] sur le channel [%s]\n"), client->name, client->channel->name);
    // On attend que le client envoie son nom, il va d'abord envoyer la taille de son nom puis son nom
    size_t *size = allouer_size_t_dynamique(0);
    read(client->socket, size, sizeof(size_t));
    char *name = allouer_chaine_dynamique(*size);
    read(client->socket, name, *size);
    // Si on reçoit un message vide, on déconnecte le client
    if(strlen(name) == 0){
        printf(LOG("CHANNEL", "La connexion avec le client [%s] a été perdue, on le déconnecte\n"), client->name);
        client_disconnect(client, client->channel->listeClients);
        pthread_exit(NULL);
    }

    // On sanitise le nom du client (seulement les lettres, les chiffres et les tirets)
    for(int i = 0; i < strlen(name); i++){
        // On remplace les caractères spéciaux par des tirets
        if(!isalnum(name[i])){
            name[i] = '-';
        }
    }
    // On vérifie si le nom est déjà pris
    if(getClientByName(name, client->channel->listeClients) != NULL){
        printf(LOG("CHANNEL", "Le nom [%s] est déjà pris, on déconnecte le client [%s]\n"), name, client->name);
        // On le envoie le nombre 1 au client pour lui dire que le nom est déjà pris
        write(client->socket, &(int){1}, sizeof(int));
        client_disconnect(client, client->channel->listeClients);
        pthread_exit(NULL);
    }
    write(client->socket, &(int){0}, sizeof(int)); // On envoie le nombre 0 au client pour lui dire que le nom est disponible



    addNameToClient(client, name);
    printf(LOG("CHANNEL", "Le client [%d] s'est identifié sous le nom [%s]\n"), client->socket, client->name);
    free(size); // On a plus besoin de cette variable donc on la libère
    free(name); // On a plus besoin de cette variable donc on la libère

    // On envoie les derniers messages du channel au client
    char* messages = read_channel_file(client->channel);
    size = allouer_size_t_dynamique(strlen(messages)+1);
    write(client->socket, size, sizeof(size_t));
    if(strlen(messages) != 0){
        printf(LOG("CHANNEL","Envoie des derniers messages du channel [%s] au client [%s]\n"), client->channel->name, client->name);
        write(client->socket, messages, strlen(messages)+1);
        
    }
    free(messages); // On a plus besoin de cette variable donc on la libère
    free(size); // On a plus besoin de cette variable donc on la libère

    
    // On initialise la structure pollfd pour le client
    struct pollfd pollfds[1];
    pollfds[0].fd = client->socket;
    pollfds[0].events = POLLIN;

    // On informe les autres clients de la connexion du client
    char *message = allouer_chaine_dynamique(strlen(client->name)+30);
    snprintf(message, strlen(client->name)+30, "Le client [%s] s'est connecté", client->name);
    sendToAllClientsInChannel("SERVEUR", message, client->channel->listeClients);
    free(message); // On a plus besoin de cette variable donc on la libère

    while (1)
    {
        // On utilise poll pour attendre que le client envoie un message
        int ret = poll(pollfds, 1, 2);
        if (ret == -1) {
            perror(ERROR("Impossible d'utiliser poll"));
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            // Pas de message du client
            continue;
        }
        if (pollfds[0].revents & POLLIN) {
            // On reçoit un message du client

            // on attend d'abord la taille du message puis le message
            size = allouer_size_t_dynamique(0);
            if(read(client->socket, size, sizeof(size_t))< 1){
                free(size); // On a plus besoin de cette variable donc on la libère
                error_report(client); //Cette fonction va déconnecter le client
                pthread_exit(NULL);
            }
            char *message = allouer_chaine_dynamique(*size + 1);
            if(read(client->socket, message, *size)< 1){
                free(size); // On a plus besoin de cette variable donc on la libère
                free(message);
                error_report(client); // Cette fonction va déconnecter le client
                pthread_exit(NULL);
            }




            printf(LOG("CHANNEL", "Le client [%s] a envoyé le message [%s]\n"), client->name, message);
            write_message_to_channel_file(client->channel,message, client->name);
            // On envoie le message à tous les clients du channel
            sendToAllClientsInChannel(client->name, message, client->channel->listeClients);

            free(size); // On a plus besoin de cette variable donc on la libère
            free(message); // On a plus besoin de cette variable donc on la libère

        }
        


    }
    


}


/*
    La fonction handleSIGINT est dédié à la fermeture du serveur, c'est ici que le serveur se ferme et que les clients sont déconnectés
    et toutes les ressources sont libérées
*/
void handleSIGINT(int signum) {
    // Fermez la socket et déconnectez tous les clients du hub
    printf("Fermerture des sockets du serveur\n");
    close(hub_sock);
    // On libère la mémoire de la liste des clients du hub
    for (int i = 0; i < hub_client_list->nbClients; i++) {
        printf(LOG("EXIT", "Deconnexion du client [%s]\n"), hub_client_list->clients[i]->name);
        close(hub_client_list->clients[i]->socket);
        free(hub_client_list->clients[i]);
    }
    free(hub_client_list->clients);
    // On destroy le mutex des channels
    pthread_mutex_destroy(&hub_client_list->listeClients_mutex);
    free(hub_client_list);

    // Fermez la socket et déconnectez tous les clients des channels
    for (int i = 0; i < channel_list->nb_channels; i++) {
        printf(LOG("EXIT", "Deconnexion des clients du channel [%s]\n"), channel_list->channels[i]->name);
        close(channel_list->channels[i]->socket);
        for (int j = 0; j < channel_list->channels[i]->listeClients->nbClients; j++) {
            printf(LOG("EXIT", "Deconnexion du client [%s]\n"), channel_list->channels[i]->listeClients->clients[j]->name);
            close(channel_list->channels[i]->listeClients->clients[j]->socket);
            free(channel_list->channels[i]->listeClients->clients[j]);
        }
        free(channel_list->channels[i]->listeClients->clients);
        pthread_mutex_destroy(&channel_list->channels[i]->listeClients->listeClients_mutex);
        free(channel_list->channels[i]->listeClients);
        free(channel_list->channels[i]);
        pthread_mutex_destroy(&channel_list->channels[i]->file_mutex);
    }
    free(channel_list->channels);
    pthread_mutex_destroy(&channel_list->listeClients_mutex);
    free(channel_list);
    
    // On arrête tous les threads
    for(int i = 0; i < nbThreads; i++){
        pthread_cancel(server[i]);
    }
    pthread_mutex_destroy(&ChannelCreateLock);

    
    printf("Nettoyage terminé\nFin du programme\n");
    exit(EXIT_SUCCESS);
}

/*

    La fonction main intialise le serveur et le met en écoute sur le port 8000 qui sera le port du hub, l'endroit où les clients se connecteront pour choisir un channel

*/
int main(int argc, char *argv[]){
    //initialise la liste des channels et on la remplie avec les channels existants
    channel_list = initListChannel();
    // On initialise la liste des clients du hub
    hub_client_list = initListeClients();
    if (pthread_mutex_init(&ChannelCreateLock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }


    // Ouverture d'un nouveau thread pour chaque channel
    for(int i = 0; i < channel_list->nb_channels; i++){
        pthread_create(&server[nbThreads], NULL, handleChannel,channel_list->channels[i]);
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
        addClientToList(current_hub_client, hub_client_list);
        printf(LOG("HUB", "Le client [%s] s'est connecté sur le hub\n"), current_hub_client->name);
        pthread_create(&server[nbThreads], NULL, handleHub, current_hub_client);
        nbThreads++;
    }

    return 0;
}