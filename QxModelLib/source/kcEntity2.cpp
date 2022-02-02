///entity的保存和读取代码
#include "StdAfx.h"
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_TypeOfLine.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <gp_Ax2.hxx>
#include "QxBaseUtils.h"
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

#define KCVC_ENT_HAS_SHAPE		1 
#define KCVC_ENT_ATTR_BLOCK		2 //记录属性的块

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
	ASSERT(FALSE);

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
	if(aShape_.IsNull() && !IsDimension()){//标注是没有对象的
		ASSERT(FALSE);
		return FALSE;
	}

	//读取通用数据
	std::string sName;
	bool bVisible = false;
	QxStgBlock *pAttrBlk = blk.GetSubBlock(KCVC_ENT_ATTR_BLOCK); //读取属性块
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
	blk.AddCodeValue(KCVC_ENT_ATTR_BLOCK,pAttrBlk,true);

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
	bool bHasShape = true;
	if(!aShape.IsNull()){
		kstgShapeData sd;
		BYTE *pBuf = NULL;
		int nSize = 0;

		if(sd.Build(aShape,pBuf,nSize)){
			blk.AddCodeValue(KCVC_ENT_HAS_SHAPE, bHasShape);
			blk.AddCodeValue(KSTG_CV_CODE_SHAPE,pBuf,nSize,false);
			return TRUE;
		}
	}
	//没有shape数据
	bHasShape = false;
	blk.AddCodeValue(KCVC_ENT_HAS_SHAPE, bHasShape);
	
	return TRUE;
}

// 从blk中解析出TopoDS_Shape对象的数据，并重新生成对象
//
TopoDS_Shape kcEntity::ParseShapeData(QxStgBlock& blk)
{
	TopoDS_Shape aShape;
	QxStgCodeValue *pCV = NULL;
	BYTE *pData = NULL;
	int nSize = 0;
	bool bHasShape = true;

	blk.GetValueByCode(KCVC_ENT_HAS_SHAPE, bHasShape);
	if (bHasShape) {
		pCV = blk.GetCodeValueByCode(KSTG_CV_CODE_SHAPE);
		if (pCV && pCV->GetValue(pData, nSize)) {
			kstgShapeData sd;
			aShape = sd.Parse(pData, nSize);
		}
		else {
			ASSERT(FALSE);
		}
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

/////////////////////////////////////////////////////////////////

// 用于派生类向主块中记录其他的值
bool kcDimEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	blk.AddCodeValue(10, dFlyout_);
	blk.AddCodeValue(11, dArrowLen_);
	blk.AddCodeValue(12, dFontHeight_);

	return true;
}

bool kcDimEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	blk.GetValueByCode(10, dFlyout_);
	blk.GetValueByCode(11, dArrowLen_);
	blk.GetValueByCode(12, dFontHeight_);

	return true;
}

bool kcDimEntity::DoAddCirc(const gp_Circ &aCirc, QxStgBlock& blk)
{
	const gp_Ax2 &ax2 = aCirc.Position();
	//org
	gp_Pnt pos = ax2.Location();
	blk.AddCodeValue(20, pos);
	// z axis
	gp_Dir zd = ax2.Direction();
	blk.AddCodeValue(21, zd);
	// x axis
	gp_Dir xd = ax2.XDirection();
	blk.AddCodeValue(22, xd);
	// radius
	double r = aCirc.Radius();
	blk.AddCodeValue(23, r);

	return true;
}

bool kcDimEntity::DoGetCirc(QxStgBlock& blk, gp_Circ &aCirc)
{
	double radius = 0;
	gp_Pnt pos;
	gp_Dir zd, xd;

	blk.GetValueByCode(20, pos);
	blk.GetValueByCode(21, zd);
	blk.GetValueByCode(22, xd);
	blk.GetValueByCode(23, radius);
	//
	gp_Ax2 ax2(pos, zd, xd);
	aCirc = gp_Circ(ax2, radius);

	return true;
}

bool kcDimEntity::DoAddPlane(const gp_Pln &aPln, QxStgBlock& blk)
{
	gp_Ax3 ax3 = aPln.Position();
	//
	gp_Pnt org = ax3.Location();
	blk.AddCodeValue(20, org);
	// Z dir
	gp_Dir zd = ax3.Direction();
	blk.AddCodeValue(21, zd);
	// X dir
	gp_Dir xd = ax3.XDirection();
	blk.AddCodeValue(22, xd);

	return true;
}

bool kcDimEntity::DoGetPlane(QxStgBlock& blk, gp_Pln &aPln)
{
	gp_Pnt org;
	gp_Dir zd, xd;

	blk.GetValueByCode(20, org);
	blk.GetValueByCode(21, zd);
	blk.GetValueByCode(22, xd);

	gp_Ax2 ax2(org, zd, xd);
	aPln.SetPosition(gp_Ax3(ax2));

	return true;
}

//----------------------------------------------------

// 用于派生类向主块中记录其他的值
bool kcLengthDimEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoAddOtherCodeValue(blk);

	DoAddPlane(aPlane_, blk);

	//其他
	blk.AddCodeValue(24, aFirstPnt_);
	blk.AddCodeValue(25, aSecondPnt_);
	
	return true;
}

bool kcLengthDimEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoGetOtherCodeValue(blk);

	DoGetPlane(blk, aPlane_);

	blk.GetValueByCode(24, aFirstPnt_);
	blk.GetValueByCode(25, aSecondPnt_);

	return true;
}

// 用于派生类向主块中记录其他的值
bool kcDiameterDimEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoAddOtherCodeValue(blk);
	//添加circ
	DoAddCirc(aCirc_, blk);

	return true;
}

bool kcDiameterDimEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoGetOtherCodeValue(blk);
	//
	DoGetCirc(blk, aCirc_);

	return true;
}

// 用于派生类向主块中记录其他的值
bool kcRadiusDimEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoAddOtherCodeValue(blk);
	//添加circ
	DoAddCirc(aCirc_, blk);
	//
	double v[3];
	KxFromOccPnt(aAnchorPnt_, v);
	blk.AddCodeValue(24, v);

	return true;
}

bool kcRadiusDimEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoGetOtherCodeValue(blk);
	//
	DoGetCirc(blk, aCirc_);
	//
	double v[3];
	blk.GetValueByCode(24, v);
	KxToOccPnt(v, aAnchorPnt_);

	return true;
}

bool kcAngleDimEntity::DoAddOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoAddOtherCodeValue(blk);
	//
	blk.AddCodeValue(20, aFirstPnt_);
	blk.AddCodeValue(21, aSecondPnt_);
	blk.AddCodeValue(22, aCenterPnt_);

	return true;
}

bool kcAngleDimEntity::DoGetOtherCodeValue(QxStgBlock& blk)
{
	kcDimEntity::DoGetOtherCodeValue(blk);

	blk.GetValueByCode(20, aFirstPnt_);
	blk.GetValueByCode(21, aSecondPnt_);
	blk.GetValueByCode(22, aCenterPnt_);

	return true;
}