#ifndef TT_BDA_H
#define TT_BDA_H

#include "stdafx.h"
#include "bdaapi_typedefs.h"
#include "ttBdaDrvApi.h"

//#define TT_IOCTL_SEND_DISEQC     0x233468
//#define TT_IOCTL_GET_DRV_VERSION 0x233464

BOOLEAN TT_BDA_MatchDevice(HANDLE *, char *);
int TT_BDA_Send_DiSEqC(HANDLE, BYTE, BYTE *);
void TT_BDA_Close(HANDLE);

#endif TT_BDA_H