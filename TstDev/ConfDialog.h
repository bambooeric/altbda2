#ifndef CONFDIALOG_H
#define CONFDIALOG_H

#include "resource.h"
//#include "ChildThread.h"

struct DIALOG_PARAMS
{
	HINSTANCE hInstance;
	struct CONF_PARAMS *pConfParams;
};

class CConfDialog
{
public:
	CConfDialog(HINSTANCE, struct CONF_PARAMS *);
	~CConfDialog();

	BOOLEAN StartDialog(void);

private:
	struct DIALOG_PARAMS DialogParams;
	HINSTANCE hInstance;

//	CChildThread* pThread;
};

#endif /* CONFDIALOG_H */
