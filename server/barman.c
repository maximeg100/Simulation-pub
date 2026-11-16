#include "pub.h"
#include "../common/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* thread_barman(void *arg){
    (void)arg; //eviter warning

    //tant que le pub est ouvert ca tourne
    while (mon_pub.pub_ouvert){
        //attente de commande
        pthread_mutex_lock(&mon_pub.mutex_file_barman);
        while (mon_pub.nb_commandes_attente == 0 && mon_pub.pub_ouvert) {
            pthread_cond_wait(&mon_pub.cond_nouvelle_commande, &mon_pub.mutex_file_barman);
        }

        if (!mon_pub.pub_ouvert && mon_pub.nb_commandes_attente == 0){
            pthread_mutex_unlock(&mon_pub.mutex_file_barman);
            break;
        }
        //enlever commande de la pile
        Commande cmd = mon_pub.file_commandes[0];
        for (int i =0;i<mon_pub.nb_commandes_attente-1;i++){
            mon_pub.file_commandes[i] = mon_pub.file_commandes[i + 1];
        }
        mon_pub.nb_commandes_attente--;
        pthread_mutex_unlock(&mon_pub.mutex_file_barman);

        //verifier stock et le decrementer
        int temps_total = 0;
        char log_msg[256];
        pthread_mutex_lock(&mon_pub.mutex_stock);
        for (int i = 0; i < cmd.nb_produits; i++) {
            int id_prod = cmd.ids_produits[i];
            if (id_prod >= 0 && id_prod < NB_PRODUITS) {
                if (mon_pub.carte[id_prod].stock > 0) {
                    mon_pub.carte[id_prod].stock--;
                    temps_total += mon_pub.carte[id_prod].temps_preparation;

                    snprintf(log_msg, sizeof(log_msg), 
                            "Preparation de '%s' (Commande #%d, Table %d) - Stock restant : %d",
                            mon_pub.carte[id_prod].nom, 
                            cmd.id_commande, 
                            cmd.id_table, 
                            mon_pub.carte[id_prod].stock);
                    log_event("BARMAN", log_msg);
                } else {
                    snprintf(log_msg, sizeof(log_msg), 
                            "Rupture de stock pour '%s' (Commande #%d, Table %d)",
                            mon_pub.carte[id_prod].nom, 
                            cmd.id_commande, 
                            cmd.id_table);
                    log_event("BARMAN", log_msg);
                }
            }
        }
        pthread_mutex_unlock(&mon_pub.mutex_stock);

        //simulation du temps de travail
        sleep(temps_total);

        // 5. Prévenir le serveur concerné que la commande est prête
        int id_srv = cmd.id_serveur;

        if (id_srv >= 0 && id_srv < NB_SERVEURS) {
            Serveur *srv = &mon_pub.serveurs[id_srv];
            pthread_mutex_lock(&srv->mutex_notif);

            if (srv->commande_en_cours != NULL) {
                srv->commande_en_cours->est_prete = true;
            }

            snprintf(log_msg, sizeof(log_msg), 
                    "Commande #%d (Table %d) prete ! Notification du serveur %d.",
                    cmd.id_commande, cmd.id_table, id_srv);
            log_event("BARMAN", log_msg);

            pthread_cond_signal(&srv->cond_commande_prete);
            pthread_mutex_unlock(&srv->mutex_notif);
        }
    }
    log_event("BARMAN", "Le barman quitte son poste.");
    return NULL;
}