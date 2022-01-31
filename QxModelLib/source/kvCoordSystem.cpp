#include "StdAfx.h"
#include <assert.h>
#include "QxBaseUtils.h"
#include "QxStgBlock.h"
#include "kvCoordSystem.h"

kvCoordSystem::kvCoordSystem(void)
{
	memset(m_szName,0,KV_CSNAME_MAX);
	m_org.set(0.0);
	m_x.set(1.0,0.0,0.0);
	m_y.set(0.0,1.0,0.0);
	m_z.set(0.0,0.0,1.0);
}

kvCoordSystem::~kvCoordSystem(void)
{
}

//设置
bool	kvCoordSystem::Set(const gp_Ax2& cs)
{
	kPoint3 o(cs.Location().X(),cs.Location().Y(),cs.Location().Z());
	kVector3 z(cs.Direction().X(),cs.Direction().Y(),cs.Direction().Z());
	kVector3 x(cs.XDirection().X(),cs.XDirection().Y(),cs.XDirection().Z());
	
	return Set(o,x,z);
}

bool	kvCoordSystem::Get(gp_Ax2& ax)
{
	ax.SetLocation(KCG_GP_PNT(m_org));
	ax.SetDirection(KCG_GP_DIR(m_z));
	ax.SetXDirection(KCG_GP_DIR(m_x));

	return true;
}

bool	kvCoordSystem::Set(const kPoint3& o,const kVector3& x,const kVector3& z)
{
	if(x.iszero() || z.iszero())
	{
		assert(0);
		return false;
	}

	m_org = o;
	kVector3 y = z ^ x;
	if(y.iszero())
	{
		assert(0);
		return false;
	}
	m_x = x;m_y = y;
	m_x.normalize();
	m_y.normalize();
	m_z = m_x ^ m_y;
	m_z.normalize();

	UpdateTrsf();

	return true;
}

bool  kvCoordSystem::Set(const gp_Pnt& o,const gp_Dir &x,const gp_Dir &z)
{
	kPoint3 org(o.X(),o.Y(),o.Z());
	kVector3 vx(x.X(),x.Y(),x.Z());
	kVector3 vz(z.X(),z.Y(),z.Z());

	return Set(org,vx,vz);
}

bool kvCoordSystem::Set(const kPoint3& o,const kVector3& x,const kVector3& y,const kVector3& z)
{
	if(x.iszero() || y.iszero() || z.iszero())
	{
		assert(false);
		return false;
	}
	m_org = o;
	m_x = x;
	m_x.normalize();
	m_y = z ^ m_x;
	m_y.normalize();
	m_z = m_x ^ m_y;
	m_z.normalize();

	UpdateTrsf();

	return true;
}

//三点生成
bool  kvCoordSystem::Create(const kPoint3& p1,const kPoint3& p2,const kPoint3& p3)
{
	kVector3 x(p1,p2),y(p1,p3);
	if(x.iszero() || y.iszero())
		return false;

	m_x = x;
	m_z = x ^ y;
	if(m_z.iszero())
		return false;

	m_org = p1;
	m_x.normalize();
	m_z.normalize();
	m_y = m_z ^ m_x;
	m_y.normalize();

	UpdateTrsf();

	return true;
}

bool	kvCoordSystem::SetName(const char *pszName)
{
	ASSERT(pszName);
	if(!pszName) return false;

	int ix,len = lstrlen(pszName);
	ASSERT(len < KV_CSNAME_MAX);
	if(len >= KV_CSNAME_MAX)
		return false;

	for(ix = 0;ix < len;ix ++)
		m_szName[ix] = pszName[ix];
	m_szName[ix] = '\0';

	return true;
}

bool	kvCoordSystem::HasName(const char *pszName) const
{
	if(!pszName || pszName[0] == '\0')
		return false;

	if(lstrcmp(m_szName,pszName) == 0)
		return true;

	return false;
}

kvCoordSystem&  kvCoordSystem::operator=(const kvCoordSystem& other)
{
	Set(other.Org(),other.X(),other.Y(),other.Z());
	SetName(other.GetName());

	return *this;
}

//更新变换矩阵
void  kvCoordSystem::UpdateTrsf()
{
	gp_Ax2 wcs;
	gp_Ax2 ucs(gp_Pnt(m_org.x(),m_org.y(),m_org.z()),
		       gp_Dir(m_z.x(),m_z.y(),m_z.z()),
			   gp_Dir(m_x.x(),m_x.y(),m_x.z()));

	m_utowTrsf.SetTransformation(gp_Ax3(ucs),gp_Ax3(wcs));
	m_wtouTrsf.SetTransformation(gp_Ax3(wcs),gp_Ax3(ucs));
}

//点和向量的坐标转换
void	kvCoordSystem::UCSToWCS(kPoint3& p)
{
	gp_Pnt pnt(p.x(),p.y(),p.z());
	pnt.Transform(m_utowTrsf);
	p.set(pnt.X(),pnt.Y(),pnt.Z());	
}

void	kvCoordSystem::WCSToUCS(kPoint3& p)
{
	gp_Pnt pnt(p.x(),p.y(),p.z());
	pnt.Transform(m_wtouTrsf);
	p.set(pnt.X(),pnt.Y(),pnt.Z());	
}

void	kvCoordSystem::UCSToWCS(kVector3& v)
{
	gp_Vec vec(v.x(),v.y(),v.z());
	vec.Transform(m_utowTrsf);
	v.set(vec.X(),vec.Y(),vec.Z());
}

void  kvCoordSystem::WCSToUCS(kVector3& v)
{
	gp_Vec vec(v.x(),v.y(),v.z());
	vec.Transform(m_wtouTrsf);
	v.set(vec.X(),vec.Y(),vec.Z());
}

bool  kvCoordSystem::BuildStgBlock(QxStgBlock& blk)
{
	double da[3];
	
	blk.Initialize(0x11aa);

	m_org.get(da);
	blk.AddCodeValue(1,da);
	m_x.get(da);
	blk.AddCodeValue(2,da);
	m_y.get(da);
	blk.AddCodeValue(3,da);
	m_z.get(da);
	blk.AddCodeValue(4,da);

	return true;
}

bool  kvCoordSystem::ParseStgBlock(QxStgBlock& blk)
{
	if(blk.GetType() != 0x11aa){
		assert(0);
		return false;
	}

	double da[3];

	blk.GetValueByCode(1,da);
	m_org.set(da);
	blk.GetValueByCode(2,da);
	m_x.set(da);
	blk.GetValueByCode(3,da);
	m_y.set(da);
	blk.GetValueByCode(4,da);
	m_z.set(da);

	UpdateTrsf();

	return true;
}