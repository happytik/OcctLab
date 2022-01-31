//////////////////////////////////////////////////////////////////////////
// 一个坐标系的信息. 
//
#ifndef _KV_COORD_SYSTEM_H_
#define _KV_COORD_SYSTEM_H_

#include "kPoint.h"
#include "kVector.h"
#include "QxLibDef.h"

#ifndef KV_CSNAME_MAX
#define KV_CSNAME_MAX		64
#endif

class gp_Ax2;
class QxStgBlock;

class QXLIB_API kvCoordSystem
{
public:
	kvCoordSystem(void);
	~kvCoordSystem(void);

	//设置
	bool					Set(const gp_Ax2& cs);
	bool					Get(gp_Ax2& ax);
	bool					Set(const kPoint3& o,const kVector3& x,const kVector3& z);
	bool					Set(const kPoint3& o,const kVector3& x,const kVector3& y,const kVector3& z);
	bool					Set(const gp_Pnt& o,const gp_Dir &x,const gp_Dir &z);
	bool					SetName(const char *pszName);
	const char*			GetName() const { return m_szName; }
	bool					HasName(const char *pszName) const;

	//三点生成
	bool					Create(const kPoint3& p1,const kPoint3& p2,const kPoint3& p3);

	kvCoordSystem&		operator=(const kvCoordSystem& other);

	const kPoint3&		Org() const;
	const kVector3&		X() const;
	const kVector3&		Y() const;
	const kVector3&		Z() const;
	kPoint3&				Org();
	kVector3&			X();
	kVector3&			Y();
	kVector3&			Z();

	//点和向量的坐标转换
	void					UCSToWCS(kPoint3& p);
	void					WCSToUCS(kPoint3& p);
	void					UCSToWCS(kVector3& v);
	void					WCSToUCS(kVector3& v);

public:
	bool					BuildStgBlock(QxStgBlock& blk);
	bool					ParseStgBlock(QxStgBlock& blk);

protected:
	//更新变换矩阵
	void					UpdateTrsf();

protected:
	char					m_szName[KV_CSNAME_MAX];//名称
	kPoint3				m_org;
	kVector3				m_x;
	kVector3				m_y;
	kVector3				m_z;

protected:
	gp_Trsf				m_utowTrsf;//UCS到WCS的变换矩阵
	gp_Trsf				m_wtouTrsf;//WCS到UCS的变换矩阵
};

inline const kPoint3&	kvCoordSystem::Org() const
{
	return m_org;
}

inline const kVector3&		kvCoordSystem::X() const
{
	return m_x;
}

inline const kVector3&		kvCoordSystem::Y() const
{
	return m_y;
}

inline const kVector3&		kvCoordSystem::Z() const
{
	return m_z;
}

inline kPoint3&	kvCoordSystem::Org()
{
	return m_org;
}

inline kVector3&	kvCoordSystem::X()
{
	return m_x;
}

inline kVector3&	kvCoordSystem::Y()
{
	return m_y;
}

inline kVector3&	kvCoordSystem::Z()
{
	return m_z;
}

#endif