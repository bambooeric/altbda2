#ifndef BDAGRAPH_H
#define BDAGRAPH_H


#pragma warning( disable : 4995 4996 ) // no depreciated warnings

#include "CallbackFilter.h"
#include "NetworkProvider.h"
#include "Configuration.h"

#include <ksproxy.h>

#include <stdio.h>

// defaults for tune request
#define DEFAULT_FREQUENCY_C		394000L
#define DEFAULT_FREQUENCY_S		11766000L
#define DEFAULT_FREQUENCY_T		722000L
#define DEFAULT_SYMBOLRATE_C	6900L
#define DEFAULT_SYMBOLRATE_S	27500L
#define DEFAULT_SYMBOLRATE_T	0L
#define DEFAULT_MODULATION_C	BDA_MOD_64QAM
#define DEFAULT_MODULATION_S	BDA_MOD_QPSK
#define DEFAULT_MODULATION_T	BDA_MOD_64QAM
#define DEFAULT_INVERSION_C		BDA_SPECTRAL_INVERSION_INVERTED
#define DEFAULT_INVERSION_S		BDA_SPECTRAL_INVERSION_AUTOMATIC
#define DEFAULT_INVERSION_T		BDA_SPECTRAL_INVERSION_NORMAL
#define DEFAULT_POLARISATION	BDA_POLARISATION_LINEAR_V
#define DEFAULT_LOW_OSCILLATOR	9750000L
#define DEFAULT_HIGH_OSCILLATOR	10600000L
#define DEFAULT_LNB_SWITCH		11700000L
#define DEFAULT_POS_OPT			0x00000000L//0x00000101L,//0x00000001L,//0x00000100L 
#define DEFAULT_BANDWIDTH		BDA_BW_AUTO
#define DEFAULT_ONID			-1L
#define DEFAULT_SID 			-1L
#define DEFAULT_TSID			-1L

struct NetworkTuners
{
	int Count;
	struct
	{
		GUID Type;
		char Name[128];
		int Id;
		BOOLEAN Availability;
	} Tuner[8];
};

class CBdaGraph
{
public:
	CBdaGraph();
	~CBdaGraph();

	BSTR friendly_name_tuner;
	void MessageCallback(MSG_CB_PROC message_callback);

	HRESULT GetNetworkTuners(struct NetworkTuners *);
	HRESULT GetNetworkTunerType(IBaseFilter *pFilter, GUID *type);
	HRESULT FindTunerFilter(int dev_no, BSTR *bStr, IBaseFilter **pFilter);
	HRESULT BuildGraph(int, enum VENDOR_SPECIFIC *);
	HRESULT RunGraph(void);
	void CloseGraph(void);

	HRESULT DVBS_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		LONG PosOpt,
		Polarisation Pol);
	HRESULT DVBS_TT_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol);
	HRESULT DVBS_Hauppauge_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		LONG PosOpt,
		Polarisation Pol,
		DWORD RollOff,
		DWORD S2RollOff,
		DWORD S2Pilot,
		BOOLEAN RawDiSEqC);
	HRESULT DVBT_Tune(
		ULONG Frequency,
		ULONG Bandwidth);
	HRESULT DVBC_Tune(
		ULONG Frequency,
		LONG SymRate,
		ModulationType ModType);
	HRESULT GetSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality);
	HRESULT DVBS_Hauppauge_DiSEqC(BYTE, BYTE *);
	void SetStreamCallbackProcedure(STR_CB_PROC);

private:
	HRESULT AddGraphToROT(IUnknown *pUnkGraph, DWORD *pdwRegister);
	HRESULT RemoveGraphFromROT(DWORD pdwRegister);
	void ReportMessage(char *text);
	HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);
	IPin *GetInPin( IBaseFilter * pFilter, int nPin );
	IPin *GetOutPin( IBaseFilter * pFilter, int nPin );

	DWORD			m_dwGraphRegister;		//registration number for the RunningObjectTable
	IFilterGraph2	*m_pFilterGraph;		// for current graph
	IBaseFilter		*m_pTunerDevice;		// for tuner device filter
	IBaseFilter		*m_pNetworkProvider;	// network provider filter
	IBaseFilter		*m_pReceiver;			// receiver
	IBaseFilter		*m_pCallbackFilter;		// calbback filter
	IMediaControl	*m_pMediaControl;		// media control
	CCallbackFilter	*pCallbackInstance;		// callback filter object
	IKsPropertySet	*m_pProprietaryInterface;	// tuner's proprietary interface
	CDVBNetworkProviderFilter *pNetworkProviderInstance; // network provider object

	MSG_CB_PROC message_callback;
};

#endif /* BDAGRAPH_H */