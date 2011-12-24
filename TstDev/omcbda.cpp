#include "stdafx.h"
#include "BdaGraph.h"
#include "omcbda.h"

HRESULT CBdaGraph::DVBS_Omicom_Set22Khz(BOOL b22Khz)
{
   CheckPointer(m_pKsTunerFilterPropSet,E_NOINTERFACE);

   char text[256];
   HRESULT hr=S_OK;
   DWORD supported;

  	hr = m_pKsTunerFilterPropSet->QuerySupported(KSPROPSETID_OmcDiSEqCProperties,
		KSPROPERTY_OMC_DISEQC_SET22K, &supported);

	if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_SET) )
	{
	  	hr = m_pKsTunerFilterPropSet->Set(KSPROPSETID_OmcDiSEqCProperties,
			KSPROPERTY_OMC_DISEQC_SET22K,
			NULL, 0,
			&b22Khz, sizeof(b22Khz));
		if FAILED(hr)
			sprintf(text,"BDA2: DVBS_Omicom_Set22Khz: failed sending KSPROPERTY_OMC_DISEQC_SET22K (0x%8.8x)", hr);
		else
			sprintf(text,"BDA2: DVBS_Omicom_Set22Khz: sent KSPROPERTY_OMC_DISEQC_SET22K");
		ReportMessage(text);
	}
	else
	{
		hr = E_NOTIMPL;
		DebugLog("BDA2: DVBS_Omicom_Set22Khz: KSPROPERTY_OMC_DISEQC_SET22K not supported !");
	}

   return hr;
}

HRESULT CBdaGraph::DVBS_Omicom_ToneBurst(BOOL bToneBurst)
{
    CheckPointer(m_pKsTunerFilterPropSet,E_NOINTERFACE);

	char text[256];
	HRESULT hr=S_OK;
	DWORD supported;

  	hr = m_pKsTunerFilterPropSet->QuerySupported(KSPROPSETID_OmcDiSEqCProperties,
		KSPROPERTY_OMC_DISEQC_TONEBURST, &supported);

	if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_SET) )
	{
	  	hr = m_pKsTunerFilterPropSet->Set(KSPROPSETID_OmcDiSEqCProperties,
			KSPROPERTY_OMC_DISEQC_TONEBURST,
			NULL, 0,
			&bToneBurst, sizeof(bToneBurst));
		if FAILED(hr)
			sprintf(text,"BDA2: DVBS_Omicom_ToneBurst: failed sending KSPROPERTY_OMC_DISEQC_TONEBURST (0x%8.8x)", hr);
		else
			sprintf(text,"BDA2: DVBS_Omicom_ToneBurst: sent KSPROPERTY_OMC_DISEQC_TONEBURST");
		ReportMessage(text);
	}
	else
	{
		hr = E_NOTIMPL;
		DebugLog("BDA2: DVBS_Omicom_ToneBurst: KSPROPERTY_OMC_DISEQC_TONEBURST not supported !");
	}

   return hr;
}

HRESULT CBdaGraph::DVBS_Omicom_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
    CheckPointer(m_pKsTunerFilterPropSet,E_NOINTERFACE);
	CheckPointer(DiSEqC_Command,E_INVALIDARG);

	OMC_BDA_DISEQC_DATA diseqc_data;
	if ( (len==0) || (len>sizeof(diseqc_data.pBuffer)) )
		return E_INVALIDARG;

	CopyMemory(diseqc_data.pBuffer,DiSEqC_Command,len);
	diseqc_data.nLen = len;
	diseqc_data.nRepeatCount = 0;

	char text[256];
	HRESULT hr=S_OK;
	DWORD supported;

  	hr = m_pKsTunerFilterPropSet->QuerySupported(KSPROPSETID_OmcDiSEqCProperties,
		KSPROPERTY_OMC_DISEQC_WRITE, &supported);

	if ( SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_SET) )
	{
	  	hr = m_pKsTunerFilterPropSet->Set(KSPROPSETID_OmcDiSEqCProperties,
			KSPROPERTY_OMC_DISEQC_WRITE,
			NULL, 0,
			&diseqc_data, sizeof(diseqc_data));
		if FAILED(hr)
			sprintf(text,"BDA2: DVBS_Omicom_DiSEqC: failed sending KSPROPERTY_OMC_DISEQC_WRITE (0x%8.8x)", hr);
		else
			sprintf(text,"BDA2: DVBS_Omicom_DiSEqC: sent KSPROPERTY_OMC_DISEQC_WRITE");
		ReportMessage(text);
	}
	else
	{
		hr = E_NOTIMPL;
		DebugLog("BDA2: DVBS_Omicom_DiSEqC: KSPROPERTY_OMC_DISEQC_WRITE not supported !");
	}

   return hr;
}
