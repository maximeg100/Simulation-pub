#ifndef PUB_H
#define PUB_H

#include <pthread.h>
#include <stdbool.h>

#define NB_TABLES 10
#define CAPACITE_TABLE 4
#define NB_SERVEURS 3
#define NB_PRODUITS 5
#define MAX_COMMANDES_QUEUE 50

// --- ÉTATS ---
typedef enum {
    TABLE_LIBRE,
    TABLE_OCCUPEE,
    TABLE_ATTENTE_SERVEUR,
    TABLE_PRISE_COMMANDE,
    TABLE_ATTENTE_LIVRAISON,
    TABLE_SERVIE
} EtatTable;

typedef enum {
    SERVEUR_DISPONIBLE,
    SERVEUR_EN_DEPLACEMENT,
    SERVEUR_PRISE_COMMANDE,
    SERVEUR_TRANSMISSION,
    SERVEUR_LIVRAISON
} EtatServeur;

// --- STRUCTURES MÉTIER ---
typedef struct {
    int id;
    char nom[32];
    char recette[64];
    int temps_preparation; // en secondes
    int stock;
} Produit;

typedef struct {
    int id_client;
    int socket_fd;
    char pseudo[32];
    int id_table;          // -1 si pas à une table
    int boisson_choisie;   // -1 si pas prêt
    int nb_consommations;  // Pour règle d'expulsion
    pthread_t thread;
    bool actif;
} ClientConnecte;

typedef struct {
    int id;
    int capacite_max;
    int nb_clients;
    ClientConnecte *clients[CAPACITE_TABLE];
    EtatTable etat;
    int id_serveur_affecte; // -1 si aucun

    // Synchronisation pour vote d'accès et protection de la table
    pthread_mutex_t mutex;
    pthread_cond_t cond_vote;
    int dernier_vote; // 0 = en attente, 1 = accepté, -1 = refusé
} Table;

typedef struct {
    int id_commande;
    int id_table;
    int id_serveur;
    int ids_produits[CAPACITE_TABLE];
    int nb_produits;
    bool est_prete;
} Commande;

typedef struct {
    int id;
    EtatServeur etat;
    pthread_t thread;
    // Notification Barman -> Serveur quand la commande est prête
    pthread_mutex_t mutex_notif;
    pthread_cond_t cond_commande_prete;
    Commande *commande_en_cours;
} Serveur;

// --- ÉTAT GLOBAL DU PUB ---
typedef struct {
    Table tables[NB_TABLES];
    Serveur serveurs[NB_SERVEURS];
    Produit carte[NB_PRODUITS];
    pthread_mutex_t mutex_stock;

    // File d'attente Producteur-Consommateur (Serveurs -> Barman)
    Commande file_commandes[MAX_COMMANDES_QUEUE];
    int nb_commandes_attente;
    pthread_mutex_t mutex_file_barman;
    pthread_cond_t cond_nouvelle_commande;

    // Réveil d'un serveur lors d'un appel table
    pthread_mutex_t mutex_appel_serveur;
    pthread_cond_t cond_appel_serveur;

    bool pub_ouvert;
} Pub;

// Déclaration de la variable globale partagée par le serveur
extern Pub mon_pub;

// --- PROTOTYPES DES FONCTIONS DU SERVEUR ---
// tables.c (Étudiant 2)
void init_tables(void);
void get_tables_status(char *buffer, size_t max_len);
int join_table(ClientConnecte *client, int id_table);
void leave_table(ClientConnecte *client);
void broadcast_chat(int id_table, int id_sender, const char *msg);

// waiter.c (Étudiant 3)
void* thread_serveur(void *arg);

// barman.c (Étudiant 3)
void* thread_barman(void *arg);

// client_handler.c (Étudiant 1)
void* thread_client_handler(void *arg);

#endif