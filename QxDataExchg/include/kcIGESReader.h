//////////////////////////////////////////////////////////////////////////
// 导入IGES
#ifndef _KC_IGES_READER_H_
#define _KC_IGES_READER_H_

#include "QxLibDef.h"

class kcModel;

class QXLIB_API kcIGESReader
{
public:
	kcIGESReader(kcModel *pModel);
	~kcIGESReader(void);

	//导入IGES文件
	bool					DoImport();

protected:
	// 将导入对象添加到model中。
	bool					AddEntity(const TopoDS_Shape& aShape);
	// 添加一个对象
	bool					AddOneShape(const TopoDS_Shape& aShape);
	// 添加具体的对象
	bool					AddVertex(const TopoDS_Shape& aShape);
	bool					AddEdge(const TopoDS_Shape& aShape);
	bool					AddWire(const TopoDS_Shape& aShape);
	bool					AddFace(const TopoDS_Shape& aShape);
	bool					AddShell(const TopoDS_Shape& aShape);
	bool					AddSolid(const TopoDS_Shape& aShape);
	bool					AddCompound(const TopoDS_Shape& aShape);

protected:
	kcModel					*m_pModel;
};

#endif