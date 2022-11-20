//////////////////////////////////////////////////////////////////////////
// ��ʾ�������͵ļ�ͷ,����һ��ֱ��
#ifndef _KC_DISP_ARROW_H_
#define _KC_DISP_ARROW_H_

#include <AIS_InteractiveContext.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "kColor.h"
#include "QxLibDef.h"

class kcDocContext;

class QXLIB_API kcDispArrow
{
public:
	kcDispArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispArrow(void);

	//��������
	void								SetColor(const kColor& color);

	//��ʾ������
	virtual BOOL						Display(BOOL bShow);
	virtual BOOL						IsDisplayed() const;
	virtual void						UpdateDisplay();//������ʾ����
	//���
	virtual void						Clear();
	virtual void						Destroy();

protected:
	// ����Ĵ�����ʾ����
	virtual bool						BuildGroupDisplay() = 0;

protected:
	Handle(AIS_InteractiveContext)		_aCtx;
	Handle(Graphic3d_Structure)		_hStructure;//��ʾ����
	kcDocContext						*pDocContext_;//
	bool								_bDataValid;
	bool								_bChanged;//���������˸ı䣬��Ҫ����������ʾ

	kPoint3							_arrowOrg;
	kVector3							_axisDire;
	double							_arrowLen;//�ܳ���
	kColor							_aColor;
};

// �߿���ʾ
class QXLIB_API kcDispFrameArrow : public kcDispArrow{
public:
	kcDispFrameArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispFrameArrow();

	/*
	enum Aspect_TypeOfLine {
	Aspect_TOL_SOLID,
	Aspect_TOL_DASH,
	Aspect_TOL_DOT,
	Aspect_TOL_DOTDASH,
	Aspect_TOL_USERDEFINED
	};
	*/
	void								SetType(int type);
	void								SetWidth(double width);

protected:
	// ����Ĵ�����ʾ����
	virtual bool						BuildGroupDisplay();

protected:
	int									m_eType;//����
	double								m_dWidth;//�߿�
};

// ʵ����ʾ
class QXLIB_API kcDispSolidArrow : public kcDispArrow{
public:
	kcDispSolidArrow(const Handle(AIS_InteractiveContext)& aCtx,kcDocContext *pDocCtx);
	virtual ~kcDispSolidArrow();

	//��ʼ��
	int								Init(const kPoint3& arrowOrg,const kVector3& axisDir,
		                                double arrowLen,double coneRadius,double coneLen,double tubeRadius);
	int								Init(const kPoint3& arrowOrg,const kVector3& axisDir,double arrowLen);
	void								SetFacetNum(int num);//�趨�ʷ���ĸ���

protected:
	// ����Ĵ�����ʾ����
	virtual bool					BuildGroupDisplay();

protected:
	double							_coneRadius;//׶�װ뾶
	double							_coneLen;//׶����
	double							_tubeRadius;//�˵İ뾶
	int								_nbFacet;//�ʷ���ĸ���

protected:
	Handle(Prs3d_ShadingAspect)		_hShadingAspect;
};

#endif