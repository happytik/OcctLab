// OcctTry.h : OcctTry 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h” 
#endif

#include "resource.h"       // 主符号
#include <Standard_Macro.hxx>
#include <Graphic3d_GraphicDriver.hxx>

// COcctLabApp:
// 有关此类的实现，请参阅 OcctTry.cpp
//

class QxPluginMgr;
class kiCommandMgr;
class kcHandleMgr;

class COcctLabApp : public CWinApp
{
public:
	COcctLabApp();

	Handle(Graphic3d_GraphicDriver)		GetGraphicDriver() const { return m_graphicDriver; }

	QxPluginMgr*				GetPluginMgr() const { return m_pPluginMgr; }


// 重写
public:
	virtual BOOL				InitInstance();
	virtual int				ExitInstance();
// 实现
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

protected:
	Handle(Graphic3d_GraphicDriver)	   m_graphicDriver;

protected:
	//这些都是和文档无关的项.
	QxPluginMgr				*m_pPluginMgr;//插件管理

	//加载插件,注册命令.
	BOOL						LoadPlugin();

	//动态加载插件
	BOOL						SearchAndLoadDynPlugin();
};

extern COcctLabApp theApp;
