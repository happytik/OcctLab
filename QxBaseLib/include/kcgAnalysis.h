#pragma once

#include <vector>
#include <map>
#include <string>

#include "QxLibDef.h"

class kcxCurveTypeInfo;
class kcxSurfTypeInfo;
typedef std::map<std::string,kcxCurveTypeInfo *> stdCurveTypeInfoMap;
typedef std::map<std::string,kcxSurfTypeInfo *> stdSurfTypeInfoMap;

// 一个跨参数域的记录
class QXLIB_API kcxCrossRec{
public:
	kcxCrossRec();
	kcxCrossRec(double dF,double dL,double dMin,double dMax);

	void						Reset();

public:
	double					dFirst_,dLast_;
	double					dMin_,dMax_;
};

//统计曲线的类型信息
class QXLIB_API kcxCurveTypeInfo{
public:
	kcxCurveTypeInfo();
	kcxCurveTypeInfo(const std::string &szName);

	void						Reset();

	void						Collect(const Handle(Geom_Curve) &aCurve,const TopoDS_Edge &aEdge);
	std::string				ToString();

public:
	std::string				sTypeName_;//类型名称
	int						nTotalNum_;//总个数
	int						nbPeriodic_;//周期个数
	int						nbCrossEnd_;//周期曲线跨起点的次数
	std::vector<kcxCrossRec>	aCrossRecVec_;
};

//统计曲面的类型
class QXLIB_API kcxSurfTypeInfo{
public:
	kcxSurfTypeInfo();
	kcxSurfTypeInfo(const std::string &szName);

	void						Reset();

	void						Collect(const Handle(Geom_Surface) &aSurf,const TopoDS_Face &aFace);

	std::string				ToString();

public:
	std::string				strTypeName_;//类型名称
	int						nCount_;//个数
	int						nbUPeriodic_,nbVPeriodic_,nbUVPeriodic_;//周期曲面的个数
	int						nbUCrossSeam_,nbVCrossSeam_;//裁剪环跨周期边界的个数
	std::vector<kcxCrossRec>	uCrossRecVec_,vCrossRecVec_;
};

class QXLIB_API kcgAnalysis
{
public:
	kcgAnalysis(void);
	~kcgAnalysis(void);

	void						CollectCurveTypeInfo(const TopoDS_Shape &aShape);
	void						CollectSurfTypeInfo(const TopoDS_Shape &aShape);

	void						Clear();

	bool						WriteToFile(const char *pszFile);

protected:
	void						CollectEdgeCurveTypeInfo(const TopoDS_Edge &aEdge);
	void						GetCurveTypeName(const Handle(Geom_Curve) &aCurve,std::string &sTypeName,std::string &sTypeFixName);

	kcxCurveTypeInfo*			GetCurveTypeInfo(const std::string &sName);

protected:
	void						CollectSurfTypeInfo(const Handle(Geom_Surface) &aSurf,const TopoDS_Face &aFace);
	void						GetSurfTypeName(const Handle(Geom_Surface) &aSurf,std::string &sTypeName,std::string &sTypeFixName);

	kcxSurfTypeInfo*			GetSurfTypeInfo(const std::string &sName);

protected:
	stdCurveTypeInfoMap		aCurveTypeInfoMap_;
	stdSurfTypeInfoMap		aSurfTypeInfoMap_;
};


