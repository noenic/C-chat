#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include "../includes/channels.h"
#include "../includes/helper.h"




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
        // On créé le dossier data
        mkdir("data", 0777);
    }

    // Ouvrir le répertoire
    if ((dir = opendir("data")) != NULL)
    {
        listChannel *list = malloc(sizeof(listChannel));
        if (list == NULL)
        {
            perror(ERROR("Erreur lors de l'allocation dynamique pour la liste des channels"));
            free(list);
            exit(EXIT_FAILURE);
        }
        list->nb_channels = 0;
        list->channels = malloc(sizeof(channel));
        // On initialise le mutex de la liste des clients
        if (pthread_mutex_init(&list->listeClients_mutex, NULL) != 0) {
            perror(ERROR("Erreur lors de l'initialisation du mutex de la liste des clients"));
            free(list->channels);
            free(list);
            exit(EXIT_FAILURE);
        }

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
        perror(ERROR("Impossible d'ouvrir le dossier data"));
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

    if (pthread_mutex_init(&c->file_mutex, NULL) != 0) {
        perror(ERROR("Erreur lors de l'initialisation du mutex du fichier du channel"));
        free(c->name);
        free(c);
        exit(EXIT_FAILURE);
    }


    

    create_channel_file(c);
    return c ;
}


/*
    * Ajoute un channel à la liste des channels
    * @param listChannel : liste des channels
    * @param channel : channel à ajouter
*/
void add_channel(listChannel *listChannel, const channel *c) {
    // On ferme le mutex de la liste des channels
    pthread_mutex_lock(&listChannel->listeClients_mutex);
    channel **newChannels = realloc(listChannel->channels, (listChannel->nb_channels + 1) * sizeof(struct channel*));
    if (newChannels == NULL) {
        perror(ERROR("Erreur lors de la réallocation de mémoire pour la liste des channels"));
        return;
    }

    listChannel->channels = newChannels;
    listChannel->channels[listChannel->nb_channels] = (struct channel*)c;
    
    listChannel->nb_channels++;
    // On ouvre le mutex de la liste des channels
    pthread_mutex_unlock(&listChannel->listeClients_mutex);
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
        if(strcmp(listChannel->channels[i]->name, name) == 0){
            return listChannel->channels[i];
        }
    }
    return NULL;
}


/*
    * Crée un nouveau fichier dans le dossier data avec le nom du channel
    * @param channel : le channel
*/
void create_channel_file(const channel *c) {
    // On créé le chemin du fichier
    char *path = allouer_chaine_dynamique(strlen(c->name) + 16);
    strcpy(path, "data/");
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
    free(path);
}


void write_message_to_channel_file(channel *c, const char *message, const char *sender) {
    // On ferme le mutex du fichier
    pthread_mutex_lock(&c->file_mutex);

    // On créé le chemin du fichier
    char *path = allouer_chaine_dynamique(strlen(c->name) + 16);
    strcpy(path, "data/");
    strcat(path, c->name);
    
    // On ouvre le fichier
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        perror(ERROR("Impossible d'ouvrir le fichier du channel"));
        exit(EXIT_FAILURE);
    }
    free(path);

    // On obtient l'heure actuelle
    time_t t;
    struct tm *tm_info;
    time(&t);
    tm_info = localtime(&t);

    // On écrit le message dans le fichier [HH:MM] [sender] : message
    fprintf(file, "[%02d:%02d] [%s] : %s\n", tm_info->tm_hour, tm_info->tm_min, sender, message);

    // On ferme le fichier
    fclose(file);
    // On ouvre le mutex du fichier
    pthread_mutex_unlock(&c->file_mutex);
}



/*
    * retourne les 10 derniers messages du channel
    * @param c : le channel
    * @return char* : l'historique du channel
*/

char *read_channel_file(channel *c) {
    // On ferme le mutex du fichier
    pthread_mutex_lock(&c->file_mutex);

    // On créé le chemin du fichier
    char *path = allouer_chaine_dynamique(strlen(c->name) + 16);
    strcpy(path, "data/");
    strcat(path, c->name);

    // On ouvre le fichier
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror(ERROR("Impossible d'ouvrir le fichier du channel"));
        exit(EXIT_FAILURE);
    }

    // Allouer un tampon pour contenir le contenu du fichier
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = (char *)malloc(file_size + 1);

    // Lire le contenu du fichier dans le tampon
    fread(content, 1, file_size, file);
    content[file_size] = '\0';

    // On ferme le fichier
    fclose(file);
    // On ouvre le mutex du fichier
    pthread_mutex_unlock(&c->file_mutex);

    free(path);

    return content;
}









