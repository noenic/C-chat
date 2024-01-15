#ifndef CHANNELS_H
#define CHANNELS_H
/*

    Ce fichier contient les fonctions permettant de gérer les channels.
        - Le fichier contenant les channels (channels.txt) 
        - La structure d'un channel
        - Les fonctions permettant de gérer les channels
        - les fichiers pour sauvegarder les messages des channels
        

*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "client.h"



// Structure d'un channel
typedef struct channel
{
    char *name;
    int port;
    int socket;
    struct listeClients *listeClients;
    pthread_mutex_t file_mutex;
} channel;


typedef struct listChannel
{
    int nb_channels;
    struct channel** channels;
    pthread_mutex_t listeClients_mutex;
} listChannel;


/*
    * Initialise la liste des channels et on la remplie avec les channels existants dans le fichier channels.txt
    * @return La liste des channels
*/
listChannel *initListChannel();


/*
    * Crée un channel
    * @param name : nom du channel
    * @param port : port du channel
    * @return Le channel créé
*/
channel *create_channel(const char *name, const int port);


/*
    * Ajoute un channel à la liste des channels
    * @param listChannel : liste des channels
    * @param channel : channel à ajouter
*/
void add_channel(listChannel *listChannel,const channel *c);


/*
    * Retourne un channel à partir de son nom
    * @param listChannel : liste des channels
    * @param name : nom du channel
    * @return Le channel
    * @return NULL si le channel n'existe pas
*/
channel *getChannelByName(const listChannel *listChannel,const char *name);



/*
    * Créé un nouveau fichier dans le dossier data/channels avec le nom du channel
    * @param channel : le channel
    * @param message : le message à écrire dans le fichier
    * @param sender : le nom de l'envoyeur du message
*/
void create_channel_file(const channel *c);


/*
    * Ecrit un message dans le fichier du channel
    * @param channel : le channel
    * @param message : le message à écrire dans le fichier
    * @param sender : le nom de l'envoyeur du message
*/
void write_message_to_channel_file(channel *c, const char *message, const char *sender);



/*
    * retourne les 10 derniers messages du channel
    * @param c : le channel
    * @return char* : l'historique du channel
*/

char *read_channel_file(channel *c);



#endif // CHANNELS_H