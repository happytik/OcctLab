#include "StdAfx.h"
#include "kiCommand.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kiInputPicker.h"
#include "QxFacePointPicker.h"
#include "kiInputEntityTool.h"

#pragma warning(suppress : 4996)

//为分离和转换对象类型使用的数组
static struct stEntShapeType{
	int		_enttype;
	TopAbs_ShapeEnum  _shapetype;
}s_aTypePair[7] = {
 	{ KCT_ENT_POINT, TopAbs_VERTEX },
	{ KCT_ENT_EDGE, TopAbs_EDGE },
	{ KCT_ENT_WIRE,  TopAbs_WIRE },
	{ KCT_ENT_FACE,  TopAbs_FACE },
	{ KCT_ENT_SHELL, TopAbs_SHELL },
	{ KCT_ENT_SOLID, TopAbs_SOLID },
	{ KCT_ENT_COMPOUND, TopAbs_COMPOUND }
};

//////////////////////////////////////////////////////////////////////////
QxShapePickInfo::QxShapePickInfo()
{
	_bValid = false;
	_t = _u = _v = 0.0;
}

QxShapePickInfo::QxShapePickInfo(const QxShapePickInfo& other)
{
	*this = other;
}

QxShapePickInfo&		QxShapePickInfo::operator=(const QxShapePickInfo& other)
{
	_bValid = other._bValid;
	_t = other._t;
	_u = other._u; _v = other._v;
	_p = other._p;
	return *this;
}



kiInputEntityTool::kiInputEntityTool(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet)
	:kiInputTool(pCommand,pszPrompt,pOptionSet),m_selSet(pCommand->GetModel())
{
	ASSERT(pCommand != NULL && pszPrompt != NULL);
	m_hiColor = Quantity_NOC_BLUE1;
	m_nTypeMask = 0;
	m_nSelLimit = 0;
	m_bSingleSelect = false;
	m_bNeedPickInfo = false;
	m_bNaturalMode = true;//默认为自然对象选择，不支持子对象选择，需要设置
	m_bTypeSwitch = false;//默认全部激活
	m_nPickedPointTypeMask = 0;

	m_nShapeTypes = 0;
	m_nOptionIdx = -1;
	pTypeOptEnum_ = NULL;
}

kiInputEntityTool::~kiInputEntityTool(void)
{
}

//设置对应选项
void  kiInputEntityTool::SetOption(int typeMask,bool bSingleSel)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}
	m_nTypeMask = typeMask;
	m_bSingleSelect = bSingleSel;
	//
	DoSplitShapeTypes();
}

void  kiInputEntityTool::SetOption(int typeMask,int num)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}

	m_nTypeMask = typeMask;
	m_nSelLimit = num;
	if(num == 1){
		m_bSingleSelect = true;
	}
	//
	DoSplitShapeTypes();
}

//分离选择对象类型
void  kiInputEntityTool::DoSplitShapeTypes()
{
	m_nShapeTypes = 0;
	if(m_nTypeMask == 0)
		return;

	int ix = 0;
	for(ix = 0;ix < 7;ix ++){
		if(m_nTypeMask & s_aTypePair[ix]._enttype){
			m_aShapeTypes[m_nShapeTypes] = s_aTypePair[ix]._shapetype;
			m_nShapeTypes ++;
		}
	}
}

void kiInputEntityTool::NeedPickInfo(bool bNeeded)
{
	m_bNeedPickInfo = bNeeded;
}

//设定为选择自然对象模式，即不支持局部对象选取
void  kiInputEntityTool::SetNaturalMode(bool bEnable)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}
	m_bNaturalMode = bEnable;
}

//设定为类型切换模式，就是多种类型可以选择切换
void  kiInputEntityTool::SetTypeSwitch(bool bEnable)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}
	m_bTypeSwitch = bEnable;
}

// 设定全局的可交换对象，选择的子对象只能在该对象内部选取
void  kiInputEntityTool::SetGlobalAISObject(const Handle(AIS_InteractiveObject) &aObj)
{
	hGlobalAISObj_ = aObj;
}

bool  kiInputEntityTool::SetEntityTypes(int typeMask)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return false;
	}

	m_nTypeMask = typeMask;
	//
	DoSplitShapeTypes();
	return true;
}

//设定拾取的个数限制
bool  kiInputEntityTool::SetEntityLimit(int nLimit)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return false;
	}
	m_nSelLimit = nLimit;
	return true;
}

//单选或多选
void  kiInputEntityTool::SetSingleSelect(bool bEnable)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}

	m_bSingleSelect = bEnable;
}

//需要点击点的信息，通常对Edge或Face而言
void  kiInputEntityTool::NeedPickedPoint(int typeMask)
{
	//需要在初始化状态下设定
	if(GetInputState() != KINPUT_INITIAL){
		ASSERT(FALSE);
		return;
	}

	m_nPickedPointTypeMask = typeMask;
}

//激活选择模式,主要激活局部对象的选择模式
void  kiInputEntityTool::ActivateSelectionModes()
{
	if(!m_bNaturalMode){
		DoActivateSelectionModes();
	}
}

//去激活选择模式,恢复到自然选择状态
void  kiInputEntityTool::DeactivateSelectionModes()
{
	DoActiveDefaultSelectionMode();
}

int  kiInputEntityTool::OnBegin()
{
	//如果选择了全局选择集中的对象，并且满足了要求
	if(IsInputDone()){
		if(m_pCommand){
			m_pCommand->OnInputFinished(this);
		}

		return KSTA_DONE;
	}

	//激活局部选择模式
	if(!m_bNaturalMode){
		DoActivateSelectionModes();
	}

	return KSTA_CONTINUE;
}

//创建类型切换选项
BOOL  kiInputEntityTool::CreateTypeSwitchOption()
{
	if(!pTypeOptEnum_){	
		pTypeOptEnum_ = new kiOptionEnum("拾取类型",'T',m_nOptionIdx);
		int ix,entyp = 0,ic = 0;
		for(ix = 0;ix < m_nShapeTypes;ix ++){
			switch(m_aShapeTypes[ix]){
			case TopAbs_VERTEX:
				pTypeOptEnum_->AddEnumItem("Vertex",'V');
				break;
			case TopAbs_EDGE:
				pTypeOptEnum_->AddEnumItem("Edge",'E');
				break;
			case TopAbs_WIRE:
				pTypeOptEnum_->AddEnumItem("Wire",'W');
				break;
			case TopAbs_FACE:
				pTypeOptEnum_->AddEnumItem("Face",'F');
				break;
			case TopAbs_SOLID:
				pTypeOptEnum_->AddEnumItem("Solid",'S');
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}

		pTypeOptEnum_->SetOptionCallBack(this);
	}
	
	// 添加到对应的选择集中
	kiOptionSet *pOptSet = GetInOptionSet();
	if(pOptSet){
		// 插入到第一个
		pOptSet->InsertOptionItem(0,pTypeOptEnum_);
	}else{
		//插入到tool自己的选项集合中
		GetToolOptionSet().InsertOptionItem(0,pTypeOptEnum_);
	}

	return TRUE;
}

// 激活设定的选择模式,可以同时是多种对象的选择
// 注意：两种方式的处理
void  kiInputEntityTool::DoActivateSelectionModes()
{
	ASSERT(!m_bNaturalMode);
	if(m_nTypeMask == 0)
		return;

	Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
	if(hAISCtx.IsNull()){
		ASSERT(FALSE);
		return;
	}

	if(m_nShapeTypes > 0){
		if(!m_bTypeSwitch){//非类型切换，激活所有
			int ix = 0;
			//AIS中同时激活多种选择mode是可以的。
			hAISCtx->Deactivate();//去激活旧的
			for(ix = 0;ix < m_nShapeTypes;ix ++){
				hAISCtx->Activate(AIS_Shape::SelectionMode(m_aShapeTypes[ix]),false);
			}
		}else{//类型切换模式
			CreateTypeSwitchOption();
			//TODO:这里先简单处理，如果有传入的，应当添加到传入的功能集中.
			m_nOptionIdx = 0;

			//激活默认模式
			hAISCtx->Deactivate();//去激活旧的
			hAISCtx->Activate(AIS_Shape::SelectionMode(m_aShapeTypes[m_nOptionIdx]),true);
		}
	}
}

//激活默认的选择模式，就是自然拓扑对象选择模式，不支持局部对象
//
void  kiInputEntityTool::DoActiveDefaultSelectionMode()
{
	if(!m_bNaturalMode){
		Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
		if(!hAISCtx.IsNull()){
			hAISCtx->Deactivate();
			hAISCtx->Activate(0,true);//默认方式
		}
	}
}

int  kiInputEntityTool::OnEnd()
{
	//选择集不能清空，后面可能还要使用。
	//由外部决定选择集是否需要清空。
	DoActiveDefaultSelectionMode();	
	//
	// 需要去除添加的类型选项
	if(pTypeOptEnum_){
		kiOptionSet *pOptSet = GetInOptionSet();
		if(pOptSet){
			pOptSet->RemoveOptionItem(pTypeOptEnum_);
		}else{
			GetToolOptionSet().RemoveOptionItem(pTypeOptEnum_);
		}
		delete pTypeOptEnum_;
		pTypeOptEnum_ = NULL;
	}
	
	return KSTA_DONE;
}

//添加全局选择集中的对象
void  kiInputEntityTool::AddInitSelected(bool bAutoReturn)
{
	int entyp = 0;
	kiSelSet *pGlobSelSet = m_pCommand->GetGlobalSelSet();
	if(pGlobSelSet && !pGlobSelSet->IsEmpty()){
		if(m_bSingleSelect){//单选一个
			if(pGlobSelSet->GetSelCount() == 1){
				kiSelEntity se = pGlobSelSet->GetFirstSelected();
				entyp = se._pEntity->GetType();
				if(entyp & m_nTypeMask){
					m_selSet.AddSelected(se);
				}
			}
		}else{
			//多选
			int nSel = 0;
			for(pGlobSelSet->InitSelected();pGlobSelSet->MoreSelected();pGlobSelSet->NextSelected()){
				kiSelEntity se = pGlobSelSet->CurSelected();
				entyp = se._pEntity->GetType();
				if(entyp & m_nTypeMask){
					m_selSet.AddSelected(se);
					nSel ++;
					if(m_nSelLimit > 0 && nSel >= m_nSelLimit)
						break;
				}
			}
		}

		if(bAutoReturn){
			pGlobSelSet->UnHilight(FALSE);
			pGlobSelSet->Clear();

			// 显示
			m_selSet.Hilight(m_hiColor,TRUE);
		}

		if(bAutoReturn){
			if((m_bSingleSelect && m_selSet.GetSelCount() == 1) ||
				(!m_bSingleSelect && m_nSelLimit > 0 && m_selSet.GetSelCount() >= m_nSelLimit))
			{
				SetInputState(KINPUT_DONE);
				// 这里不通知，外部需要判断输入是否完成
				//if(m_pCommand){
				//	m_pCommand->OnInputFinished(this);
				//}
			}
		}
	}
}

// 获取选中项,注意：区分局部对象
//
bool  kiInputEntityTool::GetSelected(kuiMouseInput& mouseInput)
{
	kcModel *pModel = GetModel();
	Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
	if(!pModel || hAISCtx.IsNull())
		return false;

	kcEntity *pEntity = NULL;
	int ityp = 0,nbSel = 0;
	Handle(AIS_InteractiveObject) hObj;
	Handle(AIS_Shape) aAIShape;
	Handle(SelectMgr_EntityOwner) aOwner;
	TopoDS_Shape aS,aSLoc;
	
	//只能选中一个,清空选择集，否则就会不断将选中
	//对象添加到选择集中
	if(m_bSingleSelect){
		ClearSel();
	}

	//拾取对象
	//hAISCtx->Select(Standard_False);
	hAISCtx->SelectDetected();

	//遍历，获取选择对象
	hAISCtx->InitSelected();
	while(hAISCtx->MoreSelected()){
		// 在分解状态下，这里返回的是完整的shape对应的ActiveObject。
		hObj = hAISCtx->SelectedInteractive();
		if(hObj.IsNull() || !hAISCtx->HasSelectedShape()){
			hAISCtx->NextSelected();
			continue;
		}

		// 子对象选择模式下，如果限制了选择对象，则需要加以判断
		if (!m_bNaturalMode && !hGlobalAISObj_.IsNull() &&
			hGlobalAISObj_ != hObj)
		{
			hAISCtx->NextSelected();
			continue;
		}
			
		aAIShape = Handle(AIS_Shape)::DownCast(hObj);
		aS = aAIShape->Shape();
		// 在分解模式下，记录的通常是子对象
		aSLoc = hAISCtx->SelectedShape();
		// 
		aOwner = hAISCtx->SelectedOwner();//总是获取该对象
		//判断是否局部对象
		bool bLocShape = false;
		TopAbs_ShapeEnum aSType;
		if(aS.ShapeType() != aSLoc.ShapeType()){//局部对象
			aSType = aSLoc.ShapeType();
			bLocShape = true;
			ASSERT(aOwner->ComesFromDecomposition());
		}else{
			aSType = aS.ShapeType();
		}
		
		ityp = ksEntityType(aSType);
		if(ityp & m_nTypeMask){//复合条件
			pEntity = pModel->GetEntity(hObj);
			//新选择对象
			kiSelEntity se(pEntity,hObj);
			se.SetEntityOwner(aOwner);
			if(bLocShape && !aSLoc.IsNull()){
				//局部对象
				se.SetLocalShape(aSLoc);
			}
			m_selSet.AddSelected(se);//添加到选择集
			nbSel ++;
			//单选处理
			if(m_bSingleSelect && nbSel == 1){
				// 如果需要拾取点信息，进行计算
				if (m_bNeedPickInfo) {
					GetPickInfo(mouseInput);
				}

				SetInputState(KINPUT_DONE);
				break;
			}
			//限制个数处理
			if(m_nSelLimit > 0 && nbSel == m_nSelLimit){
				SetInputState(KINPUT_DONE);
				break;
			}
		}
		 
		hAISCtx->NextSelected();
	}

	return true;
}

// 获取edge、wire等点击点处的信息,目前仅支持单选的情况
BOOL kiInputEntityTool::GetPickInfo(kuiMouseInput& mouseInput)
{
	m_aPickInfo._bValid = false;
	if (m_selSet.IsEmpty())
		return FALSE;

	kiSelEntity se = GetFirstSelect();
	TopoDS_Shape aS = se.SelectShape();
	if (!aS.IsNull()) {
		// edge
		if (aS.ShapeType() == TopAbs_EDGE){
			TopoDS_Edge aEdge = TopoDS::Edge(aS);
			kiCurvePicker picker(GetDocContext());

			picker.Init(aEdge);
			if (picker.Pick(mouseInput.m_x, mouseInput.m_y, 4, m_aPickInfo._t, m_aPickInfo._p)){
				m_aPickInfo._bValid = true;
				return TRUE;
			}
		}else if(aS.ShapeType() == TopAbs_FACE){
			double pt[3], uv[2];
			TopoDS_Face aFace = TopoDS::Face(aS);
			QxFacePointPicker facePicker(aFace, this->GetCurrV3dView());

			if(facePicker.PickPoint(mouseInput.m_x, mouseInput.m_y, pt, uv)){
				m_aPickInfo._bValid = true;
				m_aPickInfo._p.set(pt);
				m_aPickInfo._u = uv[0];
				m_aPickInfo._v = uv[1];

				return TRUE;
			}
		}
	}
	
	return FALSE;
}

//鼠标消息
void	kiInputEntityTool::OnLButtonDown(kuiMouseInput& mouseInput)
{
	
}

// 选择选中的对象。一般一次只能选中一个，多选通过selset来实现。
//
int  kiInputEntityTool::OnLButtonUp(kuiMouseInput& mouseInput)
{
	Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
	if(hAISCtx.IsNull())
		return KSTA_CONTINUE;

	// 隐藏
	m_selSet.UnHilight(FALSE);
	// 获取选中项
	GetSelected(mouseInput);
	// 显示
	m_selSet.Hilight(m_hiColor,TRUE);

	if(IsInputDone()){
		if(m_pCommand){
			m_pCommand->OnInputFinished(this);
		}

		return KSTA_DONE;
	}else{
		if(m_pCommand){
			m_pCommand->OnInputChanged(this);
		}
	}

	return KSTA_CONTINUE;
}

// 按空格或回车完成输入
int  kiInputEntityTool::OnEditInputFinished(const char *pszText)
{
	if(m_selSet.GetSelCount() > 0){
		SetInputState(KINPUT_DONE);
		if(m_pCommand){
			m_pCommand->OnInputFinished(this);
		}
		return KSTA_DONE;
	}

	return KSTA_CONTINUE;
}

int  kiInputEntityTool::OnMouseMove(kuiMouseInput& mouseInput)
{
	Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
	Handle(V3d_View) aView = GetCurrV3dView();
	//必须调用MoveTo，里面会进行选择操作。
	if(!hAISCtx.IsNull() && !aView.IsNull()){
		hAISCtx->MoveTo(mouseInput.m_x,mouseInput.m_y,aView,Standard_True);
	}

	return KSTA_CONTINUE;
}

int  kiInputEntityTool::OnRButtonUp(kuiMouseInput& mouseInput)
{
	if(m_selSet.GetSelCount() > 0){
		SetInputState(KINPUT_DONE);
		if(m_pCommand){
			m_pCommand->OnInputFinished(this);
		}
		return KSTA_DONE;
	}else{
		SetInputState(KINPUT_CANCEL);
		if(m_pCommand){
			m_pCommand->OnInputFinished(this);
		}
		return KSTA_CANCEL;
	}
}

// 当选项改变后的调用函数
void  kiInputEntityTool::OnOptionChanged(kiOptionItem *pOptionItem)
{
	if(pOptionItem == NULL || pOptionItem->GetType() != KOPT_INPUT_ENUM)
		return;

	if(!m_bTypeSwitch)
		return;

	kiOptionEnum *pEnumItem = (kiOptionEnum *)pOptionItem;
	int preVal = pEnumItem->GetPrevEnumValue();
	//注意：m_nOptionIdx是随选项切换而自动更新的
	//切换模式
	Handle(AIS_InteractiveContext) hAISCtx = GetAISContext();
	if(!hAISCtx.IsNull()){
		if(preVal != m_nOptionIdx){//去激活旧模式
			hAISCtx->Deactivate(AIS_Shape::SelectionMode(m_aShapeTypes[preVal]));
		}

		//激活新模式
		hAISCtx->Activate(AIS_Shape::SelectionMode(m_aShapeTypes[m_nOptionIdx]),false);
	}
}

//清除选择项
void	kiInputEntityTool::ClearSel()
{
	if(!m_selSet.IsEmpty()){
		m_selSet.UnHilight(TRUE);
		m_selSet.Clear();
	}
}

kiSelSet*  kiInputEntityTool::GetSelSet()
{
	return &m_selSet;
}

int  kiInputEntityTool::GetSelCount()
{
	return m_selSet.GetSelCount();
}

kiSelEntity  kiInputEntityTool::GetFirstSelect()
{
	ASSERT(m_selSet.GetSelCount() > 0);
	kiSelEntity se;
	if(m_selSet.IsEmpty())
		return se;
	m_selSet.InitSelected();
	se = m_selSet.CurSelected();
	return se;
}

TopoDS_Shape kiInputEntityTool::SelectedShape(int ix)
{
	ASSERT(m_selSet.GetSelCount() > 0);
	TopoDS_Shape aS;
	int ic = 0;
	for (m_selSet.InitSelected(); m_selSet.MoreSelected(); m_selSet.NextSelected(), ic++) {
		if (ic == ix) {
			aS = m_selSet.CurSelected().SelectShape();
			break;
		}
	}
	return aS;
}

bool kiInputEntityTool::GetShapePickInfo(QxShapePickInfo& info)
{
	if (!m_aPickInfo._bValid)
		return false;

	info = m_aPickInfo;
	return true;
}