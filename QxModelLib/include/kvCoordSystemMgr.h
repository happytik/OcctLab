//////////////////////////////////////////////////////////////////////////
//
#ifndef _KV_COORD_SYSTEM_MGR_H_
#define _KV_COORD_SYSTEM_MGR_H_

#include <vector>
#include <string>
#include "QxLibDef.h"
#include "kvCoordSystem.h"

class kuiModelTreeInterface;
typedef std::vector<kvCoordSystem *> TCoordSystemArray;

class QXLIB_API kvCoordSystemMgr
{
public:
	kvCoordSystemMgr(void);
	~kvCoordSystemMgr(void);

	// 初始化
	bool					Initialize();
	void					Destroy();
	void					SetModelTreeInterface(kuiModelTreeInterface *pModelTree);
	void					Clear();

	// 添加一个坐标系
	kvCoordSystem*			AddCoordSystem(const kvCoordSystem& cs);
	bool					RemoveCoordSystem(const kvCoordSystem *pCS);
	kvCoordSystem*			FindCoordSystem(const char *pszName);

	// 获取当前坐标系
	kvCoordSystem*			GetCurrentCS() const;
	
	// 设置当前坐标系
	bool					SetCurrentCS(const kvCoordSystem *pCS);

	int						GetCount() const;
	kvCoordSystem*			GetCoordSystem(int idx);

public:
	//生成名称
	static std::string		GenCoordSystemName();

protected:
	TCoordSystemArray		m_csArray;//坐标系列表,保存了所有的坐标系.
	DWORD					m_dwCurrCS;//当前坐标系的下标.
	kuiModelTreeInterface	*m_pModelTree;

protected:
	static DWORD			m_dwNextNameNo;//下一个可用的名称序号
};

#endif
