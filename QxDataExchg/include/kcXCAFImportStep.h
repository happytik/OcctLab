//////////////////////////////////////////////////////////////////
// 采用XCAF方式，导入STEP文件

#pragma once

#include <TDocStd_Document.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include "kcStepMbStringMgr.h"
#include "kcShapeAttribMgr.h"
#include "kcStepLayerMgr.h"

class TCollection_AsciiString;

#define XSTEPIMP_UNKNOWN			0
#define XSTEPIMP_SUCCESS			1	// 导入成功
#define XSTEPIMP_PART_SUCCESS		2   // 导入部分成功
#define XSTEPIMP_FAILED			-1	// 导入失败
#define XSTEPIMP_NOTHING			-2	// 没有导入任何东西
#define XSTEPIMP_READ_FAILED		-3	// 读取STEP文件失败
#define XSTEPIMP_TRANSFER_FAILED	-4	// 读取STEP文件失败
#define XSTEPIMP_NO_SOLID			-5  // 文件中没有实体对象
#define XSTEPIMP_CANCEL			-6  // 导入过程被中止了

//记录一个导入的零件


class kcXCAFImportStep
{
public:
	kcXCAFImportStep(void);
	~kcXCAFImportStep(void);

	void							EnableConvAsm(bool bEnable);

	// 读取STEP文件
	virtual int					ReadStep(const char *pszStepFile);

	// 进行转换
	virtual int					DoConvert();

protected:
	// 简单的转换方式，只考虑shape
	virtual int					DoConvertShape();

	// 复杂的转换方式，考虑整个装配体结构
	virtual int					DoConvertAsm();
	//
	virtual int					ConvertAssembly(const TDF_Label &aLabel);
	virtual int					ConvertPart(const TDF_Label &aLabel);

	//需要派生实现的类
	virtual int					DoConvOccShape(const TopoDS_Shape &aShape) = 0;

protected:
	int							EncodeFilePath(const char *pFileName,TCollection_AsciiString& ascStr);

protected:
	int							bConvAsm_;//是否转换装配体结构

protected:
	Handle(TDocStd_Document)		aDoc_;//文档对象
	Handle(XCAFDoc_ShapeTool)		aShapeTool_;
	Handle(XCAFDoc_ColorTool)		aColorTool_;
	Handle(XCAFDoc_LayerTool)		aLayerTool_;

protected:
	kcStepMbStringMgr				aStepMbStrMgr_;//记录多字节字符串，进行解码
	kcShapeAttribMgr				aShapeAttrMgr_;//记录颜色信息
	kcStepLayerMgr				aStepLayerMgr_;//记录图层信息

protected:
	int							nResult_;//
};

