# Chat en C avec serveur et clients

Ce projet consiste en un chat en C avec un serveur et des clients. Le serveur gère plusieurs canaux sur des ports différents (générés aléatoirement). Par défaut, le serveur utilise le port 8080 pour le hub.

## Instructions pour exécuter le programme

1. Accédez au dossier du serveur ou du client.
2. Exécutez le script "compile-and-run.sh" pour compiler et exécuter le programme.

## Serveur

Le serveur gère plusieurs canaux sur des ports différents. Vous pouvez spécifier le port du hub en tant qu'argument lors de l'exécution du serveur. Par défaut, le port du hub est 8080.

Le serveur crée un dossier "data" dans lequel il stocke les données des différents canaux. Si aucun canal n'existe, le serveur crée le canal "default".

Exemple d'exécution du serveur avec un port spécifié :

```bash
./compile-and-run.sh <port> [default=8080]
```


## Client

Le client peut se connecter à un canal spécifique en spécifiant le port du canal en tant qu'argument lors de l'exécution du client. Par défaut, le port du canal est 8080 (port du hub).

Exemple d'exécution du client avec un port spécifié :

```bash
./compile-and-run.sh <port> [default=8080]
```



## Poins clés

- ✔️ Créer un canal
- ✔️ connexion à un canal spécifique (par port)
- ✔️ envoyer un message / recevoir un message
- ✔️ sauvegarder les messages avec horodatage
- ✔️ allocation dynamique de la mémoire
- ✔️ utilisation de threads
- ✔️ manipulation d'objets
- ✔️ gestion des erreurs, des sockets, des signaux
- ✔️ aucun bug (y'en a, mais on va dire que non, le projet est parfait)

