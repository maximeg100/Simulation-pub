#ifndef LOGGER_H
#define LOGGER_H

// Initialise le logger et ouvre le fichier de log spécifié
void logger_init(const char *filename);

// Écrit une ligne horodatée et thread-safe dans le log et sur stdout
void log_event(const char *source, const char *message);

// Ferme le fichier de log à l'arrêt du programme
void logger_close(void);

#endif