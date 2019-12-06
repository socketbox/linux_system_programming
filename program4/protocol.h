#ifndef PROTOCOL_H
#define PROTOCOL_H

//define this here because it's applicable to all files, not b/c belongs here
#ifndef DEBUG
#define DEBUG						      0
#endif

//we know from sample files that string rep. of max file size won't exceed 5 chars
#define PROG4_MAX_FSZ					5


/***** Server *****/
#define INC_STR								"NCMPL"
#define INC										300
#define RDY_STR								"READY"
#define RDY										200
#define INVCL									500	
#define INVCL_STR							"INVCL"	
#define SKTTO									590	
#define SKTTO_STR							"SKTTO"
#define SRVR_RESP_LEN					6 

/***** Client *****/
#define PREAMB_LEN            8

//Encoding constants
#define ENCC_PAMB_STR         "6@@ENCC"
#define OTP_ENC_ID            "ENCC"
#define ENCC						      1

//Decoding constants
#define DECC_PAMB_STR         "6@@DECC"
#define OTP_DEC_ID            "DECC"
#define DECC			            2

#endif

