// OcctTryDoc.h :  COcctLabDoc 类的接口
//
#pragma once

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>

class kcModel;
class kiSnapMngr;
class kcSnapMngr;
class kiUndoMgr;
class kcContext;
class kiCommandMgr;
class kiSelSet;
class kcDocContext;

class COcctLabDoc : public CDocument
{
protected: // 仅从序列化创建
	COcctLabDoc();
	DECLARE_DYNCREATE(COcctLabDoc)

// 属性
public:
	Handle(AIS_InteractiveContext)& GetAISContext(){ return m_AISContext; };
	Handle(V3d_Viewer)		GetViewer()  { return m_Viewer; };
	
	kcDocContext*				GetDocContext() const { return pDocContext_; }

	kcModel*					GetModel() { return m_pModel; }
	kiCommandMgr*				GetCommandMgr() const { return m_pCommandMgr; }
	kcSnapMngr*				GetSnapMgr() { return m_pSnapMgr; }
	// 获取全局选择集。
	kiSelSet*				GetGlobalSelSet() const; 
	BOOL						IsNewDocument() const { return m_bNewDocument; }

	// 初始化命令管理器,因为使用了InputEdit，需要在View初始化时进行
	BOOL						InitCommandMgr();

// 操作
public:

// 重写
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 实现
public:
	virtual ~COcctLabDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//两个主要的成员.由Doc创建并负责管理
	Handle(V3d_Viewer)		m_Viewer;  //管理所有的view.
	Handle(AIS_InteractiveContext) m_AISContext;//管理交换.
	// 文档相关内容.
	kcModel					*m_pModel;//模型对象，每个Doc文档一个模型对象
	kiCommandMgr				*m_pCommandMgr;//命令管理器,每个Doc一个对象
	kcSnapMngr				*m_pSnapMgr;

	//记录了所有Doc主要相关数据的对象
	kcDocContext				*pDocContext_;

	BOOL						m_bNewDocument;//是否新建文档
	BOOL						m_bSerializeOK;//是否保存、读取成功

	BOOL						OcctInit();
	
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

	//注册系统命令
	BOOL						RegisterSysCommand();
	void						OnMenuItemDispatch(UINT nID);

public:
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete(CCmdUI *pCmdUI);
	afx_msg void OnMenuImportIges();
	afx_msg void OnMenuImportDxf();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual void OnCloseDocument();
	virtual void SetTitle(LPCTSTR lpszTitle);
	afx_msg void OnImportBrepFile();
	afx_msg void OnImportStepFile();
	afx_msg void OnMenuExportStep();
	afx_msg void OnMenuExportBrep();
	afx_msg void OnMenuExportSelstep();
	afx_msg void OnMenuExportSelbrep();
	afx_msg void OnMenuExportIges();
	afx_msg void OnExportSelIges();
};


