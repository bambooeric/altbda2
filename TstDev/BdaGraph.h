#ifndef BDAGRAPH_H
#define BDAGRAPH_H

#pragma warning( disable : 4995 4996 ) // no depreciated warnings

#include "CallbackFilter.h"
#include "NetworkProvider.h"
#include "Configuration.h"

#include <ksproxy.h>

#include <stdio.h>

#include "dwBdaApi.h"
#include <ttBdaDrvApi.h>
#include "THIOCtrl.h"


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
#define DEFAULT_POLARISATION	BDA_POLARISATION_LINEAR_H
#define DEFAULT_FEC				BDA_BCC_RATE_NOT_SET
#define DEFAULT_LOW_OSCILLATOR	9750000L
#define DEFAULT_HIGH_OSCILLATOR	10600000L
#define DEFAULT_LNB_SWITCH		11700000L
#define DEFAULT_POS_OPT			-1L
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
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		LONG PosOpt);
	HRESULT DVBS_Microsoft_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		Pilot S2Pilot,
		RollOff S2RollOff,
		ULONG LNBSource,
		BOOL bToneBurst);
	HRESULT DVBS_TT_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		LONG PosOpt);
	HRESULT DVBS_TeVii_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec);
	HRESULT DVBS_DvbWorld_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		UINT diseqc_port,
		UINT tone_burst);
	HRESULT DVBS_Hauppauge_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		DWORD S2Pilot,
		DWORD S2RollOff,
		LONG PosOpt);
	HRESULT DVBS_Conexant_Tune(
		ULONG LowBandF,
		ULONG HighBandF,
		ULONG SwitchF,
		ULONG Frequency,
		SpectralInversion SpectrInv,
		ModulationType ModType,
		LONG SymRate,
		Polarisation Pol,
		BinaryConvolutionCodeRate Fec,
		DWORD S2Pilot,
		DWORD S2RollOff,
		LONG PosOpt);
	HRESULT DVBT_Tune(
		ULONG Frequency,
		ULONG Bandwidth);
	HRESULT DVBC_Tune(
		ULONG Frequency,
		LONG SymRate,
		ModulationType ModType);

	HRESULT GetSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality);
	HRESULT GetTeViiSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality);
	HRESULT GetTechnotrendSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality);
	BOOL DVBS_Technotrend_GetProdName( char* pszProdName, size_t len );
	HRESULT DVBS_Technotrend_DiSEqC(BYTE len, BYTE *DiSEqC_Command, BYTE tb);
	HRESULT DVBS_Microsoft_DiSEqC(BYTE len, BYTE *DiSEqC_Command, BYTE repeat);
	HRESULT DVBS_Hauppauge_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Conexant_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Conexant_LNBPower (BOOL bPower);	
	HRESULT DVBS_Turbosight_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Turbosight_LNBPower (BOOL bPower);
	HRESULT DVBS_TurbosightQBOX_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_TurbosightQBOX_LNBPower (BOOL bPower);
	HRESULT DVBS_TurbosightNXP_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_TeVii_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Twinhan_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Twinhan_LNBPower(BOOL bPower);
	HRESULT DVBS_Twinhan_LNBSource (BYTE Port, BYTE ToneBurst);
	HRESULT DVBS_DvbWorld_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Omicom_DiSEqC(BYTE len, BYTE *DiSEqC_Command);
	HRESULT DVBS_Omicom_Set22Khz(BOOL b22Khz);

	void SetStreamCallbackProcedure(STR_CB_PROC);

private:
	HRESULT GetTunerPath(int idx, char* pTunerPath);
	HRESULT AddGraphToROT(IUnknown *pUnkGraph, DWORD *pdwRegister);
	HRESULT RemoveGraphFromROT(DWORD pdwRegister);
	void ReportMessage(char *text);
	HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin);
	IPin *GetInPin( IBaseFilter * pFilter, int nPin );
	IPin *GetOutPin( IBaseFilter * pFilter, int nPin );
    HRESULT GetPinMedium(IPin* pPin, REGPINMEDIUM* pMedium);

	DWORD			m_dwGraphRegister;		//registration number for the RunningObjectTable
	IFilterGraph2	*m_pFilterGraph;		// for current graph
	IBaseFilter		*m_pTunerDevice;		// for tuner device filter
	IBaseFilter		*m_pNetworkProvider;	// network provider filter
	IBaseFilter		*m_pReceiver;			// receiver
	IBaseFilter		*m_pCallbackFilter;		// calbback filter
	IMediaControl	*m_pMediaControl;		// media control	
#ifdef SG_USE
	CSampleGrabberCB *pCallbackInstance;	// SampleGrabber callback object
#else
	CCallbackFilter	*pCallbackInstance;		// callback filter object
#endif //SG_USE
	IKsPropertySet	*m_pKsTunerFilterPropSet; // Tuner filter proprietary interface
	IKsPropertySet	*m_pKsTunerPropSet;		// Tuner proprietary interface
	IKsPropertySet	*m_pKsDemodPropSet;		// Demod proprietary interface
	IKsControl		*m_pTunerControl;		// IKsControl for tuner

	CDVBNetworkProviderFilter *pNetworkProviderInstance; // network provider object

	MSG_CB_PROC message_callback;
	HANDLE hTT, hDW;
	BOOL bTVDLL;
	int iTVIdx;

	//THBDA Ioctl functions
	BOOL THBDA_IOControl( DWORD  dwIoControlCode,
		LPVOID lpInBuffer,
		DWORD  nInBufferSize,
		LPVOID lpOutBuffer,
		DWORD  nOutBufferSize,
		LPDWORD lpBytesReturned);
	BOOL THBDA_IOCTL_CHECK_INTERFACE_Fun(void);
	BOOL THBDA_IOCTL_SET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA);
	BOOL THBDA_IOCTL_GET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA);
	BOOL THBDA_IOCTL_SET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA);
	BOOL THBDA_IOCTL_GET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA);
};

#endif /* BDAGRAPH_H */
