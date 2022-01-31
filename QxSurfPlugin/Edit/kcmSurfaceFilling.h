//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SURFACE_FILLING_H_
#define _KCM_SURFACE_FILLING_H_

#include <vector>
#include <TopTools_MapOfShape.hxx>
#include "kiCommand.h"

class kiInputEntityTool;
class kcPreviewCurve;

class axCrvInfo{
public:
	axCrvInfo(){
		_bBound = true;
		_nCont = 0;
		_pPrevCrv = NULL;
	}

public:
	TopoDS_Edge		_aEdge;
	int				_nCont;
	bool			_bBound;
	kcPreviewCurve *_pPrevCrv;
};

class kcmSurfaceFilling : public kiCommand
{
public:
	kcmSurfaceFilling(void);
	virtual ~kcmSurfaceFilling(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);
	virtual int				OnApply();

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	//
	BOOL					DoFilling();

	//
	void					ClearEdgeInfo();

protected:
	kiOptionSet				m_optionSet;
	kiInputEntityTool		*m_pPickEdgeTool;
	int						m_nDegree;
	int						m_nMaxSeg;
	int						m_nbPtsOnCrv;
	int						m_nbIter;
	bool					m_bBound;//是否边界
	int						m_nContType;//连续性条件
	kiOptionEnum			*m_pOptionCont;

protected:
	std::vector<axCrvInfo *>	m_aEdge;
	TopTools_MapOfShape			m_aShapeMap;
};

#endif