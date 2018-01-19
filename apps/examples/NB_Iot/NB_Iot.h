#ifndef __NB_IOT_H__
#define __NB_IOT_H__


#define UE_LOCAL_UDP_PORT        "3005"
#define SERVER_HOST_UDP_IP       "111.198.38.70"
#define SERVER_HOST_UDP_PORT     "9999"

#define BC95_PWR_ON_TEST
//默认频段
#define SET_UE_DEFAULT_BAND      "AT+NBAND=5"         
#define UE_DEFAULT_BAND          "+NBAND:5"


#define SET_UE_REBOOT            "AT+NRB"

#define QUERY_UE_BAND            "AT+NBAND?"
#define SET_UE_BAND_5            "AT+NBAND=5"
#define SET_UE_BAND_8            "AT+NBAND=8"

#define QUERY_UE_CONNECT_MODE    "AT+NCONFIG?"
#define SET_UE_AUTOCONNECT       "AT+NCONFIG=AUTOCONNECT,TRUE"
#define SET_UE_MANUALCONNECT     "AT+NCONFIG=AUTOCONNECT,FALSE"

#define QUERY_UE_FUNC            "AT+CFUN?"
#define SET_UE_FUNC_0            "AT+CFUN=0"
#define SET_UE_FUNC_1            "AT+CFUN=1"

#define QUERY_UE_SIGNAL_QTY      "AT+CSQ"

#define QUERY_UE_ATTACH_STATS    "AT+CGATT?"
#define UE_ATTACHED_STATS        "+CGATT:1"
#define SET_UE_ATTACH            "AT+CGATT=1"

#define QUERY_UE_EREG_STATS      "AT+CEREG?"
#define UE_EREGISTERING_STATS    "+CEREG:0,2"
#define UE_EREGISTERED_STATS     "+CEREG:0,1"
#define SET_UE_EREG              "AT+CEREG=1"

#define QUERY_UE_SCCON_STATS     "AT+CSCON?"
#define SET_UE_SCCON             "AT+CSCON=1"
#define UE_CONNECTED             "+CSCON:0,1"

int NB_Iot(int argc, char *argv[]);


#endif
