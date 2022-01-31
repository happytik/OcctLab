#include "StdAfx.h"
#include "kVector.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "dl_dxf.h"
#include "dl_creationadapter.h"
#include "kcDXFReader.h"

///////////////////////////////////////////////////////////////////////////////
//
class QXLIB_API kcDXFReadAdapter : public DL_CreationAdapter
{
public:
	kcDXFReadAdapter(kcModel *pModel);
	virtual ~kcDXFReadAdapter(void);

	virtual void		addLayer(const DL_LayerData&) {}
	virtual void		addBlock(const DL_BlockData&) {}
	virtual void		endBlock() {}
	virtual void		addPoint(const DL_PointData& data);
	virtual void		addLine(const DL_LineData& data);
	virtual void		addArc(const DL_ArcData& data);
	virtual void		addCircle(const DL_CircleData& data);
	virtual void		addEllipse(const DL_EllipseData& data);

	virtual void		addPolyline(const DL_PolylineData& data);
	virtual void		addVertex(const DL_VertexData&) {}

	virtual void		addSpline(const DL_SplineData&) {}
	virtual void		addControlPoint(const DL_ControlPointData&) {}
	virtual void		addKnot(const DL_KnotData&) {}

protected:
	kcModel					*m_pModel;
};

kcDXFReadAdapter::kcDXFReadAdapter(kcModel *pModel)
:m_pModel(pModel)
{
	ASSERT(pModel);
}

kcDXFReadAdapter::~kcDXFReadAdapter(void)
{
}

void	kcDXFReadAdapter::addPoint(const DL_PointData& data)
{
	kcPointEntity *pEnt = new kcPointEntity;
	if(pEnt)
	{
		pEnt->SetPoint(data.x,data.y,data.z);
		m_pModel->AddEntity(pEnt);
	}
}

void	kcDXFReadAdapter::addLine(const DL_LineData& data)
{
	//创建直线，显示
	gp_Pnt pnt1(data.x1,data.y1,data.z1);
	gp_Pnt pnt2(data.x2,data.y2,data.z2);
	gp_Vec vd(pnt1,pnt2);
	double len = vd.Magnitude();
	if(len < KC_MIN_LEN)
		return;

	BRepBuilderAPI_MakeEdge me(pnt1,pnt2);
	if(me.IsDone())
	{
		TopoDS_Edge aEdge = me.Edge();

		kcEdgeEntity *pEntity = new kcEdgeEntity;
		if(pEntity)
		{
			pEntity->SetEdge(aEdge);
			m_pModel->AddEntity(pEntity);
		}
	}
}

static bool RandAxis(const kVector3& N,kVector3& X,kVector3& Y)
{
	kVector3 wy(0,1,0),wz(0,0,1);
	if(fabs(N[0]) < 1.0/64 && fabs(N[1]) < 1.0/64)
		X = wy ^ N;
	else
		X = wz ^ N;

	X.normalize();
	Y = N ^ X;
	Y.normalize();

	return true;
}

void	kcDXFReadAdapter::addArc(const DL_ArcData& data)
{
	kPoint3 cen(data.cx,data.cy,data.cz);
	double radius = data.radius;
	double *exdir = getExtrusion()->getDirection();
	kVector3 N(exdir),X,Y;
	RandAxis(N,X,Y);

	gp_Ax2 axis(gp_Pnt(cen[0],cen[1],cen[2]),
		gp_Dir(gp_Vec(N.x(),N.y(),N.z())),
		gp_Dir(gp_Vec(X.x(),X.y(),X.z())));
	Handle(Geom_Circle) aCircle = new Geom_Circle(axis,radius);
	Handle(Geom_TrimmedCurve) aArc = new Geom_TrimmedCurve(aCircle,data.angle1,data.angle2);
	kcEdgeEntity *pEnt = new kcEdgeEntity;
	if(!aArc.IsNull() && pEnt)
	{
		pEnt->SetCurve(aArc);
		m_pModel->AddEntity(pEnt);
	}
}

void	kcDXFReadAdapter::addCircle(const DL_CircleData& data)
{
	kPoint3 cen(data.cx,data.cy,data.cz);
	double radius = data.radius;
	double *exdir = getExtrusion()->getDirection();
	kVector3 N(exdir),X,Y;
	RandAxis(N,X,Y);

	gp_Ax2 axis(gp_Pnt(cen[0],cen[1],cen[2]),
		gp_Dir(gp_Vec(N.x(),N.y(),N.z())),
		gp_Dir(gp_Vec(X.x(),X.y(),X.z())));
	Handle(Geom_Circle) aCircle = new Geom_Circle(axis,radius);
	kcEdgeEntity *pEnt = new kcEdgeEntity;
	if(!aCircle.IsNull() && pEnt)
	{
		pEnt->SetCurve(aCircle);
		m_pModel->AddEntity(pEnt);
	}
}

void	kcDXFReadAdapter::addEllipse(const DL_EllipseData& data)
{

}

void	kcDXFReadAdapter::addPolyline(const DL_PolylineData& data)
{

}

//////////////////////////////////////////////////////////////////////////
//
kcDXFReader::kcDXFReader(kcModel *pModel)
:m_pModel(pModel)
{
	ASSERT(pModel);
}

kcDXFReader::~kcDXFReader()
{

}

//导入文件
bool	kcDXFReader::DoImport()
{
	if(!m_pModel)
		return false;

	CFileDialog dlg(TRUE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"DXF Files (*.dxf)|*.dxf||", 
		NULL );

	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor wc;

		std::string szFile = dlg.GetPathName();
		DL_Dxf *dxf = new DL_Dxf;
		kcDXFReadAdapter *pAdpter = new kcDXFReadAdapter(m_pModel);
		dxf->in(szFile,pAdpter);
		delete dxf;
		delete pAdpter;
	}

	return true;
}
