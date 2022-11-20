#include "StdAfx.h"
#include "kBase.h"
#include "kiParseString.h"
#include "kiInputTool.h"
#include "kcSysUtils.h"
#include "kiOptionSet.h"

//////////////////////////////////////////////////////////////////////////
//
// Ϊ�µ�
kiOptionItem::kiOptionItem(const char *pname,char key)
{
	ASSERT(pname);
	ASSERT(key);
	m_strName = pname;
	m_cKey = (char)toupper(key);//ͳһΪ��д�ַ������岻���ִ�Сд��
	m_nID = -1;//Ĭ��ֵ��û���趨ID
	m_bPopupItem = false;//Ĭ�ϲ�����

	pParentItem_ = NULL;
	pInputEdit_ = NULL;
	pOptionCallBack_ = NULL;
}

kiOptionItem::kiOptionItem(const char *pname,char key,int id)
{
	ASSERT(pname);
	ASSERT(key);
	ASSERT(id >= 0);
	m_strName = pname;
	m_cKey = (char)toupper(key);//ͳһΪ��д�ַ������岻���ִ�Сд��
	m_nID = id;
	m_bPopupItem = false;

	pParentItem_ = NULL;
	pInputEdit_ = NULL;
	pOptionCallBack_ = NULL;
}

kiOptionItem::~kiOptionItem()
{
}

//�Ƿ���Ч
bool  kiOptionItem::IsValid() const
{
	if(m_strName.empty() || m_cKey == 0)
		return false;
	return true;
}

//��InputEdit�����ı��ı�ʱ
int  kiOptionItem::OnEditInputChanged(const char *pszText)
{
	return 1;
}

//��InputEdit�ı��������ʱ
int  kiOptionItem::OnEditInputFinished(const char *pszText)
{
	return 1;
}

//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
int  kiOptionItem::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	return 1;
}

void  kiOptionItem::NotifyOptionChanged()
{
	if(pOptionCallBack_){
		pOptionCallBack_->OnOptionChanged(this);
	}
}

void  kiOptionItem::SetInputEdit(kiInputEdit *pInputEdit)
{
	pInputEdit_ = pInputEdit;
}

void kiOptionItem::SetOptionCallBack(kiOptionCallBack *pCallBack)
{
	pOptionCallBack_ = pCallBack;
}

bool kiOptionItem::HasOptionCallBack() const
{
	return (pOptionCallBack_ != NULL);
}

//////////////////////////////////////////////////////////////////////////
// 
kiOptionButton::kiOptionButton(const char *pname,char key)
:kiOptionItem(pname,key)
{
}

kiOptionButton::kiOptionButton(const char *pname,char key,int id)
	:kiOptionItem(pname,key,id)
{
}

//������ݵ���Ч��,�ú���û������,û��ֵ,����û������.
//
bool	kiOptionButton::ParseInputText(const char *lpszText)
{
	UNUSED_ALWAYS(lpszText);
	return true;
}

// ����:��ɫ(Y)
bool  kiOptionButton::GetPromptText(std::string& strPrompt)
{
	strPrompt = m_strName;
	strPrompt += "(";
	strPrompt += m_cKey;
	strPrompt += ")";

	return true;
}

//��InputEdit�ı��������ʱ
int  kiOptionButton::OnEditInputFinished(const char *pszText)
{
	UNUSED_ALWAYS(pszText);
	return 1;
}

//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
// button�����ǵ�����button��Ҳ������enum����button����Ҫ���ֶԴ�
//
int  kiOptionButton::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	UNUSED_ALWAYS(idx);
	if(this != pItem){
		ASSERT(FALSE);
		return 0;
	}

	//�Ƿ��Ӷ���
	if(pParentItem_){
		pParentItem_->OnEditOptionSelected(idx,this);
	}else{
		if(pOptionCallBack_){
			pOptionCallBack_->OnOptionSelected(this);
		}
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// 
kiOptionBool::kiOptionBool(const char *pname,char key,bool& bVar)
:kiOptionItem(pname,key),m_boolVar(bVar)
{
}

kiOptionBool::kiOptionBool(const char *pname,char key,int id,bool& bVar)
	:kiOptionItem(pname,key,id),m_boolVar(bVar)
{}

kiOptionBool::~kiOptionBool()
{
}

// ������ݵ���Ч��,û������ֵ,�ú���������.
bool	kiOptionBool::ParseInputText(const char *lpszText)
{
	UNUSED_ALWAYS(lpszText);
	return true;
}

// name(key)<Y>
bool	kiOptionBool::GetPromptText(std::string& strPrompt)
{
	strPrompt = m_strName;
	strPrompt += "(";
	strPrompt += m_cKey;
	strPrompt += ")";
	strPrompt += "<";
	if(m_boolVar)
		strPrompt += "��";
	else
		strPrompt += "��";
	strPrompt += ">";

	return true;
}

void  kiOptionBool::NotValue()
{
	m_boolVar = !m_boolVar;
	//
	pInputEdit_->Update();

}

//��InputEdit�ı��������ʱ
int  kiOptionBool::OnEditInputFinished(const char *pszText)
{
	return 1;
}

//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
int  kiOptionBool::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	UNUSED_ALWAYS(idx);
	if(this != pItem){
		ASSERT(FALSE);
		return 0;
	}

	m_boolVar = !m_boolVar;
	pInputEdit_->Update();//������ʾ
	NotifyOptionChanged();

	return 1;
}

//////////////////////////////////////////////////////////////////////////

// ö��,����:����<����>(����(S) ����(M))
//
kiOptionEnum::kiOptionEnum(const char *pname,char key,int& eVal)
:kiOptionItem(pname,key),m_enumVal(eVal)
{
	m_pOptionSet = new kiOptionSet;
	m_oldEnumVal = eVal;
}

kiOptionEnum::kiOptionEnum(const char *pname,char key,int id,int& eVal)
:kiOptionItem(pname,key,id),m_enumVal(eVal)
{
	m_pOptionSet = new kiOptionSet;
	m_oldEnumVal = eVal;
}

kiOptionEnum::~kiOptionEnum()
{
	if(m_pOptionSet)
		delete m_pOptionSet;
}

//���һ��ѡ��.
bool	kiOptionEnum::AddEnumItem(const char *pname,char key)
{
	ASSERT(pname);
	// check if key has exist
	int idx = m_pOptionSet->IndexOfOption(key);
	if(idx >= 0)
	{
		ASSERT(FALSE);
		return false;
	}
	//�������
	kiOptionButton *pVoidItem = new kiOptionButton(pname,key);
	m_pOptionSet->AddOption(pVoidItem);
	pVoidItem->SetParentItem(this);

	return true;
}

// ������ݵ���Ч��
//
bool	kiOptionEnum::ParseInputText(const char *lpszText)
{
	ASSERT(lpszText && lstrlen(lpszText) == 1);
	if(!lpszText || lpszText[0] == '\0' || lstrlen(lpszText) > 1)
		return false;
	
	int idx = m_pOptionSet->IndexOfOption(lpszText[0]);
	if(idx < 0)
		return false;

	if(m_enumVal != idx){
		m_oldEnumVal = m_enumVal;
		m_enumVal = idx;
		// ֪ͨѡ��ı�
		//if(_pInputTool)
		//	_pInputTool->OnOptionChanged(this);
	}
	
	return true;
}

// name<val> ,�����optionSet��ѡ��,�ⲿ����.
//
bool	kiOptionEnum::GetPromptText(std::string& strPrompt)
{
	strPrompt = m_strName;
	// current value
	strPrompt += '<';
	ASSERT(m_enumVal < m_pOptionSet->GetCount());
	kiOptionItem *pItem = m_pOptionSet->GetOption(m_enumVal);
	strPrompt += pItem->m_strName;
	strPrompt += '>';

	return true;
}

// ���ö�Ӧ��ֵ.
//
bool  kiOptionEnum::SetCurrentOption(int idx)
{
	ASSERT(m_pOptionSet);
	ASSERT(idx >= 0 && idx < m_pOptionSet->GetCount());

	if(m_enumVal != idx){
		m_oldEnumVal = m_enumVal;
		m_enumVal = idx;
		//����edit
		pInputEdit_->Update();

		// ֪ͨѡ��ı�
		NotifyOptionChanged();
	}
	return true;
}

//��InputEdit�ı��������ʱ
int  kiOptionEnum::OnEditInputFinished(const char *pszText)
{
	return 1;
}

// ��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
//
int  kiOptionEnum::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	if(pItem == this){//ѡ�б���
		pInputEdit_->PushRequest();
		//
		std::string ssPrompt;
		GetPromptText(ssPrompt);
		pInputEdit_->RequestInput(ssPrompt.c_str(),this,m_pOptionSet);

	}else{//ѡ��������
		pInputEdit_->PopRequest();
		
		SetCurrentOption(idx);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//

//��׼����ϵ��ѡ��
kiOptionStdAxis::kiOptionStdAxis(const char *pname,char key,int& eVal)
	:kiOptionEnum(pname,key,eVal)
{
	_bUseWCS = false;
	_bOnlyZ = false;
}

kiOptionStdAxis::kiOptionStdAxis(const char *pname,char key,int id,int& eVal)
	:kiOptionEnum(pname,key,eVal)
{
	_bUseWCS = false;
	_bOnlyZ = false;
}

int  kiOptionStdAxis::Initialize(bool bUseWCS,bool bOnlyZ)
{
	_bUseWCS = bUseWCS;
	_bOnlyZ = bOnlyZ;

	if(bUseWCS){
		AddEnumItem("Z+",'A');
		AddEnumItem("Z-",'S');
		if(!_bOnlyZ){
			AddEnumItem("X+",'D');
			AddEnumItem("X-",'F');
			AddEnumItem("Y+",'G');
			AddEnumItem("Y-",'H');
		}
	}else{
		AddEnumItem("LZ+",'A');
		AddEnumItem("LZ-",'S');
		if(!_bOnlyZ){
			AddEnumItem("LX+",'D');
			AddEnumItem("LX-",'F');
			AddEnumItem("LY+",'G');
			AddEnumItem("LY-",'H');
		}
	}

	return 1;
}



//////////////////////////////////////////////////////////////////////////
//
kiOptionInt::kiOptionInt(const char *pname,char key,int& iVal)
:kiOptionItem(pname,key),m_intVar(iVal)
{
}

kiOptionInt::kiOptionInt(const char *pname,char key,int id,int& iVal)
:kiOptionItem(pname,key,id),m_intVar(iVal)
{
}

//������ݵ���Ч��
bool	kiOptionInt::ParseInputText(const char *lpszText)
{
	if(lpszText == NULL || lpszText[0] == '\0') return false;
	kiParseInteger pint(lpszText);
	if(!pint.Parse(m_intVar))
		return false;

	// ֪ͨѡ��ı�
	NotifyOptionChanged();

	return true;
}

bool  kiOptionInt::GetPromptText(std::string& strPrompt)
{
	char sbuf[32];//��һЩ,�������Ƚϴ�ʱ,���ܻ������
	strPrompt = m_strName;
	strPrompt += "(";
	strPrompt += m_cKey;
	strPrompt += ")";
	strPrompt += "<";
	sprintf_s(sbuf,32,"%d",m_intVar);
	strPrompt += sbuf;
	strPrompt += ">";

	return true;
}

//��InputEdit�ı��������ʱ
int  kiOptionInt::OnEditInputFinished(const char *pszText)
{
	std::string ssInput;
	if(pszText){
		ssInput = pszText;
	}

	//����Ϊ�գ���Ϊʹ��Ĭ��ֵ
	if(ssInput.empty()){
		pInputEdit_->PopRequest();//�ָ��ɵ�����
	}else{
		kiParseInteger pint(pszText);
		if(!pint.Parse(m_intVar))
			return 0;

		pInputEdit_->PopRequest();//�ָ��ɵ�����
	}

	//��ʾ���������
	NotifyOptionChanged();
	
	return 1;
}

//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
int  kiOptionInt::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	UNUSED_ALWAYS(idx);
	if(this != pItem){
		ASSERT(FALSE);
		return 0;
	}

	if(m_bPopupItem){
		int nVal = m_intVar;
		if(kcDlgInputIntValue(m_strName.c_str(), nVal)){
			if(nVal != m_intVar){
				m_intVar = nVal;
				
				//ֵ�ı��ˣ�������ʾֵ
				if(pInputEdit_){
					pInputEdit_->Update();
				}

				//��ʾ���������
				NotifyOptionChanged();
			}
		}
		return 1;
	}

	pInputEdit_->PushRequest();//���浱ǰ����
	//
	std::string ssPrompt;
	GetPromptText(ssPrompt);
	pInputEdit_->RequestInput(ssPrompt.c_str(),this,NULL);

	return 1;
}

//////////////////////////////////////////////////////////////////////////
//
kiOptionDouble::kiOptionDouble(const char *pname,char key,double& dval)
:kiOptionItem(pname,key),m_doubleVal(dval)
{
}

kiOptionDouble::kiOptionDouble(const char *pname,char key,int id,double& dval)
:kiOptionItem(pname,key,id),m_doubleVal(dval)
{
}

//������ݵ���Ч��
bool	kiOptionDouble::ParseInputText(const char *lpszText)
{
	if(lpszText == NULL || lpszText[0] == '\0') return false;
	kiParseDouble pdbl(lpszText);
	if(!pdbl.Parse(m_doubleVal))
		return false;

	// ֪ͨѡ��ı�
	NotifyOptionChanged();

	return true;
}

bool  kiOptionDouble::GetPromptText(std::string& strPrompt)
{
	char sbuf[256];//�������п��ܺܳ�.
	ASSERT(fabs(m_doubleVal) < KDBL_MAX);
	strPrompt = m_strName;
	strPrompt += "(";
	strPrompt += m_cKey;
	strPrompt += ")";
	strPrompt += "<";
	sprintf_s(sbuf,255,"%.4f",m_doubleVal);
	strPrompt += sbuf;
	strPrompt += ">";

	return true;
}

//��InputEdit�ı��������ʱ
int  kiOptionDouble::OnEditInputFinished(const char *pszText)
{
	std::string ssInput;
	if(pszText){
		ssInput = pszText;
	}

	//����Ϊ�գ���Ϊʹ��Ĭ��ֵ
	if(ssInput.empty()){

		pInputEdit_->PopRequest();//�ָ��ɵ�����
	}else{
		kiParseDouble pdbl(pszText);
		if(!pdbl.Parse(m_doubleVal))
			return 0;

		pInputEdit_->PopRequest();//�ָ��ɵ�����
	}

	//��ʾ���������
	NotifyOptionChanged();
	
	return 1;
}

// ��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
// ��ʱ��Ӧ����ʾ����ʾ��ѡ���ֵ:
int  kiOptionDouble::OnEditOptionSelected(int idx,kiOptionItem *pItem)
{
	UNUSED_ALWAYS(idx);
	if(this != pItem){
		ASSERT(FALSE);
		return 0;
	}
	
	//����ǵ�����
	if(m_bPopupItem){
		double dVal = m_doubleVal;
		if(kcDlgInputDoubleValue(m_strName.c_str(), dVal)){
			if(dVal != m_doubleVal){
				m_doubleVal = dVal;

				//ֵ�ı��ˣ�������ʾֵ
				if(pInputEdit_){
					pInputEdit_->Update();
				}

				//��ʾ���������
				NotifyOptionChanged();
			}
		}
		return 1;
	}

	pInputEdit_->PushRequest();//���浱ǰ����
	//
	std::string ssPrompt;
	GetPromptText(ssPrompt);
	pInputEdit_->RequestInput(ssPrompt.c_str(),this,NULL);

	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
kiOptionSet::kiOptionSet()
{
	pOptionCallBack_ = NULL;
	pInputEdit_ = NULL;
}

kiOptionSet::kiOptionSet(kiOptionCallBack *pOptCallBack)
	:pOptionCallBack_(pOptCallBack)
{
	ASSERT(pOptCallBack);
	pInputEdit_ = NULL;
}

kiOptionSet::~kiOptionSet()
{
	Clear();
}

void  kiOptionSet::SetInputEdit(kiInputEdit *pInputEdit)
{
	if(!pInputEdit){
		ASSERT(FALSE);
		return;
	}

	pInputEdit_ = pInputEdit;
	
	int ix,isize = (int)aItemArray_.size();
	for(ix = 0;ix < isize;ix ++)
		aItemArray_[ix]->SetInputEdit(pInputEdit);
}

// pCallBack����Ϊ��
void  kiOptionSet::SetOptionCallBack(kiOptionCallBack *pCallBack)
{
	pOptionCallBack_ = pCallBack;

	kiOptionItem *pItem = NULL;
	int ix,isize = (int)aItemArray_.size();
	for(ix = 0;ix < isize;ix ++){
		pItem = aItemArray_.at(ix);
		// ���û�У����趨�����򱣳�ԭ�����趨
		if(pCallBack && !pItem->HasOptionCallBack()){
			pItem->SetOptionCallBack(pCallBack);
		}
	}
}

bool  kiOptionSet::HasOptionCallBack() const
{
	return (pOptionCallBack_ != NULL);
}

void  kiOptionSet::DoAddItem(kiOptionItem *pItem)
{
	ASSERT(pItem);
	//������Ҫ�Ĺ���
	if(pInputEdit_){
		pItem->SetInputEdit(pInputEdit_);
	}
	// ���item�趨���Լ��Ļص��������趨��������ԭ�е��趨
	// ��������Item����趨��Ӧ����
	if(pOptionCallBack_ && !pItem->HasOptionCallBack()){
		pItem->SetOptionCallBack(pOptionCallBack_);
	}

	aItemArray_.push_back(pItem);
}

// ���һ��ѡ�ѡ��С���Ҫ�����ظ����.
//
bool  kiOptionSet::AddOption(kiOptionItem* pItem)
{
	ASSERT(pItem && pItem->IsValid());
	if(!pItem || !pItem->IsValid())
		return false;

	// check if key has exist
	int idx = IndexOfOption(pItem->m_cKey);
	if(idx >= 0){
		TRACE("\n## option item has existed.-----");
		return false;
	}
	
	DoAddItem(pItem);
	return true;
}


kiOptionItem*  kiOptionSet::AddBoolOption(const char *pname,char key,bool& bVar)
{
	ASSERT(pname);
	if(!pname || pname[0] == '\0')
		return NULL;

	// check if key has exist
	int idx = IndexOfOption(key);
	if(idx >= 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	kiOptionBool *pItem = new kiOptionBool(pname,key,bVar);
	DoAddItem(pItem);
	
	return pItem;
}

kiOptionItem*  kiOptionSet::AddIntOption(const char *pname,char key,int& iVal)
{
	ASSERT(pname);
	if(!pname || pname[0] == '\0')
		return NULL;

	// check if key has exist
	int idx = IndexOfOption(key);
	if(idx >= 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	kiOptionInt *pItem = new kiOptionInt(pname,key,iVal);
	DoAddItem(pItem);
	
	return pItem;
}

kiOptionItem*  kiOptionSet::AddDoubleOption(const char *pname,char key,double& dval)
{
	ASSERT(pname);
	if(!pname || pname[0] == '\0')
		return NULL;

	// check if key has exist
	int idx = IndexOfOption(key);
	if(idx >= 0){
		ASSERT(FALSE);
		return NULL;
	}

	kiOptionDouble *pItem = new kiOptionDouble(pname,key,dval);
	DoAddItem(pItem);
	
	return pItem;
}

kiOptionItem*	  kiOptionSet::AddButtonOption(const char *pname,char key,int nID)
{
	ASSERT(pname);
	if(!pname || pname[0] == '\0')
		return NULL;

	// �ж�ID�Ƿ����
	kiOptionButton *pOptBtn = NULL;
	int ix,isize = (int)aItemArray_.size();
	for(ix = 0;ix < isize;ix ++){
		if(aItemArray_[ix]->GetType() == KOPT_INPUT_BUTTON){
			pOptBtn = (kiOptionButton *)aItemArray_[ix];
			if(pOptBtn->GetID() == nID){
				ASSERT(FALSE);
				return NULL;
			}
		}
	}
	// check if key has exist
	int idx = IndexOfOption(key);
	if(idx >= 0){
		ASSERT(FALSE);
		return NULL;
	}

	pOptBtn = new kiOptionButton(pname,key,nID);
	DoAddItem(pOptBtn);
	
	return pOptBtn;
}

kiOptionItem*	  kiOptionSet::AddSysButtonOption(int nID)
{
	kiOptionItem *pItem = NULL;
	switch(nID){
	case QX_SYS_BUTTON_APPLY:
		pItem = AddButtonOption("Ӧ��",'A',QX_SYS_BUTTON_APPLY);
		break;
	case QX_SYS_BUTTON_OK:
		pItem = AddButtonOption("ȷ��",'O',QX_SYS_BUTTON_OK);
		break;
	case QX_SYS_BUTTON_CANCEL:
		pItem = AddButtonOption("�˳�",'C',QX_SYS_BUTTON_CANCEL);
		break;
	default:
		break;
	}
	return pItem;
}

void  kiOptionSet::AddApplyButtonOption()
{
	AddSysButtonOption(QX_SYS_BUTTON_APPLY);
}

void  kiOptionSet::AddQuitButtonOption()
{
	AddSysButtonOption(QX_SYS_BUTTON_CANCEL);
}

// �ڶ�Ӧλ�ò������
bool kiOptionSet::InsertOptionItem(size_t idx,kiOptionItem *pItem)
{
	if(!pItem || HasOptionItem(pItem)){
		ASSERT(FALSE);
		return false;
	}
	
	stdOptionItemArray::size_type isize = aItemArray_.size();
	if(idx >= isize){
		aItemArray_.push_back(pItem);
	}else{
		aItemArray_.insert(aItemArray_.begin() + idx,pItem);
	}
	//
	if(!pItem->HasOptionCallBack() && pOptionCallBack_){
		pItem->SetOptionCallBack(pOptionCallBack_);
	}

	return true;
}

// ��ѡ���ɾ��һ��ѡ���
bool kiOptionSet::RemoveOptionItem(kiOptionItem *pItem)
{
	ASSERT(pItem);
	if(aItemArray_.empty())
		return false;

	stdOptionItemArray::size_type ix,isize = aItemArray_.size();
	for(ix = 0;ix < isize;ix ++){
		if(aItemArray_[ix] == pItem)
			break;
	}
	if(ix >= isize)
		return false;

	stdOptionItemArray nArray;
	for(ix = 0;ix < isize;ix ++){
		if(aItemArray_[ix] != pItem)
			nArray.push_back(aItemArray_[ix]);
	}
	aItemArray_.swap(nArray);

	return true;
}

void  kiOptionSet::Clear()
{
	int ix,ic = (int)aItemArray_.size();
	for(ix = 0;ix < ic;ix ++)
		delete aItemArray_[ix];
	aItemArray_.clear();
}

bool  kiOptionSet::IsEmpty() const
{
	return aItemArray_.empty();
}

int  kiOptionSet::GetCount() const
{
	return (int)aItemArray_.size();
}

kiOptionItem*  kiOptionSet::GetOption(int ix)
{
	ASSERT(ix >= 0 && ix < (int)aItemArray_.size());
	return aItemArray_[ix];
}

//���ݼ���ȡ��Ӧ�±ꡣ
int  kiOptionSet::FindOption(char c)
{
	char C = (char)toupper(c);
	kiOptionItem *pItem = NULL;
	int ix,isize = (int)aItemArray_.size();
	int iret = -1;

	for(ix = 0;ix < isize;ix ++){
		pItem = aItemArray_.at(ix);
		if(pItem->m_cKey == C){
			iret = ix;
			break;
		}
	}

	return iret;
}

int  kiOptionSet::IndexOfOption(char c)
{
	char C = (char)toupper(c);
	kiOptionItem *pItem = NULL;
	int ix,isize = (int)aItemArray_.size();
	int iret = -1;

	for(ix = 0;ix < isize;ix ++){
		pItem = aItemArray_.at(ix);
		if(pItem->m_cKey == C){
			iret = ix;
			break;
		}
	}

	return iret;
}

int  kiOptionSet::IndexOfOption(kiOptionItem *pItem)
{
	ASSERT(pItem);
	stdOptionItemArray::size_type ix,isize = aItemArray_.size();
	for(ix = 0;ix < isize;ix ++){
		if(pItem == aItemArray_[ix]){
			return (int)ix;
		}
	}
	return -1;
}

bool  kiOptionSet::HasOptionItem(kiOptionItem *pItem)
{
	if(!pItem) return false;
	int idx = IndexOfOption(pItem);
	return (idx >= 0 ? true : false);
}
