//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_UNDOMGR_H_
#define _KI_UNDOMGR_H_

#include <list>
#include <vector>
#include <string>
#include "QxLibDef.h"

class kcEntity;
class kcModel;
class kiCommand;

#define KCT_UNDO_NEWDEL			1
#define KCT_UNDO_MODIFY			2

class kiUndoItem;
	class kiNewDelUndoItem;
	class kiModifyUndoItem;

typedef std::list<kiUndoItem *>  kiUndoItemStack;

//记录一个undo和redo项
//
class kiUndoItem{
public:
	kiUndoItem(const char *lpszName,kcModel *pModel) {
		_name = lpszName;
		_pModel = pModel;
	}

	virtual int					GetType() const = 0;

	virtual bool				Undo() = 0;
	virtual bool				Redo() = 0;

	const char*					GetName() const { return _name.c_str(); }

protected:
	std::string					_name;//undo名称
	kcModel						*_pModel;
};

class QXLIB_API kiNewDelUndoItem : public kiUndoItem{
public:
	kiNewDelUndoItem(const char *lpszName,kcModel *pModel);
	~kiNewDelUndoItem();

	virtual int					GetType() const { return KCT_UNDO_NEWDEL; }

	bool						AddNewEntity(kcEntity *pEntity);
	bool						AddDelEntity(kcEntity *pEntity);

	virtual bool				Undo();
	virtual bool				Redo();

protected:
	std::vector<kcEntity *>		_aNewEntity;//新创建的对象
	std::vector<kcEntity *>		_aDelEntity;//删除的对象
};

class QXLIB_API kiModifyUndoItem : public kiUndoItem{
public:
	kiModifyUndoItem(const char *lpszName,kcModel *pModel,kiCommand *pCommand);
	~kiModifyUndoItem();

	virtual int					GetType() const { return KCT_UNDO_MODIFY; }

	virtual bool				Undo();
	virtual bool				Redo();

protected:
	kiCommand					*_pCommand;
	std::vector<kcEntity *>		_aEntity;//被修改的对象
};

class QXLIB_API kiUndoMgr
{
public:
	kiUndoMgr(kcModel *pModel);
	~kiUndoMgr(void);

	bool						Initialize();
	bool						UnInitialize();

	// 压入一个undo项。
	bool						Push(kiUndoItem *pItem);

	//
	bool						Undo();
	bool						Redo();

	bool						CanUndo() const;
	bool						CanRedo() const;

	void						Clear();

protected:
	//
	bool						PushRedo(kiUndoItem *pItem);
	//清除所有
	void						ClearAllItems();

protected:
	kcModel						*m_pModel;//关联模型。
    kiUndoItemStack				m_undoStack;//栈
	kiUndoItemStack				m_redoStack;//栈
	int							m_nUndoLimit;//undo的限制
	int							m_nRedoLimit;//redo的限制
};

#endif