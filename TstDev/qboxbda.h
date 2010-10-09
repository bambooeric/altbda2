#ifndef QBOXBDA_H
#define QBOXBDA_H

DEFINE_GUIDSTRUCT( "C6EFE5EB-855A-4f1b-B7AA-87B5E1DC4113", KSPROPERTYSET_QBOXControl );
#define KSPROPERTYSET_QBOXControl DEFINE_GUIDNAMED( KSPROPERTYSET_QBOXControl )

typedef enum
{
    KSPROPERTY_CTRL_TUNER,
	KSPROPERTY_CTRL_IR,
	KSPROPERTY_CTRL_22K_TONE,
	KSPROPERTY_CTRL_MOTOR,
	KSPROPERTY_CTRL_LNBPW,
	KSPROPERTY_CTRL_LOCK_TUNER,
	KSPROPERTY_CTRL_MAC,
	KSPROPERTY_CTRL_DEVICEID,
} KSPROPERTY_QBOXControl;

typedef struct {
	DWORD	ChannelFrequency;//
	DWORD	ulLNBLOFLowBand;
	DWORD	ulLNBLOFHighBand;
	DWORD	SymbolRate;
	BYTE	Polarity;
	BYTE	LNB_POWER;              // LNB_POWER_ON | LNB_POWER_OFF
    UCHAR	HZ_22K;                 // HZ_22K_OFF | HZ_22K_ON
    UCHAR	Tone_Data_Burst;        // Data_Burst_ON | Tone_Burst_ON |Tone_Data_Disable
    UCHAR	DiSEqC_Port;            // DiSEqC_NULL | DiSEqC_A | DiSEqC_B | DiSEqC_C | DiSEqC_D

	BYTE motor[5];
	BYTE ir_code;
	BYTE lock;
	BYTE strength;
	BYTE quality;
	BYTE FecType;
	BYTE ModuType;
	BYTE reserved[254];
} QBOXDVBSCMD, *PQBOXDVBSCMD;

#endif //QBOXBDA_H
