#include "stdafx.h"

#include "BdaGraph.h"

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
	m_pMediaControl = NULL;
	pCallbackInstance = NULL;
	pNetworkProviderInstance = NULL;
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
							Tuners->Count += 1;
						}
						else
							DebugLog("BDA2: DeviceDescription: Failed getting network tuner type");
						pFilter->Release();
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

HRESULT CBdaGraph::BuildGraph(int selected_device_enum, enum VENDOR_SPECIFIC *VendorSpecific)
{
	char text[128];
	BSTR friendly_name_receiver;

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
		char receiver_name[64];
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
	// let's look if Tuner exposes proprietary interfaces
	*VendorSpecific = VENDOR_SPECIFIC(PURE_BDA);
	hr = m_pP2->QueryInterface(IID_IKsPropertySet, (void **)&m_pProprietaryInterface);
	if(SUCCEEDED(hr))
	{
		sprintf(text,"BDA2: BuildGraph: Tuner exposes proprietary interfaces");
		ReportMessage(text);
		DWORD supported;
		// Hauppauge
		DebugLog("BDA2: BuildGraph: checking for Hauppauge DiSEqC interface");
		hr = m_pProprietaryInterface->QuerySupported(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_DISEQC), &supported);
		if(SUCCEEDED(hr) && supported)
		{
			DebugLog("BDA2: BuildGraph: found Hauppauge DiSEqC interface");
			*VendorSpecific = VENDOR_SPECIFIC(HAUP_BDA);
		}
	}
	hr = S_OK;
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
			LONG PosOpt,
			Polarisation Pol)
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
			PosOpt,
			Pol);
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
			Polarisation Pol)
{
	if(pNetworkProviderInstance)
		return pNetworkProviderInstance->DoDVBSTuning_DiSEqC(
			LowBandF,
			HighBandF,
			SwitchF,
			Frequency,
			SpectrInv,
			ModType,
			SymRate,
			Pol);
	else
		return E_FAIL;
}

HRESULT CBdaGraph::DVBS_Hauppauge_Tune(
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
			BOOLEAN RawDiSEqC)
{
	DWORD ret_len;
	DWORD instance[1024];
	char text[256];
	HRESULT hr;

	if(ModType == BDA_MOD_QPSK || ModType == BDA_MOD_NOT_DEFINED)
	{
		// DVB-S
		// set Pilot to OFF
/*		tmp_dword = 0;
		hr = m_pProprietaryInterface->Set(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_PILOT),
			instance, sizeof(instance), &tmp_dword, sizeof(tmp_dword));
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): failed setting Pilot to OFF (hr=0x%8.8x)",hr);
			ReportMessage(text);
//			return E_FAIL;
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): set Pilot to OFF");
			ReportMessage(text);
		}
		// set Roll Off
		hr = m_pProprietaryInterface->Set(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_ROLL_OFF),
			instance, sizeof(instance), &RollOff, sizeof(RollOff));
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): failed setting RollOff to #%d (hr=0x%8.8x)", RollOff, hr);
			ReportMessage(text);
//			return E_FAIL;
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): set RollOff to #%d", RollOff);
			ReportMessage(text);
		}*/
/*		hr = m_pProprietaryInterface->Get(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_PILOT),
			instance, sizeof(instance), &tmp_dword, sizeof(tmp_dword), &ret_len);
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): failed getting Pilot (hr=0x%8.8x)",hr);
			ReportMessage(text);
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): get Pilot 0x%8.8x", tmp_dword);
			ReportMessage(text);
		}
		hr = m_pProprietaryInterface->Get(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_ROLL_OFF),
			instance, sizeof(instance), &RollOff, sizeof(RollOff), &ret_len);
		if(FAILED(hr))
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): failed getting RollOff (hr=0x%8.8x)", hr);
			ReportMessage(text);
		}
		else
		{
			sprintf(text,"BDA2: DVBS_Hauppauge_Tune (QPSK): get RollOff: 0x%8.8x", RollOff);
			ReportMessage(text);
		}*/
	}
	else
	{
		// DVB-S2
		// set Pilot
		hr = m_pProprietaryInterface->Set(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_PILOT),
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
		hr = m_pProprietaryInterface->Get(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_PILOT),
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
		hr = m_pProprietaryInterface->Set(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_ROLL_OFF),
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
		hr = m_pProprietaryInterface->Get(CLSID_HauppaugeBdaTunerExtension,
			HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_ROLL_OFF),
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
		if(RawDiSEqC)
			return pNetworkProviderInstance->DoDVBSTuning_DiSEqC(
				LowBandF,
				HighBandF,
				SwitchF,
				Frequency,
				SpectrInv,
				ModType,
				SymRate,
				Pol);
		else
			return pNetworkProviderInstance->DoDVBSTuning(
				LowBandF,
				HighBandF,
				SwitchF,
				Frequency,
				SpectrInv,
				ModType,
				SymRate,
				PosOpt,
				Pol);
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

HRESULT CBdaGraph::DVBS_Hauppauge_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	int i;
	BYTE buf[188]; // DISEQC_MESSAGE_PARAMS
	HRESULT hr;
	char text[256];

	for(i=0;i<len;++i)
		buf[i] = DiSEqC_Command[i];
	for(;i<188;++i)
		buf[i] = 0x0;
	*((int *)(buf+160)) = len; //send_message_length
	*((int *)(buf+164)) = 0; //receive_message_length
	*((int *)(buf+168)) = 3; //amplitude_attenuation
	buf[172] = (BYTE)1; //tone_burst_modulated
	buf[176] = (BYTE)(HauppaugeDiSEqCVersion(DISEQC_VER_1X));
	buf[180] = (BYTE)(HauppaugeRxMode(RXMODE_NOREPLY));
	buf[184] = (BYTE)1; // last_message

	hr = m_pProprietaryInterface->Set(CLSID_HauppaugeBdaTunerExtension,
		HauppaugeBdaTunerExtension(Hauppauge_KSPROPERTY_BDA_DISEQC),
		&buf, sizeof(buf), &buf, sizeof(buf));
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_Hauppauge_DiSEqC: failed sending DISEQC_MESSAGE_PARAMS (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Hauppauge_DiSEqC: sent DISEQC_MESSAGE_PARAMS");
	ReportMessage(text);

	return S_OK;
}
