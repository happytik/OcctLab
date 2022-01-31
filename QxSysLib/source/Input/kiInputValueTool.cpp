#include "StdAfx.h"
#include "kiOptionSet.h"
#include "kiCommand.h"
#include "kiParseString.h"
#include "kiInputValueTool.h"

//////////////////////////////////////////////////////////////////////////
//输入一个整数
kiInputIntTool::kiInputIntTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)//带选项
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	m_pIntVal = NULL;
	m_nMin = 0;m_nMax = -1;//没有范围限制
}

kiInputIntTool::kiInputIntTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pParent,pPromptText,pOptionSet)
{
	m_pIntVal = NULL;
	m_nMin = 0;m_nMax = -1;//没有范围限制
}

kiInputIntTool::~kiInputIntTool(void)
{
	m_pIntVal = NULL;
}

bool	kiInputIntTool::Init(int *pVal)
{
	ASSERT(pVal);
	if(!pVal) return false;
	m_pIntVal = pVal;
	m_nMin = 0;m_nMax = -1;//没有范围限制
	return true;
}

bool	kiInputIntTool::Init(int *pVal,int nMin,int nMax)
{
	ASSERT(pVal);
	ASSERT(nMin < nMax);
	m_pIntVal = pVal;
	m_nMin = nMin;m_nMax = nMax;
	return true;
}

int		kiInputIntTool::OnBegin()
{
	ASSERT(m_pIntVal);
	return KSTA_CONTINUE;
}

// 获取提示字符串
void	kiInputIntTool::DoGetPromptText(std::string& str)
{
	ASSERT(m_pIntVal);
	char szBuf[256];
	if(m_nMin < m_nMax)
		sprintf_s(szBuf,255,"%s [%d - %d] <%d>:",m_szPromptText.c_str(),m_nMin,m_nMax,*m_pIntVal);
	else
		sprintf_s(szBuf,255,"%s <%d>:",m_szPromptText.c_str(),*m_pIntVal);
	str = szBuf;
}

// 出入文本完成，通常命令可以结束了
int	kiInputIntTool::OnEditInputFinished(const char *pInputText)
{
	if(!pInputText)
		return KSTA_CONTINUE;

	int nVal = 0;
	kiParseInteger pri(pInputText);
	if(!pri.Parse(nVal))
		return KSTA_CONTINUE;
	if(m_nMin < m_nMax){
		if(nVal < m_nMin || nVal > m_nMax)
			return KSTA_CONTINUE;
	}

	*m_pIntVal = nVal;
	SetInputState(KINPUT_DONE);
	End();//结束输入

	DoNotify();

	return KSTA_DONE;
}

//////////////////////////////////////////////////////////////////////////
//输入一个浮点数
kiInputDoubleTool::kiInputDoubleTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)//带选项
:kiInputTool(pCommand,pszPrompt,pOptionSet)
{
	m_pDoubleVal = NULL;
	m_dMin = 0.0;m_dMax = -1.0;
}

kiInputDoubleTool::kiInputDoubleTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pParent,pPromptText,pOptionSet)
{
	m_pDoubleVal = NULL;
	m_dMin = 0.0;m_dMax = -1.0;
}

kiInputDoubleTool::~kiInputDoubleTool(void)
{
	m_pDoubleVal = NULL;
}

bool	kiInputDoubleTool::Init(double *pVal)
{
	ASSERT(pVal);
	if(!pVal) return false;
	m_pDoubleVal = pVal;
	m_dMin = 0.0;m_dMax = -1.0;
	return true;
}

bool	kiInputDoubleTool::Init(double *pVal,double dMin,double dMax)
{
	ASSERT(pVal);
	ASSERT(dMin < dMax);
	if(!pVal) return false;
	m_pDoubleVal = pVal;
	m_dMin = dMin;m_dMax = dMax;
	return true;
}

//
int		kiInputDoubleTool::OnBegin()
{
	ASSERT(m_pDoubleVal);
	return KSTA_CONTINUE;
}

// 获取提示字符串
void	kiInputDoubleTool::DoGetPromptText(std::string& str)
{
	ASSERT(m_pDoubleVal);
	char szBuf[256];
	if(m_dMin < m_dMax)
		sprintf_s(szBuf,255,"%s [%.3f - %.3f] <%f>:",m_szPromptText.c_str(),m_dMin,m_dMax,*m_pDoubleVal);
	else
		sprintf_s(szBuf,255,"%s <%f>:",m_szPromptText.c_str(),*m_pDoubleVal);
	str = szBuf;
}

// 出入文本完成，通常命令可以结束了
int	kiInputDoubleTool::OnEditInputFinished(const char *pInputText)
{
	if(!pInputText || pInputText[0] == '\0'){//使用默认值
		SetInputState(KINPUT_DONE);
		End();

		DoNotify();
		return KSTA_DONE;
	}

	double dVal = 0.0;
	kiParseDouble pd(pInputText);
	if(!pd.Parse(dVal))
		return KSTA_CONTINUE;

	if(m_dMin < m_dMax){
		if(dVal < m_dMin || dVal > m_dMax)
			return KSTA_CONTINUE;
	}

	*m_pDoubleVal = dVal;
	SetInputState(KINPUT_DONE);
	End();

	DoNotify();

	return KSTA_DONE;
}

