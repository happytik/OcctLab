#include "stdafx.h"
#include <IntAna_IntConicQuad.hxx>
#include <ProjLib.hxx>
#include <ElSLib.hxx>
#include "kvGrid.h"
#include "kcModel.h"
#include "kcHandleMgr.h"
#include "QxStgBlock.h"
#include "kcBasePlane.h"


kcBasePlane::kcBasePlane(const Handle(V3d_View)& aView)
{
	_hView = aView;
	_pGrid = NULL;
	_bActive = false;//默认非激活

	_szName = "";
}


kcBasePlane::~kcBasePlane(void)
{
	Destroy();
}

//根据一个坐标系创建
int  kcBasePlane::Create(kvCoordSystem& cs,kcModel *pModel,const char *pszName)
{
	ASSERT(pModel);
	_cs = cs;
	_pModel = pModel;

	//初始化网格
	if(!InitGrid())
		return 0;
	
	kcHandleMgr *pHandleMgr = pModel->GetHandleMgr();
	_nHandle = pHandleMgr->AllocHandle(KCT_BASE_PLANE);

	if(pszName){
		_szName = pszName;
	}else{
		char szBuf[64];
		sprintf_s(szBuf,64,"基准面 %d",_nHandle);
		_szName = szBuf;
	}

	return 1;
}

bool  kcBasePlane::InitGrid()
{
	ASSERT(_pModel);
	Handle(AIS_InteractiveContext) aCtx = _pModel->GetAISContext();
	if(aCtx.IsNull()){
		ASSERT(FALSE);
		return false;
	}
	_pGrid = new kvGrid(aCtx,this);
	if(!_pGrid)
		return false;

	_pGrid->SetWidth(200.0);
	_pGrid->SetHeight(200.0);
	_pGrid->SetCellSize(5.0);
	_pGrid->SetVisible(false);//默认是不可见的

	return true;
}

void  kcBasePlane::Destroy()
{
	if(_pGrid){
		delete _pGrid;
		_pGrid = NULL;
	}
}

int  kcBasePlane::GetCoordSystem(kvCoordSystem& cs)
{
	cs = _cs;
	return 1;
}

const kVector3& kcBasePlane::GetZDirection() const
{
	return _cs.Z();
}

//判断一点是否再基准面上
bool kcBasePlane::IsOnPlane(const gp_Pnt &p)
{
	kPoint3 kp(p.X(), p.Y(), p.Z());
	_cs.WCSToUCS(kp);
	if (fabs(kp.z()) < K_DBL_EPS)
		return true;
	return false;
}

gp_Pln kcBasePlane::GetPlane()
{
	gp_Pln aPln;
	gp_Ax2 ax2;
	_cs.Get(ax2);
	aPln.SetPosition(gp_Ax3(ax2));

	return aPln;
}

// 屏幕窗口坐标转到空间点和线.
// x,y:窗口坐标,vp:空间点 vdir:空间视向.
//
bool kcBasePlane::PixelToModel(int x,int y,kPoint3& vp,kVector3& vdir)
{
	ASSERT(!_hView.IsNull());
	double vx,vy,vz,dx,dy,dz;
	_hView->ConvertWithProj(x,y,vx,vy,vz,dx,dy,dz);
	vp.set(vx,vy,vz);
	vdir.set(dx,dy,dz);
	vdir.normalize();

	return true;
}

// 空间点到屏幕坐标的转换
// wp:空间点,wcs坐标系下.x,y:返回的屏幕坐标
//
bool kcBasePlane::ModelToPixel(const kPoint3& wp,int& x,int& y)
{
	ASSERT(!_hView.IsNull());
	_hView->Convert(wp.x(),wp.y(),wp.z(),x,y);
	return true;
}

// 屏幕窗口坐标到工作平面点.
// x,y:窗口坐标,wp:工作平面上的点,wcs坐标系下.
//
bool kcBasePlane::PixelToWorkPlane(int x,int y,kPoint3& wp)
{
	ASSERT(!_hView.IsNull());
	kPoint3 vp;
	kVector3 vdir,vNorm;
	PixelToModel(x,y,vp,vdir);

	//和工作平面求交
	const kPoint3& org = _cs.Org();
	vNorm = _cs.Z();
	
	return LinPlnIntersection(vp,vdir,org,vNorm,wp);
}

void  kcBasePlane::SetActive(bool bActive)//是否设置为活动的基准面
{
	_bActive = bActive;
	if(_pGrid){
		_pGrid->SetVisible(bActive ? true : false);
	}

	if(_bActive){
		SetDefaultView();
	}
}

//设定默认实体，目前是上视图。
void  kcBasePlane::SetDefaultView()
{
	//视向设定到该局部坐标系的xy平面上
	if(_bActive){
		Standard_Boolean bImmUpdate = _hView->SetImmediateUpdate(Standard_False);
		//
		kPoint3 org = _cs.Org();
		kVector3 Y = _cs.Y(),Z = _cs.Z();

		//以下顺序是选定好的，不能改变，否则会出现ZX平面以Z为up向的情况
		_hView->SetTwist(0.0);
		_hView->SetAt(org.x(),org.y(),org.z());
		_hView->SetProj(Z.x(),Z.y(),Z.z());
		_hView->SetUp(Y.x(),Y.y(),Y.z());

		_hView->FitAll();
		_hView->ZFitAll();

		_hView->SetImmediateUpdate(bImmUpdate);
		_hView->Update();
	}
}

void  kcBasePlane::SetName(const char *szName)
{
	ASSERT(szName);
	_szName = szName;
}

std::string  kcBasePlane::GetName() const
{
	return _szName;
}

bool  kcBasePlane::HasName() const
{
	return (_szName.empty() ? false : true);
}

// 读取
BOOL  kcBasePlane::Read(CArchive& ar,kcModel *pModel)
{
	QxStgBlock blk,csblk;
	int nval = 0;

	if(!blk.Read(ar) || blk.GetType() != 0x11BB){
		ASSERT(FALSE);
		return FALSE;
	}

	if(!blk.GetValueByCode(1,_nHandle) || 
		!blk.GetValueByCode(2,_szName) ||
		!blk.GetValueByCode(3,nval))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if(!csblk.Read(ar) || !_cs.ParseStgBlock(csblk)){
		ASSERT(FALSE);
		return FALSE;
	}

	_pModel = pModel;
	_bActive = (nval > 0) ? true : false;
	if(!InitGrid())
		return FALSE;

	//标识handle已经使用了
	kcHandleMgr *pHandleMgr = pModel->GetHandleMgr();
	pHandleMgr->MaskHandle(KCT_BASE_PLANE,_nHandle);

	return TRUE;
}

// 写入
BOOL  kcBasePlane::Write(CArchive& ar)
{
	QxStgBlock blk,csblk;
	int nval = _bActive ? 1 : 0;

	blk.Initialize(0x11BB);
	blk.AddCodeValue(1,_nHandle);
	blk.AddCodeValue(2,_szName.c_str());
	blk.AddCodeValue(3,nval);

	blk.Write(ar);

	_cs.BuildStgBlock(csblk);
	csblk.Write(ar);
	
	return TRUE;
}

// 计算直线和平面的交点
bool kcBasePlane::LinPlnIntersection(const kPoint3& linpnt,const kVector3& lindir,
					 				    const kPoint3& plnpnt,const kVector3& plnnorm,kPoint3& intpt)
{
	gp_Lin  lin(gp_Pnt(linpnt.x(),linpnt.y(),linpnt.z()),gp_Dir(lindir.x(),lindir.y(),lindir.z()));
	gp_Pln	pln(gp_Pnt(plnpnt.x(),plnpnt.y(),plnpnt.z()),
				gp_Dir(plnnorm.x(),plnnorm.y(),plnnorm.z()));

	kVector3 ld = lindir,pn = plnnorm;
	ld.normalize();
	pn.normalize();
	double dot = ld * pn;
	if(fabs(dot) < 1e-8)//直线和平面平行了
		return false;

	IntAna_IntConicQuad icq;
	icq.Perform(lin,pln,0.0001);
	if(!icq.IsDone() || icq.NbPoints() < 1)
		return false;

	gp_Pnt pnt = icq.Point(1);
	intpt.set(pnt.X(),pnt.Y(),pnt.Z());

	return true;
}