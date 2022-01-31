//////////////////////////////////////////////////////////////////////////
// 曲线倒角
#ifndef _KCM_CURVE_FILLET_H_
#define _KCM_CURVE_FILLET_H_

#include <vector>
#include <TopTools_MapOfShape.hxx>
#include "kiCommand.h"

class kiOptionSet;
class kiInputEntityTool;
class kiInputDoubleTool;

class axVertexData{
public:
	axVertexData(){
		_dRadius = 0.0;
	}

public:
	TopoDS_Vertex			_aVertex;
	double					_dRadius;
};

class kcmCurveFillet : public kiCommand
{
public:
	kcmCurveFillet(void);
	virtual ~kcmCurveFillet(void);

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
	void					ClearVertexData();
	//
	BOOL					DoFillet();

protected:
	kiInputEntityTool		*m_pPickEntityTool;
	kiInputEntityTool		*m_pPickLocalShapeTool;
	kiInputDoubleTool		*m_pInputRadiusTool;
	double					m_dRadius;

protected:
	std::vector<axVertexData *>	m_aVertexData;
	TopTools_MapOfShape			m_aVertexMap;
};

class kcmTwoEdgeFillet : public kiCommand
{
public:
	kcmTwoEdgeFillet(void);
	virtual ~kcmTwoEdgeFillet(void);

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
	void					ClearVertexData();
	//
	BOOL					DoFillet();

protected:
	kiOptionSet			*pOptionSet_;
	kiInputEntityTool		*pPickEdgeTool_;
	double				dRadius_;
	bool					bToWire_;//结果是否组环
	bool					bCopy_; //是否拷贝
};


#endif