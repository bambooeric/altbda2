#ifndef HAUPPAUGE_H
#define HAUPPAUGE_H

DEFINE_GUID(CLSID_HauppaugeBdaTunerExtension,
0xfaa8f3e5, 0x31d4, 0x4e41, 0x88, 0xef, 0x00, 0xa0, 0xc9, 0xf2, 0x1f, 0xc7);

enum HauppaugeBdaTunerExtension
{
	Hauppauge_KSPROPERTY_BDA_DISEQC = 0,
	Hauppauge_KSPROPERTY_BDA_PILOT = 0x20,
	Hauppauge_KSPROPERTY_BDA_ROLL_OFF = 0x21
};

enum HauppaugeDiSEqCVersion
{
	DISEQC_VER_1X = 1,
	DISEQC_VER_2X,
	ECHOSTAR_LEGACY,	// (not supported)
	DISEQC_VER_UNDEF = 0	// undefined (results in an error)
};

enum HauppaugeRxMode
{
	RXMODE_INTERROGATION = 1, // Expecting multiple devices attached
	RXMODE_QUICKREPLY,      // Expecting 1 rx (rx is suspended after 1st rx received)
	RXMODE_NOREPLY,         // Expecting to receive no Rx message(s)
	RXMODE_DEFAULT = 0        // use current register setting
};

enum HauppaugeBurstModulationType
{
	TONE_BURST_UNMODULATED = 0,
	TONE_BURST_MODULATED
};


#endif /* HAUPPAUGE_H */
