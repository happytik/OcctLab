//////////////////////////////////////////////////////////////////////////
// 句柄管理。一个model中一个。一个model中所有对象的handle都是唯一的。
//
#ifndef _KC_HANDLE_MGR_H_
#define _KC_HANDLE_MGR_H_

#include <vector>
#include <map>
#include "QxLibDef.h"

// 从1开始的handle。0：表示非法handle。
#ifndef  KC_IS_VALID_HANDLE
#define  KC_IS_VALID_HANDLE(h) (h != 0)
#endif

class CArchive;
class kcHandleMap;
typedef std::map<int,kcHandleMap *>		stdHandleMapMap;

// 负责唯一Handle的管理，分配、释放等等
class QXLIB_API kcHandleMap{
public:
	kcHandleMap();
	~kcHandleMap();

	bool					Initialize(int capacity = 256);
	void					Clear();

	// 分配一个可用的Handle.
	int					AllocHandle();
    // 释放一个可用的Handle
	void					FreeHandle(int handle);
	// 标记一个handle已经使用了
	bool					MaskHandle(int handle);

	bool					Read(CArchive& ar);
	bool					Write(CArchive& ar);

protected:
	int					MapMaxHandle();

protected:
	BYTE					*_pHandleMap;//每个bit标记当前handle是否可用。
	int					_nMapSize;
	int					_nMaxHandle;//已经分配的最大的句柄值。
	int					_nIdleHandle;//空闲句柄个数
};

class QXLIB_API kcHandleMgr
{
public:
	kcHandleMgr(void);
	~kcHandleMgr(void);

	bool					Initialize();
	void					Clear();

	//根据类型，分配和释放Handle
	int					AllocHandle(int entType);
	void					FreeHandle(int entType,int handle);
	// 标记一个handle已经使用了
	bool					MaskHandle(int entType,int handle);

	bool					Read(CArchive& ar);
	bool					Write(CArchive& ar);

protected:
	void					InitHandleMapMap();
	kcHandleMap*			FindHandleMap(int entType);

protected:
	kcHandleMap			_layerHandleMap;//图层
	kcHandleMap			_pointHandleMap;
	kcHandleMap			_curveHandleMap;
	kcHandleMap			_wireHandleMap;
	kcHandleMap			_faceHandleMap;
	kcHandleMap			_shellHandleMap;
	kcHandleMap			_solidHandleMap;
	kcHandleMap			_compHandleMap;
	kcHandleMap			_blockHandleMap;
	kcHandleMap			aDiamDimHandleMap_;
	kcHandleMap			aRadDimHandleMap_;
	kcHandleMap			aLenDimHandleMap_;
	kcHandleMap			aAngDimHandleMap_;
	kcHandleMap			_basePlaneHandleMap;

	stdHandleMapMap		_handleMapMap;
	std::vector<int>		_handleMapArray;
};

#endif