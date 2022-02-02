#include "StdAfx.h"

#include "kcModel.h"
#include "kiCommand.h"
#include "kcSnapMngr.h"
#include "kcBasePlane.h"
#include "kiInputTool.h"

kiInputTool::kiInputTool(kiCommand *pCommand,const char *pPromptText,kiOptionSet *pOptionSet)
:m_pCommand(pCommand)
{
	ASSERT(pCommand && pPromptText);
	m_pParent = NULL;
	m_szPromptText = pPromptText;//记录提示信息
	//m_pOptionSet = pOptionSet;
	pInOptionSet_ = pOptionSet;

	if(pCommand != NULL){
		hAISCtx_ = pCommand->GetAISContext();
	}

	DoInitialize();
	Reset();

	// 只有没有设定选项时，才设定,这样可以保持OptionSet外部的设定
	if(pOptionSet && !pOptionSet->HasOptionCallBack()){
		pOptionSet->SetOptionCallBack(this);
	}
}

kiInputTool::kiInputTool(kiInputTool *pParent,const char *pPromptText,kiOptionSet *pOptionSet)
{
	ASSERT(pParent && pPromptText);
	m_pCommand = NULL;
	m_pParent = pParent;
	m_szPromptText = pPromptText;
	//m_pOptionSet = pOptionSet;
	pInOptionSet_ = pOptionSet;

	if(pParent != NULL){
		hAISCtx_ = pParent->GetAISContext();
	}

	DoInitialize();
	Reset();

	// 只有没有设定选项时，才设定,这样可以保持OptionSet外部的设定
	if(pOptionSet && !pOptionSet->HasOptionCallBack()){
		pOptionSet->SetOptionCallBack(this);
	}
}

kiInputTool::~kiInputTool(void)
{
	//if(m_pOptionSet)
	//	m_pOptionSet->SetOptionCallBack(NULL);
	//aToolOptionSet_.Clear();
	//aToolOptionSet_.SetInputTool(NULL);
}

//必要的初始化,主要获取两个变量的值
//
void  kiInputTool::DoInitialize()
{
	m_pInputEdit = GetInputEdit();
	ASSERT(m_pInputEdit);
	m_pSnapMgr = GetSnapManager();
		
	//设定状态为初始状态
	SetInputState(KINPUT_INITIAL);
}

void	kiInputTool::Reset()
{
	pCurrentOption_ = NULL;	
	pCurOptionSet_ = NULL;

	if(bHasOwnOptions_){
		SplitOptionSet(pInOptionSet_);
	}
	aToolOptionSet_.Clear();
	aToolOptionSet_.SetOptionCallBack(this);

	m_pCurSubTool = NULL;
	bHasOwnOptions_ = false;

	SetInputState(KINPUT_INITIAL);
}

// 将外部选项附到工具自身选项后面
//
void  kiInputTool::CombineOptionSet(kiOptionSet *pOptSet)
{
	kiOptionItem *pItem = NULL;
	int ix,count = 0;
	if(pOptSet){
		count = pOptSet->GetCount();
		for(ix = 0;ix < count;ix ++){
			pItem = pOptSet->GetOption(ix);
			if(!aToolOptionSet_.HasOptionItem(pItem)){
				// 暂时不改变通知对象
				//pItem->SetOptionCallBack(this);//改变通知该工具
				aToolOptionSet_.AddOption(pItem);
			}
		}
	}
}

// 将外部选项从工具总的选项集中去除掉
//
void  kiInputTool::SplitOptionSet(kiOptionSet *pOptSet)
{
	kiOptionItem *pItem = NULL;
	int ix,count = 0;
	if(pOptSet){
		count = pOptSet->GetCount();
		for(ix = 0;ix < count;ix ++){
			pItem = pOptSet->GetOption(ix);
			if(!aToolOptionSet_.HasOptionItem(pItem)){ //不存在
				//pItem->SetOptionCallBack(NULL); //不改变这个
				aToolOptionSet_.RemoveOptionItem(pItem);
			}
		}
	}
}

// 启动工具运行
//
int  kiInputTool::Begin()
{
	ASSERT(m_pInputEdit);
	ASSERT(m_pCommand || m_pParent);
	ASSERT(!m_szPromptText.empty());

	Reset();

	//允许做一些特殊的处理等
	// 如果开始完成了，例如：从全局选择集获取了对象，可以直接返回
	if(OnBegin() == KSTA_DONE)
		return KSTA_DONE;

	//获取提升信息
	std::string strPrompt;
	DoGetPromptText(strPrompt);

	// 将传入的选项合入到工具选项集中
	// 需要判断是否有自己的选项
	if(!aToolOptionSet_.IsEmpty()){
		bHasOwnOptions_ = true;
		CombineOptionSet(pInOptionSet_);
		pCurOptionSet_ = &aToolOptionSet_;
	}else{
		bHasOwnOptions_ = false;
		//直接使用输入的选项集
		pCurOptionSet_ = pInOptionSet_;
	}
	
	// 输入框显示提示，开始输入。
	m_pInputEdit->RequestInput(strPrompt.c_str(),this,pCurOptionSet_);
	
	SetInputState(KINPUT_RUNNING);//转换到运行状态

	return KSTA_CONTINUE;
}

// 结束工具运行
// 工具运行结束（内部或外部）总会调用该函数
//
int  kiInputTool::End()
{
	//将传入的选项从工具选择集中分离,只有在本工具有选项时，才需要这样
	//
	if(bHasOwnOptions_){
		SplitOptionSet(pInOptionSet_);
	}

	OnEnd();

	if(m_pInputEdit)
		m_pInputEdit->PromptText("指令");

	if(m_nInputState == KINPUT_DONE)
		return KSTA_DONE;

	return KSTA_FAILED;
}

Handle(AIS_InteractiveContext)  kiInputTool::GetAISContext() const
{
	ASSERT(!hAISCtx_.IsNull());
	return hAISCtx_;
}

kcDocContext* kiInputTool::GetDocContext() const
{
	if(m_pCommand){
		return m_pCommand->GetDocContext();
	}
	if(m_pParent){
		return m_pParent->GetDocContext();
	}
	return NULL;
}

kiInputEdit* kiInputTool::GetInputEdit() const
{
	if(m_pCommand){
		return m_pCommand->GetInputEdit();
	}

	if(m_pParent){
		return m_pParent->GetInputEdit();//这里会递归调用
	}
	ASSERT(FALSE); //不应当发生
	return NULL;
}

const Handle(V3d_View) kiInputTool::GetCurrV3dView() const
{
	Handle(V3d_View) aView;
	if(m_pCommand){
		return m_pCommand->GetCurrV3dView();
	}
	if(m_pParent){
		return m_pParent->GetCurrV3dView();
	}
	return aView;
}

// 获取所在模型
kcModel* kiInputTool::GetModel() const
{
	if (m_pCommand) {
		return m_pCommand->GetModel();
	}
	if (m_pParent) {
		return m_pParent->GetModel();
	}
	return NULL;
}

// 获取当前基准面
kcBasePlane* kiInputTool::GetCurrentBasePlane() const
{
	if(m_pCommand){
		return m_pCommand->GetCurrentBasePlane();
	}
	if(m_pParent){
		return m_pParent->GetCurrentBasePlane();
	}
	return NULL;
}

kcSnapMngr* kiInputTool::GetSnapManager() const
{
	if(m_pCommand){
		kcModel *pModel = m_pCommand->GetModel();
		if(pModel){
			return pModel->GetSnapManager();
		}
	}
	if(m_pParent){
		return m_pParent->GetSnapManager();
	}
	return NULL;
}

kiOptionSet* kiInputTool::GetCurOptionSet() const
{
	return pCurOptionSet_;
}

kiOptionSet* kiInputTool::GetInOptionSet() const
{
	return pInOptionSet_;
}

//
int kiInputTool::OnBegin()
{
	return KSTA_CONTINUE;
}

int  kiInputTool::OnEnd()
{
	KTRACE("\nkiInputTool::OnEnd()....\n");
	return KSTA_CONTINUE;
}

// 子tool的回调函数,当子tool调用结束时,调用.
int  kiInputTool::OnSubInputFinished(kiInputTool *pTool)
{
	return KSTA_CONTINUE;
}

int  kiInputTool::GetInputState() const
{
	return m_nInputState;
}

// 设置输入状态
void  kiInputTool::SetInputState(int nState)
{
	ASSERT(nState >= KINPUT_INITIAL && nState <= KINPUT_CANCEL);
	m_nInputState = nState;
}

//计算当前点
void  kiInputTool::CalcInputPoint(kuiMouseInput& mouseInput,double point[3])
{
	//是否有自动捕获点
	if(m_pSnapMgr && m_pSnapMgr->HasSnapPoint()){
		kiSnapPoint sp = m_pSnapMgr->GetSnapPoint();
		sp._point.get(point);
	}else{
		if(m_pCommand){
			kcModel *pModel = m_pCommand->GetModel();
			if(pModel){
				kcBasePlane *pBasePlane = GetCurrentBasePlane();
				ASSERT(pBasePlane);
				kPoint3 wp;
				pBasePlane->PixelToWorkPlane(mouseInput.m_x,mouseInput.m_y,wp);
				wp.get(point);
			}
		}
	}
}

////////////////////////////////////////////////////////////////
//InputEdit输入的回调函数
int  kiInputTool::OnEditInputChanged(const char *pszText)
{
	return 0;
}

// 当edit输入文本完成后，调用该函数,传入的是输入的文本信息
//
int  kiInputTool::OnEditInputFinished(const char *pInputText)
{
	std::string szPrompt;
	int nSelfOptLimit = 0,idxOpt;

	//用于区分工具自有选项和外部选项
	if(bHasOwnOptions_){
		nSelfOptLimit = pCurOptionSet_->GetCount();
		if(pInOptionSet_)
			nSelfOptLimit -= pInOptionSet_->GetCount();
	}

	if(pCurrentOption_){//字选项的输入
		//如果解析成功
		if(pCurrentOption_->ParseInputText(pInputText)){
			idxOpt = pCurOptionSet_->IndexOfOption(pCurrentOption_);
			if(pCurrentOption_->GetType() == KOPT_INPUT_BUTTON){
				if(idxOpt < nSelfOptLimit)
					OnToolOptionClicked(pCurrentOption_);
				else
					OnOptionSelected(pCurrentOption_);
			}else{
				if(idxOpt < nSelfOptLimit)
					OnToolOptionChanged(pCurrentOption_);
				else
					OnOptionChanged(pCurrentOption_);
			}
			//重新获取提示信息，返回主提示:
			DoGetPromptText(szPrompt);
			pCurrentOption_ = NULL;//置空
			m_pInputEdit->RequestInput(szPrompt.c_str(),this,pCurOptionSet_); //m_pOptionSet);
		}else{
			//要求重新输入
		}
	}else{
		DoTextInputFinished(pInputText);
	}

	return 1;
}
	
// 当edit中选择了选项后（鼠标点击、输入key），调用该函数,传入的是选中的option下标
//
int  kiInputTool::OnEditOptionSelected(int idx,kiOptionItem *pOptionItem)
{
	ASSERT(pCurOptionSet_);
	std::string szPrompt;
	int nSelfOptLimit = 0,idxOpt;

	//用于区分工具自有选项和外部选项
	if(bHasOwnOptions_){
		nSelfOptLimit = pCurOptionSet_->GetCount();
		if(pInOptionSet_)
			nSelfOptLimit -= pInOptionSet_->GetCount();
	}

	if(pCurrentOption_){//是选项的子选项被选了,目前只有enum支持子选项
		if(pCurrentOption_->GetType() == KOPT_INPUT_ENUM){
			pCurrentOption_->SetCurrentOption(idx);
			idxOpt = pCurOptionSet_->IndexOfOption(pCurrentOption_);
			if(idxOpt < nSelfOptLimit) //工具本身选项被选中了
				OnToolOptionChanged(pCurrentOption_);
			else
				OnOptionChanged(pCurrentOption_);

			//重新获取提示信息，返回主提示:
			//DoGetPromptText(szPrompt);
			//pCurrentOption_ = NULL;//置空
			//m_pInputEdit->RequestInput(szPrompt.c_str(),this,pCurOptionSet_); //m_pOptionSet);
		}
	}else{//当前主选项被选了
		pCurrentOption_ = pCurOptionSet_->GetOption(idx);
		ASSERT(pCurrentOption_);
		//bool选项，例如：拷贝(&Y)，如果点击，会变为:拷贝(&N).其他都不变
		if(pCurrentOption_->GetType() == KOPT_INPUT_BOOL){
			((kiOptionBool *)pCurrentOption_)->NotValue();
			m_pInputEdit->Update();//更新显示
			if(idx < nSelfOptLimit)
				OnToolOptionChanged(pCurrentOption_);
			else
				OnOptionChanged(pCurrentOption_);
		}else if(pCurrentOption_->GetType() == KOPT_INPUT_BUTTON){
			if(idx < nSelfOptLimit)
				OnToolOptionClicked(pCurrentOption_);
			else
				OnOptionChanged(pCurrentOption_);
		}else{//其他类型
			//列表选项，应当弹出列表
			kiOptionSet *pSubSet = pCurrentOption_->GetSubOptionSet();
			pCurrentOption_->GetPromptText(szPrompt);
			//进入子选项输入提示
			m_pInputEdit->RequestInput(szPrompt.c_str(),this,pSubSet);
		}
	}

	return 1;
}

void	kiInputTool::DoGetPromptText(std::string& str)
{
	str = m_szPromptText;
}

// 处理输入文本
int		kiInputTool::DoTextInputFinished(const char *pInputText)
{
	return 1;
}

/////////////////////////////////////////////////
// 鼠标响应处理函数

void	kiInputTool::OnLButtonDown(kuiMouseInput& mouseInput) 
{
	if(m_pCurSubTool)
		m_pCurSubTool->OnLButtonDown(mouseInput);
}

int		kiInputTool::OnLButtonUp(kuiMouseInput& mouseInput) 
{  
	if(m_pCurSubTool)
		return m_pCurSubTool->OnLButtonUp(mouseInput);
	return KSTA_CONTINUE; 
}

int		kiInputTool::OnLButtonDblClk(kuiMouseInput& mouseInput) 
{  
	if(m_pCurSubTool)
		return m_pCurSubTool->OnLButtonDblClk(mouseInput);
	return KSTA_CONTINUE; 
}

int		kiInputTool::OnMouseMove(kuiMouseInput& mouseInput) 
{  
	if(m_pCurSubTool)
		m_pCurSubTool->OnMouseMove(mouseInput);
	return KSTA_CONTINUE; 
}

void	kiInputTool::OnRButtonDown(kuiMouseInput& mouseInput) 
{
	if(m_pCurSubTool)
		m_pCurSubTool->OnRButtonDown(mouseInput);
}

// 右键默认表示取消输入。
// 如果工具需要特殊处理，需重载该函数。
//
int		kiInputTool::OnRButtonUp(kuiMouseInput& mouseInput) 
{ 
	if(m_pCurSubTool)
		return m_pCurSubTool->OnRButtonUp(mouseInput);
	return KSTA_CANCEL;//默认表示取消输入 
}

//键盘消息
int	kiInputTool::OnKeyDown(kuiKeyInput& keyInput) 
{
	if(m_pCurSubTool)
		return m_pCurSubTool->OnKeyDown(keyInput);
	return KSTA_CONTINUE; 
}

int	kiInputTool::OnKeyUp(kuiKeyInput& keyInput) 
{
	if(m_pCurSubTool)
		return m_pCurSubTool->OnKeyUp(keyInput);
	return KSTA_CONTINUE; 
}

int	kiInputTool::OnChar(kuiKeyInput& keyInput) 
{
	if(m_pCurSubTool)
		return m_pCurSubTool->OnChar(keyInput);
	return KSTA_CONTINUE; 
}

// 输入完成,调用对应的回调处理。
// 根据对应的处理函数，返回对应的状态码
//
int	kiInputTool::DoNotify()
{
	int rc = KSTA_DONE;

	if(IsInputDone()){
		if(m_pCommand){
			rc = m_pCommand->OnInputFinished(this);
		}
		if(m_pParent){
			rc = m_pParent->OnSubInputFinished(this);
		}
	}else{//应当也调用对应的回调函数，以通知处理完成，方便进行一些其他处理
		if(m_pCommand){
			rc = m_pCommand->OnInputFinished(this);
		}
	}
	
	return rc;
}

int	kiInputTool::DoNotifyOptionChanged(kiOptionItem *pOptionItem)
{
	if(m_pCommand)
		m_pCommand->OnOptionChanged(this,pOptionItem);

	if(m_pParent)
		m_pParent->OnOptionChanged(pOptionItem);

	return KSTA_CONTINUE;
}

int	kiInputTool::DoNotifyOptionClicked(kiOptionItem *pOptionItem)
{
	if(m_pCommand)
		m_pCommand->OnOptionClicked(this,pOptionItem);

	if(m_pParent)
		m_pParent->OnOptionSelected(pOptionItem);

	return KSTA_CONTINUE;
}

void kiInputTool::OnToolOptionChanged(kiOptionItem *pOptionItem)
{
}

void kiInputTool::OnToolOptionClicked(kiOptionItem *pOptionItem)
{
}

// 当选项改变后的调用函数
void  kiInputTool::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if(m_pCommand){
		m_pCommand->OnOptionChanged(pOptionItem);
	}
	if(m_pParent){
		m_pParent->OnOptionChanged(pOptionItem);
	}
}

void  kiInputTool::OnOptionSelected(kiOptionItem *pOptionItem)
{
	if(m_pCommand){
		m_pCommand->OnOptionSelected(pOptionItem);
	}

	if(m_pParent){
		m_pParent->OnOptionSelected(pOptionItem);
	}
}

