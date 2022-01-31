//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCG_SURFLIB_H_
#define _KCG_SURFLIB_H_

#include <string>
#include <map>
#include "QxLibDef.h"
#include "kPoint.h"
#include "kVector.h"

class QXLIB_API kcgSurfNameAndIndex{
public:
	static  int				GeomSurfIndex(const Handle(Geom_Surface) aSurf);
	static  const char*		GeomSurfName(const Handle(Geom_Surface) aSurf);
	static  const char*		GeomSurfName(int idx);
};

class QXLIB_API QxSurfLib
{
public:
	QxSurfLib(void);
	~QxSurfLib(void);

	//投影点到平面
	static bool				ProjectPointToPlane(const kPoint3& p,
									const kPoint3& pivot,const kVector3& dir,kPoint3& prjpnt);

	static bool				IsPointOnPlane(const kPoint3& p,
										   const kPoint3& pivot,const kVector3& dir);

	//计算face的一个点和normal
	static bool				CalcFaceNormal(const TopoDS_Face& aFace,double u,double v,bool bTopo,kPoint3& p,kVector3& normal);

	static bool				CalcFaceNormal(const TopoDS_Face& aFace,bool bTopo,kPoint3& p,kVector3& normal);
};

//平面多个环构造face
class QXLIB_API kcgBuildFaceFromPlnWire{
public:
	kcgBuildFaceFromPlnWire();

	BOOL							Init();
	BOOL							Add(const TopoDS_Edge& aEdge);
	BOOL							Add(const TopoDS_Wire& aWire);
	BOOL							Build();

	int								NbFaces() const;
	TopoDS_Face						GetFace(int ix);

protected:
	TopTools_ListOfShape			m_aShapeList;
	TopTools_SequenceOfShape		m_aFaceList;//记录结果
};


#endif