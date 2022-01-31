// source\dialogs\kuiLayersDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "kcLayer.h"
#include "kcModel.h"
#include "OxModel.h"
#include "resource.h"
#include "kuiLayersDialog.h"
#include "afxdialogex.h"

// kuiLayersDialog 对话框

IMPLEMENT_DYNAMIC(kuiLayersDialog, CDialogEx)

kuiLayersDialog::kuiLayersDialog(kcModel *pModel,CWnd* pParent /*=NULL*/)
	: CDialogEx(kuiLayersDialog::IDD, pParent)
{
	pModel_ = pModel;
	pSelLayer_ = NULL;
}

kuiLayersDialog::~kuiLayersDialog()
{
}

void kuiLayersDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_LAYERS, m_comboLayers);
}

kcLayer* kuiLayersDialog::GetSelLayer() const
{
	return pSelLayer_;
}


BEGIN_MESSAGE_MAP(kuiLayersDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &kuiLayersDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// kuiLayersDialog 消息处理程序
void kuiLayersDialog::OnBnClickedOk()
{
	int iSel = m_comboLayers.GetCurSel();
	if(iSel != CB_ERR){
		CString szName;
		m_comboLayers.GetLBText(iSel,szName);
		pSelLayer_ = pModel_->FindLayer(szName);
	}
	
	CDialogEx::OnOK();
}


BOOL kuiLayersDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	pSelLayer_ = NULL;
	if(pModel_){
		kcLayer *pLayer = NULL;
		const kcLayerList &layerList = pModel_->GetLayerList();
		kcLayerList::const_iterator ite = layerList.begin();
		for(;ite != layerList.end();++ite){
			pLayer = (*ite);
			m_comboLayers.AddString(pLayer->GetLayerName().c_str());
		}
		m_comboLayers.SetCurSel(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control

}

////////////////////////////////////////////////
//
kcLayer* PickLayerByDlg(CWnd *pParentWnd,kcModel *pModel)
{
	if(!pModel){
		ASSERT(FALSE);
		return NULL;
	}

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	kcLayer *pSelLayer = NULL;
	kuiLayersDialog dlg(pModel,pParentWnd);
	if(dlg.DoModal() == IDOK){
		pSelLayer = dlg.GetSelLayer();
	}
	return pSelLayer;
}