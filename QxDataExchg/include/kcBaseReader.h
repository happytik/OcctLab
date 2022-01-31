//////////////////////////////////////////////////////////////////////////
// 导入的基类
#ifndef _KC_BASE_READER_H_
#define _KC_BASE_READER_H_

#include <TCollection_AsciiString.hxx>
#include <TopTools_ListOfShape.hxx>
#include "QxLibDef.h"

class kcLayer;
class kcModel;
class Quantity_Color;

class QXLIB_API kcBaseReader
{
public:
	kcBaseReader(kcModel *pModel);
	~kcBaseReader(void);

	//导入
	virtual	BOOL			DoImport() = 0;

	const TopTools_ListOfShape& GetShapeList() const { return m_aShapeList; }

protected:
	// 将导入对象加入model
	virtual BOOL			AddShapeToModel();

	// 将导入对象添加到model中。
	bool					AddEntity(const TopoDS_Shape& aShape,kcLayer *pLayer);
	bool					AddEntity(const TopoDS_Shape& aShape,Quantity_Color &aCol);

protected:
	kcModel					*m_pModel;


	//导入的对象
	TopTools_ListOfShape	m_aShapeList;
};

//将路径名称重新编码为utf-8，OCCT680需要这样
int   EncodeUtf8FilePath(const char *pFileName,char *pwcFileName,int nSize);

int   EncodeFilePath(const char *pFileName,TCollection_AsciiString& ascStr);

#endif