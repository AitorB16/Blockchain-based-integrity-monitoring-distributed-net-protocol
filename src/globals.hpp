#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/* Node ID for local deploys */  
extern int ID;

/* CONFIGURATION FOLDER PATH */ 
#define CONFIG_FOLDER "../XML_config/"
#define KEYS_FOLDER "../RSA_keys/"

/* EXECUTION MODES */ 
#define DEFAULT_MODE 0
#define DEBUG_MODE 1

/* Global exec var, loaded from config */ 
extern int EXEC_MODE;

/* TIMESPACE TO ACHIEVE CONSISTENCY AT BEGINNING */ 
#define TIME_SPACE_BEFORE_AUDIT 60 //seconds

/* APPROXIMATE TIME TO DEPLOY THE WHOLE NETWORK */ 
#define DEPLOY_TIME 30 //seconds

/* MAX TIME TO CONSIDER AS DISCONNECTED */ 
#define RESPONSE_DELAY_MAX 2 //seconds

/* TRUST CONTROL */ 
extern int TRUST_LEVEL;
#define TRUST_DECREASE 1

extern int MAX_INCIDENTS;
#define INCIDENT_INCREASE 1
extern int TIME_RESET_INCIDENTS;

/* NETWORK CONTROL */ 
#define HASH_UPDATE_TIMESPACE_MAX 300 //seconds
#define HASH_UPDATE_TIMESPACE_MIN 30 //seconds

/* Percentage of net failing accepted */ 
#define THRESHOLD 2 / 3


/* AUDITOR CONTROL */ 
#define AUDITOR_INTERVAL 3 //seconds

/* Defaulf first hashes */  
#define FIRST_HASH "0000000000000000000000000000000000000000000000000000000000000000"
#define FIRST_HASH_SEC FIRST_HASH "; sec - def"

#endif

