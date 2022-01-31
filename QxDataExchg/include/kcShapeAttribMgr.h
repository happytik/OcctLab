////////////////////////////////////////////////
//
#pragma once

#include <NCollection_IndexedDataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopTools_ListOfShape.hxx>
#include "kcShapeAttrib.h"

class STEPControl_Reader;
typedef NCollection_IndexedDataMap<TopoDS_Shape,Handle(kcShapeAttrib),TopTools_ShapeMapHasher> kcShapeAttribIdxMap;

// 管理转换后的shape的属性信息.
class kcShapeAttribMgr
{
public:
	kcShapeAttribMgr(void);
	~kcShapeAttribMgr(void);

	bool						AddColorAttrib(const TopoDS_Shape &aS,double r,double g,double b);
	bool						AddColorAttrib(const TopoDS_Shape &aS,const Quantity_Color &aCol);

	// 查找shape的颜色属性
	bool						FindColorAttrib(const TopoDS_Shape &aS,Quantity_Color &aColor);
	// 查找face的颜色
	bool						FindFaceColor(const TopoDS_Shape &aS,Quantity_Color &aColor);

	kcShapeAttribIdxMap&		ShapeAttribIdxMap() { return aShapeAttribMap_; }
	void						Clear();

	// 是否存在颜色属性信息
	bool						HasColorAttribs() const;
	bool						HasFaceColor() const;

	// 获取具有颜色的shape的列表
	bool						GetShapeList(TopTools_ListOfShape &aShapeList);

public:
	//读取每个有颜色的face的颜色属性信息
	void						ReadFaceColors(STEPControl_Reader &aReader);

	//读取每个有颜色的shape的颜色属性信息
	void						ReadShapeColors(STEPControl_Reader &aReader);

protected:
	kcShapeAttribIdxMap		aShapeAttribMap_;//shape及其属性信息
	kcShapeAttribIdxMap		aFaceSpecColorMap_;//记录特有的颜色的face的信息，例如：一个solid是一种颜色，其中一个face是单独的颜色
	kcShapeAttribIdxMap		aFaceOfShapeColorMap_;//当solid、shell等有颜色定义时，这里记录shape中每个face对象，及其颜色信息
};

