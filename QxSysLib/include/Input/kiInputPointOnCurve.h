//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_INPUT_POINT_ON_CURVE_H_
#define _KI_INPUT_POINT_ON_CURVE_H_

#include "kiInputTool.h"

class kcPreviewPoint;

class QXLIB_API kiInputPointOnCurve : public kiInputTool
{
public:
	// 用于命令中的构造函数。
	kiInputPointOnCurve(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	// 用于InputTool中的构造函数。
	kiInputPointOnCurve(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputPointOnCurve(void);

	bool					Init(const TopoDS_Edge& aEdge,int itol);
	bool					Init(const TopoDS_Wire& aWire,int itol);

	kPoint3					GetPickedPoint() const { return m_aPickPoint; }
	double					GetPickedParam() const { return m_dPickParam; }
	TopoDS_Edge				GetPickedEdge() const  { return m_aPickEdge;  }

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

public:
	//鼠标消息
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput); 
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	TopoDS_Shape			m_aShape;
	int						m_nPickTol;

	bool					m_bPicked;
	TopoDS_Edge				m_aPickEdge;
	kPoint3					m_aPickPoint;
	double					m_dPickParam;

	kcPreviewPoint			*m_pPreviewPoint;
};

#endif