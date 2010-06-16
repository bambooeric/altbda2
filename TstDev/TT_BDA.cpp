#include "Configuration.h"


BOOLEAN TT_BDA_MatchDevice(HANDLE *ttHandle, char *selected_device_name)
{
	int i, n;
	TS_FilterNames filter_names;

	// look for Budget 2
	DebugLog("BDA2: TT_BDA_MatchDevice: looking for BUDGET_2");
	n = bdaapiEnumerate(BUDGET_2);
	DebugLog("BDA2: TT_BDA_MatchDevice: enumerated %d BUDGET_2", n);
	for(i=0;i<n;++i)
	{
		*ttHandle = bdaapiOpen(BUDGET_2, i);
		if(bdaapiGetDevNameAndFEType(*ttHandle, &filter_names) == RET_SUCCESS)
		{
			if(!strcmp(selected_device_name, filter_names.szTunerFilterName))
			{
				DebugLog("BDA2: TT_BDA_MatchDevice: found matching BUDGET_2 (i=%d)", i);
				return TRUE;
			}
		}
		bdaapiClose(*ttHandle);
	}
	// look for BUDGET_3
	DebugLog("BDA2: TT_BDA_MatchDevice: looking for BUDGET_3");
	n = bdaapiEnumerate(BUDGET_3);
	DebugLog("BDA2: TT_BDA_MatchDevice: enumerated %d BUDGET_3", n);
	for(i=0;i<n;++i)
	{
		*ttHandle = bdaapiOpen(BUDGET_3, i);
		if(bdaapiGetDevNameAndFEType(*ttHandle, &filter_names) == RET_SUCCESS)
		{
			if(!strcmp(selected_device_name, filter_names.szTunerFilterName))
			{
				DebugLog("BDA2: TT_BDA_MatchDevice: found matching BUDGET_3 (i=%d)", i);
				return TRUE;
			}
		}
		bdaapiClose(*ttHandle);
	}
	return FALSE;
	// look for USB_2
	DebugLog("BDA2: TT_BDA_MatchDevice: looking for USB_2");
	n = bdaapiEnumerate(USB_2);
	DebugLog("BDA2: TT_BDA_MatchDevice: enumerated %d USB_2", n);
	for(i=0;i<n;++i)
	{
		*ttHandle = bdaapiOpen(USB_2, i);
		if(bdaapiGetDevNameAndFEType(*ttHandle, &filter_names) == RET_SUCCESS)
		{
			if(!strcmp(selected_device_name, filter_names.szTunerFilterName))
			{
				DebugLog("BDA2: TT_BDA_MatchDevice: found matching USB_2 (i=%d)", i);
				return TRUE;
			}
		}
		bdaapiClose(*ttHandle);
	}
	DebugLog("BDA2: TT_BDA_MatchDevice: failed finding matching TT device");
	return FALSE;
}

void TT_BDA_Close(HANDLE handle)
{
	bdaapiClose(handle);
}

int TT_BDA_Send_DiSEqC(HANDLE handle, BYTE len, BYTE *data)
{
	int reply;

	reply = bdaapiSetDiSEqCMsg(handle, data, len, 0, 0, BDA_POLARISATION_LINEAR_H);
	if(reply)
		DebugLog("BDA2: TT_BDA_Send_DiSEqC ERROR");
	else
		DebugLog("BDA2: TT_BDA_Send_DiSEqC OK");

	return reply;
}