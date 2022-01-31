#include "StdAfx.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcModel.h"
#include "kcSnapMngr.h"
#include "kiInputEdit.h"
#include "kiInputTool.h"
#include "kiInputOptionsTool.h"
#include "kvCoordSystem.h"
#include "kcBasePlane.h"
#include "kiSelSet.h"
#include "kcDocContext.h"
#include "QxSysInterface.h"
#include "kiCommandMgr.h"
#include "kiCommand.h"

///////////////////////////////////////////////////////////////////
// 定时器处理类
class QxCmdTimer : public IQxTimerHandler{
public:
	QxCmdTimer(kiCommand *pCmd);

	virtual int			OnTimer(UINT_PTR nIDEvent,DWORD dwPassTime);

protected:
	kiCommand			*pCommand_;

};

QxCmdTimer::QxCmdTimer(kiCommand *pCmd)
	:pCommand_(pCmd)
{
}

int QxCmdTimer::OnTimer(UINT_PTR nIDEvent,DWORD dwPassTime)
{
	return pCommand_->HandlerTimer(nIDEvent,dwPassTime);
}

///////////////////////////////////////////////////////////////////
//

kiCommand::kiCommand(void)
{
	m_nID = 0;
	pDocContext_ = NULL;
	pModel_ = NULL;
	m_pInputEdit = NULL;
	
	m_strName = "";
	m_strAlias = "";
	m_strDesc = "";

	pCmdMgr_ = NULL;
	m_bRunning = FALSE;
	_bInitialized = FALSE;
	m_pSnapMgr = NULL;
	nRequestDepth_ = 0;

	m_pCurInputTool = NULL;
	pGlobalSelSet_ = NULL;

	pMainButtonOptionSet_ = NULL;
	pMainButtonOptionTool_ = NULL;
}

kiCommand::~kiCommand(void)
{
	ASSERT(!m_bRunning);
	//Destroy();
}

// 执行一次初始化和释放的地方。
int kiCommand::Initialize(int nCmdID,kcModel *pModel,kiCommandMgr *pCmdMgr)
{
	ASSERT(!m_strName.empty());
	ASSERT(pModel && pCmdMgr);
	if(_bInitialized)
		return 1;

	_bInitialized = TRUE;
	m_nID = nCmdID;
	pModel_ = pModel;
	pCmdMgr_ = pCmdMgr;
	pDocContext_ = pCmdMgr->GetDocContext();//获取关键数据
	ASSERT(pDocContext_);

	//获取
	hAISCtx_ = pModel->GetAISContext();
	ASSERT(!hAISCtx_.IsNull());

	pGlobalSelSet_ = pModel->GetGlobalSelSet();
	ASSERT(pGlobalSelSet_);

	m_pInputEdit = pDocContext_->GetInputEdit();
	ASSERT(m_pInputEdit);

	m_pSnapMgr = pDocContext_->GetSnapManager();
	ASSERT(m_pSnapMgr);

	//放在后面，需要前面的一些对象指针
	if(!CreateInputTools())
		return KSTA_ERROR;
	
	return OnInitialize();
}

int kiCommand::Destroy()
{
	// 销毁输入工具.
	DestroyInputTools();

	m_inputToolQueue.clear();
	m_pCurInputTool = NULL;

	m_bRunning = FALSE;
	
	return OnDestroy();
}

int kiCommand::OnInitialize()
{
	return KSTA_SUCCESS;
}

int kiCommand::OnDestroy()
{
	return KSTA_SUCCESS;
}

//从命令获取对应的AISContext
Handle(AIS_InteractiveContext)  kiCommand::GetAISContext() const
{
	ASSERT(!hAISCtx_.IsNull());
	return hAISCtx_;
}

kcDocContext*  kiCommand::GetDocContext() const
{
	return pDocContext_;
}

kiInputEdit*  kiCommand::GetInputEdit() const
{
	return m_pInputEdit;
}

const Handle(V3d_View)  kiCommand::GetCurrV3dView() const
{
	Handle(V3d_View) aView;
	if(pCmdMgr_){
		aView = pCmdMgr_->GetCurrV3dView();
	}
	return aView;
}

CView* kiCommand::GetActiveView() const
{
	if(pDocContext_){
		return pDocContext_->GetActiveView();
	}
	return NULL;
}

// 获取当前基准面
kcBasePlane* kiCommand::GetCurrentBasePlane() const
{
	if(pModel_){
		return pModel_->GetCurrentBasePlane();
	}
	return NULL;
}

int kiCommand::Execute()
{
	ASSERT(!m_bRunning);//没有在运行态

	ASSERT(pModel_);
	if(!pModel_){
		EndCommand(KSTA_ERROR);
		return KSTA_ERROR;
	}

	//捕获全局对象
	if(pGlobalSelSet_ != NULL){
		pGlobalSelSet_->GetSelected();
	}

	//计算捕捉点
	CalcSnapPoint();

	if(!InitInputToolQueue()){
		EndCommand(KSTA_ERROR);
		return KSTA_ERROR;
	}

	int iret = OnExecute();
	if(KSTA_ERROR == iret){
		EndCommand(KSTA_ERROR);
		return KSTA_ERROR;
	}
	//是否执行结束
	if(KSTA_DONE == iret){
		Done();
		Redraw();

		return KSTA_DONE;
	}

	// 自动执行第一个工具.或者下一个工具.
	// 可能OnExecute中已经执行了一些工具.
	if(KSTA_ERROR == NavToNextTool()){
		EndCommand(KSTA_ERROR);
		return KSTA_ERROR;
	}

	m_bRunning = TRUE;//处于运行状态

	return KSTA_CONTINUE;
}

// 执行函数
int		kiCommand::OnExecute()
{
	return KSTA_CONTINUE;
}

int		kiCommand::End(int nCode)
{
	//结束当前输入工具.
	if(m_pCurInputTool)
		m_pCurInputTool->End();

	//调用具体的结束处理.
	OnEnd(nCode);
		
	//清除输入工具
	m_inputToolQueue.clear();
	m_pCurInputTool = NULL;

	if(m_pSnapMgr)
		m_pSnapMgr->HideSnapMarker();

	m_bRunning = FALSE;
	//
	Redraw();

	if(m_pInputEdit)
		m_pInputEdit->PromptText("指令");

	return nCode;
}

int kiCommand::OnEnd(int nCode)
{
	UNUSED_ALWAYS(nCode);

	m_bRunning = FALSE;

	return KSTA_CONTINUE;
}

// 默认实现。
int kiCommand::Apply()
{
	int nErr = KSTA_CONTINUE;
	if(CanFinish())
	{
		nErr = OnApply();
		if(nErr == KSTA_DONE)
		{
			Done();
			return KSTA_DONE;
		}
		else if(nErr == KSTA_ERROR || nErr == KSTA_FAILED)
		{
			EndCommand(nErr);

			return nErr;
		}
	}
	
	return nErr;
}

//
int kiCommand::OnApply()
{
	return KSTA_DONE;
}

// 具体的确定处理函数
int kiCommand::OnOK()
{
	return KSTA_DONE;
}

// 结束当前命令执行。命令结束使用该函数。
// 命令执行结束（终止或取消）要统一由cmdMgr来执行。
//
int		kiCommand::EndCommand(int nCode)
{
	ASSERT(pCmdMgr_);
	if(pCmdMgr_){
		pCmdMgr_->EndCommand(nCode);
	}

	return KSTA_DONE;
}

// 命令执行完成。可以结束了。
int		kiCommand::Done()
{
	EndCommand(KSTA_DONE);
	Redraw();

	return KSTA_DONE;
}

// 显示提示信息到命令行
void  kiCommand::PromptMessage(const char *lpszMsg)
{
	if(m_pCurInputTool != NULL)
		return;

	if(m_pInputEdit && lpszMsg)
		m_pInputEdit->PromptText(lpszMsg);
}

// 结束当前的输入工具,不应做其他处理。
//
int		kiCommand::EndCurrentInputTool()
{
	if(m_pCurInputTool)
	{
		m_pCurInputTool->End();
		m_pCurInputTool = NULL;
	}

	return KSTA_SUCCESS;
}

//当InputEdit输入文本改变时
int  kiCommand::OnEditInputChanged(const char *pszText)
{
	return 0;
}

//当InputEdit文本输入完成时
int  kiCommand::OnEditInputFinished(const char *pszText)
{
	return 0;
}

//当一个选项被选中时，例如：通过快捷字符输入或通过点击
int  kiCommand::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	return 0;
}

// 选项的值发生了改变
void  kiCommand::OnOptionChanged(kiOptionItem *pOptItem)
{
	
}

// 选项被选中了,通常是button选项被点击了
// 这里处理系统按钮的处理
void  kiCommand::OnOptionSelected(kiOptionItem *pOptItem)
{
	ASSERT(pOptItem);
	int kstat = KSTA_CONTINUE;

	if(pOptItem->GetType() == KOPT_INPUT_BUTTON){
		kiOptionButton *pOptBtn = (kiOptionButton *)pOptItem;
		int nID = pOptBtn->GetID();
		if(nID == QX_SYS_BUTTON_CANCEL){
			//退出命令
			EndCommand(KSTA_CANCEL);
		}else if(nID == QX_SYS_BUTTON_APPLY){
			kstat = OnApply();
			// 结束命令
			if(kstat == KSTA_DONE){
				Done();
			}
		}else if(nID == QX_SYS_BUTTON_OK){
			kstat = OnOK();
			// 结束命令
			if(kstat == KSTA_DONE){
				Done();
			}
		}else{
			//其他处理
			OnOptionButtonClicked(nID);
		}
	}
}

void kiCommand::OnOptionButtonClicked(int nID)
{
	if(nID >= QX_SYS_BUTTON_ID_BASE){
		ASSERT(FALSE);
	}
}

BOOL kiCommand::CreateInputTools()
{
	return TRUE;
}

BOOL	kiCommand::InitInputToolQueue()
{
	return TRUE;
}

void	kiCommand::ClearInputToolList()
{
	m_inputToolQueue.clear();
}

// 添加一个主按钮输入工具，作为菜单
void kiCommand::AddMainButtonOptionTool(bool withApply)
{
	//如果没有，需要创建
	if(!pMainButtonOptionTool_){

		if(!pMainButtonOptionSet_){
			pMainButtonOptionSet_ = new kiOptionSet(this);
			// 添加主要button
			if(withApply){
				pMainButtonOptionSet_->AddSysButtonOption(QX_SYS_BUTTON_APPLY);
			}
			pMainButtonOptionSet_->AddSysButtonOption(QX_SYS_BUTTON_OK);
			pMainButtonOptionSet_->AddSysButtonOption(QX_SYS_BUTTON_CANCEL);
		}
		// 创建工具
		pMainButtonOptionTool_ = new kiInputOptionsTool(this,"命令选项:",pMainButtonOptionSet_);
	}

	// 避免重复添加
	TInputToolList::iterator ite = m_inputToolQueue.begin();
	for(;ite != m_inputToolQueue.end(); ++ite){
		if((*ite) == pMainButtonOptionTool_){
			ASSERT(FALSE);
			return;
		}
	}
	// 一般该工具总是在最后
	m_inputToolQueue.push_back(pMainButtonOptionTool_);
}

kiOptionSet* kiCommand::GetMainButtonOptionSet() const
{
	return pMainButtonOptionSet_;
}

int kiCommand::AddInputTool(kiInputTool *pTool)
{
	ASSERT(pTool);
	if(!pTool) return KSTA_ERROR;

	m_inputToolQueue.push_back(pTool);

	return KSTA_CONTINUE;
}

// 执行到下一个输入工具
//
int		kiCommand::NavToNextTool()
{
	KTRACE("\n kiCommand::NavToNextTool().");
	//结束旧的，认为旧的是正常结束。
	if(m_pCurInputTool)
	{
		m_pCurInputTool->End();
		m_pCurInputTool = NULL;
	}

	if(m_inputToolQueue.empty())
		return KSTA_CONTINUE;

	int iret = KSTA_CONTINUE;

	m_pCurInputTool = m_inputToolQueue.front();
	m_inputToolQueue.pop_front();
	if(m_pCurInputTool)
	{
		iret = m_pCurInputTool->Begin();
		if(iret == KSTA_ERROR)
		{
			//
			m_pCurInputTool = NULL;
			return KSTA_ERROR;
		}
	}

	return KSTA_CONTINUE;
}

int		kiCommand::NavToInputTool(kiInputTool *pTool)
{
	if(!pTool)
		return KSTA_ERROR;

	//结束旧的
	if(m_pCurInputTool)
		m_pCurInputTool->End();

	m_pCurInputTool = pTool;
	int iret = m_pCurInputTool->Begin();
	if(iret == KSTA_ERROR)
	{
		//
		m_pCurInputTool = NULL;
		return KSTA_ERROR;
	}

	return KSTA_CONTINUE;
}

BOOL	kiCommand::DestroyInputTools()
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//鼠标消息
void	kiCommand::OnLButtonDown(kuiMouseInput& mouseInput)
{
	if(m_pCurInputTool)
		m_pCurInputTool->OnLButtonDown(mouseInput);
}

int		kiCommand::OnLButtonUp(kuiMouseInput& mouseInput)
{
	int iret = KSTA_CONTINUE;
	if(m_pCurInputTool)
		m_pCurInputTool->OnLButtonUp(mouseInput);

	return KSTA_CONTINUE;
}

int		kiCommand::OnLButtonDblClk(kuiMouseInput& mouseInput)
{
	if(m_pCurInputTool)
		m_pCurInputTool->OnLButtonDblClk(mouseInput);

	return KSTA_CONTINUE;
}

int		kiCommand::OnMouseMove(kuiMouseInput& mouseInput)
{
	//
	if(m_pSnapMgr)
	{
		m_pSnapMgr->DoSnap(mouseInput.m_x,mouseInput.m_y);
	}
	if(m_pCurInputTool)
	{
		m_pCurInputTool->OnMouseMove(mouseInput);
	}
	return KSTA_CONTINUE;
}

void	kiCommand::OnRButtonDown(kuiMouseInput& mouseInput)
{
	if(m_pCurInputTool)
	{
		m_pCurInputTool->OnRButtonDown(mouseInput);
	}
}

// 应用，有mgr来检查。
// 取消等,有外部的CmdMgr来进行.
//
int		kiCommand::OnRButtonUp(kuiMouseInput& mouseInput)
{
	int nErr = KSTA_CONTINUE;
	//如果有工具进行输入，由工具进行执行
	if(m_pCurInputTool)
	{
		nErr = m_pCurInputTool->OnRButtonUp(mouseInput);
		if(nErr == KSTA_CANCEL)
		{
			if(!CanFinish())//不能完成，则认为是取消命令
			{
				EndCommand(KSTA_CANCEL);
				return KSTA_CANCEL;
			}
			else//取消当前输入，可能是跳过该输入，也可能是可以执行命令了，例如：输入多点构造样条等。
			{
				nErr = Apply();
				return nErr;
			}
		}else if (nErr == KSTA_DONE) {
			//应用
			if (CanFinish()) {
				//
				nErr = Apply();
				return nErr;
			}
		}
	}
	else
	{
		//如果不能够完成，认为取消命令执行。
		if(!CanFinish())
		{
			EndCommand(KSTA_CANCEL);
			return KSTA_CANCEL;
		}
		else
		{
			//应用
			nErr = Apply();
			return nErr;
		}
	}

	return KSTA_CONTINUE;
}

// 清除全局选择集。
bool	kiCommand::ClearGlobalSelSet()
{
	ASSERT(pGlobalSelSet_);
	if(!pGlobalSelSet_) return false;

	pGlobalSelSet_->UnHilight();
	pGlobalSelSet_->Clear();

	return true;
}

bool kiCommand::IsGlobalSelSetEmpty()
{
	if(pGlobalSelSet_ != NULL){
		return pGlobalSelSet_->IsEmpty();
	}
	return true;
}

kiSelSet* kiCommand::GetGlobalSelSet() const
{
	return pGlobalSelSet_;
}

// 重新绘制
void kiCommand::Redraw()
{
	//Handle(V3d_View) aView = pModel_->GetActiveView();
	//if(!aView.IsNull()){
	//	aView->Redraw();
	//}
	//这里，基于窗口的消息
	if(pDocContext_){
		pDocContext_->RedrawCurView();
		//pDocContext_->RefreshCurView();
	}
}

// 计算捕捉点
BOOL	kiCommand::CalcSnapPoint()
{
	ASSERT(pModel_);
	ASSERT(m_pSnapMgr);
	m_pSnapMgr->Clear();

	pModel_->CalcSnapPoint(m_pSnapMgr);
	//
	m_pSnapMgr->CalcScreenPoint();

	return TRUE;
}

// 添加临时捕捉点
bool	kiCommand::AddTempSnapPoint(double x,double y,double z)
{
	if(!m_pSnapMgr)
		return false;

	kiSnapPoint sp(x,y,z);
	return m_pSnapMgr->AddSnapPoint(KSNAP_TMP_POINT,sp);
}

void	kiCommand::ClearTempSnapPoint()
{
	if(m_pSnapMgr)
		m_pSnapMgr->ClearTempPoint();
}

//the aux function used inside the command
kVector3 kiCommand::GetWorkPlaneZDir()
{
	kcBasePlane *pBasePlane = GetCurrentBasePlane();
	ASSERT(pBasePlane);
	kvCoordSystem cs;
	pBasePlane->GetCoordSystem(cs);
	return cs.Z();
}

void  kiCommand::GetWorkPlaneCoordSystem(kvCoordSystem& cs)
{
	kcBasePlane *pBasePlane = GetCurrentBasePlane();
	ASSERT(pBasePlane);
	if(pBasePlane){
		pBasePlane->GetCoordSystem(cs);
	}
}

//请求一个InputEdit输入
int  kiCommand::RequestEditInput(const char *pszPromptText,kiOptionSet *pOptionSet)
{
	ASSERT(m_pInputEdit);
	ASSERT(pszPromptText);

	m_pInputEdit->RequestInput(pszPromptText,this,pOptionSet);

	return KSTA_CONTINUE;
}

// 将当前输入请求压栈
int  kiCommand::PushCurrentRequest()
{
	ASSERT(m_pInputEdit);
	if(m_pInputEdit->PushRequest()){
		nRequestDepth_ ++;
		return 1;
	}
	
	return 0;
}

int  kiCommand::PopRequest()
{
	ASSERT(m_pInputEdit);
	if(nRequestDepth_ <= 0)
		return 0;

	m_pInputEdit->PopRequest();
	nRequestDepth_ --;

	return 1;
}

//
void  kiCommand::InitSet(const char *pszName,const char *pszAlias,const char *pszDesc)
{
	ASSERT(pszName && pszAlias && pszDesc);
	m_strName = pszName;
	m_strAlias = pszAlias;
	m_strDesc = pszDesc;
}

void kiCommand::UAddToModel(const std::vector<kcEntity*> &aEntVec)
{
	if(pModel_ != NULL && !aEntVec.empty()){
		pModel_->BeginUndo(GetName());
		//
		pModel_->AddEntity(aEntVec,TRUE);
		pModel_->EndUndo();
	}
}

void kiCommand::UAddToModel(kcEntity *pEntity,BOOL bUpdateView)
{
	if(pEntity && pEntity->IsValid() && pModel_){
		pModel_->BeginUndo(GetName());
		//
		pModel_->AddEntity(pEntity,bUpdateView);
		pModel_->EndUndo();
	}
}

void kiCommand::UAddToModel(const TopoDS_Shape &aS,BOOL bUpdateView)
{
	if(!aS.IsNull()){
		kcEntity *pEntity = kcEntityFactory::CreateEntity(aS);
		if(pEntity && pEntity->IsValid()){
			UAddToModel(pEntity,bUpdateView);
		}
	}
}

///////////////////////
//键盘消息

int  kiCommand::OnKeyDown(kuiKeyInput& keyInput)
{
	keyInput;
	return KSTA_CONTINUE;
}

int  kiCommand::OnKeyUp(kuiKeyInput& keyInput)
{
	keyInput;
	return KSTA_CONTINUE;
}

int  kiCommand::OnChar(kuiKeyInput& keyInput)
{
	keyInput;
	return KSTA_CONTINUE;
}

/////////////////////////////////////////////////////////////
// 定时器处理

bool kiCommand::SetTimer(int nId,UINT uElapse)
{
	if(HasTimer(nId)){
		ASSERT(FALSE);
		return false;
	}

	IQxTimerManager *pTimerMgr = pDocContext_->GetTimerManager();
	QxCmdTimer *pHandler = new QxCmdTimer(this);
	UINT_PTR nIDEvent = pTimerMgr->SetTimer(pHandler,uElapse);
	if(nIDEvent == 0){ //失败
		ASSERT(FALSE);
		return false;
	}
	
	//记录下来
	aTimerIdMap_.insert(stdTimerIDMap::value_type(nId,nIDEvent));

	return true;
}

bool kiCommand::KillTimer(int nId)
{
	UINT_PTR nIDEvent = GetTimerEventID(nId);
	if(nIDEvent == 0)
		return false;

	IQxTimerManager *pTimerMgr = pDocContext_->GetTimerManager();
	pTimerMgr->KillTimer(nIDEvent);

	return true;
}

bool kiCommand::PauseTimer(int nId)
{
	UINT_PTR nIDEvent = GetTimerEventID(nId);
	if(nIDEvent > 0){
		IQxTimerManager *pTimerMgr = pDocContext_->GetTimerManager();
		return pTimerMgr->PauseTimer(nIDEvent);
	}
	return false;
}

bool	 kiCommand::ResumeTimer(int nId)
{
	UINT_PTR nIDEvent = GetTimerEventID(nId);
	if(nIDEvent > 0){
		IQxTimerManager *pTimerMgr = pDocContext_->GetTimerManager();
		return pTimerMgr->ResumeTimer(nIDEvent);
	}
	return false;
}

bool kiCommand::HasTimer(int nId)
{
	stdTimerIDMap::iterator ite = aTimerIdMap_.find(nId);
	return (ite != aTimerIdMap_.end());
}

int kiCommand::OnTimer(int nId,DWORD dwPassTime)
{
	nId;dwPassTime;
	return 0;
}

// 这里，将nIDEvent转为命令内部的id值。
//
int kiCommand::HandlerTimer(UINT_PTR nIDEvent,DWORD dwPassTime)
{
	int rc = 0;
	stdTimerIDMap::iterator ite = aTimerIdMap_.begin();
	for(;ite != aTimerIdMap_.end();++ite){
		if((*ite).second == nIDEvent){
			rc = OnTimer((*ite).first,dwPassTime);
			break;
		}
	}
	return rc;
}

// 根据命令内的id获取Timer的nID值。
UINT_PTR kiCommand::GetTimerEventID(int nID)
{
	stdTimerIDMap::iterator ite = aTimerIdMap_.find(nID);
	if(ite != aTimerIdMap_.end()){
		return (*ite).second;
	}
	return 0;
}