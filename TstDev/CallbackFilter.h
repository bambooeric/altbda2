#ifndef CALLBACKFILTER_H
#define CALLBACKFILTER_H

//#pragma warning(disable: 4097 4511 4512 4514 4705)

// {260A8904-FA59-4789-80EA-9A4D92BB6A9C}

#include <Streams.h>
#include <mmreg.h>
#include <msacm.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>
#include <bdatif.h>
#include <uuids.h>
#include <tuner.h>
#include <commctrl.h>
#include <atlbase.h>
#include <strsafe.h>

#include <initguid.h>

#include "Dll.h"

DEFINE_GUID(CLSID_CallbackFilter, 
0x260a8904, 0xfa59, 0x4789, 0x80, 0xea, 0x9a, 0x4d, 0x92, 0xbb, 0x6a, 0x9c);

class CCallbackFilter;

class CCallbackFilterPin : public CRenderedInputPin
{
	CCallbackFilter		* const m_pFilter;		// Main renderer object
    CCritSec			* const m_pReceiveLock;	// Sample critical section

public:
    CCallbackFilterPin(TCHAR *pObjectName,
				CCallbackFilter *pFilter,
				CCritSec *pLock,
				CCritSec *pReceiveLock,
				LPUNKNOWN pUnk,
				HRESULT * phr);

    HRESULT CheckMediaType(const CMediaType* pmt);
    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP ReceiveCanBlock();

};

class CCallbackFilter : public CBaseFilter//, ITuneRequestInfo
{

    CCritSec m_Lock;                // Main renderer critical section
    CCritSec m_ReceiveLock;         // Sublock for received samples

    CCallbackFilterPin *m_pPin;          // A simple rendered input pin
public:

    DECLARE_IUNKNOWN;
 
    CCallbackFilter(LPUNKNOWN pUnk, HRESULT *pHr);
	~CCallbackFilter();

    // Pin enumeration
    CBasePin * GetPin(int);
    int GetPinCount();
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	HRESULT OnBufferData(char *data, int len);

	void SetStreamCallbackProcedure(STR_CB_PROC);
/*
	STDMETHODIMP GetLocatorData(ITuneRequest *);
	STDMETHODIMP GetComponentData(ITuneRequest *CurrentRequest);
	STDMETHODIMP CreateComponentList(ITuneRequest *CurrentRequest);
	STDMETHODIMP GetNextProgram(ITuneRequest *CurrentRequest, ITuneRequest **TuneRequest);
	STDMETHODIMP GetPreviousProgram(ITuneRequest *CurrentRequest, ITuneRequest **TuneRequest);
	STDMETHODIMP GetNextLocator(ITuneRequest *CurrentRequest, ITuneRequest **TuneRequest);
	STDMETHODIMP GetPreviousLocator(ITuneRequest *CurrentRequest, ITuneRequest **TuneRequest);
*/
private:
	STR_CB_PROC Callback;
	char packet_residue[188];
	int packet_residue_len;
};

#endif /* CALLBACKFILTER_H */
