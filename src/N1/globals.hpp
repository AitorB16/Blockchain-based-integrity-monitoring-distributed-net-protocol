#ifndef GLOBALS_HPP
#define GLOBALS_HPP

//CONFIGURATION FILE
#define CONFIG_FILE "config.xml"

//EXECUTION MODES
#define DEFAULT_MODE 0
#define DEBUG_MODE 1

//global exec var, loaded from config
extern int EXEC_MODE;

//TIMESPACE TO ACHIEVE CONSISTENCY AT BEGINNING
#define TIME_SPACE_BEFORE_AUDIT 60

//MAX TIME TO CONSIDER AS DISCONNECTED
#define RESPONSE_DELAY_MAX 2 //seconds

//TRUST CONTROL
#define TRUST_LEVEL 5
#define TRUST_DECREASE_CONST 1

//NETWORK CONTROL
#define HASH_UPDATE_TIMESPACE_MAX 120
#define HASH_UPDATE_TIMESPACE_MIN 5
//Percentage of net failing accepted
#define THRESHOLD 2 / 3


//AUDITOR
#define AUDITOR_INTERVAL 5 //seconds
//theoretically every AUDITOR_INTERVAL seconds, each node is audited (real random); AUDITOR_INTERVAL * (TRUST_LEVEL + 2), to ensure not reseting trustlvl too fast.
#define TIME_RESET_TRUST_LVL AUDITOR_INTERVAL * (TRUST_LEVEL + 2) //seconds

#define FIRST_HASH "0000000000000000000000000000000000000000000000000000000000000000"
#define FIRST_HASH_SEC FIRST_HASH "; sec - def"

#endif

//X seconds/DEF_TIMER_AUDIT = TIMES BEIGN AUDITED PER X seconds; X/5 = <2 + TRUST_LEVEL> -> x=50
