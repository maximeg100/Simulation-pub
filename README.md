# Simulation Pub
Simulation d'un pub dans lequel plusieurs clients interagissent avec le personnel de l'établissement.


## Structure et Rôle des Fichiers

### 1. Racine du Projet
* **`Makefile`** : Script d'automatisation de la compilation. Compile les exécutables `pub` et `client` avec les bibliothèques requises (`-pthread`), prépare le dossier des logs et nettoie le projet (`clean`).
* **`.gitignore`** : Fichier de configuration Git excluant les artefacts de compilation (binaires `pub`, `client`, fichiers objets `.o`) ainsi que les journaux d'exécution (`logs/*.log`).
* **`README.md`** : Documentation générale du projet décrivant l'architecture, le protocole réseau et le mode d'emploi pour la compilation et le lancement.

---

### 2. Module Commun (`common/`)
Contient le code partagé entre l'application serveur (`pub`) et l'application cliente (`client`).

* **`common/protocol.h`** : Contrat de communication réseau. Définit le port par défaut (8080), la taille des tampons, ainsi que l'ensemble des commandes textuelles échangées via les sockets TCP (`PSEUDO`, `GET_TABLES`, `JOIN`, `CHAT`, `READY`, `ASK_VOTE`, `KICK`, etc.).
* **`common/logger.h`** : Fichier d'en-tête du module de journalisation. Déclare les prototypes des fonctions d'initialisation, d'écriture sécurisée et de fermeture des fichiers de traces.
* **`common/logger.c`** : Implémentation du système d'historique thread-safe. Utilise un verrou (`pthread_mutex_t`) pour horodater et écrire de manière concurrente chaque événement à la fois sur le terminal et dans un fichier `.log` sans collision d'écriture.

---

### 3. Module Serveur (`server/`)
Regroupe le cœur applicatif de la simulation du pub, la gestion des threads et de la synchronisation concurrente.

* **`server/pub.h`** : En-tête central de l'architecture du serveur. Contient les constantes de dimensionnement (10 tables, 3 serveurs, carte des boissons), les énumérations d'états (tables et serveurs), toutes les structures de données (`Table`, `ClientConnecte`, `Commande`, `Serveur`, `Pub`) et l'ensemble des verrous (`pthread_mutex_t`) et conditions (`pthread_cond_t`).
* **`server/main_pub.c`** : Point d'entrée de l'application serveur. Configure la socket d'écoute TCP (`socket`, `bind`, `listen`), initialise l'état global du pub (tables, carte, stocks), démarre les threads de fond (barman et serveurs) et gère la boucle d'attente des nouvelles connexions (`accept`).
* **`server/client_handler.c`** : Gestionnaire individuel des connexions entrantes. Chaque client connecté possède un thread exécutant ce code pour écouter les messages reçus sur la socket, parser les requêtes textuelles selon le protocole et appeler les fonctions métier associées.
* **`server/tables.c`** : Logique de la salle et du salon de discussion. Gère l'initialisation des tables, le calcul des disponibilités, le mécanisme de vote/accord pour intégrer une table occupée, ainsi que la diffusion des messages (`CHAT`) entre clients assis à une même table.
* **`server/waiter.c`** : Comportement des serveurs (3 threads concurrents). Gère les déplacements en salle, la vérification de l'état de préparation des clients, la prise de commande groupée, la transmission en cuisine, la livraison des boissons et l'expulsion éventuelle des clients inactifs.
* **`server/barman.c`** : Gestionnaire du bar (thread unique). Traite la file d'attente des commandes selon un modèle Producteur-Consommateur, décrémente les stocks de manière exclusive, simule le temps de préparation des recettes et avertit le serveur assigné dès que le plateau est prêt.

---

### 4. Module Client (`client/`)
* **`client/main_client.c`** : Point d'entrée du client console. Assure la connexion TCP au serveur et fait tourner deux flux concurrents :
  1. Un thread d'écoute réseau en arrière-plan qui reçoit et affiche les messages du serveur (chat, notifications de vote, boissons reçues, alertes).
  2. Un thread principal de lecture clavier permettant à l'utilisateur de saisir son pseudo, choisir une table, discuter ou passer commande sans bloquer l'affichage.
