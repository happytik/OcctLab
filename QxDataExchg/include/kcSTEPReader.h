//////////////////////////////////////////////////////////////////////////
// 实现STEP格式的导入
#ifndef _KC_STEP_READER_H_
#define _KC_STEP_READER_H_

#include <vector>
#include <string>
#include <IFSelect_ReturnStatus.hxx>
#include <TDF_Label.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include "kcBaseReader.h"
#include "kcStepMbStringMgr.h"

typedef std::vector<std::string> stdStringVector;

class STEPCAFControl_Reader;
class kcModel;
class kcShapeAttribMgr;

class QXLIB_API kcSTEPReader : public kcBaseReader{
public:
	kcSTEPReader(kcModel *pModel);
	virtual ~kcSTEPReader();

	//导入
	virtual	BOOL			DoImport();

	//读取一个step文件
	bool					ReadStepFile(const char *pszFile);

	//
	kcShapeAttribMgr*		ShapeAttribMgr() const { return pShapeAttribMgr_; }

protected:
	IFSelect_ReturnStatus	ReadSTEP(const Standard_CString& aFileName);

protected:
	kcShapeAttribMgr		*pShapeAttribMgr_;
};

class QXLIB_API kcXCAFSTEPReader : public kcBaseReader{
public:
	kcXCAFSTEPReader(kcModel *pModel);
	virtual ~kcXCAFSTEPReader();

//导入
	virtual	BOOL			DoImport();

protected:
	IFSelect_ReturnStatus	ReadSTEP(const Standard_CString& aFileName);
	void					MapFace(const TopoDS_Shape &aS);
	void					ReadFaceColors(STEPCAFControl_Reader &aCAFReader);
	
	void					AddLabel(TDF_Label& aLabel,const TopLoc_Location& tloc,Handle(XCAFDoc_ColorTool)& aColTool);
	void					DecodeNameString(const TCollection_ExtendedString &aExtStr,std::string &szName);

private:
	int					m_nLevel;
	TopTools_MapOfShape	aFaceMap_;
	kcStepMbStringMgr		aStepMbStrMgr_;
};
#endif