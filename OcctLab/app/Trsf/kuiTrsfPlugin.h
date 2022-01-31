//////////////////////////////////////////////////////////////////////////
//
#ifndef _KUI_TRSF_PLUGIN_H_
#define _KUI_TRSF_PLUGIN_H_

#include "QxCmdPlugin.h"

class kuiTrsfPlugin : public QxCmdPlugin
{
public:
	kuiTrsfPlugin(void);
	virtual ~kuiTrsfPlugin(void);

	//初始化和释放
	virtual BOOL					Initialize();

protected:
	virtual kiCommand*			DoCreateCommand(int nLocalID);
	virtual bool					DoReleaseCommand(kiCommand *pCommand);
};

#endif