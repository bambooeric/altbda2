#include "stdafx.h"

#include "BdaGraph.h"
#include "cxtbda.h"
#include "omcbda.h"
#include <TeVii.h>

#define Z(a) memset(&a, 0, sizeof(a))

CBdaGraph::CBdaGraph()
{
	message_callback = NULL;
	m_dwGraphRegister = 0;
	m_pFilterGraph = NULL;
	m_pTunerDevice = NULL;
	m_pNetworkProvider = NULL;
	m_pReceiver = NULL;
	m_pCallbackFilter = NULL;
	m_pProprietaryInterface = NULL;
	m_pTunerControl = NULL;
	m_pMediaControl = NULL;
	pCallbackInstance = NULL;
	pNetworkProviderInstance = NULL;
	hTT = hDW =  INVALID_HANDLE_VALUE;
	iTVIdx = -1;
}

CBdaGraph::~CBdaGraph()
{
}

HRESULT CBdaGraph::GetNetworkTuners(struct NetworkTuners *Tuners)
{
	Tuners->Count = 0;

	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		DebugLog("BDA2: DeviceDescription: Failed creating Device Enumerator");
		return hr;
	}

	// Obtain a class enumerator for the BDA tuner category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		int dev_enum_i = 0;

		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK && Tuners->Count < 8)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				++dev_enum_i;
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					BSTR bStr = varName.bstrVal;
					WideCharToMultiByte(CP_ACP, 0, bStr, -1, Tuners->Tuner[Tuners->Count].Name, sizeof(Tuners->Tuner[Tuners->Count].Name), NULL, NULL);

					// check the BDA tuner type and availability
					// To create an instance of the filter, do the following:
					IBaseFilter *pFilter;
					hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)&pFilter);
					if(SUCCEEDED(hr))
					{
						GUID type;

						hr = GetNetworkTunerType(pFilter, &type);
						if(SUCCEEDED(hr))
						{
							Tuners->Tuner[Tuners->Count].Id = dev_enum_i;
							Tuners->Tuner[Tuners->Count].Type = type;
							Tuners->Tuner[Tuners->Count].Availability = TRUE; // TODO - check availability
						}
						else
							DebugLog("BDA2: DeviceDescription: Failed getting network tuner type");

						char* tuner_name = Tuners->Tuner[Tuners->Count].Name;
						if ( strstr(tuner_name, "DVBLink Tuner" ) )
						{
							//Tuners->Tuner[Tuners->Count].Type = CLSID_NetworkProvider;
							Tuners->Tuner[Tuners->Count].Availability = FALSE;
						}
						
						// Technotrend check
						DEVICE_CAT TTDevCat=UNKNOWN;
						if ( strstr(tuner_name, BDG2_NAME_S_TUNER) ||
							strstr(tuner_name, BDG2_NAME_C_TUNER) ||
							strstr(tuner_name, BDG2_NAME_T_TUNER) ||
							strstr(tuner_name, BDG2_NAME_S_TUNER_FAKE) ||
							strstr(tuner_name, BDG2_NAME_C_TUNER_NEW) ||
							strstr(tuner_name, BDG2_NAME_S_TUNER_NEW) ||
							strstr(tuner_name, BDG2_NAME_T_TUNER_NEW) )
							TTDevCat=BUDGET_2;
						if ( strstr(tuner_name, BUDGET3NAME_TUNER) ||
							strstr(tuner_name,  BUDGET3NAME_ATSC_TUNER) )
							TTDevCat=BUDGET_3;
						if ( strstr(tuner_name, USB2BDA_DVB_NAME_S_TUNER) ||
							strstr(tuner_name, USB2BDA_DVB_NAME_C_TUNER) ||
							strstr(tuner_name, USB2BDA_DVB_NAME_T_TUNER) ||
							strstr(tuner_name, USB2BDA_DVB_NAME_S_TUNER_FAKE) )
							TTDevCat=USB_2;
						if ( strstr(tuner_name, USB2BDA_DVBS_NAME_PIN) )
							TTDevCat=USB_2_PINNACLE;
						if ( strstr(tuner_name, USB2BDA_DSS_NAME_TUNER) )
							TTDevCat=USB_2_DSS;
						if ( strstr(tuner_name, PREMIUM_NAME_DIGITAL_TUNER) )
							TTDevCat=PREMIUM;

						IPin* pPin = GetOutPin(pFilter, 0);
						if ( pPin && (TTDevCat!=UNKNOWN) )
						{
							DebugLog("BDA2: DeviceDescription: Check Technotrend device info");
							REGPINMEDIUM Medium;
							memset(&Medium, 0, sizeof(Medium));
							if (GetPinMedium(pPin, &Medium) == S_OK)
							{
								hTT = bdaapiOpenHWIdx(TTDevCat,Medium.dw1);
								if (INVALID_HANDLE_VALUE!=hTT)
								{
									if (!DVBS_Technotrend_GetProdName(tuner_name,sizeof(Tuners->Tuner[Tuners->Count].Name)))
										DebugLog("BDA2: DeviceDescription: Can't get Technotrend product name");
									bdaapiClose (hTT);
									hTT = INVALID_HANDLE_VALUE;
								}
								else
									DebugLog("BDA2: DeviceDescription: Can't open Technotrend device via TT BDA API");
							}
							else
								DebugLog("BDA2: DeviceDescription: Can't get Tuner pin medium");
						}
						pFilter->Release();
						Tuners->Count += 1;
					}
					else
						DebugLog("BDA2: DeviceDescription: Failed binding moniker to object");
					VariantClear(&varName);
					pPropBag->Release();
				}
				else
					DebugLog("BDA2: DeviceDescription: Failed getting FriendlyName of the BDA_NETWORK_TUNER");
				pMoniker->Release();
			}
			else
				DebugLog("BDA2: DeviceDescription: Failed binding PropertyBag to storage");
		}
		pEnumCat->Release();
		pSysDevEnum->Release();
	}
	else
		DebugLog("BDA2: DeviceDescription: Failed creating KSCATEGORY_BDA_NETWORK_TUNER class enumerator");
	return hr;
}

BOOL CBdaGraph::DVBS_Technotrend_GetProdName( char* pszProdName, size_t len )
{
	if (INVALID_HANDLE_VALUE==hTT)
		return FALSE;
	TS_FilterNames TTInfo;
	TYPE_RET_VAL rc = bdaapiGetDevNameAndFEType (hTT, &TTInfo);
	if (RET_SUCCESS != rc)
		return FALSE;
	strncpy(pszProdName, TTInfo.szProductName, len);
	return TRUE;
}

HRESULT CBdaGraph::GetTunerPath(int idx, char* pTunerPath)
{
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		DebugLog("BDA2: GetTunerPath: Failed creating Device Enumerator");
		return hr;
	}

	// Obtain a class enumerator for the BDA tuner category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		int i = 0;

		while((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (i < 8))
		{
			++i;
			if (i==idx)
			{
				LPOLESTR pszName;
				hr = pMoniker->GetDisplayName(NULL, NULL, &pszName);
				WideCharToMultiByte(CP_ACP, 0, pszName, -1, pTunerPath, MAX_PATH, 0, 0);
				CoTaskMemFree((void*)pszName);
				pMoniker->Release();
				break;
			}			
		}
		pEnumCat->Release();
	}
	else
		DebugLog("BDA2: GetTunerPath: Failed creating KSCATEGORY_BDA_NETWORK_TUNER class enumerator");

	pSysDevEnum->Release();
	return hr;
}

HRESULT CBdaGraph::GetNetworkTunerType(IBaseFilter *pFilter, GUID *type)
{
	BDANODE_DESCRIPTOR descriptors[20];
	IBDA_Topology *pTopology;
	ULONG cnt;
	HRESULT hr;

	*type = CLSID_NULL;

	hr = pFilter->QueryInterface(IID_IBDA_Topology, (void **)&pTopology);
	if(SUCCEEDED(hr))
	{
		hr = pTopology->GetNodeDescriptors(&cnt, 20, descriptors);
		if(SUCCEEDED(hr))
		{
			for(unsigned i=0; i<cnt; ++i)
			{
				if(IsEqualGUID(descriptors[i].guidFunction, KSNODE_BDA_QPSK_DEMODULATOR))
				{
					*type = CLSID_DVBSNetworkProvider;
					break;
				}
				else
				if(IsEqualGUID(descriptors[i].guidFunction, KSNODE_BDA_COFDM_DEMODULATOR))
				{
					*type = CLSID_DVBTNetworkProvider;
					break;
				}
				else
				if(IsEqualGUID(descriptors[i].guidFunction, KSNODE_BDA_QAM_DEMODULATOR))
				{
					*type = CLSID_DVBCNetworkProvider;
					break;
				}
				else
				if(IsEqualGUID(descriptors[i].guidFunction, KSNODE_BDA_8VSB_DEMODULATOR))
				{
					*type = CLSID_ATSCNetworkProvider;
					break;
				}
			}
		}
		else
			DebugLog("BDA2: GetNetworkTunerType: Failed getting node descriptors");
	}
	else
		DebugLog("BDA2: GetNetworkTunerType: Failed getting IBDA_Topology");

	return hr;
}

HRESULT CBdaGraph::FindTunerFilter(int dev_no, BSTR *bStr, IBaseFilter **pFilter)
{
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		DebugLog("BDA2: FindTunerFilter: Failed creating SystemDeviceEnum");
		return hr;
	}
	// Obtain a class enumerator for the BDA tuner category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_NETWORK_TUNER, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		int dev_enum_i = 0;
		while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			++dev_enum_i;
			if(dev_no == dev_enum_i)
			{
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
				if (SUCCEEDED(hr))
				{
					// To retrieve the filter's friendly name, do the following:
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						// Display the name in your UI somehow.
						*bStr = SysAllocString(varName.bstrVal);
						VariantClear(&varName);

						pPropBag->Release();
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)pFilter);
						if(FAILED(hr))
							DebugLog("BDA2: FindTunerFilter: Failed binding moniker to object");
					}
					else
						DebugLog("BDA2: FindTunerFilter: Failed reading FriendlyName");
				}
				else
					DebugLog("BDA2: FindTunerFilter: Failed binding PropertyBag to storage");
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	else
		DebugLog("BDA2: FindTunerFilter: Failed creating KSCATEGORY_BDA_NETWORK_TUNER class enumerator");
	pSysDevEnum->Release();
	return hr;
}

void CBdaGraph::MessageCallback(MSG_CB_PROC callback)
{
	message_callback = callback;
}

#ifdef _DEBUG
HRESULT CBdaGraph::AddGraphToROT(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker *pMoniker;
	IRunningObjectTable *pROT;
	WCHAR wsz[128];
	HRESULT hr;

	if(FAILED(GetRunningObjectTable(0, &pROT)))
	{
		DebugLog("BDA2: AddGraphToROT: Failed getting ROT");
		return E_FAIL;
	}
	wsprintfW(wsz, L"FilterGraph %08x pid %08x - TstDev\0", (DWORD_PTR) pUnkGraph, GetCurrentProcessId());
	hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if(FAILED(hr))
		DebugLog("BDA2: AddGraphToROT: Failed creating item moniker");
	else
	{
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, pdwRegister);
		if(FAILED(hr))
			DebugLog("BDA2: AddGraphToROT: Failed registering to ROT");
	}
	return hr;
}
HRESULT CBdaGraph::RemoveGraphFromROT(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;

	if(FAILED(GetRunningObjectTable(0, &pROT)))
	{
		DebugLog("BDA2: RemoveGraphFromROT: Failed getting ROT");
		return E_FAIL;
	}
	return pROT->Revoke(pdwRegister);
}
#endif

void CBdaGraph::ReportMessage(char *text)
{
	DebugLog(text);
	if(message_callback)
	{
		struct MESSAGE_CALLBACK_DATA d;

		d.h = 0x0;
		d.message = text;

		message_callback(MESSAGE_CALLBACK_ID2, (char *)&d);
	}
}

HRESULT CBdaGraph::GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin) {
	CComPtr< IEnumPins > pEnum;
	*ppPin = NULL;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if(FAILED(hr))  return hr;

	ULONG ulFound;
	IPin *pPin;
	hr = E_FAIL;

	while (S_OK == pEnum->Next(1, &pPin, &ulFound)) {
		PIN_DIRECTION pindir = (PIN_DIRECTION)3;
		pPin->QueryDirection(&pindir);
		if (pindir == dirrequired) {
			if (iNum == 0) {
				*ppPin = pPin;  // Return the pin's interface
				hr = S_OK;      // Found requested pin, so clear error
				break;
			}
			iNum--;
		} 
		pPin->Release();
	}

	return hr;
}

IPin *CBdaGraph::GetInPin( IBaseFilter * pFilter, int nPin ) {
	CComPtr<IPin> pComPin=0;
	GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
	return pComPin;
}

IPin *CBdaGraph::GetOutPin( IBaseFilter * pFilter, int nPin ) {
	CComPtr<IPin> pComPin=0;
	GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
	return pComPin;
}

HRESULT CBdaGraph::GetPinMedium(IPin* pPin, REGPINMEDIUM* pMedium)
{
	if ( pPin == NULL || pMedium == NULL )
		return E_POINTER;

	CComPtr <IKsPin> pKsPin;
    KSMULTIPLE_ITEM *pmi;

    HRESULT hr = pPin->QueryInterface(IID_IKsPin, (void **)&pKsPin);
    if ( FAILED(hr) )
        return hr;  // Pin does not support IKsPin.

    hr = pKsPin->KsQueryMediums(&pmi);
    if ( FAILED(hr) )
        return hr;  // Pin does not support mediums.

    if ( pmi->Count )
    {
        // Use pointer arithmetic to reference the first medium structure.
        REGPINMEDIUM *pTemp = (REGPINMEDIUM*)(pmi + 1);
        memcpy(pMedium, pTemp, sizeof(REGPINMEDIUM));
    }

    CoTaskMemFree(pmi);

    return S_OK;
}

HRESULT CBdaGraph::BuildGraph(int selected_device_enum, enum VENDOR_SPECIFIC *VendorSpecific)
{
	char text[128];
	BSTR friendly_name_receiver;
	char receiver_name[128];

	HRESULT hr;

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph2, (void **)&m_pFilterGraph);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Can't instantiate FilterGraph");
		ReportMessage(text);
		return hr;
	}
#ifdef _DEBUG
	hr = AddGraphToROT(m_pFilterGraph, &m_dwGraphRegister);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Can't add FilterGraph to ROT");
		ReportMessage(text);
		return hr;
	}
#endif
	hr = S_OK;
	pNetworkProviderInstance = new CDVBNetworkProviderFilter(NULL, &hr);
	if (pNetworkProviderInstance == NULL || FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed instantiating custom Network Provider Filter");
		ReportMessage(text);
		return hr;
	}
	hr = pNetworkProviderInstance->QueryInterface(IID_IBaseFilter,(void **)&m_pNetworkProvider);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed creating custom Network Provider");
		ReportMessage(text);
		return hr;
	}
	hr = m_pFilterGraph->AddFilter(m_pNetworkProvider, L"Custom DVB-S|T|C Network Provider");
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed adding custom Network Provider filter to the graph");
		ReportMessage(text);
		return hr;
	}
	hr = FindTunerFilter(selected_device_enum, &friendly_name_tuner, &m_pTunerDevice);
	if(hr != S_OK)
	{
		sprintf(text,"BDA2: BuildGraph: Can't find Tuner filter #%d", selected_device_enum);
		ReportMessage(text);
		return hr;
	}
	// add tuner to the graph
sprintf(text,"BDA2: BuildGraph: Adding Network Tuner to graph");
ReportMessage(text);
	hr = m_pFilterGraph->AddFilter(m_pTunerDevice, friendly_name_tuner);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed adding Tuner filter to the graph");
		ReportMessage(text);
		return hr;
	}
	else
	{
		// Display the name in your UI somehow.
		WideCharToMultiByte(CP_ACP, 0, friendly_name_tuner, -1, receiver_name, sizeof(receiver_name), NULL, NULL);
		sprintf(text,"BDA2: BuildGraph: Added ID #%d '%s' Tuner filter to the graph",
			selected_device_enum, receiver_name);
		ReportMessage(text);
	}
	// connect Network provider with Tuner
	IPin *m_pP1, *m_pP2;
	m_pP1 = GetOutPin(m_pNetworkProvider, 0);
	if(m_pP1 == 0)
	{
		sprintf(text,"BDA2: BuildGraph: Failed finding the Output Pin of Network Provider");
		ReportMessage(text);
		return E_FAIL;
	}
	m_pP2 = GetInPin(m_pTunerDevice, 0);
	if(m_pP2 == 0)
	{
		sprintf(text,"BDA2: BuildGraph: Failed finding the Input Pin of Tuner filter");
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: BuildGraph: Connecting Network Provider with the Tuner filter");
	ReportMessage(text);

	hr = m_pFilterGraph->ConnectDirect(m_pP1, m_pP2, NULL);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed connecting Network Provider with the Tuner filter");
		ReportMessage(text);
		return hr;
	}
	sprintf(text,"BDA2: BuildGraph: Connected Network Provider with the Tuner filter");
	ReportMessage(text);

	*VendorSpecific = VENDOR_SPECIFIC(PURE_BDA);

	// Technotrend check
	DEVICE_CAT TTDevCat=UNKNOWN;
	if ( strstr(receiver_name, BDG2_NAME_S_TUNER) ||
		strstr(receiver_name, BDG2_NAME_C_TUNER) ||
		strstr(receiver_name, BDG2_NAME_T_TUNER) ||
		strstr(receiver_name, BDG2_NAME_S_TUNER_FAKE) ||
		strstr(receiver_name, BDG2_NAME_C_TUNER_NEW) ||
		strstr(receiver_name, BDG2_NAME_S_TUNER_NEW) ||
		strstr(receiver_name, BDG2_NAME_T_TUNER_NEW) )
		TTDevCat=BUDGET_2;
	if ( strstr(receiver_name, BUDGET3NAME_TUNER) ||
		strstr(receiver_name,  BUDGET3NAME_ATSC_TUNER) )
		TTDevCat=BUDGET_3;
	if ( strstr(receiver_name, USB2BDA_DVB_NAME_S_TUNER) ||
		strstr(receiver_name, USB2BDA_DVB_NAME_C_TUNER) ||
		strstr(receiver_name, USB2BDA_DVB_NAME_T_TUNER) ||
		strstr(receiver_name, USB2BDA_DVB_NAME_S_TUNER_FAKE) )
		TTDevCat=USB_2;
	if ( strstr(receiver_name, USB2BDA_DVBS_NAME_PIN) )
		TTDevCat=USB_2_PINNACLE;
	if ( strstr(receiver_name, USB2BDA_DSS_NAME_TUNER) )
		TTDevCat=USB_2_DSS;
	if ( strstr(receiver_name, PREMIUM_NAME_DIGITAL_TUNER) )
		TTDevCat=PREMIUM;

	m_pP1 = GetOutPin(m_pTunerDevice, 0);
	if ( m_pP1 && (TTDevCat!=UNKNOWN) )
	{
		DebugLog("BDA2: BuildGraph: checking for Technotrend DiSEqC interface");
		REGPINMEDIUM Medium;
		memset(&Medium, 0, sizeof(Medium));//CLSID clsMedium; DWORD dw1; DWORD dw2;
		if (GetPinMedium(m_pP1, &Medium) == S_OK)
		{
			hTT = bdaapiOpenHWIdx(TTDevCat,Medium.dw1);
			if (INVALID_HANDLE_VALUE!=hTT)
			{
				DebugLog("BDA2: BuildGraph: found Technotrend DiSEqC interface");
				*VendorSpecific = VENDOR_SPECIFIC(TT_BDA);
			}
		}
		else
			DebugLog("BDA2: BuildGraph: Can't get Tuner pin medium");
	}

	CComPtr <IKsObject> m_piKsObject; //KsObject Interface
	hr = m_pTunerDevice->QueryInterface(IID_IKsObject,(void**)&m_piKsObject);
	if SUCCEEDED(hr)
	{
		hDW = m_piKsObject->KsGetObjectHandle();
		DW_ID InfoDW;
		if SUCCEEDED(dwBDAGetDeviceID(hDW, &InfoDW))
		{
			DebugLog("BDA2: BuildGraph: found DvbWorld DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(DW_BDA);
		}
	}

	m_pTunerControl = NULL;
	hr = m_pTunerDevice->QueryInterface(IID_IKsControl,(void**)&m_pTunerControl);
	if (hr==S_OK)
	{
		KSPROPERTY prop;
		ULONG bytesReturned = 0;
		DWORD supported = 0;
		Z(prop);

		prop.Set = KSPROPSETID_OMCDiSEqCProperties;
		prop.Id = KSPROPERTY_OMC_DISEQC_WRITE;
		prop.Flags = KSPROPERTY_TYPE_BASICSUPPORT;
		hr = m_pTunerControl->KsProperty(&prop,sizeof(prop),
			(void*)&supported, sizeof(supported),&bytesReturned);
		if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_SET) )
		{
			DebugLog("BDA2: BuildGraph: found Omicom DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(OMC_BDA);
		}
	}

	// let's look if Tuner exposes proprietary interfaces
	hr = m_pP2->QueryInterface(IID_IKsPropertySet, (void **)&m_pProprietaryInterface);
	if (hr==S_OK)
	{
		sprintf(text,"BDA2: BuildGraph: Tuner exposes proprietary interfaces");
		ReportMessage(text);
		DWORD supported;
		// Hauppauge
		DebugLog("BDA2: BuildGraph: checking for Hauppauge DiSEqC interface");
		hr = m_pProprietaryInterface->QuerySupported(KSPROPSETID_BdaTunerExtensionPropertiesHaup,
			KSPROPERTY_BDA_DISEQC_MESSAGE, &supported);
		if(SUCCEEDED(hr) && supported)
		{
			DebugLog("BDA2: BuildGraph: found Hauppauge DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(HAUP_BDA);
		}
		// Conexant
		hr = m_pProprietaryInterface->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
			KSPROPERTY_BDA_DISEQC_MESSAGE, &supported);
		if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_GET) && (supported & KSPROPERTY_SUPPORT_SET) )
		{
			DebugLog("BDA2: BuildGraph: found Conexant DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(CXT_BDA);
		}
		// Turbosight
		hr = m_pProprietaryInterface->QuerySupported(KSPROPSETID_BdaTunerExtensionProperties,
			KSPROPERTY_BDA_DISEQC_MESSAGE, &supported);
		if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_GET) && (!(supported & KSPROPERTY_SUPPORT_SET)) )
		{
			DebugLog("BDA2: BuildGraph: found Turbosight DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(TBS_BDA);
		}
		if (THBDA_IOCTL_CHECK_INTERFACE_Fun())
		{
			DebugLog("BDA2: BuildGraph: found Twinhan DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(TH_BDA);
		}
	}

	if (*VendorSpecific == PURE_BDA)
	{
		char receiver_path[MAX_PATH];
		hr = GetTunerPath(selected_device_enum, receiver_path);
		int m = FindDevices();
		for (int i=0; i<m; i++)
			if ( (!strcmp(receiver_name,GetDeviceName(i))) && (!strcmp(receiver_path,GetDevicePath(i))) )
			{
				if (OpenDevice(i,NULL,NULL))
				{
					DebugLog("BDA2: BuildGraph: found TeVii DiSEqC interface");
					*VendorSpecific = VENDOR_SPECIFIC(TV_BDA);
					iTVIdx=i;
				}
				else
					DebugLog("BDA2: BuildGraph: Can't open TeVii device interface");
				break;
			}
	}

	hr = S_OK;
#ifdef SG_USE
	pCallbackInstance = new CSampleGrabberCB();
	if (!pCallbackInstance)
	{
		sprintf(text,"BDA2: BuildGraph: Failed instantiating SampleGrabber Callback");
		ReportMessage(text);
		return hr;
	}
    hr = CoCreateInstance(
                        CLSID_SampleGrabber,
                        NULL, 
                        CLSCTX_INPROC_SERVER,
                        IID_IBaseFilter, 
                        reinterpret_cast<void**>(&m_pCallbackFilter)
                        );
	if (FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed creating SampleGrabber Filter");
		ReportMessage(text);
		return hr;
	}

	CComPtr <ISampleGrabber> sg;
	hr = m_pCallbackFilter->QueryInterface(IID_ISampleGrabber, (void**)&sg);
	if FAILED(hr)
	{
		sprintf(text,"BDA2: BuildGraph: Failed QI SampleGrabber Filter");
		ReportMessage(text);
		return hr;
	}
	AM_MEDIA_TYPE mt;
	memset(&mt,0,sizeof(mt));
	mt.majortype = MEDIATYPE_Stream;
	mt.subtype == KSDATAFORMAT_SUBTYPE_BDA_MPEG2_TRANSPORT;
	hr = sg->SetMediaType(&mt);
	hr = sg->SetOneShot(FALSE);
	hr = sg->SetBufferSamples(FALSE);
	hr = sg->SetCallback(pCallbackInstance, 0);
#else
	pCallbackInstance = new CCallbackFilter(NULL, &hr);
	if (pCallbackInstance == NULL || FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed instantiating Callback Filter");
		ReportMessage(text);
		return hr;
	}
	hr = pCallbackInstance->QueryInterface(IID_IBaseFilter,(void **)&m_pCallbackFilter);
//	m_pCallbackFilter->AddRef();
	if (FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed creating Callback Filter");
		ReportMessage(text);
		return hr;
	}
#endif //SG_USE

	hr = m_pFilterGraph->AddFilter(m_pCallbackFilter, L"Callback");
	if (FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed adding Callback Filter to the FilterGraph");
		ReportMessage(text);
		return hr;
	}

	sprintf(text,"BDA2: BuildGraph: Trying to connect Tuner filter directly to Callback");
	ReportMessage(text);

	// try connecting Callback directly with Tuner
	m_pP1 = GetOutPin(m_pTunerDevice, 0);
	if(m_pP1 == 0)
	{
		sprintf(text,"BDA2: BuildGraph: Failed finding the Output Pin of the Tuner");
		ReportMessage(text);
		return E_FAIL;
	}
	m_pP2 = GetInPin(m_pCallbackFilter, 0);
	if(m_pP2 == 0)
	{
		sprintf(text,"BDA2: BuildGraph: Failed finding the Input Pin of the Callback Filter");
		ReportMessage(text);
		return E_FAIL;
	}
	hr = m_pFilterGraph->ConnectDirect(m_pP1, m_pP2, NULL);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed connecting the Tuner Filter with the Callback Filter");
		ReportMessage(text);

		// look for Receiver
		ICreateDevEnum *pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum, (void **)&pSysDevEnum);
		if (FAILED(hr))
		{
			sprintf(text,"BDA2: BuildGraph: Failed creating device enumerator for Tuner");
			ReportMessage(text);
			return hr;
		}
		// Obtain a class enumerator for the BDA receiver category.
		IEnumMoniker *pEnumCat = NULL;
		hr = pSysDevEnum->CreateClassEnumerator(KSCATEGORY_BDA_RECEIVER_COMPONENT, &pEnumCat, 0);
		if (hr == S_OK) 
		{
			// Enumerate the monikers.
			IMoniker *pMoniker = NULL;
			ULONG cFetched;
			IBaseFilter *m_pFilter = NULL;
			while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
			{
				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
				if (SUCCEEDED(hr))
				{
					// To retrieve the filter's friendly name, do the following:
					VARIANT varName;
					VariantInit(&varName);
					hr = pPropBag->Read(L"FriendlyName", &varName, 0);
					if (SUCCEEDED(hr))
					{
						char receiver_name[64];
						// Display the name in your UI somehow.
						friendly_name_receiver = SysAllocString(varName.bstrVal);
						WideCharToMultiByte(CP_ACP, 0, friendly_name_receiver, -1, receiver_name, sizeof(receiver_name), NULL, NULL);
						VariantClear(&varName);

						pPropBag->Release();
						hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)&m_pFilter);
						if(FAILED(hr))
						{
							sprintf(text,"BDA2: BuildGraph: Failed binding moniker to object BDA_RECEIVER");
							ReportMessage(text);
							return hr;
						}
						// add potential receiver to the graph
						sprintf(text,"BDA2: BuildGraph: Adding potential Receiver (%s) to graph", receiver_name);
						ReportMessage(text);
						hr = m_pFilterGraph->AddFilter(m_pFilter, friendly_name_receiver);
						if(FAILED(hr))
						{
							sprintf(text,"BDA2: BuildGraph: Failed adding potential Receiver filter to the graph");
							ReportMessage(text);
							return hr;
						}
						sprintf(text,"BDA2: BuildGraph: Added potential Receiver (%s) to graph", receiver_name);
						ReportMessage(text);
						pMoniker->Release();
						// connect Tuner with potential Receiver
						m_pP1 = GetOutPin(m_pTunerDevice, 0);
						if(m_pP1 == 0)
						{
							sprintf(text,"BDA2: BuildGraph: Failed finding the Output Pin of Tuner");
							ReportMessage(text);
							return E_FAIL;
						}
						m_pP2 = GetInPin(m_pFilter, 0);
						if(m_pP2 == 0)
						{
							sprintf(text,"BDA2: BuildGraph: Failed finding the Input Pin of potential Receiver");
							ReportMessage(text);
							hr = E_FAIL;
						}
						else
							hr = m_pFilterGraph->ConnectDirect(m_pP1, m_pP2, NULL);
						if(FAILED(hr))
						{
							// found not compatible Receiver
							sprintf(text,"BDA2: BuildGraph: Failed connecting Tuner with potential Receiver '%s'", receiver_name);
							ReportMessage(text);

							m_pFilterGraph->RemoveFilter(m_pFilter);
							m_pFilter->Release();
							SysFreeString(friendly_name_receiver);
						}
						else
						{
							// found Receiver
							sprintf(text,"BDA2: BuildGraph: Found matching Receiver '%s'", receiver_name);
							ReportMessage(text);

							m_pReceiver = m_pFilter;
							break;
						}
					}
					else
					{
						sprintf(text,"BDA2: BuildGraph: Failed getting FriendlyName of potential Receiver");
						ReportMessage(text);
						return hr;
					}
				}
				else
				{
					sprintf(text,"BDA2: BuildGraph: Failed binding PropertyBag to storage for potential Receiver");
					ReportMessage(text);
					return hr;
				}
			}
			pEnumCat->Release();
			pSysDevEnum->Release();
		}
		else
		{
			sprintf(text,"BDA2: BuildGraph: Failed creating KSCATEGORY_BDA_RECEIVER_COMPONENT class enumerator");
			ReportMessage(text);
			return hr;
		}
		if(m_pReceiver == NULL)
		{
			sprintf(text,"BDA2: BuildGraph: Failed finding matching Receiver for the Tuner");
			ReportMessage(text);
			return hr;
		}
		// Capture filter found
		m_pP1 = GetOutPin(m_pReceiver, 0);
		if(m_pP1 == 0)
		{
			sprintf(text,"BDA2: BuildGraph: Failed finding the Output Pin of the Receiver");
			ReportMessage(text);
			return E_FAIL;
		}
		m_pP2 = GetInPin(m_pCallbackFilter, 0);
		if(m_pP2 == 0)
		{
			sprintf(text,"BDA2: BuildGraph: Failed finding the Input Pin of the Callback Filter");
			ReportMessage(text);
			return E_FAIL;
		}
		hr = m_pFilterGraph->ConnectDirect(m_pP1, m_pP2, NULL);
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: BuildGraph: Failed connecting the Capture Filter with the Callback Filter");
			ReportMessage(text);
			return hr;
		}
	}
	hr = m_pFilterGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Failed getting the MediaControl interface");
		ReportMessage(text);
		return hr;
	}
	return S_OK;
}

HRESULT CBdaGraph::RunGraph()
{
	HRESULT hr;
	char text[128];

	if(m_pMediaControl)
		hr = m_pMediaControl->Run();
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: RunGraph: Failed putting the Graph in Run state");
		ReportMessage(text);
		return hr;
	}
	return S_OK;
}

void CBdaGraph::CloseGraph()
{
	if(m_pFilterGraph == NULL)
		return;

	if (hTT!=INVALID_HANDLE_VALUE)
		bdaapiClose(hTT);

	if (iTVIdx!=-1)
		CloseDevice(iTVIdx);

	if(m_pMediaControl)
	{
		m_pMediaControl->Stop();
		m_pMediaControl->Release();
		m_pMediaControl = NULL;
	}
	if(m_pCallbackFilter)
	{
		m_pFilterGraph->RemoveFilter(m_pCallbackFilter);
		m_pCallbackFilter->Release();
		m_pCallbackFilter = NULL;
	}
	if(m_pReceiver)
	{
		m_pFilterGraph->RemoveFilter(m_pReceiver);
		m_pReceiver->Release();
		m_pReceiver = NULL;
	}
	if(m_pTunerDevice)
	{
		m_pFilterGraph->RemoveFilter(m_pTunerDevice);
		m_pTunerDevice->Release();
		m_pTunerDevice = NULL;
	}
	if(m_pNetworkProvider)
	{
		m_pFilterGraph->RemoveFilter(m_pNetworkProvider);
		m_pNetworkProvider->Release();
		m_pNetworkProvider = NULL;
	}
	if(m_pProprietaryInterface)
	{
		m_pProprietaryInterface->Release();
		m_pProprietaryInterface = NULL;
	}
	if(m_pTunerControl)
	{
		m_pTunerControl->Release();
		m_pTunerControl = NULL;
	}
#ifdef _DEBUG
	if(m_dwGraphRegister)
	{
		RemoveGraphFromROT(m_dwGraphRegister);
		m_dwGraphRegister = 0;
	}
#endif
	if(pCallbackInstance)
		pCallbackInstance = NULL;
	if(pNetworkProviderInstance)
		pNetworkProviderInstance = NULL;
	m_pFilterGraph->Release();
	m_pFilterGraph = NULL;
}

void CBdaGraph::SetStreamCallbackProcedure(STR_CB_PROC callback)
{
	if(pCallbackInstance)
		pCallbackInstance->SetStreamCallbackProcedure(callback);
}

HRESULT CBdaGraph::DVBS_Tune(
			ULONG LowBandF,
			ULONG HighBandF,
			ULONG SwitchF,
			ULONG Frequency,
			SpectralInversion SpectrInv,
			ModulationType ModType,
			LONG SymRate,
			Polarisation Pol,
			BinaryConvolutionCodeRate Fec,
			LONG PosOpt)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBSTuning(
			LowBandF,
			HighBandF,
			SwitchF,
			Frequency,
			SpectrInv,
			ModType,
			SymRate,
			Pol,
			Fec,
			PosOpt);
	else
		return E_FAIL;
}

HRESULT CBdaGraph::DVBS_TT_Tune(
			ULONG LowBandF,
			ULONG HighBandF,
			ULONG SwitchF,
			ULONG Frequency,
			SpectralInversion SpectrInv,
			ModulationType ModType,
			LONG SymRate,
			Polarisation Pol,
			BinaryConvolutionCodeRate Fec,
			LONG PosOpt)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBSTuning(
			LowBandF,
			HighBandF,
			SwitchF,
			Frequency,
			SpectrInv,
			ModType,
			SymRate,
			Pol,
			Fec,
			PosOpt);
	return E_FAIL;
}

HRESULT CBdaGraph::DVBS_TeVii_Tune(
			ULONG LowBandF,
			ULONG HighBandF,
			ULONG SwitchF,
			ULONG Frequency,
			SpectralInversion SpectrInv,
			ModulationType ModType,
			LONG SymRate,
			Polarisation Pol,
			BinaryConvolutionCodeRate Fec
			)
{
	if (iTVIdx<0)
		return E_FAIL;

	TPolarization TVPol;
	switch(Pol)
	{
	case BDA_POLARISATION_LINEAR_H:
	case BDA_POLARISATION_CIRCULAR_L:
		TVPol=TPol_Horizontal;break;
	case BDA_POLARISATION_LINEAR_V:
	case BDA_POLARISATION_CIRCULAR_R:
		TVPol=TPol_Vertical;break;
	default:
		TVPol=TPol_None;
	}

	TMOD TVMod;
	switch (ModType)
	{
	case BDA_MOD_QPSK:
		TVMod=TMOD_QPSK;break;
	case BDA_MOD_BPSK:
		TVMod=TMOD_BPSK;break;
	case BDA_MOD_16QAM:
		TVMod=TMOD_16QAM;break;
	case BDA_MOD_32QAM:
		TVMod=TMOD_32QAM;break;
	case BDA_MOD_64QAM:
		TVMod=TMOD_64QAM;break;
	case BDA_MOD_128QAM:
		TVMod=TMOD_128QAM;break;
	case BDA_MOD_256QAM:
		TVMod=TMOD_256QAM;break;
	case BDA_MOD_8VSB:
		TVMod=TMOD_8VSB;break;
	case BDA_MOD_NBC_QPSK:
		TVMod=TMOD_DVBS2_QPSK;break;
	case BDA_MOD_NBC_8PSK:
	case BDA_MOD_8PSK:
		TVMod=TMOD_DVBS2_8PSK;break;
	case BDA_MOD_16APSK:
		TVMod=TMOD_DVBS2_16PSK;break;
	case BDA_MOD_32APSK:
		TVMod=TMOD_DVBS2_32PSK;break;
	default:
		TVMod=TMOD_AUTO;
	}

	TFEC TVFec;
	switch(Fec)
	{
	case BDA_BCC_RATE_1_2:
		TVFec=TFEC_1_2;
	case BDA_BCC_RATE_2_3:
		TVFec=TFEC_2_3;
	case BDA_BCC_RATE_3_4:
		TVFec=TFEC_3_4;
	case BDA_BCC_RATE_4_5:
		TVFec=TFEC_4_5;
	case BDA_BCC_RATE_5_6:
		TVFec=TFEC_5_6;
	case BDA_BCC_RATE_6_7:
		TVFec=TFEC_6_7;
	case BDA_BCC_RATE_7_8:
		TVFec=TFEC_7_8;
	case BDA_BCC_RATE_8_9:
		TVFec=TFEC_8_9;
	case BDA_BCC_RATE_9_10:
		TVFec=TFEC_9_10;
	default:
		TVFec=TFEC_AUTO;
	}

	if (TuneTransponder(iTVIdx, Frequency, SymRate*1000, Frequency > SwitchF ? HighBandF:LowBandF, TVPol,
		Frequency > SwitchF, TVMod, TVFec))
		return S_OK;
	else
		return E_FAIL;
}

HRESULT CBdaGraph::DVBS_DvbWorld_Tune(
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
			UINT tone_burst )
{
	if (INVALID_HANDLE_VALUE==hDW)
		return E_FAIL;

	UINT pol=0;
	switch(Pol)
	{
	case BDA_POLARISATION_LINEAR_H:
	case BDA_POLARISATION_CIRCULAR_L:
		pol=1;break;
	case BDA_POLARISATION_LINEAR_V:
	case BDA_POLARISATION_CIRCULAR_R:
		pol=0;break;
	}

	return dwBDATune(hDW,Frequency,SymRate,Frequency > SwitchF ? HighBandF:LowBandF,pol,Frequency > SwitchF,Fec,
		ModType,diseqc_port,tone_burst);
}

HRESULT CBdaGraph::DVBS_Hauppauge_Tune(
			ULONG LowBandF,
			ULONG HighBandF,
			ULONG SwitchF,
			ULONG Frequency,
			SpectralInversion SpectrInv,
			ModulationType ModType,
			LONG SymRate,
			Polarisation Pol,
			BinaryConvolutionCodeRate Fec,
			DWORD S2RollOff,
			DWORD S2Pilot,
			LONG PosOpt)
{
	DWORD ret_len;
	DWORD instance[1024];
	char text[256];
	HRESULT hr;

	if(ModType == BDA_MOD_8PSK)
	{
		// DVB-S2
		// set Pilot
		hr = m_pProprietaryInterface->Set(KSPROPSETID_BdaTunerExtensionPropertiesHaup,
			KSPROPERTY_BDA_PILOT_HAUP,
			instance, 32, &S2Pilot, sizeof(S2Pilot));
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): failed setting Pilot to %d (hr=0x%8.8x)", S2Pilot, hr);
			ReportMessage(text);
//			return E_FAIL;
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): set Pilot to %d", S2Pilot);
			ReportMessage(text);
		}
		hr = m_pProprietaryInterface->Get(KSPROPSETID_BdaTunerExtensionPropertiesHaup,
			KSPROPERTY_BDA_PILOT_HAUP,
			instance, 32, &S2Pilot, sizeof(S2Pilot), &ret_len);
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): failed getting Pilot (hr=0x%8.8x)", hr);
			ReportMessage(text);
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): get Pilot: 0x%8.8x", S2Pilot);
			ReportMessage(text);
		}
		// set Roll Off
		hr = m_pProprietaryInterface->Set(KSPROPSETID_BdaTunerExtensionPropertiesHaup,
			KSPROPERTY_BDA_ROLLOFF_HAUP,
			instance, 32, &S2RollOff, sizeof(S2RollOff));
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): failed setting RollOff to %d (hr=0x%8.8x)", S2RollOff, hr);
			ReportMessage(text);
//			return E_FAIL;
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): set RollOff to %d", S2RollOff);
			ReportMessage(text);
		}
		hr = m_pProprietaryInterface->Get(KSPROPSETID_BdaTunerExtensionPropertiesHaup,
			KSPROPERTY_BDA_ROLLOFF_HAUP,
			instance, 32, &S2RollOff, sizeof(S2RollOff), &ret_len);
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): failed getting RollOff (hr=0x%8.8x)", hr);
			ReportMessage(text);
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (8PSK): get RollOff: 0x%8.8x", S2RollOff);
			ReportMessage(text);
		}
	}
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBSTuning(
			LowBandF,
			HighBandF,
			SwitchF,
			Frequency,
			SpectrInv,
			ModType,
			SymRate,
			Pol,
			Fec,
			PosOpt);
	return E_FAIL;
}

HRESULT CBdaGraph::DVBS_Conexant_Tune(
			ULONG LowBandF,
			ULONG HighBandF,
			ULONG SwitchF,
			ULONG Frequency,
			SpectralInversion SpectrInv,
			ModulationType ModType,
			LONG SymRate,
			Polarisation Pol,
			BinaryConvolutionCodeRate Fec,
			DWORD S2RollOff,
			DWORD S2Pilot,
			LONG PosOpt)
{
	char text[256];
	HRESULT hr;

	if(ModType == BDA_MOD_8PSK)
	{
		// DVB-S2
		BDA_NBC_PARAMS NBCMessageParams;
		switch (S2Pilot)
		{
		case PILOT_OFF:
			NBCMessageParams.pilot = PHANTOM_PILOT_OFF;
			break;
		case PILOT_ON:
			NBCMessageParams.pilot = PHANTOM_PILOT_OFF;
			break;
		default:
			NBCMessageParams.pilot = PHANTOM_PILOT_UNKNOWN;
		}

		switch (S2RollOff)
		{
		case ROLLOFF_20:
			NBCMessageParams.rolloff = PHANTOM_ROLLOFF_020;
			break;
		case ROLLOFF_25:
			NBCMessageParams.rolloff = PHANTOM_ROLLOFF_025;
			break;
		case ROLLOFF_35:
			NBCMessageParams.rolloff = PHANTOM_ROLLOFF_035;
			break;
		default:
			NBCMessageParams.pilot = PHANTOM_PILOT_UNKNOWN;
		}

		KSPROPERTY instance_data;

		// set NBC Params
		hr = m_pProprietaryInterface->Set(KSPROPSETID_BdaTunerExtensionProperties,
			KSPROPERTY_BDA_NBC_PARAMS,
			&instance_data, sizeof(instance_data), &NBCMessageParams, sizeof(NBCMessageParams));
		if FAILED(hr)
		{
			sprintf(text,"BDA2: DVBS_Conexant_Tune (8PSK): failed setting Pilot, RollOff to (hr=0x%8.8x)", hr);
			ReportMessage(text);
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Conexant_Tune (8PSK): set Pilot, RollOff");
			ReportMessage(text);
		}
	}
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBSTuning(
			LowBandF,
			HighBandF,
			SwitchF,
			Frequency,
			SpectrInv,
			ModType,
			SymRate,
			Pol,
			Fec,
			PosOpt);
	return E_FAIL;
}

HRESULT CBdaGraph::DVBT_Tune(
			ULONG Frequency,
			ULONG Bandwidth)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBTTuning(Frequency, Bandwidth);
	else
		return E_FAIL;
}

HRESULT CBdaGraph::DVBC_Tune(
	ULONG Frequency,
	LONG SymRate,
	ModulationType ModType)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBCTuning(Frequency, SymRate, ModType);
	else
		return E_FAIL;
}

HRESULT CBdaGraph::GetSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->GetSignalStatistics(pPresent, pLocked, pStrength, pQuality);
	else
		return E_FAIL;
}

HRESULT CBdaGraph::GetTechnotrendSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality)
{
	if (INVALID_HANDLE_VALUE==hTT)
		return E_FAIL;

	TYPE_RET_VAL rc;
	DWORD stat[4];
	rc = bdaapiGetTuneStats (hTT,stat,sizeof(stat));
	if (rc)
		return E_FAIL;
	*pLocked=stat[2];
	*pStrength=stat[1];
	*pQuality=stat[3];
	*pPresent=stat[0];
	return S_OK;
}

HRESULT CBdaGraph::GetTeViiSignalStatistics(BOOLEAN *pPresent, BOOLEAN *pLocked, LONG *pStrength, LONG *pQuality)
{
	if (iTVIdx<0)
		return E_FAIL;

	DWORD Strength, Quality;
	BOOL Lock;
	if (GetSignalStatus(iTVIdx, &Lock, &Strength, &Quality))
	{
		*pLocked=Lock;
		*pStrength=Strength;
		*pQuality=Quality;
		*pPresent=TRUE;
		return S_OK;
	}
	else
		return E_FAIL;
}

char* ErrorMessageTTBDA ( TYPE_RET_VAL err )
{
	char *err_str;
	switch ( err )
	{
	case RET_SUCCESS:
		return NULL;
	case RET_NOT_IMPL:
		err_str = "operation is not implemented for the opened handle";
		break;
	case RET_NOT_SUPPORTED:
    	err_str = "operation is not supported for the opened handle";
		break;
	case RET_ERROR_HANDLE:
    	err_str = "the given HANDLE seems not to be correct";
		break;
	case RET_IOCTL_NO_DEV_HANDLE:
    	err_str = "the internal IOCTL subsystem has no device handle";
		break;
	case RET_IOCTL_FAILED:
        err_str = "the internal IOCTL failed";
		break;
	case RET_IR_ALREADY_OPENED:
        err_str = "the infrared interface is already initialised";
		break;
	case RET_IR_NOT_OPENED:
        err_str = "the infrared interface is not initialised";
		break;
	case RET_TO_MANY_BYTES:
		err_str = "length exceeds maximum in EEPROM-Userspace operation";
		break;
	case RET_CI_ERROR_HARDWARE:
		err_str = "common interface hardware error";
		break;
	case RET_CI_ALREADY_OPENED:
		err_str = "common interface already opened";
		break;
	case RET_TIMEOUT:
		err_str = "operation finished with timeout";
		break;
	case RET_READ_PSI_FAILED:
		err_str = "read psi failed";
		break;
	case RET_NOT_SET:
		err_str = "not set";
		break;
	case RET_ERROR:
		err_str = "operation finished with general error";
		break;
	case RET_ERROR_POINTER:
		err_str = "operation finished with illegal pointer";
		break;
	case RET_INCORRECT_SIZE:
		err_str = "the tunerequest structure did not have the expected size";
		break;
	case RET_TUNER_IF_UNAVAILABLE:
		err_str = "the tuner interface was not available";
		break;
	case RET_UNKNOWN_DVB_TYPE:
		err_str = "an unknown DVB type has been specified for the tune request";
		break;
	case RET_BUFFER_TOO_SMALL:
		err_str = "length of buffer is too small";
		break;
	default:
		err_str = "unknown error";
	}
	return err_str;
}

HRESULT CBdaGraph::DVBS_Technotrend_DiSEqC(BYTE len, BYTE *DiSEqC_Command, BYTE tb)
{
	if (INVALID_HANDLE_VALUE==hTT)
		return E_FAIL;

	TYPE_RET_VAL rc;
	char text[256];
	rc = bdaapiSetDiSEqCMsg(hTT,DiSEqC_Command,len,0,tb,BDA_POLARISATION_LINEAR_H);
	if (rc)
	{
		sprintf(text,"BDA2: DVBS_Technotrend_DiSEqC: failed - %s", ErrorMessageTTBDA(rc));
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Technotrend_DiSEqC: success");
	ReportMessage(text);
	return S_OK;
}

HRESULT CBdaGraph::DVBS_TeVii_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>6))
		return E_INVALIDARG;

	if (iTVIdx<0)
		return E_FAIL;

	if (SendDiSEqC(iTVIdx, DiSEqC_Command, len, 0, FALSE))
	{
		ReportMessage("BDA2: DVBS_TeVii_DiSEqC: success");
		return S_OK;
	}
	else
	{
		ReportMessage("BDA2: DVBS_TeVii_DiSEqC: failed");
		return E_FAIL;
	}
}

HRESULT CBdaGraph::DVBS_Twinhan_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>12))
		return E_INVALIDARG;

	DiSEqC_DATA diseqc_cmd;
	diseqc_cmd.command_len = len;
	memcpy(diseqc_cmd.command, DiSEqC_Command, len);
	if (THBDA_IOCTL_SET_DiSEqC_Fun(&diseqc_cmd))
	{
		ReportMessage("BDA2: DVBS_Twinhan_DiSEqC: success");
		return S_OK;
	}
	ReportMessage("BDA2: DVBS_Twinhan_DiSEqC: failed");
	return E_FAIL;
}

HRESULT CBdaGraph::DVBS_Twinhan_LNBPower(BOOL bPower)
{
	LNB_DATA lnb_data;
	if (THBDA_IOCTL_GET_LNB_DATA_Fun(&lnb_data))
	{
		lnb_data.LNB_POWER = bPower ? LNB_POWER_ON : LNB_POWER_OFF;
		if (THBDA_IOCTL_SET_LNB_DATA_Fun(&lnb_data))
		{
			ReportMessage("BDA2: DVBS_Twinhan_LNBPower: success");
			return S_OK;
		}
	}
	ReportMessage("BDA2: DVBS_Twinhan_LNBPower: failed");
	return S_OK;
}

HRESULT CBdaGraph::DVBS_Twinhan_LNBSource (BYTE Port, BYTE ToneBurst)
{
	LNB_DATA lnb_data;
	if (THBDA_IOCTL_GET_LNB_DATA_Fun(&lnb_data))
	{
		lnb_data.DiSEqC_Port = Port;
		lnb_data.Tone_Data_Burst = ToneBurst;
		if (THBDA_IOCTL_SET_LNB_DATA_Fun(&lnb_data))
		{
			ReportMessage("BDA2: DVBS_Twinhan_LNBSource: success");
			return S_OK;
		}
	}
	ReportMessage("BDA2: DVBS_Twinhan_LNBSource: failed");
	return S_OK;
}

HRESULT CBdaGraph::DVBS_DvbWorld_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>6))
		return E_INVALIDARG;

	if (INVALID_HANDLE_VALUE==hDW)
		return E_FAIL;

	char text[256];
	HRESULT hr = dwBDADiseqSendCommand(hDW,DiSEqC_Command,len);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_DvbWorld_DiSEqC: failed sending DiSEqC command(0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_DvbWorld_DiSEqC: sent DiSEqC command");
	ReportMessage(text);
	return S_OK;
}
