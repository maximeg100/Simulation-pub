#ifndef PROTOCOL_H
#define PROTOCOL_H
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

#define CMD_PSEUDO      "PSEUDO"       // PSEUDO|<nom>
#define CMD_GET_TABLES  "GET_TABLES"   // Demande la liste des tables
#define CMD_JOIN        "JOIN"         // JOIN|<id_table>
#define CMD_LEAVE_TABLE "LEAVE_TABLE"  // Quitte la table actuelle
#define CMD_VOTE        "VOTE"         // VOTE|YES ou VOTE|NO
#define CMD_CHAT        "CHAT"         // CHAT|<message>
#define CMD_GET_MENU    "GET_MENU"     // Demande la carte des boissons
#define CMD_CALL_WAITER "CALL_WAITER"  // Appelle un serveur à la table
#define CMD_READY       "READY"        // READY|<id_boisson> (Client prêt à commander)
#define CMD_QUIT        "QUIT"         // Quitte le pub

// Réponses Serveur -> Client
#define RSP_OK          "OK"
#define RSP_ERROR       "ERR"
#define RSP_TABLES      "TABLES_LIST"
#define RSP_MENU        "MENU_LIST"
#define RSP_CHAT        "CHAT_MSG"     // CHAT_MSG|<pseudo>|<message>
#define RSP_ASK_VOTE    "ASK_VOTE"     // ASK_VOTE|<id_candidat>|<pseudo>
#define RSP_WAITER      "WAITER_MSG"   // Message d'un serveur (ex: "Êtes-vous prêts ?")
#define RSP_DELIVER     "DELIVER"      // DELIVER|<nom_boisson>
#define RSP_KICK        "KICK"         // KICK|<raison>
#endif