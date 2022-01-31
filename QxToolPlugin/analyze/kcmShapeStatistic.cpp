#include "stdafx.h"
#include <map>
#include "kiOptionSet.h"
#include "kcEntity.h"
#include "kcModel.h"
#include "QxSurfLib.h"
#include "kcgAnalysis.h"
#include "kcmShapeStatistic.h"

kcmShapeStatistic::kcmShapeStatistic()
{
	m_strName = "Shape Statistic";
	m_strAlias = "ShapeStat";
	m_strDesc = "对象统计";

	_pOptionSet = NULL;
}

kcmShapeStatistic::~kcmShapeStatistic()
{
}

int  kcmShapeStatistic::OnExecute()
{
	RequestEditInput("choise a action",_pOptionSet);
	return KSTA_CONTINUE;
}

// 表明命令可以完成,主要和Apply函数结合使用.
BOOL  kcmShapeStatistic::CanFinish()//命令是否可以完成
{
	return TRUE;
}

// 当输入工具获取结果,结束时,会回调对应命令的该函数.
// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//
int  kcmShapeStatistic::OnInputFinished(kiInputTool *pTool)
{
	return kiCommand::OnInputFinished(pTool);
}

	//选项被选中了
void  kcmShapeStatistic::OnOptionSelected(kiOptionItem *pOptItem)
{
	if(pOptItem->GetID() == 1){
		//DoCountSurfInfo();
		DoCollectSurfTypeInfo();
	}
	if(pOptItem->GetID() == 2){
		//AfxMessageBox("curve info");
		DoCollectCurveTypeInfo();
	}
	if(pOptItem->GetID() == 3){
		DoGetOrientStat();
	}
}

void  kcmShapeStatistic::DoCollectSurfTypeInfo()
{
	kcgAnalysis cgAnalysis;
	kcEntityList entList;
	kcEntityList::iterator ite;
	kcEntity *pEntity = NULL;
	kcModel *pModel = GetModel();
	
	CWaitCursor wc;

	pModel->GetAllEntity(entList);
	for(ite = entList.begin();ite != entList.end();ite ++){
		pEntity = *ite;
		TopoDS_Shape aShape = pEntity->GetShape();
		if(!aShape.IsNull()){
			cgAnalysis.CollectSurfTypeInfo(aShape);
		}
	}
	
	const char *pszFile = "d:\\DbgDump\\surftypeinfo.txt";
	if(cgAnalysis.WriteToFile(pszFile)){
		//打开
		::ShellExecute(NULL,"Open",pszFile,NULL,NULL,SW_SHOWNORMAL);
	}
}

void  kcmShapeStatistic::DoCollectCurveTypeInfo()
{
	kcgAnalysis cgAnalysis;
	kcEntityList entList;
	kcEntityList::iterator ite;
	kcEntity *pEntity = NULL;
	kcModel *pModel = GetModel();
	
	CWaitCursor wc;

	pModel->GetAllEntity(entList);
	for(ite = entList.begin();ite != entList.end();ite ++){
		pEntity = *ite;
		TopoDS_Shape aShape = pEntity->GetShape();
		if(!aShape.IsNull()){
			cgAnalysis.CollectCurveTypeInfo(aShape);
		}
	}
	
	const char *pszFile = "d:\\DbgDump\\crvtypeinfo.txt";
	if(cgAnalysis.WriteToFile(pszFile)){
		//打开
		::ShellExecute(NULL,"Open",pszFile,NULL,NULL,SW_SHOWNORMAL);
	}
}

void  kcmShapeStatistic::DoCountSurfInfo()
{
	std::map<int,int> surfCntMap;
	std::map<int,int>::iterator mit;
	kcEntityList entList;
	kcEntityList::iterator ite;
	kcEntity *pEntity = NULL;
	kcModel *pModel = GetModel();

	pModel->GetAllEntity(entList);
	for(ite = entList.begin();ite != entList.end();ite ++){
		pEntity = *ite;
		TopoDS_Shape aShape = pEntity->GetShape();
		if(!aShape.IsNull()){
			if(aShape.ShapeType() > TopAbs_FACE)
				continue;

			TopExp_Explorer topExp;
			for(topExp.Init(aShape,TopAbs_FACE);topExp.More();topExp.Next()){
				TopoDS_Face aFace = TopoDS::Face(topExp.Current());
				TopLoc_Location L;
				Handle(Geom_Surface) aSurf = BRep_Tool::Surface(aFace,L);
				if(!aSurf.IsNull()){
					int idx = kcgSurfNameAndIndex::GeomSurfIndex(aSurf);
					if(idx >= 0){
						mit = surfCntMap.find(idx);
						if(mit == surfCntMap.end()){
							surfCntMap.insert(std::map<int,int>::value_type(idx,1));
						}else{
							(*mit).second ++;
						}
					}
				}
			}
		}
	}

	if(!surfCntMap.empty()){
		CString szMsg,ss;
		for(mit = surfCntMap.begin();mit != surfCntMap.end();mit ++){
			int idx,cnt;
			idx = (*mit).first;
			cnt = (*mit).second;
			ss.Format("%s count is %d.",kcgSurfNameAndIndex::GeomSurfName(idx),cnt);
			szMsg += ss + "\r\n";
		}

		AfxMessageBox(szMsg);
	}
}

// 统计Face和wire反向的情况
void  kcmShapeStatistic::DoGetOrientStat()
{
	int nbRevFace = 0;
	int nbRevWire1 = 0,nbRevWire2 = 0;
	kcEntityList entList;
	kcEntityList::iterator ite;
	kcEntity *pEntity = NULL;
	kcModel *pModel = GetModel();

	pModel->GetAllEntity(entList);
	for(ite = entList.begin();ite != entList.end();ite ++){
		pEntity = *ite;
		TopoDS_Shape aShape = pEntity->GetShape();
		if(!aShape.IsNull()){
			if(aShape.ShapeType() > TopAbs_FACE)
				continue;

			TopExp_Explorer exp;
			exp.Init(aShape,TopAbs_FACE);
			for(;exp.More();exp.Next()){
				const TopoDS_Face &aFace = TopoDS::Face(exp.Current());

				bool bFaceRev = false;
				//face反向个数
				if(aFace.Orientation() == TopAbs_REVERSED){
					bFaceRev = true;
					nbRevFace ++;
				}

				TopoDS_Iterator ite(aFace,false);
				for(;ite.More();ite.Next()){
					const TopoDS_Wire &aW = TopoDS::Wire(ite.Value());

					if(bFaceRev && aW.Orientation() == TopAbs_REVERSED){
						nbRevWire1 ++;
					}
					if(!bFaceRev && aW.Orientation() == TopAbs_REVERSED){
						nbRevWire2 ++;
					}
				}
			}
		}
	}

	CString szMsg,ss;
	szMsg.Format("Reversed Face : %d",nbRevFace);
	ss.Format("\nReversed Face,Reversed Wire to Surf : %d",nbRevWire1); 
	szMsg += ss;
	ss.Format("\nForward Face,Reversed Wire to Surf : %d",nbRevWire2);
	szMsg += ss;

	AfxMessageBox(szMsg);
}

// 创建必要的输入工具,在Initialize中调用，每个命令仅仅调用一次。
BOOL  kcmShapeStatistic::CreateInputTools()
{
	_pOptionSet = new kiOptionSet;
	kiOptionButton *pButton = new kiOptionButton("surface stat",'S',1);
	pButton->SetOptionCallBack(this);
	_pOptionSet->AddOption(pButton);

	pButton = new kiOptionButton("curve stat",'C',2);
	pButton->SetOptionCallBack(this);
	_pOptionSet->AddOption(pButton);

	pButton = new kiOptionButton("orient stat",'O',3);
	pButton->SetOptionCallBack(this);
	_pOptionSet->AddOption(pButton);

	return TRUE;
}

// 销毁创建的输入工具.每个命令仅调用一次.
BOOL  kcmShapeStatistic::DestroyInputTools()
{
	KC_SAFE_DELETE(_pOptionSet);
	return TRUE;
}

// 在begin中调用，主要是初始化工具队列。
BOOL  kcmShapeStatistic::InitInputToolQueue()
{
	return TRUE;
}

