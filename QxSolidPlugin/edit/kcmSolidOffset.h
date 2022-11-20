//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SOLID_OFFSET_H_
#define _KCM_SOLID_OFFSET_H_

#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kiInputOptionsTool;
class kcDispSolidArrow;
class kiOptionItem;

class kcmSolidOffset : public kiCommand
{
public:
	kcmSolidOffset(int subCmd);
	virtual ~kcmSolidOffset(void);

	// ��������������,��Ҫ��Apply�������ʹ��.
	virtual  BOOL			CanFinish();//�����Ƿ�������

	// �����빤�߻�ȡ���,����ʱ,��ص���Ӧ����ĸú���.
	// ���ڼ���ִ��������л����빤��.Ҳ���ܷ���KSTA_DONE,��������ִ�н���.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	// ��ѡ��ı��ĵ��ú���
	virtual void			OnOptionChanged(kiOptionItem *pOptionItem);
protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

	// �����Ӧ�ô�����
	virtual int				OnApply();

protected:
	// ������Ҫ�����빤��
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					BuildOffsetShape();

protected:
	int						nSubCmd_; //������ 0:Face 1:SHELL 2:SOLID
	kiOptionSet				*pOptionSet_; 
	kiOptionItem			*pSideOptItem_;
	
	kiInputEntityTool		*pInputEntity_;
	kiInputOptionsTool		*pOptionsTool_;

	double					dOffset_;//ƫ��ֵ
	int						nOffsetMethod_;//ƫ�Ʒ���
	bool					bOutSide_;//���⻹������
	int						nJoinType_;//
};

#endif