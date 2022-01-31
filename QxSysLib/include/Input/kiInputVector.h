//////////////////////////////////////////////////////////////////////////
// 输入一个向量
#ifndef _KI_INPUT_VECTOR_H_
#define _KI_INPUT_VECTOR_H_

#include "kPoint.h"
#include "kVector.h"
#include "kiInputTool.h"
#include "kiOptionSet.h"


//输入矢量的类型。
typedef enum _enumInputVectorType{
	eInputWorkPlaneVector, //输入工作平面内的矢量
	eInputXVector, //X向矢量,相对当前工作平面来说
	eInputYVector, //Y向矢量,相对当前工作平面来说
	eInputZVector, //Z向矢量,相对当前工作平面来说
	eInputSpaceVector //空间中的矢量,自由矢量
}eInputVectorType;

class kiInputPoint;
class kcPreviewLine;

class QXLIB_API kiInputVector : public kiInputTool
{
public:
	kiInputVector(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);//带选项
	kiInputVector(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);

	virtual ~kiInputVector(void);

	//设置类型
	void					SetType(eInputVectorType eType);
	void					FixVectorDir(eInputVectorType eType);//固定在一个方向上
	void					SetOrgPoint(const kPoint3& org);//设定了起始点
	void					SetPrompt(const char *pszPrompt1,const char *pszPrompt2);//设置两个提示字符串

	// 开始命令的执行
	virtual int				Begin();
	// 结束命令的执行,nCode是输入的结果.
	virtual int				End();

	// 子tool的回调函数,当子tool调用结束时,调用.
	virtual int				OnSubInputFinished(kiInputTool *pTool);

	// 当选项改变后的调用函数
	virtual void			OnOptionChanged(kiOptionItem *pOptionItem);

public:
	//鼠标消息
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput); 
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);

public:
	bool					GetPoint(kPoint3& point) const;
	bool					GetVector(kVector3& vec) const;

protected:
	void					Reset();
	void					Destroy();

	bool					CreateInputTool();

	//计算向量
	bool					CalcVector(kuiMouseInput& mouseInput);

protected:
	eInputVectorType		m_eType;//输入类型
	bool					m_bFixedDir;//是否固定方向
	bool					m_bSetOrgPoint;//是否设定了起始点.
	std::string				m_strPrompt1,m_strPrompt2;
	int						m_nVecType;
	kPoint3					m_point;
	kVector3				m_vec;//输入的点和向量

protected:
	int						m_nState;//状态
	kiInputPoint			*m_pInputOrgPoint;
	kiInputPoint			*m_pInputVecPoint;
	kPoint3					m_aOrg;
	kPoint3					m_aDirPnt;
	kiOptionSet				m_dirOptionSet;//方向相关选项
	kiOptionEnum			*m_pOptionType;
	kiOptionDouble			*m_pOptionLength;
	double					m_dLength;
	kcPreviewLine			*m_pPreviewLine;
};

#endif