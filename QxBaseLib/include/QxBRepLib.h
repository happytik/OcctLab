//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCG_BREPLIB_H_
#define _KCG_BREPLIB_H_

#include <vector>
#include <Geom_Curve.hxx>
#include "QxLibDef.h"
#include "kVector.h"

typedef std::vector<Handle(Geom_Curve)> GeomCurveArray;

class QXLIB_API QxBRepLib
{
public:
	static double								EdgeLength(const TopoDS_Edge& aEdge);//计算edge的长度
	static double								EdgeLengthEx(const TopoDS_Edge& aEdge);//计算edge的长度
	static double								WireLength(const TopoDS_Wire& aWire);//计算edge的长度

	//两点生成线段
	static TopoDS_Edge						MakeSegEdge(const gp_Pnt &p1,const gp_Pnt &p2);
	//从曲线生成Edge
	static TopoDS_Edge						MakeEdge(const Handle(Geom_Curve) &aCurve);

	//由曲面生成Face
	static TopoDS_Face						MakeFace(const Handle(Geom_Surface) &aSurf);

	// shape的拷贝。和emptycopy不同的深度拷贝。
	static  TopoDS_Edge							CopyEdge(const TopoDS_Edge& aEdge);
	static  TopoDS_Wire							CopyWire(const TopoDS_Wire& aWire);
	static  TopoDS_Face							CopyFace(const TopoDS_Face& aFace);
	static  TopoDS_Shape						CopyShape(const TopoDS_Shape& aShape);

	// 移动对象
	static  TopoDS_Shape						MoveShape(const TopoDS_Shape& aShape,const kVector3& vec,BOOL bCopy = TRUE);

	// 从edge生成wire
	static  TopoDS_Wire							EdgeToWire(const TopoDS_Edge& aEdge);

	// 旋转
	// p:基点,dir:单位向量，ang：旋转角度，弧度。
	static  TopoDS_Shape						RotateShape(const TopoDS_Shape& aShape,const kPoint3& p,const kVector3& dir,double ang);
	// 缩放
	static  TopoDS_Shape						ScaleShape(const TopoDS_Shape& aShape,const kPoint3& p,double scale,BOOL bCopy = TRUE);

	//edge和wire是否相连
	static   BOOL								IsConnected(const TopoDS_Wire& aW,const TopoDS_Edge& aE);

	//获取face的外环
	//使用BRepTools::OuterWire的代码，略作修改。
	static  TopoDS_Wire							OuterWire(const TopoDS_Face& aFace);

	//根据一系列wire,创建平面face.wire不相交,可能相互包含.主要用于从文字轮廓
	//生成一系列平面.
	static  int									BuildPlnFace(TopTools_ListOfShape& aWList,TopTools_ListOfShape& aFList);

	//wire 1是否包含wire2.注意:wire1和wire2不相交.当然可以判断是否相交.但这里为了效率没有处理.
	static  BOOL								IsWire2InWire1(const TopoDS_Wire& aW1,const TopoDS_Wire& aW2);

	//从一个外环和一组内环创建face
	static TopoDS_Face							BuildPlaneFace(const TopoDS_Wire& aOW,const TopTools_ListOfShape& aIWList);

	//从一系列相连的edge和wire创建一个wire.

public:
	// 从一系列曲线，生成一个wire
	static TopoDS_Wire			BuildOneWire(const GeomCurveArray &aCrvAry);

	// 从一系列曲线，生成一系列wire
	static TopoDS_Shape			BuildWires(const GeomCurveArray &aCrvAry);

	// 从一系列曲线，生成一个edge的组合对象
	static TopoDS_Compound		BuildEdgeCompound(const GeomCurveArray &aCrvAry);

	// 修复shape对象，返回修复后的对象
	static TopoDS_Shape			FixShape(const TopoDS_Shape &aS);
};

// edge的分割
class QXLIB_API kcgEdgeSplit{
public:
	//将一条edge分割为两条
	static   BOOL			SplitEdge(const TopoDS_Edge& aEdge,double t,
									  TopoDS_Edge& aE1,TopoDS_Edge& aE2);

	//用edgeTool分割edge,生成的新edge保存在seq中，返回新edge的个数
	static   int			SplitEdgeByEdge(const TopoDS_Edge& aEdge,const TopoDS_Edge& aEdge2,
									  TopTools_SequenceOfShape& seqOfShape);

	// 用wire分割edge
	static	int				SplitEdgeByWire(const TopoDS_Edge& aEdge,
											const TopoDS_Wire& aWire,
											TopTools_SequenceOfShape& seqOfShape);
};

class QXLIB_API kcgWireSplit{
public:
	// 将一条wire分割为两条wire或edge
	static	BOOL			SplitWire(const TopoDS_Wire& aWire,gp_Pnt pnt,double distol,
									  TopTools_SequenceOfShape& seqOfShape);

	// 使用一条edge分割wire
	static  int				SplitWireByEdge(const TopoDS_Wire& aWire,
											const TopoDS_Edge& aEdge,
											TopTools_SequenceOfShape& seqOfShape);

	// 使用一条wire分割令一条wire
	static	int				SplitWireByWire(const TopoDS_Wire& aWire,
											const TopoDS_Wire& aWire2,
											TopTools_SequenceOfShape& seqOfShape);
};

#endif