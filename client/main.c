#define _GNU_SOURCE // pour eviter les errors de précompilation
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include "helper.h"

#define HUB 200
#define CHANNEL 201

int handleHUB(int sock);
int handleCHANNEL(int sock);
struct sockaddr_in server;

int main(int argc, char *argv[])
{
 
    struct hostent *hp;
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    hp = gethostbyname("localhost");
    server.sin_family = AF_INET;
    server.sin_addr=*((struct in_addr *)hp->h_addr);
    server.sin_port = htons(atoi(argv[1]));
    connect(sock, (struct sockaddr *)&server, sizeof(server));
    int serverType;
    read(sock, &serverType, sizeof(int));
    // Si on est connecté au HUB il faut choisir un channel
    if(serverType == HUB){
        int port = handleHUB(sock);
        // On se connecte au channel
        sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server;
        struct hostent *hp;
        hp = gethostbyname("localhost");
        server.sin_family = AF_INET;
        server.sin_addr=*((struct in_addr *)hp->h_addr);
        server.sin_port = htons(port);
        connect(sock, (struct sockaddr *)&server, sizeof(server));
        read(sock, &serverType, sizeof(int));
        if (serverType == CHANNEL){
            handleCHANNEL(sock);
        }
        else{
            printf("Erreur lors de la connexion au channel\n");
        }

    }
    else if(serverType == CHANNEL){ 
        handleCHANNEL(sock);
    }
    close(sock);
    return 0;
}







int handleHUB(int sock){
    printf("Vous êtes connecté au hub...\nRécupération des channels...\n\n");
    int nbChannels; // Le serveur nous envoie le nombre de channels
    int size;       // Le serveur nous envoie la taille de la liste des channels

    read(sock, &nbChannels, sizeof(int)); // On récupère le nombre de channels

    read(sock, &size, sizeof(int)); // On récupère la taille de la liste des channels

    char *channels = allouer_chaine_dynamique(size); // On alloue la mémoire pour la liste des channels
    read(sock, channels, size); // On récupère la liste des channels
    char **channelsList = allouer_liste_dynamique(nbChannels); // On alloue la mémoire pour la liste des channels
    char *token = strtok(channels, "\n"); // On découpe la chaîne de caractères en fonction des retours à la ligne
    int i = 0;
    while(token != NULL){
        channelsList[i] = allouer_chaine_dynamique(strlen(token) + 1); // On alloue la mémoire pour le nom du channel
        strcpy(channelsList[i], token); // On copie le nom du channel dans la liste
        token = strtok(NULL, "\n"); // On passe au channel suivant
        i++;
    }
    free(channels); // On libère la mémoire de la chaîne de caractères

    printf("Liste des channels disponibles:\n");
    for(int i = 0; i < nbChannels; i++){
        printf("- %s\n", channelsList[i]);
    }
    printf("\n");
    printf("Choisissez un channel,créez-en écrivant un nom pas encore utilisé\nVotre choix: ");
    size_t sizeChoice;
    char *choice = get_line(&sizeChoice); // On récupère le choix du client
    int found = 0;
    for(int i = 0; i < nbChannels; i++){
        if(strcmp(choice, channelsList[i]) == 0){
            found = 1;
            break;
        }
    }
    free_liste_dynamique(channelsList, nbChannels); // On libère la mémoire de la liste des channels
    // Si le channel n'existe pas on le crée
    if(!found){
        printf("Création du channel [%s]...\n", choice);
    }
    else{
        printf("Connexion au channel [%s]...\n", choice);
    }
    write(sock, choice, sizeChoice); // On envoie le choix du client au serveur
    free(choice); // On libère la mémoire de la variable choice

    // // On attend que le serveur nous envoie le port du channel
    int port;
    read(sock, &port, sizeof(int));
    printf("Port du channel: %d\n", port);
    // On ferme le socket
    close(sock);
    return port;
}

int handleCHANNEL(int sock){
    printf("Vous êtes connecté au channel...\n");
    //  On lui envoie la taille du pseudo pour qu'il puisse allouer la mémoire
    size_t sizePseudo;
    printf("Entrez votre pseudo: ");
    char *pseudo = get_line(&sizePseudo);
    sizePseudo = strlen(pseudo)+1;
    write(sock, &sizePseudo, sizeof(size_t));
    write(sock, pseudo, sizePseudo);

    // On attend que le serveur nous envoie un signal pour nous dire que le pseudo est valide (1 si valide, 0 sinon)
    int validPseudo = 0;
    read(sock, &validPseudo, sizeof(int));

    printf("validPseudo: %d\n", validPseudo);
    if (validPseudo == 0) {
        printf("Pseudo invalide, il est probablement déjà utilisé\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct pollfd pollfds[2];
    pollfds[0].fd = STDIN_FILENO;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = sock;
    pollfds[1].events = POLLIN;
    size_t sizeMessage;

    while (1)
    {
        fflush(stdout);
        int ret = poll(pollfds, 2, 2);

        if (ret == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if (ret == 0)

        {
        
            // Effacer la ligne précédente
            printf("\033[2K");
            // Revenir au début de la ligne
            printf("\r");
            // Obtenir la position actuelle du curseur
            int cursorPosition = 0;
            // ioctl(STDOUT_FILENO, TIOCGWINSZ, &cursorPosition);

            char *message = get_line(&sizeMessage); 
            // Envoyer la taille du message au serveur
            size_t sizeMessage = strlen(message) + 1;
            write(sock, &sizeMessage, sizeof(size_t));

            // Envoyer le message au serveur
            write(sock, message, sizeMessage);

            // Revenir à la position initiale du curseur
            printf("\033[%dD", cursorPosition);

            if (strcmp(message, "/quit") == 0)
            {
                break;
            }
        
            
        }

        if (pollfds[1].revents & POLLIN) {

            // Lire la taille du message
            read(sock, &sizeMessage, sizeof(size_t));

            // Allouer dynamiquement de la mémoire pour le message
            char *receivedMessage = (char *)malloc(sizeMessage);
            if (receivedMessage == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            // Lire le message
            read(sock, receivedMessage, sizeMessage);
            // On desactive l'event POLLIN pour ne pas recevoir le message qu'on vient d'envoyer
            pollfds[1].events = 0;
            printf("%s\n", receivedMessage);


            // Libérer la mémoire allouée dynamiquement
            free(receivedMessage);
            // On reactive l'event POLLIN
            pollfds[1].events = POLLIN;
        }
    }

    return 0;
}