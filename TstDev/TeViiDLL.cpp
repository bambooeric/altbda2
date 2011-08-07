#include "stdafx.h"
#include "TeViiDLL.h"

int (*TeVii_GetAPIVersion)() = NULL;
int (*TeVii_FindDevices)() = NULL;
const char* (*TeVii_GetDeviceName)(int idx) = NULL;
const char* (*TeVii_GetDevicePath)(int idx) = NULL;
BOOL (*TeVii_OpenDevice)(int idx, lpCapFunc func, void* lParam) = NULL;
BOOL (*TeVii_CloseDevice)(int idx) = NULL;
BOOL (*TeVii_TuneTransponder)(int idx, DWORD Freq, DWORD SymbRate, DWORD LOF, TPolarization Pol, BOOL  F22KHz, TMOD MOD, TFEC FEC) = NULL;
BOOL (*TeVii_GetSignalStatus)(int idx, BOOL* IsLocked, DWORD* Strength, DWORD* Quality) = NULL;
BOOL (*TeVii_SendDiSEqC)(int idx, BYTE* Data, DWORD Len, DWORD Repeats, BOOL Flg) = NULL;
BOOL (*TeVii_SetRemoteControl)(int idx, lpRCFunc lpCallback, void* lParam) = NULL;

static HMODULE h=NULL;

int LoadTeViiDLL()
{
	if (h > (HANDLE)31)
		return 0;
	h = LoadLibrary("TeVii.dll");
	if (h < (HANDLE)32)
		return -1;
#define A(a,b) (FARPROC&)b = GetProcAddress(h, #a); if(!b) { FreeLibrary(h); return -2; }
#define O(a,b) (FARPROC&)b = GetProcAddress(h, #a);
	A(GetAPIVersion,TeVii_GetAPIVersion);
	A(FindDevices,TeVii_FindDevices);
	A(GetDeviceName,TeVii_GetDeviceName);
	A(GetDevicePath,TeVii_GetDevicePath);
	A(OpenDevice,TeVii_OpenDevice);
	A(CloseDevice,TeVii_CloseDevice);
	A(TuneTransponder,TeVii_TuneTransponder);
	A(GetSignalStatus,TeVii_GetSignalStatus);
	A(SendDiSEqC,TeVii_SendDiSEqC);
	A(SetRemoteControl,TeVii_SetRemoteControl);
	return 0;
}

void UnloadTeViiDLL()
{
	if (h > (HANDLE)31)
		FreeLibrary(h);
	h = 0;
}
