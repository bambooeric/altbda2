#ifndef TBSBDA_H
#define TBSBDA_H

// this is defined in bda tuner/demod driver source (splmedia.h)
const GUID KSPROPSETID_BdaTunerExtensionProperties =
{0xfaa8f3e5, 0x31d4, 0x4e41, {0x88, 0xef, 0xd9, 0xeb, 0x71, 0x6f, 0x6e, 0xc9}};

// this is defined in bda tuner/demod driver source (splmedia.h)
typedef enum {
    KSPROPERTY_BDA_DISEQC_MESSAGE = 0,  //Custom property for Diseqc messaging
    KSPROPERTY_BDA_DISEQC_INIT,         //Custom property for Intializing Diseqc.
    KSPROPERTY_BDA_SCAN_FREQ,           //Not supported 
    KSPROPERTY_BDA_CHANNEL_CHANGE,      //Custom property for changing channel
    KSPROPERTY_BDA_DEMOD_INFO,          //Custom property for returning demod FW state and version
    KSPROPERTY_BDA_EFFECTIVE_FREQ,      //Not supported 
    KSPROPERTY_BDA_SIGNAL_STATUS,       //Custom property for returning signal quality, strength, BER and other attributes
    KSPROPERTY_BDA_LOCK_STATUS,         //Custom property for returning demod lock indicators 
    KSPROPERTY_BDA_ERROR_CONTROL,       //Custom property for controlling error correction and BER window
    KSPROPERTY_BDA_CHANNEL_INFO,        //Custom property for exposing the locked values of frequency,symbol rate etc after
                                        //corrections and adjustments

} KSPROPERTY_BDA_TUNER_EXTENSION;

const BYTE DISEQC_TX_BUFFER_SIZE = 150; // 3 bytes per message * 50 messages
const BYTE DISEQC_RX_BUFFER_SIZE = 8;   // reply fifo size, hardware limitation

/*******************************************************************************************************/
/* PHANTOM_LNB_BURST */
/*******************************************************************************************************/
typedef enum _PHANTOMLnbburst  {
    PHANTOM_LNB_BURST_MODULATED=1,                /* Modulated: Tone B               */
    PHANTOM_LNB_BURST_UNMODULATED,                /* Not modulated: Tone A           */
    PHANTOM_LNB_BURST_UNDEF=0                     /* undefined (results in an error) */
}   PHANTOM_LNB_BURST;

/*******************************************************************************************************/
/* PHANTOM_LNB_TONEBURST_EN_SET */
/*******************************************************************************************************/
typedef enum _PHANTOMLnbToneBurstEn  {
    PHANTOM_LNB_TONEBURST_ENABLE=1,                /* Enable tone burst               */
    PHANTOM_LNB_TONEBURST_DISABLE                  /* Disable tone burst           */    
}   PHANTOM_LNB_TONEBURST_EN_SET;

/*******************************************************************************************************/
/* PHANTOM_RXMODE */
/*******************************************************************************************************/
typedef enum _PHANTOMRxMode  {
    PHANTOM_RXMODE_INTERROGATION=0,              /* Demod expects multiple devices attached */
    PHANTOM_RXMODE_QUICKREPLY=1,                 /* demod expects 1 rx (rx is suspended after 1st rx received) */
    PHANTOM_RXMODE_NOREPLY=2                     /* demod expects to receive no Rx message(s) */
}   PHANTOM_RXMODE;

/////////////LIUZHENG ,ADDED ///////////
typedef enum _TBSDVBSExtensionPropertiesCMDMode {
    TBSDVBSCMD_LNBPOWER=0x00,      
    TBSDVBSCMD_MOTOR=0x01,            
    TBSDVBSCMD_22KTONEDATA=0x02,               
    TBSDVBSCMD_DISEQC=0x03              
}   TBSDVBSExtensionPropertiesCMDMode;


// DVB-S/S2 DiSEqC message parameters
typedef struct _DISEQC_MESSAGE_PARAMS
{
    UCHAR      uc_diseqc_send_message[DISEQC_TX_BUFFER_SIZE+1];
    UCHAR      uc_diseqc_send_message_length;

    UCHAR      uc_diseqc_receive_message[DISEQC_RX_BUFFER_SIZE+1];
    UCHAR      uc_diseqc_receive_message_length;    
    
    PHANTOM_LNB_BURST burst_tone;	//Burst tone at last-message: (modulated = ToneB; Un-modulated = ToneA). 
    PHANTOM_RXMODE    receive_mode;	//Reply mode: interrogation/no reply/quick reply.
    TBSDVBSExtensionPropertiesCMDMode tbscmd_mode;

    UCHAR      HZ_22K;			// HZ_22K_OFF | HZ_22K_ON
    UCHAR      Tone_Data_Burst;		// Data_Burst_ON | Tone_Burst_ON |Tone_Data_Disable
    UCHAR      uc_parity_errors;	// Parity errors:  0 indicates no errors; binary 1 indicates an error.
    UCHAR      uc_reply_errors;		// 1 in bit i indicates error in byte i. 
    BOOL       b_last_message;		// Indicates if current message is the last message (TRUE means last message).
    BOOL       b_LNBPower;		// liuzheng added for lnb power static
    
} DISEQC_MESSAGE_PARAMS, *PDISEQC_MESSAGE_PARAMS;

//------------------------------------------------------------
//
//
//  BDA Demodulator Extension Properties
//
// {B51C4994-0054-4749-8243-029A66863636}
const GUID KSPROPSETID_CustomIRCaptureProperties = 
{ 0xb51c4994, 0x54, 0x4749, { 0x82, 0x43, 0x2, 0x9a, 0x66, 0x86, 0x36, 0x36 }};

#define IRCAPTURE_COMMAND_START        1
#define IRCAPTURE_COMMAND_STOP         2
#define IRCAPTURE_COMMAND_FLUSH        3

typedef struct 
{   
    DWORD             dwAddress;
    DWORD             dwCommand;  

} KSPROPERTY_IRCAPTURE_KEYSTROKES_S, *PKSPROPERTY_IRCAPTURE_KEYSTROKES_S;

typedef struct 
{    
    CHAR             CommandCode;    

} KSPROPERTY_IRCAPTURE_COMMAND_S, *PKSPROPERTY_IRCAPTURE_COMMAND_S;

typedef enum
{
    KSPROPERTY_IRCAPTURE_KEYSTROKES         = 0,
    KSPROPERTY_IRCAPTURE_COMMAND            = 1

}KSPROPERTY_IRCAPTURE_PROPS;

#endif //TBSBDA_H
