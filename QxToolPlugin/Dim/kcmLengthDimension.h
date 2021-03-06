//////////////////////////////////////////////////////////////////////////
// 标注命令。线性标注
#ifndef _KCM_LENGTH_DIMENSION_H_
#define _KCM_LENGTH_DIMENSION_H_

#include "kiCommand.h"

class kiInputEntityTool;
class kiInputLength;
class kcPreviewLine;

class kcmLengthDimension : public kiCommand
{
public:
	kcmLengthDimension(int subCmd);
	virtual ~kcmLengthDimension(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

protected:
	// 获取直线相关信息
	bool					GetPickedLineInfo();

	BOOL					CalcPointAndDir();
	//
	BOOL					BuildDimension();
	bool					BuildLineLenDim();

protected:
	kiOptionSet				*pOptionSet_;
	kiOptionSet				*pOptionSet2_;
	kiInputEntityTool		*pPickLineTool_;
	kiInputLength			*pInputLenTool_;
	kiInputEntityTool		*m_pPickFirstShape;
	kiInputEntityTool		*m_pPickSecondShape;
	kiInputLength			*m_pInputLength;
	kiOptionSet				m_optionSet;
	kiOptionEnum			*m_pTypeOpt;
	int						m_nType;
	double					m_dArrowSize;

protected:
	int						nSubCmd_; //子命令
	double					dFlyout_;
	double					dArrowLen_;
	double					dFontHeight_;

	int						nState_;
	gp_Pnt					aFirstPnt_, aSecondPnt_;
	gp_Pln					aPlane_;
	double					basePnt_[3], offDir_[3];

	Handle(Geom_Plane)		m_aPlane;
	kPoint3					m_aBasePoint;
	kVector3				m_aOffDir;
	kPoint3					m_aPnt1,m_aPnt2;
	kcPreviewLine			*m_pPreviewLine;
};

#endif