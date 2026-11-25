#include "pub.h"
#include "../common/protocol.h"
#include "../common/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <string.h>

void* thread_serveur(void *arg) {
    //serveur en attente d'appel et attribution de table
    Serveur *moi = (Serveur *)arg;
    char log_msg[256];

    snprintf(log_msg, sizeof(log_msg), "Le serveur #%d commence son service.", moi->id);
    log_event("SERVEUR", log_msg);

    while (mon_pub.pub_ouvert) {
        int id_table_cible = -1;
        pthread_mutex_lock(&mon_pub.mutex_appel_serveur);
        while (id_table_cible == -1 && mon_pub.pub_ouvert) {
            for (int i =0; i< NB_TABLES; i++){
                pthread_mutex_lock(&mon_pub.tables[i].mutex);

                if (mon_pub.tables[i].etat == TABLE_ATTENTE_SERVEUR && mon_pub.tables[i].id_serveur_affecte == -1){
                    mon_pub.tables[i].id_serveur_affecte = moi->id;
                    mon_pub.tables[i].etat = TABLE_PRISE_COMMANDE;
                    id_table_cible = i;
                    pthread_mutex_unlock(&mon_pub.tables[i].mutex);
                    break;
                }
                pthread_mutex_unlock(&mon_pub.tables[i].mutex);
            }
            // Si après le tour des 10 tables, id_table_cible est toujours -1 :
            if (id_table_cible == -1 && mon_pub.pub_ouvert) {
                pthread_cond_wait(&mon_pub.cond_appel_serveur, &mon_pub.mutex_appel_serveur);
            }
        }
        pthread_mutex_unlock(&mon_pub.mutex_appel_serveur);

        // Si le pub a fermé pendant l'attente, on sort
        if (!mon_pub.pub_ouvert) {
            break;
        }

        //le serveur se deplace et verifie si les clients sont prets
        moi->etat = SERVEUR_EN_DEPLACEMENT;
        snprintf(log_msg, sizeof(log_msg), "Le serveur #%d se déplace vers la table %d.", moi->id, id_table_cible);
        log_event("SERVEUR", log_msg);
        sleep(2); //simule déplacement

        moi->etat = SERVEUR_PRISE_COMMANDE;
        Table *tbl = &mon_pub.tables[id_table_cible];
        pthread_mutex_lock(&tbl->mutex);

        Commande cmd;
        cmd.id_table = id_table_cible;
        cmd.id_serveur = moi->id;
        cmd.nb_produits = 0;
        cmd.est_prete = false;

        bool tous_prets = true;
        for (int i =0; i<tbl->nb_clients;i++) {
            ClientConnecte *cli = tbl->clients[i];
            if (cli != NULL) {
                if (cli->boisson_choisie == -1){
                    tous_prets = false;
                } else {
                    cmd.ids_produits[cmd.nb_produits] = cli->boisson_choisie;
                    cmd.nb_produits++;
                    cli->boisson_choisie = -1;
                }
            }
        }
        if (tous_prets == false || cmd.nb_produits == 0) { //si les clients partent,a verifier
            const char *msg = RSP_WAITER "|Tout le monde n'est pas pret!\n";
            for (int i = 0; i < tbl->nb_clients; i++) {
                if (tbl->clients[i] != NULL) {
                    send(tbl->clients[i]->socket_fd, msg, strlen(msg), 0);
                }
            }
            tbl->etat = TABLE_OCCUPEE;
            tbl->id_serveur_affecte = -1;
            pthread_mutex_unlock(&tbl->mutex);

            moi->etat = SERVEUR_DISPONIBLE;
            continue;
        }

        tbl->etat = TABLE_ATTENTE_LIVRAISON;
        pthread_mutex_unlock(&tbl->mutex);

        //le serveur transmet au barman la commande au barman
        moi->etat = SERVEUR_TRANSMISSION;
        pthread_mutex_lock(&moi->mutex_notif);
        moi->commande_en_cours = &cmd;
        pthread_mutex_unlock(&moi->mutex_notif);
        pthread_mutex_lock(&mon_pub.mutex_file_barman);
        if (mon_pub.nb_commandes_attente < MAX_COMMANDES_QUEUE){
            static int compteur_cmd = 1;
            cmd.id_commande = compteur_cmd++;
            mon_pub.file_commandes[mon_pub.nb_commandes_attente] = cmd;
            mon_pub.nb_commandes_attente++;

            pthread_cond_signal(&mon_pub.cond_nouvelle_commande);
        }  
        pthread_mutex_unlock(&mon_pub.mutex_file_barman); 

        //le serveur attend la préparation du barman
        pthread_mutex_lock(&moi->mutex_notif);
        while (!cmd.est_prete && mon_pub.pub_ouvert) {
            pthread_cond_wait(&moi->cond_commande_prete, &moi->mutex_notif);
        }
        moi->commande_en_cours = NULL;
        pthread_mutex_unlock(&moi->mutex_notif);

        //le serveur livre les verres
        moi->etat = SERVEUR_LIVRAISON;
        sleep(2); //simulation de déplacement 
        pthread_mutex_lock(&tbl->mutex);

        for (int i =0; i<tbl->nb_clients;i++){
            const char *msg = RSP_DELIVER "| Votre Commande est servie !\n";
            if (tbl->clients[i] != NULL){
                send(tbl->clients[i]->socket_fd, msg, strlen(msg), 0);
                tbl->clients[i]->nb_consommations++;
            }
            
        }
        tbl->etat = TABLE_SERVIE;
        tbl->id_serveur_affecte = -1;
        pthread_mutex_unlock(&tbl->mutex);

        moi->etat = SERVEUR_DISPONIBLE;
    }

    snprintf(log_msg, sizeof(log_msg), "Le serveur #%d termine son service.", moi->id);
    log_event("SERVEUR", log_msg);
    return NULL;
}