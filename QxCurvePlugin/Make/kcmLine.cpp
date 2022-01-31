#include "StdAfx.h"
#include <AIS_Shape.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include "kcModel.h"
#include "kcEntity.h"
#include "kcBasePlane.h"
#include "kcPreviewObj.h"
#include "kcPreviewText.h"
#include "kiInputPointTool.h"
#include "kcgTextLib.h"
#include "kiCommandMgr.h"
#include "kcDispArrow.h"
#include "kcmLine.h"

#define CSTA_LINE_POINT1  0
#define CSTA_LINE_POINT2  1

kcmLine::kcmLine(void)
{
	m_strName = "Line";
	m_strAlias = "lin";
	m_strDesc = "绘制单一直线.";

	_pInputStartPoint = NULL;
	_pInputEndPoint = NULL;

	_pPreviewLine = NULL;

	_bMultiLine = true;
	_bMakeWire = false;

	m_dLen = 1.0;

	m_nState = CSTA_LINE_POINT1;
}

kcmLine::~kcmLine(void)
{
	KC_SAFE_DELETE(_pPreviewLine);
}

int  kcmLine::OnExecute()
{
	m_nState = CSTA_LINE_POINT1;
	_pointList.clear();
	_prevLineList.clear();

	return KSTA_CONTINUE;
}

int  kcmLine::OnEnd(int nCode)
{
	KC_SAFE_DELETE(_pPreviewLine);
	std::vector<kcPreviewLine *>::size_type ix,isize = _prevLineList.size();
	for(ix = 0;ix < isize;ix ++){
		delete _prevLineList[ix];
	}
	_prevLineList.clear();

	return nCode;
}

BOOL	kcmLine::CanFinish()//命令是否可以完成
{
	if(!_pInputStartPoint || !_pInputEndPoint)
		return FALSE;

	if(m_nState == CSTA_LINE_POINT2)
		return TRUE;

	return FALSE;
}

BOOL  kcmLine::MakeLineEntities()
{
	int ix,isize = (int)_pointList.size();
	if(isize < 2){
		return FALSE;
	}

	kPoint3 p1,p2;
	gp_Pnt pnt1,pnt2;
	kcEntityList entList;
	BRepBuilderAPI_MakeWire mkWire;

	p1 = _pointList[0];
	for(ix = 1;ix < isize;ix ++){
		p2 = _pointList[ix];
		if(p1.distance(p2) < KC_MIN_LEN)
			continue;

		pnt1.SetCoord(p1.x(),p1.y(),p1.z());
		pnt2.SetCoord(p2.x(),p2.y(),p2.z());

		if(_bMakeWire){
			BRepBuilderAPI_MakeEdge me(pnt1,pnt2);
			if(me.IsDone()){
				TopoDS_Edge aEdge = me.Edge();
				mkWire.Add(aEdge);
			}else{
				return FALSE;
			}
		}else{
			gp_Vec vd(pnt1,pnt2);
			double len = vd.Magnitude();

			Handle(Geom_Line) hLin = new Geom_Line(pnt1,gp_Dir(vd));
			Handle(Geom_Curve) hCur = new Geom_TrimmedCurve(hLin,0.0,len);
			kcEdgeEntity *pEntity = new kcEdgeEntity;
			pEntity->SetCurve(hCur);

			entList.push_back(pEntity);
		}

		p1 = p2;
	}

	if(_bMakeWire){
		if(mkWire.IsDone()){
			TopoDS_Wire aWire = mkWire.Wire();

			kcWireEntity *pWireEntity = new kcWireEntity;
			pWireEntity->SetWire(aWire);

			entList.push_back(pWireEntity);
		}else{
			return FALSE;
		}
	}

	if(!entList.empty()){
		kcModel *pModel = GetModel();
		pModel->BeginUndo(GetName());
		
		kcEntityList::iterator ite;
		for(ite = entList.begin();ite != entList.end();ite ++){
			pModel->AddEntity(*ite);
		}
		pModel->EndUndo();

		Redraw();
	}
	
	return TRUE;
}

//输入工具结束时
int  kcmLine::OnInputFinished(kiInputTool *pTool)
{
	if(pTool == _pInputStartPoint){
		AddTempSnapPoint(_startPoint[0],_startPoint[1],_startPoint[2]);
		_pointList.push_back(_startPoint);

		m_nState = CSTA_LINE_POINT2;
		NavToNextTool();

	}else if(pTool == _pInputEndPoint){
		if(_pInputEndPoint->IsInputDone()){
			_pointList.push_back(_endPoint);

			if(_pPreviewLine){
				_prevLineList.push_back(_pPreviewLine);
				_pPreviewLine = NULL;
			}

			if(_bMultiLine){
				_startPoint = _endPoint;

				AddTempSnapPoint(_endPoint[0],_endPoint[1],_endPoint[2]);
				NavToInputTool(_pInputEndPoint);
			}else{
				ClearTempSnapPoint();

				MakeLineEntities();
				//结束命令
				Done();
				return KSTA_DONE;
			}
		}
	}

	return KSTA_CONTINUE;
}

int    kcmLine::OnInputChanged(kiInputTool *pTool)
{
	if(pTool == _pInputEndPoint){
		if(m_nState == CSTA_LINE_POINT2){
			gp_Pnt pnt,pnt1;
			pnt.SetCoord(_endPoint[0],_endPoint[1],_endPoint[2]);
			pnt1.SetCoord(_startPoint[0],_startPoint[1],_startPoint[2]);

			if(_pPreviewLine == NULL){
				_pPreviewLine = new kcPreviewLine(GetAISContext(),GetDocContext());
				_pPreviewLine->SetColor(1.0,0.0,0.0);
			}

			if(_pPreviewLine){
				_pPreviewLine->Update(pnt1,pnt);
				_pPreviewLine->Display(TRUE);
				if(_pPreviewLine->IsDisplayed())
					KTRACE("display..\n");			
			}

			Redraw();
		}
	}

	return kiCommand::OnInputChanged(pTool);
}

// 创建必要的输入工具
BOOL  kcmLine::CreateInputTools()
{
	m_optionSet.AddBoolOption("连续曲线",'M',_bMultiLine);
	m_optionSet.AddBoolOption("组成环",'W',_bMakeWire);

	_pInputStartPoint = new kiInputPointTool(this,"输入起点:");
	_pInputStartPoint->Initialize(&_startPoint);

	_pInputEndPoint = new kiInputPointTool(this,"输入终点",&m_optionSet);
	_pInputEndPoint->Initialize(&_endPoint);
	_pInputEndPoint->SetRealtimeUpdate(true);
		
	return TRUE;
}

BOOL  kcmLine::DestroyInputTools()
{
	m_optionSet.Clear();
	KC_SAFE_DELETE(_pInputStartPoint);
	KC_SAFE_DELETE(_pInputEndPoint);

	return TRUE;
}

BOOL  kcmLine::InitInputToolQueue()
{
	ASSERT(_pInputStartPoint);
	ASSERT(_pInputEndPoint);

	AddInputTool(_pInputStartPoint);
	AddInputTool(_pInputEndPoint);

	return TRUE;
}

int  kcmLine::OnMouseMove(kuiMouseInput& mouseInput)
{
	kiCommand::OnMouseMove(mouseInput);

	return kiCommand::OnMouseMove(mouseInput);
}

int  kcmLine::OnLButtonUp(kuiMouseInput& mouseInput)
{
	kiCommand::OnLButtonUp(mouseInput);

	return KSTA_CONTINUE;
}

int  kcmLine::OnRButtonUp(kuiMouseInput& mouseInput)
{
	if(m_nState == CSTA_LINE_POINT2 && _pointList.size() >= 2){
		MakeLineEntities();

		Done();
		return KSTA_DONE;
	}

	return KSTA_CONTINUE;
}