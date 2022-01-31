#include "stdafx.h"
#include "kiInputPoint.h"
#include "kcPreviewObj.h"
#include "kiCommand.h"
#include "kcBasePlane.h"
#include "kvCoordSystem.h"
#include "QxCurveLib.h"
#include "kiParseString.h"
#include "kiInputDirTool.h"

kiInputDirTool::kiInputDirTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pCommand,pPromptText,pOptionSet)
{
	_pDir = NULL;
	_bUseWCS = false;
	_bOnlyZDir = false;
	_bUnited = true;
	_nStdAxis = Z_POS;//Z+向
}

// 用于InputTool中的构造函数。
kiInputDirTool::kiInputDirTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet)
	:kiInputTool(pParent,pPromptText,pOptionSet)
{
	_pDir = NULL;
	_bUseWCS = false;
	_bOnlyZDir = false;
	_bUnited = true;
	_nStdAxis = Z_POS;//Z+向
}

kiInputDirTool::~kiInputDirTool(void)
{
}

int	kiInputDirTool::Initialize(kVector3 *pDir,bool bUseWCS,bool bOnlyZDir,bool bUnited)
{
	ASSERT(pDir);
	_pDir = pDir;
	_bUseWCS = bUseWCS;
	_bOnlyZDir = bOnlyZDir;
	_bUnited = bUnited;

	return 1;
}

// 命令执行前的初始化
int	kiInputDirTool::OnBegin()
{
	ASSERT(_pDir);
	kiOptionSet& optSet = GetToolOptionSet();

	kiOptionStdAxis *pStdAxisOption = new kiOptionStdAxis("标准方向",'S',1,_nStdAxis);
	pStdAxisOption->Initialize(_bUseWCS,_bOnlyZDir);
	optSet.AddOption(pStdAxisOption);
	optSet.SetOptionCallBack(this);
	
	return KSTA_CONTINUE;
}

int	kiInputDirTool::OnEnd()
{
	GetToolOptionSet().Clear();
	return KSTA_CONTINUE;
}

// 获取提示字符串
void	kiInputDirTool::DoGetPromptText(std::string& str)
{
	char szText[256];
	sprintf_s(szText,256,"%s <%f,%f,%f> :",m_szPromptText.c_str(),_pDir[0],_pDir[1],_pDir[2]);
	str = szText;
}

// 当选项改变后的调用函数
void	kiInputDirTool::OnToolOptionChanged(kiOptionItem *pOptionItem)
{

}

void  kiInputDirTool::OnToolOptionClicked(kiOptionItem *pOptionItem)
{
}

void  kiInputDirTool::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if(pOptionItem->GetID() == 1){//标准坐标系
		double v[3];

		kGetAxisDir((eStdAxis)_nStdAxis,v);
		_pDir->set(v[0],v[1],v[2]);

		m_pInputEdit->Update();//更新显示
	}
}

//当edit输入文本完成后，调用该函数,传入的是输入的文本信息
int  kiInputDirTool::OnEditInputFinished(const char *pInputText)
{
	if(!pInputText || pInputText[0] == '\0'){//使用默认值或选中的标准值
		double v[3];
		kGetAxisDir((eStdAxis)_nStdAxis,v);
		_pDir->set(v[0],v[1],v[2]);

		SetInputState(KINPUT_DONE);
		End();
		DoNotify();

		return KSTA_DONE;
	}

	//解析结果
	kPoint3 pnt;
	kiParseStringToPoint3 psp(pInputText);
	if(psp.Parse(pnt)){
		_pDir->set(pnt[0],pnt[1],pnt[2]);
		if(_bUnited)
			_pDir->normalize();

		SetInputState(KINPUT_DONE);
		End();

		DoNotify();

		return KSTA_DONE;
	}

	return KSTA_CONTINUE;
}