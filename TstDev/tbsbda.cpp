#include "stdafx.h"

#include "BdaGraph.h"
#include "tbsbda.h"

HRESULT CBdaGraph::DVBS_Turbosight_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>6))
		return E_INVALIDARG;

	HRESULT hr;
	ULONG bytesReturned = 0;
	char text[256];

	DISEQC_MESSAGE_PARAMS diseqc_msg;
	memset(&diseqc_msg,0,sizeof(diseqc_msg));
	memcpy(diseqc_msg.uc_diseqc_send_message, DiSEqC_Command, len);
	diseqc_msg.uc_diseqc_send_message_length = len;
	diseqc_msg.uc_diseqc_receive_message_length = 0;
	diseqc_msg.Tone_Data_Burst = PHANTOM_LNB_TONEBURST_DISABLE;
	diseqc_msg.burst_tone = PHANTOM_LNB_BURST_MODULATED;
	diseqc_msg.tbscmd_mode = TBSDVBSCMD_MOTOR;
	diseqc_msg.receive_mode = PHANTOM_RXMODE_NOREPLY;
	diseqc_msg.b_last_message = TRUE;

	hr = m_pKsTunerPropSet->Get(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_DISEQC_MESSAGE,
		&diseqc_msg, sizeof(diseqc_msg), &diseqc_msg, sizeof(diseqc_msg), &bytesReturned);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_Turbosight_DiSEqC: failed sending DISEQC_MESSAGE_PARAMS (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Turbosight_DiSEqC: sent DISEQC_MESSAGE_PARAMS");
	ReportMessage(text);

	return S_OK;
}

HRESULT CBdaGraph::DVBS_Turbosight_LNBPower(BOOL bPower)
{
	HRESULT hr;
	ULONG bytesReturned = 0;
	char text[256];

	DISEQC_MESSAGE_PARAMS diseqc_msg;
	memset(&diseqc_msg,0,sizeof(diseqc_msg));
    diseqc_msg.tbscmd_mode = TBSDVBSCMD_LNBPOWER;
    diseqc_msg.b_LNBPower = bPower;

	hr = m_pKsTunerPropSet->Get(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_DISEQC_MESSAGE,
		&diseqc_msg, sizeof(diseqc_msg), &diseqc_msg, sizeof(diseqc_msg), &bytesReturned);
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_Turbosight_LNBPower: failed set LNB Power state (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_Turbosight_LNBPower: set LNB Power state");
	ReportMessage(text);

	return S_OK;
}
