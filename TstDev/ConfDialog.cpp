
#include "ConfDialog.h"
//#include "Dialog1.h"

void __cdecl TheThread(void *ptr);
HWND hwndDialog = NULL; // Window handle of dialog box

CConfDialog::CConfDialog(HINSTANCE DllInstance, struct CONF_PARAMS *conf_params)
{
	DialogParams.hInstance = DllInstance;
	DialogParams.pConfParams = conf_params;
//	pThread = NULL;
}

CConfDialog::~CConfDialog()
{
/*	if(pThread->GetThreadPriority() == THREAD_PRIORITY_NORMAL)
	{
		pThread->ExitInstance();
		pThread->Delete();
	}*/
}

BOOLEAN CConfDialog::StartDialog()
{
/*AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	// Dialog already open and running?
	if(pThread)
	{
		return FALSE;
	}
	// create new thread and run Dialog
	if(DialogParams.pConfParams)
	{
		pThread = new CChildThread( RUNTIME_CLASS( CDialog1 ), IDD_CONFIGURATION );
		pThread->CreateThread();
	}
*/
	return TRUE;
}

//AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

