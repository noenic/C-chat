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
#include "includes/helper.h"

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
    if (argc != 2)
    {
        server.sin_port = htons(8000);
    }
    else{
        server.sin_port = htons(atoi(argv[1]));
    }
    connect(sock, (struct sockaddr *)&server, sizeof(server));
    int serverType;
    read(sock, &serverType, sizeof(int));
    // Si on est connecté au HUB il faut choisir un channel
    if(serverType == HUB){
        // Si on est connecté au HUB il faut choisir un channel
        // la fonction handleHUB retourne le port du channel
        int port = handleHUB(sock);
        // On se connecte au channel
        sock = socket(AF_INET, SOCK_STREAM, 0);
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

    //  On attend que le serveur nous envoie le port du channel
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
    size_t *size = allouer_size_t_dynamique(0);
    printf("Entrez votre pseudo: ");
    char *pseudo = get_line(size);
    write(sock, size, sizeof(size_t));
    write(sock, pseudo, *size);

    // On attend que le serveur nous envoie un signal pour nous dire que le pseudo est valide (1 si valide, 0 sinon)
    int validPseudo = 1;
    read(sock, &validPseudo, sizeof(int));
    if (validPseudo == 1) {
        printf("Pseudo invalide, il est probablement déjà utilisé\n");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct pollfd pollfds[2];
    pollfds[0].fd = sock;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = STDIN_FILENO;
    pollfds[1].events = POLLIN;
    time_t t;
    struct tm *tm_info;
    int return_code = 1; // 0 si le message est une commande, 1 si un message normal, 2 si le client veut quitter le channel
    
    printf("Vous êtes connecté au channel...\n");
    // On attend un message du serveur sur l'historique des messages
    read(sock, size, sizeof(size_t));
    if (*size > 1) {
        printf("Historique des derniers messages:\n");
        char *message = allouer_chaine_dynamique(*size);
        read(sock, message, *size);
        printf("%s\n", message);
        free(message);
    }
    printf("[--:--] [%s] : ", pseudo);
    printf("\033[K");
    fflush(stdout);
    while (1)
    {
        int ret = poll(pollfds, 2, -1);
        if (ret == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (pollfds[0].revents & POLLIN)
        {
            // On attend un message du serveur
            if(read(sock, size, sizeof(size_t)) == 0){
                printf("Le serveur a fermé la connexion\n");
                free(size);
                free(pseudo);
                break;
            }
   
            char *message = allouer_chaine_dynamique(*size);
            if(read(sock, message, *size) == 0){
                printf("Le serveur a fermé la connexion\n");
                free(message);
                free(size);
                free(pseudo);
                break;
            }
            // On revient au début de la ligne
            printf("\r");
            // On efface la ligne actuelle
            printf("\033[K");
            // On obtient l'heure actuelle
            time(&t);
            tm_info = localtime(&t);
            // On affiche le message
            // [HH:MM] message
            printf("[%02d:%02d] %s\n", tm_info->tm_hour, tm_info->tm_min, message);
            printf("[--:--] [%s] : ", pseudo);
            printf("\033[K");
            fflush(stdout);


            free(message);
        }
        if (pollfds[1].revents & POLLIN)
        {
            
            // On efface la ligne actuelle
            char *message = get_line(size); 

            return_code = is_not_command(message,sock); // On vérifie si le message est une commande, si oui elle sera exécutée dans la fonction

            switch (return_code)
            {
                case 1:
                    // On affiche le message
                    printf("\033[1A\r"); // On revient au début de la ligne précédente pour changer l'heure
                    time(&t);
                    tm_info = localtime(&t);
                    printf("[%02d:%02d]\n", tm_info->tm_hour, tm_info->tm_min);
                    write(sock, size, sizeof(size_t));
                    write(sock, message, *size);
                    free(message);
                    break;
                case 2:
                    free(size);
                    free(message);
                    free(pseudo);
                    close(sock);
                    exit(EXIT_SUCCESS);
            }
            // On clear le cache
            printf("[--:--] [%s] : ", pseudo);
            printf("\033[K");
            fflush(stdout);
        }
    }
}