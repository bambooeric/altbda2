/* AltDVB v2.2 device interface, by Diodato */
/* PASCAL calling convention */

#include "stdafx.h"
#include "DvbDeviceControl.h"

CDvbDeviceControl *wrapper = NULL;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		wrapper = new CDvbDeviceControl(hModule);
		if(!wrapper) return FALSE;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		if(wrapper)
			delete(wrapper);
		break;
	case DLL_THREAD_DETACH:
		break;
	}
    return TRUE;
}

int __declspec(dllexport) __stdcall DvbDeviceControl(int id, char *data)
{
	switch(id)
	{
	case DRIVER_DESCRIPTION: /* get driver description */
		{
			DbgLog((LOG_TRACE,0,TEXT("DRIVER_DESCRIPTION")));
			return wrapper->DriverDescription((struct DRIVER_DATA *)data);
		}
		break;
	case DEVICE_DESCRIPTION: /* get device description */
		{
			DbgLog((LOG_TRACE,0,TEXT("DEVICE_DESCRIPTION")));
			return wrapper->DeviceDescription((struct DEVICE_DATA *)data);
		}
		break;
	case MESSAGE_CALLBACK: /* set message callback */
		{
			DbgLog((LOG_TRACE,0,TEXT("SET_MESSAGE_CALLBACK")));
			return wrapper->MessageCallback((MSG_CB_PROC)data);
		}
		break;
	case OPEN_DEVICE: /* open device */
		{
			DbgLog((LOG_TRACE,0,TEXT("OPEN_DEVICE")));
			return wrapper->OpenDevice((struct OPEN_DEVICE_DATA *)data);
		}
		break;
	case CLOSE_DEVICE: /* close device */
		{
			DbgLog((LOG_TRACE,0,TEXT("CLOSE_DEVICE")));
			return wrapper->CloseDevice((int *)data);
		}
		break;
	case DEVICE_NAME: /* get device name */
		{
			DbgLog((LOG_TRACE,0,TEXT("DEVICE_NAME")));
			return wrapper->DeviceName((struct DEVICE_NAME_DATA *)data);
		}
		break;
	case PID_FILTER: /* set PID filter */
		{
			DbgLog((LOG_TRACE,0,TEXT("PID_FILTER")));
			return wrapper->PidFilter((struct PID_FILTER_DATA *)data);
		}
		break;
	case STREAM_CALLBACK: /* set TS packet callback */
		{
			DbgLog((LOG_TRACE,0,TEXT("STREAM_CALLBACK")));
			return wrapper->StreamCallback((STR_CB_PROC)data);
		}
		break;
	case TUNE: /* perform tuning */
		{
			DbgLog((LOG_TRACE,0,TEXT("TUNE")));
			return wrapper->Tune((struct TUNE_DATA *)data);
		}
		break;
	case SIGNAL_STATISTICS: /* get signal statistics */
		{
			DbgLog((LOG_TRACE,0,TEXT("SIGNAL_STATISTICS")));
			return wrapper->SignalStatistics((struct SIGNAL_STATISTICS_DATA *)data);
		}
		break;
	case DISEQC_COMMAND: /* send DiSEqC command */
		{
			DbgLog((LOG_TRACE,0,TEXT("DISEQC_COMMAND")));
			return wrapper->DiSEqC_Command((struct DISEQC_COMMAND_DATA *)data);
		}
		break;
	case DISEQC_SUPPORT: /* get DiSEqC support */
		{
			DbgLog((LOG_TRACE,0,TEXT("DISEQC_SUPPORT")));
			return wrapper->DiSEqC_Support((struct DISEQC_SUPPORT_DATA *)data);
		}
		break;
	case DEVICE_CONFIG: /* set private device config (GUI) */
		{
			DbgLog((LOG_TRACE,0,TEXT("DEVICE_CONFIG")));
			return wrapper->DeviceConfig();
		}
		break;
	case DEVICE_WHOLE_TRANSP: /* get device's capability to open whole transponder */
		{
			DbgLog((LOG_TRACE,0,TEXT("DEVICE_WHOLE_TRANSP")));
			return wrapper->DeviceOpensWholeTransponder((struct WHOLE_TRANSPONDER_DATA *)data);
		}
		break;
	default: /* shouldn't be called */
		{
			DebugLog("BDA2: DvbDeviceControl: id=0x%8.8x", id);
		}
	}
	return AltxDVB_OK;
}
