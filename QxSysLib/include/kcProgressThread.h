#pragma once

#include <string>
#include <afxmt.h>
#include "QxLibDef.h"

class QXLIB_API kcProgressThread
{
public:
	kcProgressThread(void);
	virtual ~kcProgressThread(void);

	bool					Start();
	int					Stop();

public:
	static void			ThreadProc(void *pArg);

public:
	virtual void			Run();

public:
	bool					bRunning_;
	bool					bQuit_;

public:
	virtual std::string	GetProgText();
	virtual void			SetProgText(const std::string &szText);

protected:
	std::string			szProgText_;
	CCriticalSection		aCSLock_;
};

