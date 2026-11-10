#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void logger_init(const char *filename) {
    // ouvre le fichier log si il est pas NULL et qu'il est unlock
    pthread_mutex_lock(&log_mutex);
    if (log_file == NULL){
        log_file = fopen(filename, "a");
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_event(const char *source, const char *message){
    //permet a n'importe quel thread d'enregistrer une action sans bug
    pthread_mutex_lock(&log_mutex);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[16];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    
    printf("[%s] [%s] %s\n", time_str, source, message);

    if (log_file != NULL) {
        fprintf(log_file, "[%s] [%s] %s\n", time_str, source, message); // ecriture dans le fichier de log
        fflush(log_file); // force l'enregistrement immediat
    }

    pthread_mutex_unlock(&log_mutex);

}

void logger_close(void) {
    pthread_mutex_lock(&log_mutex);
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
}


// === CODE TEMPORAIRE DE TEST ===
int main(void) {
    // 1. Initialiser le fichier de log
    logger_init("test_logger.log");

    // 2. Écrire des messages de test
    log_event("SYSTEM", "Demarrage du test du logger");
    
    char buffer[128];
    int id_table = 3;
    snprintf(buffer, sizeof(buffer), "Le client Alice a rejoint la table %d", id_table);
    log_event("TABLE", buffer);

    log_event("BARMAN", "Preparation d'une biere terminee");

    // 3. Fermer le fichier
    logger_close();

    printf("--> Test termine avec succes !\n");
    return 0;
}