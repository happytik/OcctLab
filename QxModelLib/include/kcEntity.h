//////////////////////////////////////////////////////////////////////////
// entity包括几何信息和显示数据。
// 显示数据使用AIS来表示和记录。

#ifndef _KC_ENTITY_H_
#define _KC_ENTITY_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <AIS_InteractiveObject.hxx>
#include <Quantity_Color.hxx>
#include <gp_Circ.hxx>
#include "kBase.h"
#include "kPoint.h"
#include "kVector.h"
#include "AIS_EntityShape.h"
#include "QxLibDef.h"

#define KC_ENT_NAME_MAX		64

typedef std::vector<TopoDS_Shape>  QxTopShapeArray;
typedef std::list<TopoDS_Shape> QxTopShapeList;

// 类型使用高word作为主类型掩码,低word作为子类型定义.
// 如果还要再定义子子类型,可以类似分配掩码.
// 例如:0x201 是curve类型下的直线.
#define KCT_ENT_POINT			0x01
#define KCT_ENT_EDGE			0x02
#define KCT_ENT_WIRE			0x04 //普通wire
#define KCT_ENT_FACE			0x08
#define KCT_ENT_SHELL			0x10
#define KCT_ENT_SOLID			0x20
#define KCT_ENT_COMPOUND		0x40
#define KCT_ENT_BLOCK			0x80
#define KCT_ENT_DIM			0x100 //dim的
#define KCT_ENT_DIM_LENGTH		(KCT_ENT_DIM + 1)
#define KCT_ENT_DIM_DIAMETER	(KCT_ENT_DIM + 2)
#define KCT_ENT_DIM_RADIUS		(KCT_ENT_DIM + 3)
#define KCT_ENT_DIM_ANGLE		(KCT_ENT_DIM + 4)

#define KCT_ENT_ALL				(KCT_ENT_POINT | KCT_ENT_EDGE | KCT_ENT_WIRE | KCT_ENT_FACE | KCT_ENT_SHELL | KCT_ENT_SOLID | KCT_ENT_COMPOUND)

#ifndef IS_POINT_ENTITY
#define IS_POINT_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_POINT)
#endif

#ifndef IS_EDGE_ENTITY
#define IS_EDGE_ENTITY(pEnt)   (pEnt->GetType() & KCT_ENT_EDGE)
#endif

#ifndef IS_WIRE_ENTITY
#define IS_WIRE_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_WIRE)
#endif

#ifndef IS_FACE_ENTITY
#define IS_FACE_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_FACE)
#endif

#ifndef IS_SHELL_ENTITY
#define IS_SHELL_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_SHELL)
#endif


#ifndef IS_SOLID_ENTITY
#define IS_SOLID_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_SOLID)
#endif

#ifndef IS_COMPOUND_ENTITY
#define IS_COMPOUND_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_COMPOUND)
#endif

#ifndef IS_BLOCK_ENTITY
#define IS_BLOCK_ENTITY(pEnt)	(pEnt->GetType() & KCT_ENT_BLOCK)
#endif

class CArchive;
class kcLayer;
class kcModel;
class kiSnapMngr;
class QxStgBlock;

class kcEntity;
	class kcPointEntity;
	class kcEdgeEntity;
	class kcWireEntity;//组合线
	class kcFaceEntity;
	class kcShellEntity;
	class kcSolidEntity;
	class kcCompoundEntity;//组合，任意对象的组合。
	class kcBlockEntity; //块对象，可以包含多个各种类型的kcEntity对象
	class kcDimEntity;
		class kcLengthDimEntity;
		class kcDiameterDimEntity;
		class kcRadiusDimEntity;
		class kcAngleDimEntity;
	
typedef std::list<kcEntity *>  kcEntityList;
typedef std::vector<kcEntity *> kcEntityArray;
typedef std::map<kcEntity *,int> kcEntityMap;

// 和occ的定义一致
typedef enum tagLineStyle{
	eTOL_SOLID,
	eTOL_DASH,
	eTOL_DOT,
	eTOL_DOTDASH,
	eTOL_USERDEFINED
}eLineStyle;

//辅助函数
QXLIB_API bool  ksIsValidEntityType(int entType);
QXLIB_API int   ksEntityType(const TopAbs_ShapeEnum shapeType);
QXLIB_API TopAbs_ShapeEnum  ksShapeType(int entType);
QXLIB_API int   ksSeparateEntityTypes(int enttypes,int *pTypeArray);//返回类型个数

// 基本的管理对象，功能包括：
// 1、记录几何和拓扑数据
// 2、记录相关的属性
// 3、记录和管理显示对象
// 4、读写相关数据
// 5、添加一些定制或扩展功能等
//
class QXLIB_API kcEntity
{
public:
	kcEntity(void);
	kcEntity(kcLayer *pLayer);
	virtual ~kcEntity(void);

	//返回类型
	virtual int				GetType() const { return _entType; }
	DWORD					GetHandle() const { return _nHandle; }
	const char*				GetName() const { return _szName;}
	bool						HasName() const;
	bool						SetNameDesc(const char *pszDesc); //设定名称中描述部分，用于组成名称

	// 获取对应的低层对象,返回局部,不能够外部修改.
	virtual TopoDS_Shape		GetShape();

	virtual BOOL				IsValid() const { return bValid_; }
	virtual void				SetModified(BOOL bModified);
	virtual BOOL				IsModified() const { return bModified_; }

	//是否标注对象
	virtual bool				IsDimension() const { return false; }

	// 设置和获取图层。
	virtual BOOL				SetLayer(kcLayer *pLayer);
	virtual kcLayer*			GetLayer() const { return _pLayer; }

	virtual bool				SetOwner(kcEntity *pOwner);
	virtual kcEntity*			GetOwner() const;

	virtual BOOL				Destroy();

public://一些属性判断
	//是否闭合
	virtual bool				IsClosed() { return false;}

public:
	// 计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr) {  return FALSE; }

public:
	// 读取
	virtual BOOL				Read(CArchive& ar);
	// 写入
	virtual BOOL				Write(CArchive& ar);
	// 从ar中读出一个kcEntity对象
	static kcEntity*			ReadEntity(CArchive &ar);

protected:
	// 添加公用属性
	virtual BOOL				AddCommonAttrib(QxStgBlock& blk);
	//
	virtual BOOL				ParseCommonAttrib(QxStgBlock& blk);
	//
	virtual BOOL				AddShapeData(QxStgBlock& blk,const TopoDS_Shape& aShape);
	virtual TopoDS_Shape		ParseShapeData(QxStgBlock& blk);		

	// 用于派生类写入或读出自己的数据
	virtual bool				DoWriteOtherData(CArchive& ar) { return TRUE; }
	virtual bool				DoReadOtherData(CArchive& ar) { return TRUE; }
	// 用于派生类向主块中记录其他的值
	virtual bool				DoAddOtherCodeValue(QxStgBlock& blk) { return TRUE; }
	virtual bool				DoGetOtherCodeValue(QxStgBlock& blk) { return TRUE; }
	// 用于派生类添加自己的属性值
	virtual BOOL				DoAddAttrib(QxStgBlock& blk) { return TRUE; }
	virtual BOOL				DoParseAttrib(QxStgBlock& blk) { return TRUE; }

public:
	//拷贝自身
	virtual kcEntity*			Clone() {  return NULL; }

protected:
	//拷贝公共属性
	virtual bool				CloneCommonAttribs(kcEntity *pEntity);

public:
	// 显示
	virtual BOOL				Display(bool bShow,BOOL bUpdateView = TRUE);
	virtual BOOL				IsDisplayed() const;
	virtual BOOL				HasDispalyObj() const;
	const Handle(AIS_InteractiveObject)&	DisplayObj() const { return hAISObject_; }

protected:
	// 生成显示对象
	// 为新对象生成显示对象
	virtual	BOOL				BuildDisplayObj();
	// 更新显示对象，可能有旧的显示对象。
	virtual BOOL				UpdateDisplayObj(BOOL bUpdateView);
	virtual BOOL				DestroyDisplayObj();
	// 具体的显示对象
	virtual BOOL				DoDisplay(BOOL bUpdateView);

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL				DoBuildDisplayObj();
	// 设置显示对象属性
	virtual void				DoSetDisplayAttribs();
	// 设置特定Face的属性
	virtual void				DoSetFaceDisplayAttribs() {}

protected:
	void						Reset();
	void						SetFaceColorAttribs();

protected:
	int						_entType;//类型
	std::string				_sTypeName;//例如："Point或者点"
	int						_nHandle;//句柄,整个模型唯一,有效值从1开始.
	char						_szName[KC_ENT_NAME_MAX];//名称
	std::string				sNameDesc_; //名称中的描述部分，用于组成最终名称
	TopoDS_Shape				aShape_;//OCCT shape的信息
	kcModel					*_pModel;//所在model。
	kcLayer					*_pLayer;//所在图层
	kcEntity					*pOwner_; //所属对象，目前主要用于block中对象
	bool						bVisible_; //对象是否可见

public://属性操作
	virtual bool				SetVisible(bool bVisible,BOOL bUpdate);
	virtual bool				IsVisible();
	virtual void				SetColor(double r,double g,double b,BOOL bUpdate);
	virtual void				SetColor(int icolor,BOOL bUpdate);
	virtual void				GetColor(double& r,double& g,double& b);
	virtual Quantity_Color	GetColor() const;
	virtual bool				SetLineStyle(eLineStyle linStyle,BOOL bUpdate);
	virtual eLineStyle		GetLineStyle() const { return m_eLineStyle; }
	virtual	bool				SetLineWidth(double width,BOOL bUpdate);
	virtual double			GetLineWidth() const { return m_dLineWidth; }
	virtual bool				SetTransparency(double transp,BOOL bUpdate);
	virtual double			GetTransparency() const { return m_dTransparency; }
	virtual bool				SetDisplayModel(int model,BOOL bUpdate);
	virtual int				GetDisplayModel() const { return m_nDisplayModel; }

protected:
	void						ResetAttribs();

protected://属性
	kVector3					m_color;//显示颜色
	eLineStyle				m_eLineStyle;
	double					m_dLineWidth;//线宽
	double					m_dTransparency;//透明度,0-1之间.
	int						m_nDisplayModel;//显示模式

protected:
	Handle(AIS_InteractiveContext)		_hAISContext;
	Handle(AIS_InteractiveObject)		hAISObject_;//记录显示对象
	Handle(AIS_EntityShape)				hAISEntObj_;//EntityShape显示对象

	friend class kcLayer;

protected:
	//
	void						SetValid(BOOL bValid);

private:
	BOOL						bValid_;//是否有有效数据
	BOOL						bModified_;//数据是否被修改了
};

// 一个点
class QXLIB_API kcPointEntity : public kcEntity{
public:
	kcPointEntity();
	kcPointEntity(double point[3]);
	virtual ~kcPointEntity();

	BOOL						SetPoint(double point[3]);
	BOOL						SetPoint(double x,double y,double z);
	BOOL						GetPoint(double point[3]);

protected:
	BOOL						BuildShape();

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

protected:
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);

public:
	//拷贝自身
	virtual kcEntity*			Clone();

public:
	double					_dPoint[3];
};

// 所有curve entity的基类。
//
class QXLIB_API kcEdgeEntity : public kcEntity
{
public:
	kcEdgeEntity(void);
	virtual ~kcEdgeEntity(void);

	//是否闭合
	virtual bool				IsClosed();

	virtual Handle(Geom_Curve)	GetCurve();
	virtual Handle(Geom_Curve)	GetCurve(double& t0,double& t1);
	virtual const TopoDS_Edge	GetEdge() const; 
	BOOL						SetCurve(const Handle(Geom_Curve)& aCurve);
	BOOL						SetEdge(const TopoDS_Edge& aEdge);
	
	//拷贝自身
	virtual kcEntity*			Clone();

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

protected:
	//
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);
};

// 一个组合线
class QXLIB_API kcWireEntity : public kcEntity{
public:
	kcWireEntity();
	virtual ~kcWireEntity();

	//是否闭合
	virtual bool				IsClosed();

	BOOL						SetWire(const TopoDS_Wire& aWire);
	const TopoDS_Wire			GetWire() const;

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

public:
	//拷贝自身
	virtual kcEntity*			Clone();
protected:
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);
};

// 曲面
class QXLIB_API kcFaceEntity : public kcEntity{
public:
	kcFaceEntity();
	virtual ~kcFaceEntity();

	BOOL						SetSurface(const Handle(Geom_Surface)& aSurf);
	BOOL						SetFace(const TopoDS_Face& aFace);
	const TopoDS_Face			GetFace() const;
	const Handle(Geom_Surface) GetSurface();

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

protected:
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);

public:
	//拷贝自身
	virtual kcEntity*			Clone();
};

// shell
class QXLIB_API kcShellEntity : public kcEntity{
public:
	kcShellEntity();
	virtual ~kcShellEntity();

	BOOL						SetShell(const TopoDS_Shell& aShell);
	const TopoDS_Shell		GetShell() const;

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

public:
	//拷贝自身
	virtual kcEntity*			Clone();

protected:
	virtual void				DoSetFaceDisplayAttribs();

protected:
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);
};

//　实体
class QXLIB_API kcSolidEntity : public kcEntity{
public:
	kcSolidEntity();
	virtual ~kcSolidEntity();

	BOOL						SetSolid(const TopoDS_Solid& aSolid);
	const TopoDS_Solid		GetSolid() const;

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

public:
	//拷贝自身
	virtual kcEntity*			Clone();

protected:
	virtual void				DoSetFaceDisplayAttribs();

protected:
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);
};

//组合，任意对象的组合。
class QXLIB_API kcCompoundEntity : public kcEntity{
public:
	kcCompoundEntity();
	virtual ~kcCompoundEntity();

	BOOL						SetCompound(const TopoDS_Compound& aComp);
	const TopoDS_Compound		GetCompound() const;

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

public:
	//拷贝自身
	virtual kcEntity*			Clone();

protected:
	virtual void				DoSetFaceDisplayAttribs();

protected:
	//
	virtual BOOL				DoAddAttrib(QxStgBlock& blk);
	virtual BOOL				DoParseAttrib(QxStgBlock& blk);
}; 

//组合，任意对象的组合。
class QXLIB_API kcBlockEntity : public kcEntity{
public:
	kcBlockEntity();
	virtual ~kcBlockEntity();

	// 获取对应的低层对象,返回局部,不能够外部修改.
	virtual TopoDS_Shape		GetShape();

	// 添加一个对象，对象将记录在该对象内部
	bool						AddEntity(kcEntity *pEntiey);
	const kcEntityList&		GetEntityList() const;

	const TopoDS_Compound		GetCompound() const;

public:
	//计算自动捕捉点信息
	virtual BOOL				CalcSnapPoint(kiSnapMngr& snapMgr);

public:
	//拷贝自身
	virtual kcEntity*			Clone();

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL				DoBuildDisplayObj();
	// 设置显示对象属性
	virtual void				DoSetDisplayAttribs();

protected:
	void						DoSetEntityAttribs(kcEntity *pEnt);

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool				DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool				DoGetOtherCodeValue(QxStgBlock& blk);
	// 用于派生类写入或读出自己的数据
	virtual bool				DoWriteOtherData(CArchive& ar);
	virtual bool				DoReadOtherData(CArchive& ar);

protected:
	kcEntityList				aEntList_; //记录一系列Entity对象
	kcEntityMap				aEntMap_;
	TopoDS_Compound			aComp_; //组合生成的shape

protected:
	int						nReadEntNum_;//读取的ent个数
}; 

class QXLIB_API kcDimEntity : public kcEntity{
public:
	kcDimEntity();
	virtual ~kcDimEntity();

	//是否标注对象
	virtual bool		IsDimension() const { return true; }

	void				SetFlyout(double dValue);
	double				GetFlyout() const { return dFlyout_; }

	void				SetArrowLength(double len);
	double				GetArrowLength() const { return dArrowLen_; }

	void				SetFontHeight(double h);
	double				GetFontHeight() const { return dFontHeight_; }

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool		DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool		DoGetOtherCodeValue(QxStgBlock& blk);
	//
	bool				DoAddCirc(const gp_Circ &aCirc, QxStgBlock& blk);
	bool				DoGetCirc(QxStgBlock& blk, gp_Circ &aCirc);
	//
	bool				DoAddPlane(const gp_Pln &aPln, QxStgBlock& blk);
	bool				DoGetPlane(QxStgBlock& blk, gp_Pln &aPln);
	// 设置显示对象属性
	virtual void		DoSetDisplayAttribs();

protected:
	double				dFlyout_;
	double				dArrowLen_;//箭头的长度
	double				dFontHeight_; //字体高度
};


// 长度标注
class QXLIB_API kcLengthDimEntity : public kcDimEntity{
public:
	kcLengthDimEntity();
	~kcLengthDimEntity();

	bool			Initialize(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pln &pln);

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL	DoBuildDisplayObj();

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool	DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool	DoGetOtherCodeValue(QxStgBlock& blk);

protected:
	gp_Pnt			aFirstPnt_;
	gp_Pnt			aSecondPnt_;
	gp_Pln			aPlane_;
};

class QXLIB_API kcDiameterDimEntity : public kcDimEntity{
public:
	kcDiameterDimEntity();
	~kcDiameterDimEntity();

	bool			Initialize(const gp_Circ &aCirc);

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL	DoBuildDisplayObj();

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool	DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool	DoGetOtherCodeValue(QxStgBlock& blk);

protected:
	gp_Circ			aCirc_;
};

class QXLIB_API kcRadiusDimEntity : public kcDimEntity{
public:
	kcRadiusDimEntity();
	~kcRadiusDimEntity();

	bool			Initialize(const gp_Circ &aCirc,const gp_Pnt& pnt);

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL	DoBuildDisplayObj();

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool	DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool	DoGetOtherCodeValue(QxStgBlock& blk);

protected:
	gp_Circ			aCirc_;
	gp_Pnt			aAnchorPnt_;
};


class QXLIB_API kcAngleDimEntity : public kcDimEntity{
public:
	kcAngleDimEntity();
	~kcAngleDimEntity();

	//
	bool			Initialize(const TopoDS_Edge &aE1, const TopoDS_Edge &aE2);
	bool			Initialize(const gp_Pnt &p1, const gp_Pnt &p2, const gp_Pnt &cp);

protected:
	// 根据记录的数据和属性，生成一个ais显示对象
	// 显示对象记录在m_aisObj中
	virtual BOOL	DoBuildDisplayObj();

protected:
	// 用于派生类向主块中记录其他的值
	virtual bool	DoAddOtherCodeValue(QxStgBlock& blk);
	virtual bool	DoGetOtherCodeValue(QxStgBlock& blk);


protected:
	gp_Pnt			aFirstPnt_;
	gp_Pnt			aSecondPnt_;
	gp_Pnt			aCenterPnt_;
};

#endif