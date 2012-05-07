#include "stdafx.h"
#include "BdaGraph.h"

//THBDA Ioctl functions
HRESULT CBdaGraph::THBDA_IOControl( DWORD  dwIoControlCode,
										  LPVOID lpInBuffer,
										  DWORD  nInBufferSize,
										  LPVOID lpOutBuffer,
										  DWORD  nOutBufferSize,
										  LPDWORD lpBytesReturned)
{
    CheckPointer(m_pKsTunerPropSet, E_NOINTERFACE)

    KSPROPERTY instance_data;

    ULONG    ulOutBuf = 0;
    ULONG    ulReturnBuf = 0;
    THBDACMD THBDACmd;

    THBDACmd.CmdGUID = GUID_THBDA_CMD;
    THBDACmd.dwIoControlCode = dwIoControlCode;
    THBDACmd.lpInBuffer = lpInBuffer;
    THBDACmd.nInBufferSize = nInBufferSize;
    THBDACmd.lpOutBuffer = lpOutBuffer;
    THBDACmd.nOutBufferSize = nOutBufferSize;
    THBDACmd.lpBytesReturned = lpBytesReturned;

    return m_pKsTunerPropSet->Set(GUID_THBDA_TUNER, 
                              NULL, 
	  						  &instance_data, sizeof(instance_data),
                              &THBDACmd, sizeof(THBDACmd));
}


//***************************************************************//
//************** Basic IOCTL sets  (must support) ***************//
//***************************************************************//

HRESULT CBdaGraph::THBDA_IOCTL_CHECK_INTERFACE_Fun(void)
{
    DWORD dwBytesReturned = 0;
    return THBDA_IOControl((DWORD) THBDA_IOCTL_CHECK_INTERFACE,
									NULL,
									0,
									NULL,
									0,
									(LPDWORD)&dwBytesReturned);
}

//***************************************************************//
//********************* DVB-S (must support)*********************//
//***************************************************************//

HRESULT CBdaGraph::THBDA_IOCTL_SET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   dwBytesReturned  = 0;	

	return THBDA_IOControl(	(DWORD)THBDA_IOCTL_SET_LNB_DATA,
							(LPVOID)pLNB_DATA, 
							sizeof(LNB_DATA),     
							NULL, 
							0,                    
							(LPDWORD)&dwBytesReturned);
}

HRESULT CBdaGraph::THBDA_IOCTL_GET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   dwBytesReturned  = 0;	

	return THBDA_IOControl(	(DWORD)THBDA_IOCTL_GET_LNB_DATA,							  
							NULL, 
							0,
							(LPVOID)pLNB_DATA, 
							sizeof(LNB_DATA),                       
							(LPDWORD)&dwBytesReturned);
}

HRESULT CBdaGraph::THBDA_IOCTL_SET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA)
{	
	DWORD   dwBytesReturned  = 0;
	return THBDA_IOControl(	(DWORD)THBDA_IOCTL_SET_DiSEqC,
							(LPVOID)pDiSEqC_DATA, 
							sizeof(DiSEqC_DATA),     
							NULL, 
							0,                    
							(LPDWORD)&dwBytesReturned);
}

HRESULT CBdaGraph::THBDA_IOCTL_GET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA)
{	
	DWORD   dwBytesReturned  = 0;
	return THBDA_IOControl(	(DWORD)THBDA_IOCTL_GET_DiSEqC,							  
							NULL, 
							0,
							(LPVOID)pDiSEqC_DATA, 
							sizeof(DiSEqC_DATA),                       
							(LPDWORD)&dwBytesReturned);
}
