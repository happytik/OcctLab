///entity的保存和读取代码
#include "StdAfx.h"
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include "kcModel.h"
#include "kcLayer.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"
#include "kcHandleMgr.h"
#include "QxBRepLib.h"
#include "QxStgBlock.h"
#include "QxStgModelDef.h"
#include "AIS_EntityShape.h"
#include "kcEntity.h"

///////////////////////////////////////////////////////
//
// 读取,需要读取信息块,类型块外部已经读取了

// 从ar中读出一个kcEntity对象
kcEntity* kcEntity::ReadEntity(CArchive &ar)
{
	QxStgBlock tblk;
	int entType = 0;
	
	//读取类型
	if(!tblk.Read(ar,KSTG_BLOCK_ENTITY_TYPE) ||
		!tblk.GetValueByCode(KSTG_CV_CODE_TYPE,entType))
	{
		ASSERT(FALSE);
		return NULL;
	}
	
	//创建并读取对象
	kcEntity *pEntity = kcEntityFactory::CreateByType(entType);
	if(pEntity && pEntity->Read(ar)){
		return pEntity;
	}
	KC_SAFE_DELETE(pEntity);

	return NULL;
}

//
BOOL kcEntity::Read(CArchive& ar)
{
	QxStgBlock blk;
	if(!blk.Read(ar,KSTG_BLOCK_ENTITY)){
		ASSERT(FALSE);
		return FALSE;
	}

	//解析出shape对象
	aShape_ = ParseShapeData(blk);
	if(aShape_.IsNull()){
		ASSERT(FALSE);
		return FALSE;
	}

	//读取通用数据
	std::string sName;
	bool bVisible = false;
	QxStgBlock *pAttrBlk = blk.GetSubBlock(100); //读取属性块
	if(!blk.GetValueByCode(KSTG_CV_CODE_HANDLE,_nHandle) ||
		!blk.GetValueByCode(KSTG_CV_CODE_VISIBLE,bVisible) ||
		!pAttrBlk)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	if(blk.GetValueByCode(KSTG_CV_CODE_NAME,sName)){//可能为空
		//记录名称
		strncpy_s(_szName,KC_ENT_NAME_MAX,sName.c_str(),sName.size());
	}

	//读派生类数据
	if(!DoGetOtherCodeValue(blk)){
		return FALSE;
	}

	//从属性块中读取	
	if(!ParseCommonAttrib(*pAttrBlk) || !DoParseAttrib(*pAttrBlk)){
		return FALSE;
	}

	//读派生类写入的块
	if(!DoReadOtherData(ar)){
		return FALSE;
	}
	
	SetModified(TRUE);
	SetValid(TRUE);

	return TRUE;
}

// 写入
BOOL	kcEntity::Write(CArchive& ar)
{
	//写入类型block,方便解析
	QxStgBlock tblk;
	tblk.Initialize(KSTG_BLOCK_ENTITY_TYPE);
	tblk.AddCodeValue(KSTG_CV_CODE_TYPE,_entType);
	tblk.Write(ar);

	//写入数据,属性单独写入一个块中
	QxStgBlock blk;
	QxStgBlock *pAttrBlk = new QxStgBlock(KSTG_BLOCK_ENTITY_ATTRIB);

	blk.Initialize(KSTG_BLOCK_ENTITY);
	//写入shape的数据
	AddShapeData(blk,aShape_);
	// 加入共同的值
	//句柄
	blk.AddCodeValue(KSTG_CV_CODE_HANDLE,_nHandle);
	//名称
	if(_szName[0] != '\0'){
		blk.AddCodeValue(KSTG_CV_CODE_NAME,_szName);
	}
	//是否可见
	bool bVisible = IsVisible();
	blk.AddCodeValue(KSTG_CV_CODE_VISIBLE,bVisible);
	// 添加派生类的数据
	DoAddOtherCodeValue(blk);
	
	//加入通用属性
	AddCommonAttrib(*pAttrBlk);
	//添加各自的属性
	DoAddAttrib(*pAttrBlk);

	//加入属性块
	blk.AddCodeValue(100,pAttrBlk,true);

	blk.Write(ar);

	//可以在该块后面，继续写入派生类的块
	DoWriteOtherData(ar);

	return TRUE;
}

// 添加公用属性
BOOL	kcEntity::AddCommonAttrib(QxStgBlock& blk)
{
	//颜色
	double c[3];
	m_color.get(c);
	blk.AddCodeValue(KSTG_CV_CODE_COLOR,c);
	//

	return TRUE;
}

//
BOOL	kcEntity::ParseCommonAttrib(QxStgBlock& blk)
{
	double col[3];

	if(!blk.GetValueByCode(KSTG_CV_CODE_COLOR,col)){
		ASSERT(FALSE);
	}

	//颜色
	m_color.set(col);

	return TRUE;
}

// 将TopoDS_Shape对象的数据，转换成byte数组，添加到blk中
//
BOOL  kcEntity::AddShapeData(QxStgBlock& blk,const TopoDS_Shape& aShape)
{
	if(!aShape.IsNull()){
		kstgShapeData sd;
		BYTE *pBuf = NULL;
		int nSize = 0;

		if(sd.Build(aShape,pBuf,nSize)){
			blk.AddCodeValue(KSTG_CV_CODE_SHAPE,pBuf,nSize,false);
			return TRUE;
		}
	}
	
	return FALSE;
}

// 从blk中解析出TopoDS_Shape对象的数据，并重新生成对象
//
TopoDS_Shape	kcEntity::ParseShapeData(QxStgBlock& blk)
{
	TopoDS_Shape aShape;
	QxStgCodeValue *pCV = NULL;
	BYTE *pData = NULL;
	int nSize = 0;

	pCV = blk.GetCodeValueByCode(KSTG_CV_CODE_SHAPE);
	if(pCV && pCV->GetValue(pData,nSize)){
		kstgShapeData sd;
		aShape = sd.Parse(pData,nSize);	
	}else{
		ASSERT(FALSE);
	}

	return aShape;
}

///////////////////////////////////////////////////////
//

BOOL	kcPointEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcPointEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
BOOL	kcEdgeEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcEdgeEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
BOOL	kcWireEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcWireEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
BOOL	kcFaceEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcFaceEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}


///////////////////////////////////////////////////////
//
BOOL	kcShellEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcShellEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
BOOL	kcSolidEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcSolidEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
BOOL	kcCompoundEntity::DoAddAttrib(QxStgBlock& blk)
{
	return TRUE;
}

BOOL	kcCompoundEntity::DoParseAttrib(QxStgBlock& blk)
{
	return TRUE;
}

///////////////////////////////////////////////////////
//
bool  kcBlockEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	//记录子对象的个数
	int num = (int)aEntList_.size();
	blk.AddCodeValue(20,num);

	return true;
}

bool kcBlockEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	if(!blk.GetValueByCode(20,nReadEntNum_)){
		nReadEntNum_ = 0;
	}
	return true;
}

bool kcBlockEntity::DoWriteOtherData(CArchive& ar)
{
	//依次写入每个子Entity
	kcEntityList::iterator ite = aEntList_.begin();
	for(;ite != aEntList_.end();++ite){
		(*ite)->Write(ar);
	}
	return true;
}

bool kcBlockEntity::DoReadOtherData(CArchive& ar)
{
	//依次读取每个子Entity
	int ix = 0;
	kcEntity *pEntity = NULL;
	for(ix = 0;ix < nReadEntNum_;ix ++){
		pEntity = kcEntity::ReadEntity(ar);
		if(!pEntity){
			return false;
		}
		//
		AddEntity(pEntity);
	}

	return true;
}