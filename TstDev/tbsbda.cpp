#include "stdafx.h"

#include "BdaGraph.h"
#include "tbsbda.h"
#include "qboxbda.h"

HRESULT CBdaGraph::DVBS_TurbosightNXP_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(m_pKsTunerPropSet,E_NOINTERFACE);
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>6))
		return E_INVALIDARG;

	HRESULT hr;
	char text[256];

	TBSDISEQC_MESSAGE_PARAMS diseqc_msg;
	memset(&diseqc_msg,0,sizeof(diseqc_msg));
	CopyMemory(diseqc_msg.uc_diseqc_send_message, DiSEqC_Command, len);
	diseqc_msg.uc_diseqc_send_message_length = len;

	hr = m_pKsTunerPropSet->Set(KSPROPSETID_BdaTunerExtensionProperties,
		KSPROPERTY_BDA_DISEQC_MESSAGE,
		&diseqc_msg, sizeof(diseqc_msg), &diseqc_msg, sizeof(diseqc_msg));
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_TurbosightNXP_DiSEqC: failed sending TBSDISEQC_MESSAGE_PARAMS (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_TurbosightNXP_DiSEqC: sent TBSDISEQC_MESSAGE_PARAMS");
	ReportMessage(text);

	return S_OK;
}

HRESULT CBdaGraph::DVBS_Turbosight_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(m_pKsTunerPropSet,E_NOINTERFACE);
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
	diseqc_msg.Tone_Data_Burst = PHANTOM_LNB_BURST_DISABLE;
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
	CheckPointer(m_pKsTunerPropSet,E_NOINTERFACE);
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

HRESULT CBdaGraph::DVBS_TurbosightQBOX_DiSEqC(BYTE len, BYTE *DiSEqC_Command)
{
	CheckPointer(m_pKsTunerFilterPropSet,E_NOINTERFACE);
	CheckPointer(DiSEqC_Command,E_POINTER);
	if ((len==0) || (len>5))
		return E_INVALIDARG;

	HRESULT hr;
	ULONG bytesReturned = 0;
	char text[256];

	QBOXDVBSCMD QBOXCmd;
    ZeroMemory(&QBOXCmd, sizeof(QBOXCmd));
	CopyMemory(QBOXCmd.motor, DiSEqC_Command, len);

	hr = m_pKsTunerFilterPropSet->Set(KSPROPERTYSET_QBOXControl,
		KSPROPERTY_CTRL_MOTOR,
		NULL, 0,
		&QBOXCmd, sizeof(QBOXCmd));
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_TurbosightQBOX_DiSEqC: failed sending DISEQC_MESSAGE_PARAMS (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_TurbosightQBOX_DiSEqC: sent DISEQC_MESSAGE_PARAMS");
	ReportMessage(text);

	return S_OK;
}

HRESULT CBdaGraph::DVBS_TurbosightQBOX_LNBPower(BOOL bLNBPower)
{
	CheckPointer(m_pKsTunerFilterPropSet,E_NOINTERFACE);
	HRESULT hr;
	ULONG bytesReturned = 0;
	char text[256];

	QBOXDVBSCMD QBOXCmd;
	DWORD type_support = 0;

    ZeroMemory(&QBOXCmd, sizeof(QBOXCmd));
	QBOXCmd.LNB_POWER = bLNBPower;

	hr = m_pKsTunerFilterPropSet->Set(KSPROPERTYSET_QBOXControl,
		KSPROPERTY_CTRL_LNBPW,
		NULL, 0,
		&QBOXCmd, sizeof(QBOXCmd));
	if(FAILED(hr))
	{
		sprintf(text,"BDA2: DVBS_TurbosightQBOX_LNBPower: failed set LNB Power state (0x%8.8x)", hr);
		ReportMessage(text);
		return E_FAIL;
	}
	sprintf(text,"BDA2: DVBS_TurbosightQBOX_LNBPower: set LNB Power state");
	ReportMessage(text);

	return S_OK;
}
