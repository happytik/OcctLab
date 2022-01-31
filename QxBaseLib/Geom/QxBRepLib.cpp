#include "StdAfx.h"
#include <vector>
#include <algorithm>
#include <TopExp.hxx>
#include <Precision.hxx>
#include <ShapeAlgo.hxx>
#include <BRepTools.hxx>
#include <Bnd_SeqOfBox.hxx>
#include <BRepBndLib.hxx>

#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <ShapeAlgo.hxx>
#include <ShapeAlgo_AlgoContainer.hxx>
#include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GC_MakeSegment.hxx>
#include <BOPAlgo_Tools.hxx>
#include <TopExp_Explorer.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeExtend_Status.hxx>

#include "QxPrecision.h"
#include "QxDbgTool.h"
#include "kcgIntersect.h"
#include "QxBRepLib.h"

double  QxBRepLib::EdgeLength(const TopoDS_Edge& aEdge)//计算edge的长度
{
	double len = 0.0,dF,dL;
	Handle(Geom_Curve) aCurve;

	aCurve = BRep_Tool::Curve(aEdge,dF,dL);
	if(!aCurve.IsNull()){
		GeomAdaptor_Curve AC(aCurve);
		len = GCPnts_AbscissaPoint::Length(AC,dF,dL);
	}

	return len;
}

//计算edge的长度
double  QxBRepLib::EdgeLengthEx(const TopoDS_Edge& aEdge)
{
	GProp_GProps LProps;
	BRepGProp::LinearProperties(aEdge,LProps);
	return LProps.Mass();
}

//计算Wire的长度
double  QxBRepLib::WireLength(const TopoDS_Wire& aWire)
{
	GProp_GProps LProps;
	BRepGProp::LinearProperties(aWire,LProps);
	return LProps.Mass();
}

//两点生成线段
TopoDS_Edge  QxBRepLib::MakeSegEdge(const gp_Pnt &p1,const gp_Pnt &p2)
{
	TopoDS_Edge aEdge;
	GC_MakeSegment mkSeg(p1,p2);
	if(mkSeg.IsDone()){
		Handle(Geom_Curve) aSeg = mkSeg.Value();
		aEdge = MakeEdge(aSeg);
	}

	return aEdge;
}

//从曲线生成Edge
TopoDS_Edge  QxBRepLib::MakeEdge(const Handle(Geom_Curve) &aCurve)
{
	TopoDS_Edge aEdge;
	if(!aCurve.IsNull()){
		try{
			BRepBuilderAPI_MakeEdge mkEdge(aCurve);
			if(mkEdge.IsDone()){
				aEdge = mkEdge.Edge();
			}
		}catch(Standard_Failure){
		}
	}
	return aEdge;
}

//由曲面生成Face
TopoDS_Face  QxBRepLib::MakeFace(const Handle(Geom_Surface) &aSurf)
{
	TopoDS_Face aFace;

	if(!aSurf.IsNull()){
		try{
			BRepBuilderAPI_MakeFace mf(aSurf,Precision::Confusion());
			if(mf.IsDone()){
				aFace = mf.Face();
			}
		}catch(Standard_Failure){
		}
	}

	return aFace;
}

// shape的拷贝。和emptycopy不同的深度拷贝。
TopoDS_Edge  QxBRepLib::CopyEdge(const TopoDS_Edge& aEdge)
{
	TopoDS_Edge aCopyEdge;
	if(aEdge.IsNull())
		return aCopyEdge;

	try
	{
		BRepBuilderAPI_Copy cpy(aEdge);
		if(cpy.IsDone())
		{
			TopoDS_Shape aS = cpy.Shape();
			if(!aS.IsNull() &&
				aS.ShapeType() == TopAbs_EDGE)
			{
				aCopyEdge = TopoDS::Edge(aS);
			}
		}
	}
	catch (Standard_Failure){
		KTRACE("\n copy edge failed.");
	}

	return aCopyEdge;
}

TopoDS_Wire		QxBRepLib::CopyWire(const TopoDS_Wire& aWire)
{
	TopoDS_Wire aCopyWire;
	if(aWire.IsNull())
		return aCopyWire;

	try{
		BRepBuilderAPI_Copy cpy(aWire);
		if(cpy.IsDone())
		{
			TopoDS_Shape aS = cpy.Shape();
			if(!aS.IsNull() &&
				aS.ShapeType() == TopAbs_WIRE)
			{
				aCopyWire = TopoDS::Wire(aS);
			}
		}
	}
	catch(Standard_Failure){
		KTRACE("\n copy wire failed.");
	}

	return aCopyWire;
}

TopoDS_Face		QxBRepLib::CopyFace(const TopoDS_Face& aFace)
{
	TopoDS_Face aCopyFace;
	if(aFace.IsNull())
		return aCopyFace;

	try
	{
		BRepBuilderAPI_Copy cpy(aFace);
		if(cpy.IsDone())
		{
			TopoDS_Shape aS = cpy.Shape();
			if(!aS.IsNull() &&
				aS.ShapeType() == TopAbs_FACE)
			{
				aCopyFace = TopoDS::Face(aS);
			}
		}
	}
	catch (Standard_Failure){
		KTRACE("\n copy face failed.");
	}

	return aCopyFace;
}

//拷贝一个shape，深度拷贝
//
TopoDS_Shape	QxBRepLib::CopyShape(const TopoDS_Shape& aShape)
{
	TopoDS_Shape aCopyShape;
	if(aShape.IsNull())
		return aCopyShape;

	try{
		BRepBuilderAPI_Copy cpy(aShape);
		if(cpy.IsDone()){
			aCopyShape = cpy.Shape();
		}
	}catch (Standard_Failure){
		KTRACE("\n copy shape failed.");
	}

	return aCopyShape;
}

// 移动对象
TopoDS_Shape	QxBRepLib::MoveShape(const TopoDS_Shape& aShape,const kVector3& vec,BOOL bCopy)
{
	TopoDS_Shape aMovShape;
	if(vec.length() < KDBL_MIN)
		aMovShape = aShape;
	else
	{
		gp_Trsf trsf;
		trsf.SetTranslation(gp_Vec(vec[0],vec[1],vec[2]));
		try
		{
			BRepBuilderAPI_Transform trans(aShape,trsf,bCopy ? true : false);
			if(trans.IsDone())
				aMovShape = trans.Shape();

		}catch (Standard_Failure){
		}
	}

	return aMovShape;
}

// 从edge生成wire
TopoDS_Wire		QxBRepLib::EdgeToWire(const TopoDS_Edge& aEdge)
{
	TopoDS_Wire aWire;
	if(aEdge.IsNull())
		return aWire;

	try{
		BRepBuilderAPI_MakeWire mw(aEdge);
		if(mw.IsDone())
			aWire = mw.Wire();
	}catch(Standard_Failure){}

	return aWire;
}

// 旋转
// p:基点,dir:单位向量，ang：旋转角度，弧度。
TopoDS_Shape	QxBRepLib::RotateShape(const TopoDS_Shape& aShape,const kPoint3& p,const kVector3& dir,double ang)
{
	TopoDS_Shape aRotShape;
	gp_Ax1 ax1;
	gp_Trsf trsf;
	ax1.SetLocation(gp_Pnt(p[0],p[1],p[2]));
	ax1.SetDirection(gp_Dir(dir[0],dir[1],dir[2]));
	trsf.SetRotation(ax1,ang);

	try{
		BRepBuilderAPI_Transform trans(aShape,trsf,Standard_True);
		if(trans.IsDone())
			aRotShape = trans.Shape();

	}catch(Standard_Failure){

	}
	
	return aRotShape;
}

// 缩放
TopoDS_Shape	QxBRepLib::ScaleShape(const TopoDS_Shape& aShape,const kPoint3& p,double scale,BOOL bCopy)
{
	TopoDS_Shape aScaleShape;
	gp_Trsf trsf;
	trsf.SetScale(gp_Pnt(p[0],p[1],p[2]),scale);

	try{
		BRepBuilderAPI_Transform trans(aShape,trsf,bCopy ? true : false);
		if(trans.IsDone())
			aScaleShape = trans.Shape();

	}catch(Standard_Failure){
	}

	return aScaleShape;
}

//edge和wire是否相连
BOOL			QxBRepLib::IsConnected(const TopoDS_Wire& aW,const TopoDS_Edge& aE)
{
	if(aW.IsNull() || aE.IsNull())
		return FALSE;

	TopoDS_Vertex  v1,v2,v3,v4;
	TopExp::Vertices(aE,v1,v2);
	TopExp::Vertices(aW,v3,v4);
	gp_Pnt p1,p2,p3,p4;
	p1 = BRep_Tool::Pnt(v1);
	p2 = BRep_Tool::Pnt(v2);
	p3 = BRep_Tool::Pnt(v3);
	p4 = BRep_Tool::Pnt(v4);
	double dTol = Precision::Confusion();
	if(p1.Distance(p3) < dTol || p1.Distance(p4) < dTol ||
		p2.Distance(p3) < dTol || p2.Distance(p4) < dTol)
		return TRUE;

	return FALSE;
}

//使用BRepTools::OuterWire的代码，略作修改。
TopoDS_Wire		QxBRepLib::OuterWire(const TopoDS_Face& aFace)
{
	TopoDS_Wire Wres;
	TopExp_Explorer expw (aFace,TopAbs_WIRE);
	BOOL bOverlap = FALSE;//是否包围盒相互覆盖

	if (expw.More()) 
	{
		Wres = TopoDS::Wire(expw.Current());
		expw.Next();
		if (expw.More()) 
		{
			Standard_Real UMin, UMax, VMin, VMax;
			Standard_Real umin, umax, vmin, vmax;
			BRepTools::UVBounds(aFace,Wres,UMin,UMax,VMin,VMax);
			while (expw.More()) 
			{
				const TopoDS_Wire& W = TopoDS::Wire(expw.Current());
				BRepTools::UVBounds(aFace,W,umin, umax, vmin, vmax);
				//如下简单判断范围可能不行，例如：一个大的外wire和一个小的wire，当小环很
				//贴近大环时，由于包围盒算法，可能导致两种包围盒重叠，导致错误的选择。
				//下面算法适用于好的情况。
				if ((umin <= UMin) &&
					(umax >= UMax) &&
					(vmin <= VMin) &&
					(vmax >= VMax)) 
				{//w包围盒包围了记录的包围盒。
					Wres = W;
					UMin = umin;
					UMax = umax;
					VMin = vmin;
					VMax = vmax;
				}
				else
				{
					//判断是否重叠
					if(umin > UMax || umax < UMin || vmin > VMax || vmax < VMin)
					{//不可能重叠
					}
					else
					{
						//如果一个区间在另一个区间内，则d1,d2应当异号。
						double Ud1 = UMax - umax,Ud2 = UMin - umin;
						double Vd1 = VMax - vmax,Vd2 = VMin - vmin;
						if((Ud1 * Ud2 >= 0.0) && (Vd1 * Vd2 > 0.0) ||
							(Ud1 * Ud2 > 0.0) && (Vd1 * Vd2 >= 0.0))
						{
							KTRACE("\n wire overlap [%f,%f]*[%f,%f] ps [%f,%f]*[%f,%f]",
								umin,umax,vmin,vmax,UMin,UMax,VMin,VMax);
							bOverlap = TRUE;
							break;
						}
					}
				}
				expw.Next();
			}
		}
	}
	if(!bOverlap)
	{
		return Wres;
	}

	//重叠使用另一个算法
	TopoDS_Wire aOWire;
	try
	{
		aOWire = ShapeAlgo::AlgoContainer()->OuterWire(aFace);//Note:aFace 出现异常???
	}
	catch(Standard_Failure)
	{
		aOWire = Wres;//返回先前的。
	}

	return aOWire;
}


//根据一系列wire,创建平面face.wire不相交,可能相互包含.主要用于从文字轮廓
//生成一系列平面.
int		QxBRepLib::BuildPlnFace(TopTools_ListOfShape& aWList,TopTools_ListOfShape& aFList)
{
	int nFs = 0;

	//先根据包围盒,判断是否相互包含.
	Bnd_SeqOfBox seqBox;
	Bnd_Box bb;
	TopTools_SequenceOfShape seqW,seqAlone;
	TopTools_ListIteratorOfListOfShape its(aWList);
	for(;its.More();its.Next())
	{
		TopoDS_Wire aW = TopoDS::Wire(its.Value());
		BRepBndLib::Add(aW,bb);
		seqBox.Append(bb);
		seqW.Append(aW);
	}

	TopTools_DataMapOfShapeListOfShape mSS;//记录了包含关系,key的shape包含了list中的shape.
	for(int ix = 1;ix <= seqW.Length();ix ++)
	{
		TopoDS_Wire aW1 = TopoDS::Wire(seqW.Value(ix));
		const Bnd_Box& bb1 = seqBox.Value(ix);

		BOOL bInter = FALSE;
		for(int jx = 1;jx <= seqW.Length();jx ++)
		{
			if(ix == jx)
				continue;

			TopoDS_Wire aW2 = TopoDS::Wire(seqW.Value(jx));
			const Bnd_Box& bb2 = seqBox.Value(jx);

			if(bb1.IsOut(bb2) && bb2.IsOut(bb1))
			{
				continue;
			}
			else
			{
				//相交,判断两者的关系.先1后2,和先2后1.注意:包围盒相交甚至包含,但wire本身可能不相交
				if(QxBRepLib::IsWire2InWire1(aW1,aW2))
				{
					if(!mSS.IsBound(aW1))
					{
						TopTools_ListOfShape aList;
						mSS.Bind(aW1,aList);
					}
					BOOL bF = FALSE;
					TopTools_ListIteratorOfListOfShape lit;
					for(lit.Initialize(mSS.Find(aW1));lit.More();lit.Next())
					{
						if(aW2.IsEqual(lit.Value()))
						{
							bF = TRUE;
							break;
						}
					}
					if(!bF)
						mSS.ChangeFind(aW1).Append(aW2);
					bInter = TRUE;
				}
				else if(QxBRepLib::IsWire2InWire1(aW2,aW1))
				{
					if(!mSS.IsBound(aW2))
					{
						TopTools_ListOfShape aList;
						mSS.Bind(aW2,aList);
					}
					BOOL bF = FALSE;
					TopTools_ListIteratorOfListOfShape lit;
					for(lit.Initialize(mSS.Find(aW2));lit.More();lit.Next())
					{
						if(aW1.IsEqual(lit.Value()))
						{
							bF = TRUE;
							break;
						}
					}
					if(!bF)
						mSS.ChangeFind(aW2).Append(aW1);
					bInter = TRUE;
				}
				else
				{
					continue;//包围盒相交,实际不相交
				}
			}
		}

		if(!bInter)//什么都不相交,独立的wire
		{
			seqAlone.Append(aW1);
		}
	}
	//对相互包含的wire,判断相应关系,创建平面
	TopTools_DataMapIteratorOfDataMapOfShapeListOfShape mapit;
	for(mapit.Initialize(mSS);mapit.More();mapit.Next())
	{
		TopoDS_Wire aOW = TopoDS::Wire(mapit.Key());
		const TopTools_ListOfShape& aList = mapit.Value();

		//创建face
		TopoDS_Face aF = QxBRepLib::BuildPlaneFace(aOW,aList);
		if(!aF.IsNull())
		{
			aFList.Append(aF);
			nFs ++;
		}
	}

	//对独立的wire创建平面.
	for(int ix = 1;ix <= seqAlone.Length();ix ++)
	{
		TopoDS_Wire aW = TopoDS::Wire(seqAlone.Value(ix));
		BRepBuilderAPI_MakeFace MF(aW,Standard_True);
		if(MF.IsDone())
		{
			TopoDS_Face aNF = MF.Face();
			if(!aNF.IsNull())
			{
				aFList.Append(aNF);
				nFs ++;
			}
		}
	}

	return nFs;
}

//wire 1是否包含wire2.
BOOL	QxBRepLib::IsWire2InWire1(const TopoDS_Wire& aW1,const TopoDS_Wire& aW2)
{
	BRepBuilderAPI_FindPlane fpln(aW1);
	if(!fpln.Found())
		return FALSE;

	gp_Pln aPln = fpln.Plane()->Pln();
	//构造face
	BRepBuilderAPI_MakeFace mf(aPln,aW1);
	if(!mf.IsDone())
		return FALSE;

	TopoDS_Face aNF = mf.Face();

	//判断w2的一个点是否在mf上.
	BOOL bIN = FALSE;
	int nC = 0;
	BRepTopAdaptor_FClass2d fcls(aNF,Precision::PConfusion());

	TopExp_Explorer ex;
	for(ex.Init(aW2,TopAbs_VERTEX);ex.More();ex.Next())
	{
		TopoDS_Vertex aV = TopoDS::Vertex(ex.Current());
		gp_Pnt pnt = BRep_Tool::Pnt(aV);

		GeomAPI_ProjectPointOnSurf pp(pnt,fpln.Plane());
		if(pp.IsDone() && pp.NbPoints() > 0)
		{
			gp_Pnt2d uv;
			double u,v;
			pp.LowerDistanceParameters(u,v);
			uv.SetCoord(u,v);
			//验证是否在aW1内部
			TopAbs_State sta = fcls.Perform(uv);
			if(sta == TopAbs_IN)
			{
				bIN = TRUE;
				break;
			}
		}
		else
		{
			nC ++;
			if(nC > 3)
				break;
		}
	}

	return bIN;
}

//从一个外环和一组内环创建face
TopoDS_Face QxBRepLib::BuildPlaneFace(const TopoDS_Wire& aOW,const TopTools_ListOfShape& aIWList)
{

	TopoDS_Face aNF;

	try{
		//先使用外环创建一个平面
		BRepBuilderAPI_MakeFace MF(aOW,Standard_True);
		if(MF.IsDone())
		{
			aNF = MF.Face();
			if(aNF.IsNull())
				return aNF;
		}

		//下面依次对内环进行判断,主要是要保证环闭合,并且环的方向正确
		BRep_Builder B;
		TopTools_ListIteratorOfListOfShape lit;
		for(lit.Initialize(aIWList);lit.More();lit.Next())
		{
			TopoDS_Wire aW = TopoDS::Wire(lit.Value());

			if(aW.IsNull() || !BRep_Tool::IsClosed(aW))
			{
				KTRACE("\n inner wire make null or not closed.");
				continue;
			}

			//判断内环的方向
			TopoDS_Shape aTS = aNF.EmptyCopied();
			TopoDS_Face aTF = TopoDS::Face(aTS);
			aTF.Orientation(TopAbs_FORWARD);
			B.Add(aTF,aW);
			BRepTopAdaptor_FClass2d fcls(aTF,Precision::PConfusion());
			TopAbs_State sta = fcls.PerformInfinitePoint();
			if(sta == TopAbs_OUT)
				aW.Reverse();

			MF.Add(aW);
		}

		//获取最终的face
		aNF = MF.Face();

	}catch(Standard_Failure){
		return aNF;
	}

	return aNF;
}

// 从一系列曲线，生成一个wire
TopoDS_Wire QxBRepLib::BuildOneWire(const GeomCurveArray &aCrvAry)
{
	TopoDS_Wire aWire;
	if(aCrvAry.empty())
		return aWire;

	TopoDS_Compound aComp = QxBRepLib::BuildEdgeCompound(aCrvAry);
	if(aComp.IsNull())
		return aWire;

	TopoDS_Shape aS,aNW;
	int rc = BOPAlgo_Tools::EdgesToWires(aComp,aS,false);
	if(rc == 0){
		TopTools_IndexedMapOfShape aWMap;
		TopExp::MapShapes(aS,TopAbs_WIRE,aWMap);	
		if(aWMap.Size() != 1)
			return aWire;

		//需要修复一下，否则可能有问题
		aNW = QxBRepLib::FixShape(aWMap(1));
		if(!aNW.IsNull()){
			aWire = TopoDS::Wire(aNW);
		}else{
			aWire = TopoDS::Wire(aWMap(1));
		}
		return aWire;
	}

	return aWire;
}

// 从一系列曲线，生成一系列wire
TopoDS_Shape QxBRepLib::BuildWires(const GeomCurveArray &aCrvAry)
{
	TopoDS_Shape aShape;
	if(aCrvAry.empty())
		return aShape;

	TopoDS_Compound aComp = QxBRepLib::BuildEdgeCompound(aCrvAry);
	if(aComp.IsNull())
		return aShape;

	TopoDS_Shape aS,aW,aNW;
	int rc = BOPAlgo_Tools::EdgesToWires(aComp,aS,false);
	if(rc == 0){
		TopoDS_Compound aWComp;
		BRep_Builder aBB;
		TopExp_Explorer aExp;

		aBB.MakeCompound(aWComp);
		for(aExp.Init(aS,TopAbs_WIRE);aExp.More();aExp.Next()){
			aW = aExp.Current();
			//修复一下
			aNW = QxBRepLib::FixShape(aW);
			if(!aNW.IsNull()){
				aBB.Add(aWComp,aNW);
			}else{
				aBB.Add(aWComp,aW);
			}
		}

		return aWComp;
	}
	return aShape;
}

// 从一系列曲线，生成一个edge的组合对象
TopoDS_Compound QxBRepLib::BuildEdgeCompound(const GeomCurveArray &aCrvAry)
{
	BRep_Builder aBB;
	TopoDS_Compound aComp;

	if(aCrvAry.empty())
		return aComp;

	GeomCurveArray::size_type ix,isize = aCrvAry.size();
	TopoDS_Edge aEdge;

	aBB.MakeCompound(aComp);
	for(ix = 0;ix < isize;ix ++){
		aEdge = QxBRepLib::MakeEdge(aCrvAry[ix]);
		if(!aEdge.IsNull()){
			aBB.Add(aComp,aEdge);
		}
	}
	
	return aComp;
}

// 修复shape对象，返回修复后的对象
TopoDS_Shape QxBRepLib::FixShape(const TopoDS_Shape &aS)
{
	TopoDS_Shape aFS;
	if(aS.IsNull()) return aFS;

	Handle(ShapeFix_Shape) sfs = new ShapeFix_Shape(aS);
	try{
		sfs->Perform();
	}catch(Standard_Failure){
		return aS;	
	}
	//
	aFS = aS;
	if(sfs->Status(ShapeExtend_DONE)){
		aFS = sfs->Shape();
	}
	return aFS;
}

//////////////////////////////////////////////////////////////////////////
//
//将一条edge分割为两条
BOOL	kcgEdgeSplit::SplitEdge(const TopoDS_Edge& aEdge,double t,
					 		    TopoDS_Edge& aE1,TopoDS_Edge& aE2)
{
	if(aEdge.IsNull())
		return FALSE;

	double df,dl;
	Handle(Geom_Curve) aCur = BRep_Tool::Curve(aEdge,df,dl);
	if(aCur.IsNull())
		return FALSE;

	if(df > dl)
	{
		double tmp = df;dl = df;df = tmp;
	}
	double dtol = 1e-6;
	if(t - dtol < df || t + dtol > dl)
		return FALSE;

	//创建两个新的edge
	try
	{
		TopAbs_Orientation ot = aEdge.Orientation();
		aE1 = BRepBuilderAPI_MakeEdge(aCur,df,t);
		aE2 = BRepBuilderAPI_MakeEdge(aCur,t,dl);
		if(aE1.IsNull() || aE2.IsNull())
			return FALSE;

		// 保持方向性
		aE1.Orientation(ot);
		aE2.Orientation(ot);
		// 保证edge的顺序和edge一致。方便后续的处理。
		if(ot == TopAbs_REVERSED)
		{
			TopoDS_Edge aTE = aE1;
			aE1 = aE2;
			aE2 = aTE;
		}
	}
	catch (Standard_Failure)
	{
		return FALSE;
	}

	return TRUE;
}

// 用一条edge分割另一条edge,新edge保存在list中，返回新edge的个数
//
int		kcgEdgeSplit::SplitEdgeByEdge(const TopoDS_Edge& aEdge,const TopoDS_Edge& aEdge2,
					 				  TopTools_SequenceOfShape& seqOfShape)
{
	int nC = 0;
	std::vector<double>  arpars;//交点参数列表

	//
	double df,dl;
	Handle(Geom_Curve) aCur = BRep_Tool::Curve(aEdge,df,dl);
	if(aCur.IsNull())
		return 0;

	if(df > dl)
	{
		double tmp = df;dl = df;df = tmp;
	}

	//求交
	//求交点
	kcgCurveCurveInter ccInter;
	if(!ccInter.Intersect(aEdge,aEdge2))
		return 0;

	nC = ccInter.NbInterPoints();
	if(nC == 0)
		return 0;

	arpars.push_back(df);
	kcxCCIntpt ipt;
	int ix,iC = 0;
	for(ix = 0;ix < nC;ix ++)
	{
		ipt = ccInter.InterPoint(ix);
		arpars.push_back(ipt._ipt1._t);
	}
	arpars.push_back(dl);

	//排序,从小到大
	std::sort(arpars.begin(),arpars.end());

	//为每对参数间生成一个edge
	TopAbs_Orientation ot = aEdge.Orientation();
	double t1,t2;
	TopoDS_Edge aNE;
	int isize = (int)arpars.size();
	nC = 0;
	for(ix = 0;ix < isize - 1;ix ++)
	{
		t1 = arpars.at(ix);
		t2 = arpars.at(ix + 1);
		if(fabs(t1 - t2) < K_PAR_TOL)
			continue;
		try
		{
			aNE = BRepBuilderAPI_MakeEdge(aCur,t1,t2);
			if(!aNE.IsNull())
			{
				//注意方向，保证新分割edge的顺序和edge的方向一致
				aNE.Orientation(ot);
				if(ot == TopAbs_FORWARD)
					seqOfShape.Append(aNE);
				else
					seqOfShape.Prepend(aNE);
				nC ++;
			}
		}catch (Standard_Failure){
			continue;
		}
	}

	return nC;
}

// 用wire分割edge
// 依次用wire的edge和edge求交.
//
int		kcgEdgeSplit::SplitEdgeByWire(const TopoDS_Edge& aEdge,
									  const TopoDS_Wire& aWire,
									  TopTools_SequenceOfShape& seqOfShape)
{
	int nC = 0;
	std::vector<double>  arpars;//交点参数列表

	//
	double df,dl;
	Handle(Geom_Curve) aCur = BRep_Tool::Curve(aEdge,df,dl);
	if(aCur.IsNull())
		return 0;

	if(df > dl)
	{
		double tmp = df;dl = df;df = tmp;
	}

	//求交
	//依次和wire的每个edge求交点
	kcgCurveCurveInter ccInter;
	TopExp_Explorer ex;
	kcxCCIntpt ipt;
	int ix,nbIpt;

	arpars.push_back(df);
	for(ex.Init(aWire,TopAbs_EDGE);ex.More();ex.Next())
	{
		TopoDS_Edge aE = TopoDS::Edge(ex.Current());
		if(!ccInter.Intersect(aEdge,aE))
			continue;

		nbIpt = ccInter.NbInterPoints();
		for(ix = 0;ix < nbIpt;ix ++)
		{
			ipt = ccInter.InterPoint(ix);
			arpars.push_back(ipt._ipt1._t);
		}
	}
	arpars.push_back(dl);

	//排序,从小到大
	std::sort(arpars.begin(),arpars.end());

	//为每对参数间生成一个edge
	TopAbs_Orientation ot = aEdge.Orientation();
	double t1,t2;
	TopoDS_Edge aNE;
	int isize = (int)arpars.size();
	nC = 0;
	for(ix = 0;ix < isize - 1;ix ++)
	{
		t1 = arpars.at(ix);
		t2 = arpars.at(ix + 1);
		if(fabs(t1 - t2) < K_PAR_TOL)
			continue;
		try
		{
			aNE = BRepBuilderAPI_MakeEdge(aCur,t1,t2);
			if(!aNE.IsNull())
			{
				//注意方向，保证新分割edge的顺序和edge的方向一致
				aNE.Orientation(ot);
				if(ot == TopAbs_FORWARD)
					seqOfShape.Append(aNE);
				else
					seqOfShape.Prepend(aNE);
				nC ++;
			}
		}catch (Standard_Failure){
			continue;
		}
	}

	return nC;
}

//////////////////////////////////////////////////////////////////////////
// 将一条wire分割为两条
BOOL	kcgWireSplit::SplitWire(const TopoDS_Wire& aWire,gp_Pnt pnt,double distol,
							    TopTools_SequenceOfShape& seqOfShape)
{
	kPoint3 p;
	kcgPointCurveInter pcInter;
	int nc = 0;
	TopTools_ListOfShape llf,llr;
	BRepTools_WireExplorer wex;
	TopoDS_Edge aE1,aE2,aTE;
	TopoDS_Shape aS;
	TopoDS_Vertex vf,vl;
	gp_Pnt vpf,vpl;
	BOOL bFound = FALSE;

	// 是否和首末点重合。
	TopExp::Vertices(aWire,vf,vl);
	vpf = BRep_Tool::Pnt(vf);
	vpl = BRep_Tool::Pnt(vl);
	if(vpf.Distance(pnt) < distol || vpl.Distance(pnt) < distol)
		return FALSE;
    
	p.set(pnt.X(),pnt.Y(),pnt.Z());
	for(wex.Init(aWire);wex.More();wex.Next())
	{
		TopoDS_Edge aE = TopoDS::Edge(wex.Current());
		if(bFound)
		{
			aTE = QxBRepLib::CopyEdge(aE);
			if(aTE.IsNull())
				return FALSE;
			llr.Append(aTE);
		}
		else
		{
			//先检查是否和节点重合
			TopExp::Vertices(aE,vf,vl);
			vpf = BRep_Tool::Pnt(vf);
			vpl = BRep_Tool::Pnt(vl);
			if(vpf.Distance(pnt) < distol)
			{
				bFound = TRUE;
				aTE = QxBRepLib::CopyEdge(aE);
				if(aTE.IsNull())
					return FALSE;
				llr.Append(aTE);
			}
			else if(vpl.Distance(pnt) < distol)
			{
				bFound = TRUE;
				aTE = QxBRepLib::CopyEdge(aE);
				if(aTE.IsNull())
					return FALSE;
				llf.Append(aTE);
			}
			else
			{
				if(pcInter.Intersect(p,aE))
				{
					kcxCurveIntpt ipt = pcInter.InterPoint(0);
					if(!kcgEdgeSplit::SplitEdge(aE,ipt._t,aE1,aE2))
						return FALSE;
					llf.Append(aE1);
					llr.Append(aE2);
					bFound = TRUE;
				}
				else
				{
					aTE = QxBRepLib::CopyEdge(aE);
					if(aTE.IsNull())
						return FALSE;
					llf.Append(aTE);
				}
			}
		}
	}

	//组环
	TopoDS_Wire aNW;
	try{
		BRepBuilderAPI_MakeWire mw;
		TopTools_ListIteratorOfListOfShape lite;
		for(lite.Initialize(llf);lite.More();lite.Next())
			mw.Add(TopoDS::Edge(lite.Value()));
		if(mw.IsDone())
			aNW = mw.Wire();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aNW.IsNull())
		return FALSE;

	seqOfShape.Append(aNW);

	try{
		BRepBuilderAPI_MakeWire mw;
		TopTools_ListIteratorOfListOfShape lite;
		for(lite.Initialize(llr);lite.More();lite.Next())
			mw.Add(TopoDS::Edge(lite.Value()));
		if(mw.IsDone())
			aNW = mw.Wire();
	}catch(Standard_Failure){
		return FALSE;
	}
	if(aNW.IsNull())
		return FALSE;

	seqOfShape.Append(aNW);

	return TRUE;
}

// 使用一条edge分割wire
int		kcgWireSplit::SplitWireByEdge(const TopoDS_Wire& aWire,
									  const TopoDS_Edge& aEdge,
									  TopTools_SequenceOfShape& seqOfShape)
{
	if(aWire.IsNull() || aEdge.IsNull())
		return 0;

	int  nC = 0;
	TopTools_SequenceOfShape seqSplit;
	TopTools_ListOfShape wlist;
	TopoDS_Wire aW;
	BRepTools_WireExplorer wexp;
	TopoDS_Edge aTE;

	// 依次剖分wire中的edge，并进行必要的连接。
	// 保证edge的顺序。
	for(wexp.Init(aWire);wexp.More();wexp.Next())
	{
		TopoDS_Edge aE = TopoDS::Edge(wexp.Current());
		//
		seqSplit.Clear();
		// 使用aEdge进行分割，可能会有多条edge生成。
		nC = kcgEdgeSplit::SplitEdgeByEdge(aE,aEdge,seqSplit);
		if(nC == 0)//没有分割，记录旧的
		{
			aTE = QxBRepLib::CopyEdge(aE);
			wlist.Append(aTE);
		}
		else
		{
			//分割成功，将前面记录的edge和分割成的第一个edge拼接成新的wire
			TopoDS_Wire aNW;
			try{
				TopTools_ListIteratorOfListOfShape  ll(wlist);
				BRepBuilderAPI_MakeWire  mw;
				for(;ll.More();ll.Next())
					mw.Add(TopoDS::Edge(ll.Value()));
				wlist.Clear();
				//添加分割的第一个
				mw.Add(TopoDS::Edge(seqSplit.First()));
				if(mw.IsDone())
					aNW = mw.Wire(); 
			}catch(Standard_Failure){
				return 0;
			}
			if(aNW.IsNull())
				return FALSE;
			seqOfShape.Append(aNW);

			//中间部分直接添加
			int ix;
			for(ix = 2;ix < nC;ix ++)//注意：下标充1开始。忽略首尾。
				seqOfShape.Append(seqSplit.Value(ix));

			wlist.Append(seqSplit.Last());
		}
	}
	//拼接最后的
	if(wlist.Extent() > 0)
	{
		TopoDS_Wire aNW;
		try{
			TopTools_ListIteratorOfListOfShape  ll(wlist);
			BRepBuilderAPI_MakeWire  mw;
			for(;ll.More();ll.Next())
				mw.Add(TopoDS::Edge(ll.Value()));
			if(mw.IsDone())
				aNW = mw.Wire(); 
		}catch(Standard_Failure){
			return 0;
		}
		if(aNW.IsNull())
			return FALSE;
		seqOfShape.Append(aNW);
	}

	return seqOfShape.Length();
}

// 使用一条wire分割令一条wire
int	kcgWireSplit::SplitWireByWire(const TopoDS_Wire& aWire,
								  const TopoDS_Wire& aWire2,
								  TopTools_SequenceOfShape& seqOfShape)
{
	if(aWire.IsNull() || aWire2.IsNull())
		return 0;

	int  nC = 0;
	TopTools_ListOfShape wlist;
	TopTools_SequenceOfShape seqSplit;
	BRepTools_WireExplorer wexp;
	TopoDS_Edge aTE;

	//依次剖分wire中的edge，并进行必要的连接。
	for(wexp.Init(aWire);wexp.More();wexp.Next())
	{
		TopoDS_Edge aE = TopoDS::Edge(wexp.Current());
		
		seqSplit.Clear();
		//wire分割edge
		nC = kcgEdgeSplit::SplitEdgeByWire(aE,aWire2,seqSplit);
		if(nC == 0)//没有分割，记录旧的
		{
			aTE = QxBRepLib::CopyEdge(aE);
			wlist.Append(aTE);
		}
		else
		{
			//分割成功，将前面记录的edge和分割成的第一个edge拼接成新的wire
			TopoDS_Wire aNW;
			try{
				TopTools_ListIteratorOfListOfShape  ll(wlist);
				BRepBuilderAPI_MakeWire  mw;
				for(;ll.More();ll.Next())
					mw.Add(TopoDS::Edge(ll.Value()));
				wlist.Clear();
				//添加分割的第一个
				mw.Add(TopoDS::Edge(seqSplit.First()));
				if(mw.IsDone())
					aNW = mw.Wire(); 
			}catch(Standard_Failure){
				return 0;
			}
			if(aNW.IsNull())
				return FALSE;
			seqOfShape.Append(aNW);

			//中间部分直接添加
			int ix;
			for(ix = 2;ix < nC;ix ++)//注意：下标充1开始。忽略首尾。
				seqOfShape.Append(seqSplit.Value(ix));

			wlist.Append(seqSplit.Last());
		}
	}

	//拼接最后的
	if(wlist.Extent() > 0)
	{
		TopoDS_Wire aNW;
		try{
			TopTools_ListIteratorOfListOfShape  ll(wlist);
			BRepBuilderAPI_MakeWire  mw;
			for(;ll.More();ll.Next())
				mw.Add(TopoDS::Edge(ll.Value()));
			if(mw.IsDone())
				aNW = mw.Wire(); 
		}catch(Standard_Failure){
			return 0;
		}
		if(aNW.IsNull())
			return FALSE;
		seqOfShape.Append(aNW);
	}

	return seqOfShape.Length();
}