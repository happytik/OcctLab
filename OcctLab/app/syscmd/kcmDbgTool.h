#pragma once

#include "kiCommand.h"

class kcmInputFaceNoLF : public kiCommand
{
public:
	kcmInputFaceNoLF();
	virtual ~kcmInputFaceNoLF();

	// 开始执行命令.
	//执行和结束接口
	virtual	 int				Execute();
	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

protected:
	TopoDS_Compound			MakeWireText(const char *lpszText,LOGFONT& lf,double dHeight,double dGap);
};

//
class kcmTestForDebug : public kiCommand
{
public:
	kcmTestForDebug();
	virtual ~kcmTestForDebug();

	// 开始执行命令.
	//执行和结束接口
	virtual	 int				Execute();
	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL				CanFinish();//命令是否可以完成

protected:
	void						PntInSolidTest();
};