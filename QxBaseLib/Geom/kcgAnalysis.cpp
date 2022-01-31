#include "stdafx.h"
#include <BRepBndLib.hxx>
#include <Bnd_SeqOfBox.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopTools_MapOfShape.hxx>
#include "QxBaseDef.h"
#include "QxDbgTool.h"
#include "kcgAnalysis.h"

//////////////////////////////////////////////////////////////////
//
kcxCrossRec::kcxCrossRec()
{
	Reset();
}

kcxCrossRec::kcxCrossRec(double dF,double dL,double dMin,double dMax)
{
	dFirst_ = dF;dLast_ = dL;
	dMin_ = dMin;dMax_ = dMax;
}

void  kcxCrossRec::Reset()
{
	dFirst_ = dLast_ = 0.0;
	dMin_ = dMax_ = 0.0;
}

//////////////////////////////////////////////////////////////////
//

kcxCurveTypeInfo::kcxCurveTypeInfo()
{
	Reset();
}

kcxCurveTypeInfo::kcxCurveTypeInfo(const std::string &szName)
{
	Reset();
	sTypeName_ = szName;
}

void  kcxCurveTypeInfo::Reset()
{
	sTypeName_ = "";
	nTotalNum_ = 0;
	nbPeriodic_ = nbCrossEnd_ = 0;
	aCrossRecVec_.clear();
}

void  kcxCurveTypeInfo::Collect(const Handle(Geom_Curve) &aCurve,const TopoDS_Edge &aEdge)
{
	if(aCurve.IsNull() || aEdge.IsNull())
		return;

	nTotalNum_ ++;
	if(aCurve->IsPeriodic()){
		nbPeriodic_ ++;
	}

	if(aCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){
		GeomAdaptor_Curve adc(aCurve);
		double t0,t1,resol;
		Handle(Geom_TrimmedCurve) aTrmCrv = Handle(Geom_TrimmedCurve)::DownCast(aCurve);
		
		t0 = aCurve->FirstParameter();
		t1 = aCurve->LastParameter();
		resol = adc.Resolution(1.0e-5);
		if(aTrmCrv->FirstParameter() < t0 - resol || 
			aTrmCrv->LastParameter() > t1 + resol)
		{
			nbCrossEnd_ ++;

			kcxCrossRec rec(t0,t1,aTrmCrv->FirstParameter(),aTrmCrv->LastParameter());
			aCrossRecVec_.push_back(rec);
		}
	}
}

std::string  kcxCurveTypeInfo::	ToString()
{
	std::string rs;
	char szBuf[256];

	sprintf_s(szBuf,255,"-- %s total has %d\n",sTypeName_.c_str(),nTotalNum_);
	rs = szBuf;
	sprintf_s(szBuf,255,"      periodic curve has %d\n",nbPeriodic_);
	rs += szBuf;
	sprintf_s(szBuf,255,"      Cross end has %d\n",nbCrossEnd_);
	rs += szBuf;

	std::vector<kcxCrossRec>::size_type ix,isize = aCrossRecVec_.size();
	for(ix = 0;ix < isize;ix ++){
		const kcxCrossRec &aRec = aCrossRecVec_.at(ix);
		//
		sprintf_s(szBuf,255,"          Range:[%f,%f], Trim:[%f,%f]\n",aRec.dFirst_,
			aRec.dLast_,aRec.dMin_,aRec.dMax_);
		rs += szBuf;
	}

	return rs;
}

//////////////////////////////////////////////////////////////////
//

//统计曲面的类型
kcxSurfTypeInfo::kcxSurfTypeInfo()
{
	Reset();
}

kcxSurfTypeInfo::kcxSurfTypeInfo(const std::string &szName)
{
	Reset();
	strTypeName_ = szName;
}

void  kcxSurfTypeInfo::Reset()
{
	strTypeName_ = "";
	nCount_ = 0;
	nbUPeriodic_ = nbVPeriodic_ = nbUVPeriodic_ = 0;
	nbUCrossSeam_ = nbVCrossSeam_ = 0;
	//
	uCrossRecVec_.clear();
	vCrossRecVec_.clear();
}

void  kcxSurfTypeInfo::Collect(const Handle(Geom_Surface) &aSurf,const TopoDS_Face &aFace)
{
	if(aFace.IsNull() || aSurf.IsNull())
		return;

	bool bUPeriodic = aSurf->IsUPeriodic(),bVPeriodic = aSurf->IsVPeriodic();
	nCount_ ++;
	if(bUPeriodic && bVPeriodic){
		nbUVPeriodic_ ++;
	}else if(bUPeriodic){
		nbUPeriodic_ ++;
	}else if(bVPeriodic){
		nbVPeriodic_ ++;
	}

	double U1,U2,V1,V2;
	double uptol,vptol,sptol = 1e-6;

	GeomAdaptor_Surface adpSurf(aSurf);
	uptol = adpSurf.UResolution(sptol);
	vptol = adpSurf.VResolution(sptol);

	if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		double utrm1,utrm2,vtrm1,vtrm2;
		Handle(Geom_RectangularTrimmedSurface) aTrmSurf = 
			Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf);
		//
		Handle(Geom_Surface) aBasSurf = aTrmSurf->BasisSurface();
		aBasSurf->Bounds(U1,U2,V1,V2);
		//
		aSurf->Bounds(utrm1,utrm2,vtrm1,vtrm2);

		if(utrm1 < U1 - uptol || utrm2 > U2 + uptol){
			nbUCrossSeam_ ++;
			//
			kcxCrossRec rec(U1,U2,utrm1,utrm2);
			uCrossRecVec_.push_back(rec);
		}

		if(vtrm1 < V1 - vptol || vtrm2 > V2 + vptol){
			nbVCrossSeam_ ++;
			//
			kcxCrossRec rec(V1,V2,vtrm1,vtrm2);
			vCrossRecVec_.push_back(rec);
		}

	}else{
		//检查是否跨seam edge
		if(bUPeriodic || bVPeriodic){
			double umin,umax,vmin,vmax;
			try{
				aSurf->Bounds(U1,U2,V1,V2);
				BRepTools::UVBounds(aFace,umin,umax,vmin,vmax);
			}catch(Standard_Failure){
				return;
			}

			if(bUPeriodic && (umin < U1 - uptol || umax > U2 + uptol)){
				nbUCrossSeam_ ++;
				//
				kcxCrossRec crsRec(U1,U2,umin,umax);
				uCrossRecVec_.push_back(crsRec);
			}
			if(bVPeriodic && (vmin < V1 - vptol || vmax > V2 + vptol)){
				nbVCrossSeam_ ++;
				//
				kcxCrossRec crsRec(V1,V2,vmin,vmax);
				vCrossRecVec_.push_back(crsRec);
			}
		}
	}
}

std::string  kcxSurfTypeInfo::ToString()
{
	char szBuf[256];
	std::string strInfo;

	sprintf_s(szBuf,255,"-- %s count is %d\n",strTypeName_.c_str(),nCount_);
	strInfo = szBuf;
	if(nbUPeriodic_ > 0){
		sprintf_s(szBuf,255,"    U  Periodic : %d\n",nbUPeriodic_);
		strInfo += szBuf;
	}
	if(nbVPeriodic_ > 0){
		sprintf_s(szBuf,255,"    V  Periodic : %d\n",nbVPeriodic_);
		strInfo += szBuf;
	}
	if(nbUVPeriodic_ > 0){
		sprintf_s(szBuf,255,"    UV periodic : %d\n",nbUVPeriodic_);
		strInfo += szBuf;
	}
	if(nbUCrossSeam_ > 0){
		sprintf_s(szBuf,255,"    U Cross Seam : %d\n",nbUCrossSeam_);
		strInfo += szBuf;
		//
		std::vector<kcxCrossRec>::size_type ix,isize = uCrossRecVec_.size();
		for(ix = 0;ix < isize;ix ++){
			const kcxCrossRec &crsRec = uCrossRecVec_.at(ix);
			sprintf_s(szBuf,255,"        Range : [%f - %f], Box : [%f - %f]\n",
				crsRec.dFirst_,crsRec.dLast_,crsRec.dMin_,crsRec.dMax_);
			strInfo += szBuf;
		}
	}
	if(nbVCrossSeam_ > 0){
		sprintf_s(szBuf,255,"    V Cross Seam : %d\n",nbVCrossSeam_);
		strInfo += szBuf;
		//
		std::vector<kcxCrossRec>::size_type ix,isize = vCrossRecVec_.size();
		for(ix = 0;ix < isize;ix ++){
			const kcxCrossRec &crsRec = vCrossRecVec_.at(ix);
			sprintf_s(szBuf,255,"        Range : [%f - %f], Box : [%f - %f]\n",
				crsRec.dFirst_,crsRec.dLast_,crsRec.dMin_,crsRec.dMax_);
			strInfo += szBuf;
		}
	}

	return strInfo;
}

//////////////////////////////////////////////////////////////////
//

kcgAnalysis::kcgAnalysis(void)
{
}


kcgAnalysis::~kcgAnalysis(void)
{
	Clear();
}

void  kcgAnalysis::CollectCurveTypeInfo(const TopoDS_Shape &aShape)
{
	if(aShape.IsNull() || aShape.ShapeType() > TopAbs_EDGE)
		return;

	TopTools_MapOfShape aEdgeMap;
	TopExp_Explorer exp;
	for(exp.Init(aShape,TopAbs_EDGE);exp.More();exp.Next()){
		TopoDS_Edge aE = TopoDS::Edge(exp.Current());
		//
		if(aEdgeMap.Add(aE)){//没有重复
			//
			CollectEdgeCurveTypeInfo(aE);			
		}
	}
}

void  kcgAnalysis::CollectEdgeCurveTypeInfo(const TopoDS_Edge &aEdge)
{
	Handle(Geom_Curve) aCurve;
	double dF,dL;

	aCurve = BRep_Tool::Curve(aEdge,dF,dL);
	if(aCurve.IsNull())
		return;

	kcxCurveTypeInfo *pTypeInfo = NULL;
	std::string sName,sFixName;
	GetCurveTypeName(aCurve,sName,sFixName);

	pTypeInfo = GetCurveTypeInfo(sName);
	pTypeInfo->Collect(aCurve,aEdge);
	
	if(!sFixName.empty()){
		Handle(Geom_Curve) aBasCrv;
		if(aCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){
			Handle(Geom_TrimmedCurve) aTrmCrv = Handle(Geom_TrimmedCurve)::DownCast(aCurve);
			aBasCrv = aTrmCrv->BasisCurve();
		}
		if(aCurve->IsKind(STANDARD_TYPE(Geom_OffsetCurve))){
			Handle(Geom_OffsetCurve) aOffCrv = Handle(Geom_OffsetCurve)::DownCast(aCurve);
			aBasCrv = aOffCrv->BasisCurve();
		}

		if(!aBasCrv.IsNull()){
			pTypeInfo = GetCurveTypeInfo(sFixName);
			pTypeInfo->Collect(aBasCrv,aEdge);
		}
	}
}

void  kcgAnalysis::GetCurveTypeName(const Handle(Geom_Curve) &aCurve,std::string &sTypeName,std::string &sTypeFixName)
{
	Handle(Standard_Type) aType = aCurve->DynamicType();
	sTypeName = aType->Name();

	Handle(Geom_Curve) aBasCrv;
	if(aCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))){
		Handle(Geom_TrimmedCurve) aTrmCrv = Handle(Geom_TrimmedCurve)::DownCast(aCurve);
		aBasCrv = aTrmCrv->BasisCurve();
	}
	if(aCurve->IsKind(STANDARD_TYPE(Geom_OffsetCurve))){
		Handle(Geom_OffsetCurve) aOffCrv = Handle(Geom_OffsetCurve)::DownCast(aCurve);
		aBasCrv = aOffCrv->BasisCurve();
	}

	if(!aBasCrv.IsNull()){
		aType = aBasCrv->DynamicType();

		sTypeFixName = sTypeName + "-";
		sTypeFixName += aType->Name();
	}
}

kcxCurveTypeInfo*  kcgAnalysis::GetCurveTypeInfo(const std::string &sName)
{
	kcxCurveTypeInfo *pTypeInfo = NULL;
	stdCurveTypeInfoMap::iterator ite = aCurveTypeInfoMap_.find(sName);
	if(ite == aCurveTypeInfoMap_.end()){
		pTypeInfo = new kcxCurveTypeInfo(sName);
		aCurveTypeInfoMap_.insert(stdCurveTypeInfoMap::value_type(sName,pTypeInfo));
	}else{
		pTypeInfo = (*ite).second;
	}
	return pTypeInfo;
}

void  kcgAnalysis::CollectSurfTypeInfo(const TopoDS_Shape &aShape)
{
	if(aShape.IsNull() || aShape.ShapeType() > TopAbs_FACE)
		return;

	TopExp_Explorer exp;
	for(exp.Init(aShape,TopAbs_FACE);exp.More();exp.Next()){
		TopoDS_Face aFace = TopoDS::Face(exp.Current());
		//
		TopLoc_Location L;
		Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace,L);
		if(!aSurf.IsNull()){
			CollectSurfTypeInfo(aSurf,aFace);
		}
	}
}

void  kcgAnalysis::CollectSurfTypeInfo(const Handle(Geom_Surface) &aSurf,const TopoDS_Face &aFace)
{
	std::string sTypeName,sTypeFixName;
	kcxSurfTypeInfo *pTypeInfo = NULL;
	
	GetSurfTypeName(aSurf,sTypeName,sTypeFixName);

	pTypeInfo = GetSurfTypeInfo(sTypeName);
	pTypeInfo->Collect(aSurf,aFace);

	if(!sTypeFixName.empty()){
		Handle(Geom_Surface) aBasSurf;
		if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
			Handle(Geom_RectangularTrimmedSurface) aTrmSurf = 
				Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf);
			aBasSurf = aTrmSurf->BasisSurface();

		}else if(aSurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface))){
			Handle(Geom_OffsetSurface) aOffSurf = 
				Handle(Geom_OffsetSurface)::DownCast(aSurf);
			aBasSurf = aOffSurf->BasisSurface();
		}

		if(!aBasSurf.IsNull()){
			pTypeInfo = GetSurfTypeInfo(sTypeFixName);
			pTypeInfo->Collect(aBasSurf,aFace);
		}
	}

}

// get class name,if trim surf or off surf,get class name - base surf class name
//
void  kcgAnalysis::GetSurfTypeName(const Handle(Geom_Surface) &aSurf,std::string &sTypeName,std::string &sTypeFixName)
{
	Handle(Geom_Surface) aBasSurf;
	Handle(Standard_Type) aType = aSurf->DynamicType();
	sTypeName = aType->Name();

	if(aSurf->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface))){
		Handle(Geom_RectangularTrimmedSurface) aTrmSurf = 
			Handle(Geom_RectangularTrimmedSurface)::DownCast(aSurf);
		aBasSurf = aTrmSurf->BasisSurface();
	}else if(aSurf->IsKind(STANDARD_TYPE(Geom_OffsetSurface))){
		Handle(Geom_OffsetSurface) aOffSurf = Handle(Geom_OffsetSurface)::DownCast(aSurf);
		aBasSurf = aOffSurf->BasisSurface();
	}

	if(!aBasSurf.IsNull()){
		aType = aBasSurf->DynamicType();
		sTypeFixName = sTypeName + '-';
		sTypeFixName += aType->Name();
	}
}

// get typeinfo by name,create new one if it is not existed
//
kcxSurfTypeInfo*  kcgAnalysis::	GetSurfTypeInfo(const std::string &sName)
{
	kcxSurfTypeInfo *pTypeInfo = NULL;
	stdSurfTypeInfoMap::iterator iteMap = aSurfTypeInfoMap_.find(sName);
	if(iteMap == aSurfTypeInfoMap_.end()){
		pTypeInfo = new kcxSurfTypeInfo(sName);
		aSurfTypeInfoMap_.insert(stdSurfTypeInfoMap::value_type(sName,pTypeInfo));
	}else{
		pTypeInfo = (*iteMap).second;
	}

	return pTypeInfo;
}

void  kcgAnalysis::Clear()
{
	stdCurveTypeInfoMap::iterator iteC;
	for(iteC = aCurveTypeInfoMap_.begin();iteC != aCurveTypeInfoMap_.end();++iteC){
		delete (*iteC).second;
	}
	aCurveTypeInfoMap_.clear();

	stdSurfTypeInfoMap::iterator iteS = aSurfTypeInfoMap_.begin();
	for(;iteS != aSurfTypeInfoMap_.end();++iteS){
		delete (*iteS).second;
	}
	aSurfTypeInfoMap_.clear();
}

bool  kcgAnalysis::WriteToFile(const char *pszFile)
{
	ASSERT(pszFile != NULL);
	FILE *fp = NULL;

	if(aSurfTypeInfoMap_.empty() && aCurveTypeInfoMap_.empty())
		return false;

	fopen_s(&fp,pszFile,"w");
	if(fp == NULL){
		//TRACE0("\n open file %s failed.\n",pszFile);
		return false;
	}

	if(!aCurveTypeInfoMap_.empty()){
		fprintf_s(fp,"##------curve type info------\n\n");
		std::string str;
		stdCurveTypeInfoMap::iterator iteC = aCurveTypeInfoMap_.begin();
		for(;iteC != aCurveTypeInfoMap_.end();++iteC){
			str = (*iteC).second->ToString();
			fprintf_s(fp,"%s\n",str.c_str());
		}
	}

	if(!aSurfTypeInfoMap_.empty()){
		fprintf_s(fp,"##------surface type info------\n\n");
		
		stdSurfTypeInfoMap::iterator iteMap;
		for(iteMap = aSurfTypeInfoMap_.begin();iteMap != aSurfTypeInfoMap_.end();++iteMap){
			std::string ss = (*iteMap).second->ToString();
			fprintf_s(fp,"%s",ss.c_str());
		}
	}

	fclose(fp);

	return true;
}

