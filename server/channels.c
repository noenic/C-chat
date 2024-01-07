#include "channels.h"
#include <string.h>
#include <unistd.h>
#include "helper.h"
#include <sys/stat.h>
#include <dirent.h>




/*
    * Initialise la liste des channels et on la remplie avec les channels existants dans le fichier channels.txt
    * @return La liste des channels
*/
listChannel *initListChannel(){
    DIR *dir;
    struct dirent *ent;

    // On vérifie si le dossier data existe
    if (access("data", F_OK) != 0)
    {
        // On créé le dossier data/channels
        mkdir("data", 0777);
    }


    // On vérifie si le dossier data/channels existe
    if (access("data/channels", F_OK) != 0)
    {
        // On créé le dossier data/channels
        mkdir("data/channels", 0777);
    }

    // Ouvrir le répertoire
    if ((dir = opendir("data/channels")) != NULL)
    {
        listChannel *list = malloc(sizeof(listChannel));
        if (list == NULL)
        {
            perror(ERROR("Erreur lors de l'allocation dynamique pour la liste des channels"));
            exit(EXIT_FAILURE);
        }
        list->nb_channels = 0;
        list->channels = malloc(sizeof(channel));
        // Lire chaque fichier du répertoire
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            {
                channel *chan = create_channel(ent->d_name, 0);
                add_channel(list, chan);

                printf(LOG("CHANNEL", "Le channel [%s] a été créé\n"), ent->d_name);
            }
        }

        // Fermer le répertoire
        closedir(dir);

        // Si le répertoire est vide, ajouter le fichier par défaut
        if (list->nb_channels == 0)
        {
            channel *default_channel = create_channel("default", 0);
            printf(LOG("CHANNEL", "Le channel [default] a été créé (par défaut)\n"));
            add_channel(list, default_channel);

        }
        return list;
    }
    else
    {
        // Erreur lors de l'ouverture du répertoire
        perror(ERROR("Impossible d'ouvrir le dossier data/channels"));
        exit(EXIT_FAILURE);
    }

}

/*
    * Crée un channel
    * @param name : nom du channel
    * @param port : port du channel
    * @return Le channel créé
*/
channel *create_channel(const char *name, const int port){
    channel *c = malloc(sizeof(channel));
    c->name = malloc(strlen(name)+1);
    strcpy(c->name, name);
    c->listeClients = initListeClients();
    c->port = port;

    c->socket = -1; // -1 car le channel n'est pas encore connecté

    create_channel_file(c);
    return c ;
}


/*
    * Ajoute un channel à la liste des channels
    * @param listChannel : liste des channels
    * @param channel : channel à ajouter
*/
void add_channel(listChannel *listChannel, const channel *c) {
    channel *newChannels = realloc(listChannel->channels, (listChannel->nb_channels + 1) * sizeof(channel));
    // On crée newChannels pour éviter de perdre le pointeur 
    if (newChannels == NULL) {
        perror(ERROR("Erreur lors de la réallocation de mémoire pour la liste des channels"));
        return;
    }
    listChannel->channels = newChannels;
    listChannel->channels[listChannel->nb_channels] = *c;
    listChannel->nb_channels++;
}




/*
    * Retourne un channel à partir de son nom
    * @param listChannel : liste des channels
    * @param name : nom du channel
    * @return Le channel
    * @return NULL si le channel n'existe pas
*/
channel *getChannelByName(const listChannel *listChannel,const char *name){
    for(int i = 0; i < listChannel->nb_channels; i++){
        if(strcmp(listChannel->channels[i].name, name) == 0){
            return &listChannel->channels[i];
        }
    }
    return NULL;
}


/*
    * Crée un nouveau fichier dans le dossier data/channels avec le nom du channel
    * @param channel : le channel
*/
void create_channel_file(const channel *c) {
    // On créé le chemin du fichier
    char *path = allouer_chaine_dynamique(strlen(c->name) + 16);
    strcpy(path, "data/channels/");
    strcat(path, c->name);
    
    // On vérifie si le fichier existe déjà
    if (access(path, F_OK) != 0) {
        // On créé le fichier
        FILE *file = fopen(path, "w");
        if (file == NULL) {
            perror(ERROR("Impossible de créer le fichier du channel"));
            exit(EXIT_FAILURE);
        }
        fclose(file);
    }
}


void write_message_to_channel_file(const channel *c, const char *message, const char *sender) {
    // On créé le chemin du fichier
    char *path = allouer_chaine_dynamique(strlen(c->name) + 16);
    strcpy(path, "data/channels/");
    strcat(path, c->name);
    
    // On ouvre le fichier
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        perror(ERROR("Impossible d'ouvrir le fichier du channel"));
        exit(EXIT_FAILURE);
    }

    // On écrit le message dans le fichier [sender] : message
    fprintf(file, "[%s] : %s\n", sender, message);

    // On ferme le fichier
    fclose(file);
}
