#include "stdafx.h"

#include "BdaGraph.h"
#include "omcbda.h"

#define Z(a) memset(&a, 0, sizeof(a))

HRESULT CBdaGraph::DVBS_Omicom_ToneBurst(BYTE ToneBurst)
{
    CheckPointer(m_pTunerControl,E_POINTER);

	char text[256];
	HRESULT hr=S_OK;
	KSPROPERTY prop;
	Z(prop);
	prop.Set = KSPROPSETID_OMCDiSEqCProperties;
	prop.Id = KSPROPERTY_OMC_DISEQC_SET22K;
	prop.Flags = KSPROPERTY_TYPE_SET;
    ULONG bytesReturned = 0;
	DWORD dw22Khz = ToneBurst;

	hr = m_pTunerControl->KsProperty(&prop,sizeof(prop),
		(void*)&dw22Khz, sizeof(dw22Khz),&bytesReturned);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_Omicom_ToneBurst: failed sending KSPROPERTY_OMC_DISEQC_SET22K (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Omicom_ToneBurst: sent KSPROPERTY_OMC_DISEQC_SET22K");
	ReportMessage(text);

	return S_OK;
}

HRESULT CBdaGraph::DVBS_Omicom_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
    CheckPointer(m_pTunerControl,E_POINTER);
	CheckPointer(DiSEqC_Command,E_POINTER);

	OMC_BDA_DISEQC_DATA diseqc_data;
	if ( (len==0) || (len>sizeof(diseqc_data.pBuffer)) )
		return E_INVALIDARG;
	memcpy(diseqc_data.pBuffer,DiSEqC_Command,len);
	diseqc_data.nLen = len;
	diseqc_data.nRepeatCount = 0;

	char text[256];
	HRESULT hr=S_OK;
	KSPROPERTY prop;
	Z(prop);
	prop.Set = KSPROPSETID_OMCDiSEqCProperties;
	prop.Id = KSPROPERTY_OMC_DISEQC_WRITE;
	prop.Flags = KSPROPERTY_TYPE_SET;
    ULONG bytesReturned = 0;

	hr = m_pTunerControl->KsProperty(&prop,sizeof(prop),
		(void*)&diseqc_data, sizeof(diseqc_data),&bytesReturned);

	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_Omicom_DiSEqC: failed sending KSPROPERTY_OMC_DISEQC_WRITE (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Omicom_DiSEqC: sent KSPROPERTY_OMC_DISEQC_WRITE");
	ReportMessage(text);

	return S_OK;
}
