#pragma once
#include "afxcmn.h"

#include "resource.h"

//属性框的处理类，该类将相应内容添加到该对话框中
class kcPropHandler{
public:
	virtual ~kcPropHandler(){}

	//主要接口，设置属性信息
	virtual int SetProperty(CTreeCtrl &propTree) = 0;
};

// kcPropertyDlg 对话框

class kcPropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(kcPropertyDlg)

public:
	kcPropertyDlg(kcPropHandler *pPropHandler,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~kcPropertyDlg();

// 对话框数据
	enum { IDD = IDD_PROPERTY_DIALOG };

	void  ClearAllProperty();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();

protected:
	kcPropHandler		*m_pPropHandler;//属性处理对象指针，外部传入，这里不能释放
	CTreeCtrl m_propTree;//属性树
};
