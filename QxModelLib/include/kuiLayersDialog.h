#pragma once
#include "afxwin.h"


// kuiLayersDialog 对话框
class kcModel;

class kuiLayersDialog : public CDialogEx
{
	DECLARE_DYNAMIC(kuiLayersDialog)

public:
	kuiLayersDialog(kcModel *pModel,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~kuiLayersDialog();

	kcLayer*  GetSelLayer() const;

// 对话框数据
	enum { IDD = IDD_LAYERS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

protected:
	kcModel		*pModel_; //模型
	kcLayer		*pSelLayer_;

	CComboBox m_comboLayers;
public:
	virtual BOOL OnInitDialog();
};
