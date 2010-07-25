#include "stdafx.h"

#include "Configuration.h"

CConfiguration::CConfiguration()
{
//	pConfDialog = NULL;
	conf_params.VendorSpecific = VENDOR_SPECIFIC(PURE_BDA);
}

CConfiguration::~CConfiguration()
{
//	if(pConfDialog)
//		delete(pConfDialog);
}

void CConfiguration::Configure(HINSTANCE hInstance, int selected_device_enum,
							   char *selected_device_name)
{
	int len;

//	pConfDialog = new CConfDialog(hInstance, &conf_params);

	ConfigurationFileExists = FALSE;
	if(len = GetModuleFileNameA(hInstance, DLLFilePath, sizeof(DLLFilePath)))
		if(len < sizeof(DLLFilePath) && len >3)
		{
			ConfigurationFileExists = ReadConfigurationFile();
		}

		if(!ConfigurationFileExists)
	{
		DebugLog("BDA2: DLLMain: Failed getting configuration file");
		if(!CreateConfigurationFile())
			DebugLog("BDA2: DLLMain: Failed creating default configuration file");
		else
			DebugLog("BDA2: DLLMain: Created default configuration file");
	}
}

BOOLEAN CConfiguration::ReadConfigurationFile()
{
	char ConfFilePath[256];
	char ret_str[128];
	int ret_len;

	DWORD junk;
	int fv_len = GetFileVersionInfoSizeA(DLLFilePath, &junk);
	if(fv_len)
	{
		BYTE *fv_data = new BYTE[fv_len];
		BYTE *ptr;
		UINT txt_len;

		GetFileVersionInfoA(DLLFilePath, junk, fv_len, (void *)fv_data);
		if(VerQueryValueA(fv_data,"\\",(void **)&ptr,&txt_len))
		{
			VS_FIXEDFILEINFO *vs = (VS_FIXEDFILEINFO *)ptr;
			sprintf(conf_params.ConfVer, "v%d.%d.%d.%d",
				vs->dwFileVersionMS & 0xFFFF0000,
				vs->dwFileVersionMS & 0x0000FFFF,
				vs->dwFileVersionLS & 0xFFFF0000,
				vs->dwFileVersionLS & 0x0000FFFF);
		}
		free(fv_data);
	}
	else
		sprintf(conf_params.ConfVer,"Unknown");

	switch(conf_params.VendorSpecific)
	{
	case TT_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_8VSB;
		conf_params.ConfDiSEqC = DISEQC_TONEBURST | DISEQC_RAW;
		DebugLog("BDA2: Technotrend BDA API used !");
		break;
	case TH_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_8VSB;
		conf_params.ConfDiSEqC = DISEQC_TONEBURST | DISEQC_RAW;
		DebugLog("BDA2: Twinhan BDA extension used !");
		break;
	case DW_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_NBC_8PSK;
		conf_params.ConfDiSEqC = DISEQC_TONEBURST | DISEQC_RAW;
		DebugLog("BDA2: DvbWolrld BDA extension used !");
		break;
	case HAUP_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_NBC_8PSK;
		conf_params.ConfDiSEqC = DISEQC_RAW;
		DebugLog("BDA2: Hauppauge BDA extension used !");
		break;
	case CXT_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_NBC_8PSK;
		conf_params.ConfDiSEqC = DISEQC_RAW;
		DebugLog("BDA2: Conexant BDA extension used !");
		break;
	case TBS_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_NBC_8PSK;
		conf_params.ConfDiSEqC = DISEQC_RAW;
		DebugLog("BDA2: Turbosight BDA extension used !");
		break;
	case TV_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_8PSK;
		conf_params.ConfDiSEqC = DISEQC_RAW;
		DebugLog("BDA2: TeVii BDA extension used !");
		break;
	case OMC_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_8PSK;
				conf_params.ConfDiSEqC = DISEQC_RAW;
		DebugLog("BDA2: Omicom BDA extension used !");
		break;
	case WIN7_BDA:
		conf_params.ConfMod8PSK = BDA_MOD_8PSK;
				conf_params.ConfDiSEqC = DISEQC_TONEBURST | DISEQC_RAW;
		DebugLog("BDA2: Windows7 BDA extension used !");
		break;
	case PURE_BDA:
	default:
		conf_params.ConfMod8PSK = BDA_MOD_8PSK;
		conf_params.ConfDiSEqC = DISEQC_COMMITED;
		DebugLog("BDA2: Generic BDA used + DiseqC 1.0 (via InputRange)!");
		break;
	}

	strncpy(ConfFilePath, DLLFilePath, sizeof(ConfFilePath));
	ConfFilePath[strlen(ConfFilePath)-1] = 'g';
	ConfFilePath[strlen(ConfFilePath)-2] = 'f';
	ConfFilePath[strlen(ConfFilePath)-3] = 'c';

	ret_len = GetPrivateProfileStringA(
		"Dev_Bda2Driver DVBS", // section
		"S2_ROLLOFF", //key
		"NOT_SET", // default string
		ret_str,
		sizeof(ret_str),
		ConfFilePath);
	DebugLog("BDA2: ReadConfigurationFile: S2_ROLLOFF = %s (default is NOT_SET)", ret_str);
	if(!strcmp(ret_str,"20"))
		conf_params.S2RollOff = ROLLOFF_20;
	else
	if(!strcmp(ret_str,"25"))
		conf_params.S2RollOff = ROLLOFF_25;
	else
	if(!strcmp(ret_str,"35"))
		conf_params.S2RollOff = ROLLOFF_35;
	else
		conf_params.S2RollOff = ROLLOFF_NOT_SET; // default

	ret_len = GetPrivateProfileStringA(
		"Dev_Bda2Driver DVBS", // section
		"S2_PILOT", //key
		"NOT_SET", // default string
		ret_str,
		sizeof(ret_str),
		ConfFilePath);
	DebugLog("BDA2: ReadConfigurationFile: S2_PILOT = %s (default is NOT_SET)", ret_str);
	if(!strcmp(ret_str,"ON"))
		conf_params.S2Pilot = PILOT_ON;
	else
	if(!strcmp(ret_str,"OFF"))
		conf_params.S2Pilot = PILOT_OFF;
	else
		conf_params.S2Pilot = PILOT_NOT_SET; // default

	// check if file exists
	{
		FILE *fp = fopen(ConfFilePath, "r");
		if(!fp)
			return FALSE;
		fclose(fp);
	}

	return TRUE;
}


BOOLEAN CConfiguration::CreateConfigurationFile()
{
	char ConfFilePath[256];
	FILE *fp;

	strncpy(ConfFilePath, DLLFilePath, sizeof(ConfFilePath));
	ConfFilePath[strlen(ConfFilePath)-1] = 'g';
	ConfFilePath[strlen(ConfFilePath)-2] = 'f';
	ConfFilePath[strlen(ConfFilePath)-3] = 'c';
	fp = fopen(ConfFilePath, "w");
	if(fp)
	{
		fprintf(fp,"; Dev_Bda2Driver.int configuration parameters\n");
		fprintf(fp,"[Dev_Bda2Driver DVBS]\n\n");
		fprintf(fp,";S2 Roll Off\n");
		fprintf(fp,";   NOT_SET (default for S & S2)\n");
		fprintf(fp,";   20 - 0.20\n");
		fprintf(fp,";   25 - 0.25\n");
		fprintf(fp,";   35 - 0.35\n");
		fprintf(fp,"S2_ROLLOFF = NOT_SET\n\n");
		fprintf(fp,";S2 Pilot\n");
		fprintf(fp,";   NOT_SET (default)\n");
		fprintf(fp,";   ON\n");
		fprintf(fp,";   OFF\n");
		fprintf(fp,"S2_PILOT = NOT_SET\n\n");
		fprintf(fp,"[Dev_Bda2Driver DVBT]\n");
		fprintf(fp,"[Dev_Bda2Driver DVBC]\n");
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

BOOLEAN CConfiguration::DoConfigurationDialog()
{
/*	if(pConfDialog)
		return pConfDialog->StartDialog();*/
	return TRUE;
}
