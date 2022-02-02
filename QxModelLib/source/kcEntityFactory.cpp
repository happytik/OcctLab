#include "StdAfx.h"
#include "kcEntity.h"
#include "kcEntityFactory.h"

kcEntityFactory::kcEntityFactory(void)
{
}

kcEntityFactory::~kcEntityFactory(void)
{
}

//根据类型，创建对应的entity对象
kcEntity* kcEntityFactory::CreateByType(int type)
{
	kcEntity *pEntity = NULL;
	switch(type){
	case KCT_ENT_POINT:
		pEntity = new kcPointEntity;
		break;
	case KCT_ENT_EDGE:
		pEntity = new kcEdgeEntity;
		break;
	case KCT_ENT_WIRE:
		pEntity = new kcWireEntity;
		break;
	case KCT_ENT_FACE:
		pEntity = new kcFaceEntity;
		break;
	case KCT_ENT_SHELL:
		pEntity = new kcShellEntity;
		break;
	case KCT_ENT_SOLID:
		pEntity = new kcSolidEntity;
		break;
	case KCT_ENT_COMPOUND:
		pEntity = new kcCompoundEntity;
		break;
	case KCT_ENT_BLOCK:
		pEntity = new kcBlockEntity;
		break;
	case KCT_ENT_DIM_LENGTH:
		pEntity = new kcLengthDimEntity;
		break;
	case KCT_ENT_DIM_DIAMETER:
		pEntity = new kcDiameterDimEntity;
		break;
	case KCT_ENT_DIM_RADIUS:
		pEntity = new kcRadiusDimEntity;
		break;
	case KCT_ENT_DIM_ANGLE:
		pEntity = new kcAngleDimEntity;
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return pEntity;
}

kcEntity* kcEntityFactory::Create(const TopoDS_Shape& aShape)
{
	return kcEntityFactory::CreateEntity(aShape);	
}

// 从shape创建对应的Entity对象
kcEntity* kcEntityFactory::CreateEntity(const TopoDS_Shape& aShape)
{
	if(aShape.IsNull())
		return NULL;

	kcEntity *pNewEntity = NULL;
	TopAbs_ShapeEnum atyp = aShape.ShapeType();
	switch(atyp){
		case TopAbs_VERTEX:
			{
				TopoDS_Vertex aVertex = TopoDS::Vertex(aShape);
				gp_Pnt pnt = BRep_Tool::Pnt(aVertex);
				kcPointEntity *pPointEntity = new kcPointEntity;
				pPointEntity->SetPoint(pnt.X(),pnt.Y(),pnt.Z());
				pNewEntity = pPointEntity;
			}
			break;
		case TopAbs_EDGE:
			{
				TopoDS_Edge aEdge = TopoDS::Edge(aShape);
				kcEdgeEntity *pCurveEntity = new kcEdgeEntity;
				pCurveEntity->SetEdge(aEdge);
				pNewEntity = pCurveEntity;
			}
			break;
		case TopAbs_WIRE:
			{
				TopoDS_Wire aWire = TopoDS::Wire(aShape);
				kcWireEntity *pWireEntity = new kcWireEntity;
				pWireEntity->SetWire(aWire);
				pNewEntity = pWireEntity;
			}
			break;
		case TopAbs_FACE:
			{
				TopoDS_Face aFace = TopoDS::Face(aShape);
				kcFaceEntity *pFaceEntity = new kcFaceEntity;
				pFaceEntity->SetFace(aFace);
				pNewEntity = pFaceEntity;
			}
			break;
		case TopAbs_SHELL:
			{
				TopoDS_Shell aShell = TopoDS::Shell(aShape);
				kcShellEntity *pShellEntity = new kcShellEntity;
				pShellEntity->SetShell(aShell);
				pNewEntity = pShellEntity;
			}
			break;
		case TopAbs_SOLID:
			{
				TopoDS_Solid aSolid = TopoDS::Solid(aShape);
				kcSolidEntity *pSolidEntity = new kcSolidEntity;
				pSolidEntity->SetSolid(aSolid);
				pNewEntity = pSolidEntity;
			}
			break;
		case TopAbs_COMPOUND:
			{
				TopoDS_Compound aComp = TopoDS::Compound(aShape);
				kcCompoundEntity *pCompEntity = new kcCompoundEntity;
				pCompEntity->SetCompound(aComp);
				pNewEntity = pCompEntity;
			}
			break;
		default:
			break;
	}

	return pNewEntity;
}

//创建一个或一系列entity,如果shape是compound，则是否分解为子对象
bool kcEntityFactory::Create(const TopoDS_Shape& aShape,
								std::vector<kcEntity *>& aEntList,
								bool bDeCompound)
{
	kcEntity *pEntity = NULL;
	if(bDeCompound && aShape.ShapeType() == TopAbs_COMPOUND){
		TopoDS_Iterator ite;
		for(ite.Initialize(aShape);ite.More();ite.Next()){
			pEntity = Create(ite.Value());
			if(!pEntity)
				return false;

			aEntList.push_back(pEntity);
		}
	}else{
		pEntity = Create(aShape);
		if(!pEntity)
			return false;

		aEntList.push_back(pEntity);
	}

	return true;
}

//从曲线创建
kcEntity*  kcEntityFactory::Create(const Handle(Geom_Curve) &aCurve)
{
	kcEntity *pEntity = NULL;
	if(!aCurve.IsNull()){
		try{
			BRepBuilderAPI_MakeEdge ME(aCurve);
			if(ME.IsDone()){
				TopoDS_Edge aEdge = ME.Edge();
				pEntity = Create(aEdge);
			}
		}catch(Standard_Failure){
			TRACE("\n###---???--- Create entity from curve failed.\n");
		}
	}
	return pEntity;
}

kcEntity* kcEntityFactory::Create(const Handle(Geom_Curve) &aCurve,int color,const char *pDesc)
{
	kcEntity *pEntity = NULL;
	if(!aCurve.IsNull()){
		kcEntityFactory entFac;
		try{
			BRepBuilderAPI_MakeEdge ME(aCurve);
			if(ME.IsDone()){
				TopoDS_Edge aEdge = ME.Edge();
				pEntity = entFac.Create(aEdge);
				if(pEntity){
					if(color >= 0){
						pEntity->SetColor(color,FALSE);
					}
					if(pDesc){
						pEntity->SetNameDesc(pDesc);
					}
				}
			}
		}catch(Standard_Failure){
			TRACE("\n###---???--- Create entity from curve failed.\n");
		}
	}
	return pEntity;
}

//从曲面创建
kcEntity* kcEntityFactory::Create(const Handle(Geom_Surface) &aSurf)
{
	kcEntity *pEntity = NULL;
	if(!aSurf.IsNull()){
		try{
			BRepBuilderAPI_MakeFace mkFace(aSurf,Precision::Confusion());
			if(mkFace.IsDone()){
				TopoDS_Face aF = mkFace.Face();
				pEntity = Create(aF);
			}
		}catch(Standard_Failure){
			TRACE("\n###---???--- Create entity from surface failed.\n");
		}
	}
	return pEntity;
}