//
#include "stdafx.h"
#include "QxCommandItem.h"
#include "kiCommand.h"
#include "kcmSolidSphere.h"
#include "kcmSolidBox.h"
#include "kcmSolidCylinder.h"
#include "kcmSolidCone.h"
#include "kcmSolidDeCompound.h"
#include "kcmSolidBoolean.h"
#include "kcmSolidPrism.h"
#include "kcmSolidRevol.h"
#include "kcmSolidPipe.h"
#include "kcmSolidPipeShell.h"
#include "kcmSolidFillet.h"
#include "kcmSolidSewing.h"
#include "kcmSolidOffset.h"
#include "kcmThickSolid.h"
#include "kcmSolidLoft.h"
#include "kcmSolidDraft.h"
#include "kcmSolidTrim.h"
#include "kcmOneFaceBase.h"
#include "QxSolidPlugin.h"

//����һ���Լ�����
class QxSolidPluginImpl : public QxCmdPluginImpl{
public:
	QxSolidPluginImpl();
	~QxSolidPluginImpl();

	// ��ȡ����е��������
	virtual int		GetCommandNum() const;
	// ��ȡ���������
	virtual const char*	GetPluginName() const;

	// �����ʼ�����ͷ�
	virtual bool		Initialize();
	virtual void		Release();

	// ���������ڲ���е�ID�������������
	virtual kiCommand*			DoCreateCommand(int nLocalID);
	// �ͷŴ������������
	virtual bool					DoReleaseCommand(kiCommand *pCommand);
};

QxCmdPluginImpl* QxCmdPluginMain(QxCmdPluginCfg *pCfg)
{
	QxSolidPluginImpl *pImpl = new QxSolidPluginImpl;
	return pImpl;
}

//
void QxCmdPluginFree(QxCmdPluginImpl *pImpl)
{
	if(pImpl){
		delete pImpl;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
QxSolidPluginImpl::QxSolidPluginImpl()
{
	eTopMenuID_ = QX_TOP_MENU_SOLID;
}

QxSolidPluginImpl::~QxSolidPluginImpl()
{
}

// ��ȡ����е��������
int QxSolidPluginImpl::GetCommandNum() const
{
	return 40;
}

// ��ȡ���������
const char* QxSolidPluginImpl::GetPluginName() const
{
	return "Occtʵ�����Ͳ��";
}

enum eCmdPluginCmdType{
	CMD_SOLID_BOX = 0,
	CMD_SOLID_SPHERE,
	CMD_SOLID_CYLINDER,
	CMD_SOLID_CONE,
	CMD_ID_BOOL_FUSE,
	CMD_ID_BOOL_COMMON,
	CMD_ID_BOOL_CUT,
	CMD_ID_BOOL_SPLIT,
	CMD_SOLID_DECOMP,
	CMD_SOLID_PRISM,
	CMD_SOLID_DRAFT,
	CMD_SOLID_REVOL,
	CMD_SOLID_PIPE,
	CMD_SOLID_PIPE_SHELL,
	CMD_SOLID_SWEEP,
	CMD_SOLID_SWEEP_SHELL,
	CMD_SOLID_LOFT,
	CMD_SOLID_BOOL_SPLITTER,
	CMD_SOLID_TRIM_BYSHELL,
	CMD_SOLID_TRIM_SPLIT,
	CMD_SOLID_SEWING,
	CMD_SOLID_OFFSET_FACE,
	CMD_SOLID_OFFSET_SHELL,
	CMD_SOLID_OFFSET_SOLID,
	CMD_SOLID_SHEET,
	CMD_SOLID_THICK,
	CMD_SOLID_FILLET,
	CMD_SOLID_FILLET_ALL,
	CMD_SOLID_FACE_COPY,
	CMD_SOLID_FACE_LOOP,
	CMD_SOLID_FACE_INTERP,
	CMD_SOLID_FACE_EXPSTEP,
	CMD_SOLID_EXTRACT_NFACE,
	CMD_SOLID_FACE_PSTONP,
	CMD_SOLID_FACE_CHGUORG,
	CMD_SOLID_FACE_CHGVORG,
};

// �����ʼ�����ͷ�
bool QxSolidPluginImpl::Initialize()
{
	//const char *pszTopMenu = "ʵ��(&D)";

	DoAddCommandItem((int)CMD_SOLID_BOX,"������(&B)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_SPHERE,"��(&S)");
	DoAddCommandItem((int)CMD_SOLID_CYLINDER,"Բ����(&C)");
	DoAddCommandItem((int)CMD_SOLID_CONE,"׶��(&C)");

	DoAddCommandItem((int)CMD_ID_BOOL_FUSE,"���","����(&F)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_ID_BOOL_COMMON,"���","����(&J)");
	DoAddCommandItem((int)CMD_ID_BOOL_CUT,"���","�(&C)");
	DoAddCommandItem((int)CMD_ID_BOOL_SPLIT,"���","�ָ�ʵ��(&S)");

	DoAddCommandItem((int)CMD_SOLID_PRISM,"����(&P)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_DRAFT,"��ģ����(&E)");
	DoAddCommandItem((int)CMD_SOLID_REVOL,"��ת(&R)");
	DoAddCommandItem((int)CMD_SOLID_PIPE,"�ܵ�","����(&C)");
	DoAddCommandItem((int)CMD_SOLID_PIPE_SHELL,"�ܵ�","������(&D)");
	DoAddCommandItem((int)CMD_SOLID_SWEEP,"����ɨ��","����(&C)");
	DoAddCommandItem((int)CMD_SOLID_SWEEP_SHELL,"����ɨ��","������(&D)");
	DoAddCommandItem((int)CMD_SOLID_LOFT,"����(&L)");

	DoAddCommandItem((int)CMD_SOLID_DECOMP,"�ֽ�(&D)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_TRIM_BYSHELL,"�ü�","ʹ����ü�(&S)");
	DoAddCommandItem((int)CMD_SOLID_TRIM_SPLIT,"�ü�","�ָ�ü�(&P)");

	DoAddCommandItem((int)CMD_SOLID_SEWING,"�������(&S)");
	DoAddCommandItem((int)CMD_SOLID_OFFSET_FACE,"ƫ��","FACE offset");
	DoAddCommandItem((int)CMD_SOLID_OFFSET_SHELL, "ƫ��", "SHELL offset");
	DoAddCommandItem((int)CMD_SOLID_OFFSET_SOLID, "ƫ��","SOLID offset");
	DoAddCommandItem((int)CMD_SOLID_SHEET,"���(&S)");
	DoAddCommandItem((int)CMD_SOLID_THICK,"�Ⱥ�(&T)");

	DoAddCommandItem((int)CMD_SOLID_FILLET,"����(&F)","�Ȱ뾶��Բ��(&F)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_FILLET_ALL,"����(&F)","ȫ���ߵ�Բ��(&F)");

	DoAddCommandItem((int)CMD_SOLID_FACE_COPY,"������","�濽��(&C)",KUI_MENU_BEFORE_SEP);
	DoAddCommandItem((int)CMD_SOLID_FACE_LOOP,"������","�������(&C)");
	DoAddCommandItem((int)CMD_SOLID_FACE_INTERP,"������","��ֵ��(&I)");
	DoAddCommandItem((int)CMD_SOLID_FACE_EXPSTEP,"������","�����浽STEP");
	DoAddCommandItem((int)CMD_SOLID_EXTRACT_NFACE,"������","��ǰ��n����(&N)");

	DoAddCommandItem((int)CMD_SOLID_FACE_PSTONP,"������������","ת������(&N)");
	DoAddCommandItem((int)CMD_SOLID_FACE_CHGUORG,"������������","�ı�U�����(&U)");
	DoAddCommandItem((int)CMD_SOLID_FACE_CHGVORG,"������������","�ı�V�����(&V)");

	return true;
}

void QxSolidPluginImpl::Release()
{
}

// ���������ڲ���е�ID�������������
kiCommand* QxSolidPluginImpl::DoCreateCommand(int nLocalID)
{
	kiCommand *pCommand = NULL;
	switch(nLocalID){
	case CMD_SOLID_BOX:		pCommand = new kcmSolidBox;  break;
	case CMD_SOLID_SPHERE:		pCommand = new kcmSolidSphere; break;
	case CMD_SOLID_CYLINDER:	pCommand = new kcmSolidCylinder; break;
	case CMD_SOLID_CONE:		pCommand = new kcmSolidCone; break;
	case CMD_ID_BOOL_FUSE:	pCommand = new kcmSolidBoolFuse; break;
	case CMD_ID_BOOL_COMMON:	pCommand = new kcmSolidBoolCommon; break;
	case CMD_ID_BOOL_CUT:	pCommand = new kcmSolidBoolCut; break;
	case CMD_ID_BOOL_SPLIT:	pCommand = new kcmSolidSplit; break;
	case CMD_SOLID_DECOMP:		pCommand = new kcmSolidDeCompound; break;
	case CMD_SOLID_PRISM:		pCommand = new kcmSolidPrism; break;
	case CMD_SOLID_DRAFT:		pCommand = new kcmSolidDraft; break;
	case CMD_SOLID_REVOL:		pCommand = new kcmSolidRevol; break;
	case CMD_SOLID_PIPE:		pCommand = new kcmSolidPipe; break;
	case CMD_SOLID_PIPE_SHELL: pCommand = new kcmSolidPipeShell; break;
	case CMD_SOLID_SWEEP:		pCommand = new kcmSolidSweep; break;
	case CMD_SOLID_SWEEP_SHELL:	pCommand = new kcmSolidSweepShell; break;
	case CMD_SOLID_LOFT:		pCommand = new kcmSolidLoft; break;
	case CMD_SOLID_TRIM_BYSHELL:	pCommand = new kcmSolidTrimByShell; break;
	case CMD_SOLID_TRIM_SPLIT:		pCommand = new kcmSolidTrimSplit; break;
	case CMD_SOLID_SEWING:		pCommand = new kcmSolidSewing; break;
	case CMD_SOLID_OFFSET_FACE:		pCommand = new kcmSolidOffset(0); break;
	case CMD_SOLID_OFFSET_SHELL:		pCommand = new kcmSolidOffset(1); break;
	case CMD_SOLID_OFFSET_SOLID:		pCommand = new kcmSolidOffset(2); break;
	case CMD_SOLID_SHEET:		pCommand = new kcmSheetSolid; break;
	case CMD_SOLID_THICK:		pCommand = new kcmThickSolid; break;
	case CMD_SOLID_FILLET:		pCommand = new kcmSolidFillet; break;
	case CMD_SOLID_FILLET_ALL:		pCommand = new kcmSolidFilletAllEdge; break;
	case CMD_SOLID_FACE_COPY:	pCommand = new kcmOneFaceCopy; break;
	case CMD_SOLID_FACE_LOOP:	pCommand = new kcmOneFaceParLoop; break;
	case CMD_SOLID_FACE_INTERP: pCommand = new kcmInterpOneFace; break;
	case CMD_SOLID_FACE_EXPSTEP:	pCommand = new kcmExpOneFaceToStep; break;
	case CMD_SOLID_EXTRACT_NFACE:	pCommand = new kcmExtractNFace; break;
	case CMD_SOLID_FACE_PSTONP:	pCommand = new kcmPeriodicBSpSurfTry(1); break;
	case CMD_SOLID_FACE_CHGUORG:	pCommand = new kcmPeriodicBSpSurfTry(2); break;
	case CMD_SOLID_FACE_CHGVORG:	pCommand = new kcmPeriodicBSpSurfTry(3); break;
	default:
		break;
	}
	return pCommand;
}

// �ͷŴ������������
bool QxSolidPluginImpl::DoReleaseCommand(kiCommand *pCommand)
{
	if(pCommand){
		pCommand->Destroy();
		delete pCommand;
		return true;
	}
	return false;
}