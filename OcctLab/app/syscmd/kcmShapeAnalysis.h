//////////////////////////////////////////////////////////////////////////
//
#ifndef _KCM_SHAPE_ANALYSIS_H_
#define _KCM_SHAPE_ANALYSIS_H_

#include <vector>
#include <string>
#include "kColor.h"
#include "kiCommand.h"
#include "kiOptionSet.h"

class kiInputEntityTool;
class kcDispFrameArrow;
class kcDispSolidArrow;
class kcPreviewLine;
class kcPreviewNurbsPoles;
class kcPreviewSegments;
class kcPreviewMesh;
class kcPreviewCurve;

class kcmShapeAnalysisNormal : public kiCommand
{
public:
	kcmShapeAnalysisNormal(void);
	virtual ~kcmShapeAnalysisNormal(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL		CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	//选项被选中了
	virtual void			OnOptionSelected(kiOptionItem *pOptItem);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					CalcNormal();

	void					ClearArrowDisp();

protected:
	kiInputEntityTool		*m_pInputEntity;
	kiOptionSet			m_optionSet;
	bool					m_bTopoNormal;

	std::vector<kcDispSolidArrow *>	m_arrowList;
};

// 分析曲面的法失和裁剪环的方向
//
class kcmShapeAnalysisFaceOrient : public kiCommand
{
public:
	kcmShapeAnalysisFaceOrient(void);
	virtual ~kcmShapeAnalysisFaceOrient(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					ShowFaceWiresAndNorm();
	BOOL					ShowWiresAndNorm(const TopoDS_Face& aFace);

	void					ClearArrowDisp();

protected:
	kiInputEntityTool		*m_pInputEntity;
	kiInputEntityTool		*m_pInputFace;

	kiOptionSet			m_optionSet;
	bool					m_bTopoNormal;//拓扑法失
	bool					m_bUseWireOrit;//是否考虑上环的方向
	bool					m_bCorrectWireOrder;//是否修正环的方向

	std::vector<kcDispSolidArrow *>	  m_arrowList;
};

//显示曲面的详细信息
class kiFacePropInfo : public kiCommand
{
public:
	kiFacePropInfo();
	virtual ~kiFacePropInfo();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					ShowFacePropInfo();

protected:
	kiInputEntityTool		*m_pInputFace;

	kiOptionSet			m_optionSet;
	bool					bCumOri_;//是否考虑父对象方向
};

//显示曲面的裁剪环，考虑或不考虑face方向.考虑或不考虑edge方向.
class kiFaceWireOri : public kiCommand
{
public:
	kiFaceWireOri();
	virtual ~kiFaceWireOri();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					ShowFaceWireOri();

	void					BuildSurfNormArrow(const TopoDS_Face& aFace);
	void					BuildDispArrow(const TopoDS_Edge& aE,const TopoDS_Face& aF,bool bCumEdgeOri);
	void					ClearArrowDisp();

	kcDispSolidArrow*		BuildDispArrow(const gp_Pnt& pnt,const gp_Vec& dire,double arrlen,const kColor color);

protected:
	kiInputEntityTool		*m_pInputFace;

	kiOptionSet			m_optionSet;
	bool					bCumFaceOri_;//是否考虑face方向
	bool					bCumEdgeOri_;//是否考虑Edge方向

	std::vector<kcDispSolidArrow *> sldArrowVec_;//箭头显示对象
};

//显示控制点
class kiDisplayNbsPoles : public kiCommand{
public:
	kiDisplayNbsPoles();
	~kiDisplayNbsPoles();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

	bool					DoShowPoles();

protected:
	kiInputEntityTool		*pPickEntTool_;
	kcPreviewNurbsPoles	*pPreviewPoles_;
};

//批量分析功能
class kiBatAnalyze : public kiCommand{
public:
	kiBatAnalyze();
	virtual ~kiBatAnalyze();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

protected:
	virtual int				OnExecute();
	virtual int				OnEnd(int nCode);

protected:
	bool					GetStepFileList(const CString &szFolder);

protected:
	std::string				strPath_;
	std::vector<std::string>	stepFileList_;//文件列表
};

//曲线曲率显示
class kcmCurveCurvature : public kiCommand{
public:
	kcmCurveCurvature();
	~kcmCurveCurvature();

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL			CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	//选项改变了
	virtual void			OnOptionChanged(kiOptionItem *pOptItem);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);
	
protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

	bool					DoShowCurvature();
	bool					DoBuildDispData();

protected:
	kiInputEntityTool		*pPickEntTool_;
	kcPreviewSegments		*pPreviewSegs_;

	kiOptionSet			aOptionSet_;

	int					nbSamp_;//采样点个数
	double				dScale_;//缩放比例

private:
	std::vector<TopoDS_Shape>	aShapeAry_;
};

//剖分网格显示
class kcmMeshDisplay : public kiCommand{
public:
	kcmMeshDisplay();
	~kcmMeshDisplay();

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	bool					DoDispMesh();

protected:
	kiInputEntityTool		*pPickEntTool_;
	kcPreviewMesh			*pPreivewMesh_;

	kiOptionSet			aOptionSet_;	
};

//曲面方向显示
class kcmFaceOrientAnalysis : public kiCommand{
public:
	kcmFaceOrientAnalysis();
	~kcmFaceOrientAnalysis();

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	// 当输入项改变时，会调用该回调函数
	virtual  int			OnInputChanged(kiInputTool *pTool);

	//选项被选中了
	virtual void			OnOptionSelected(kiOptionItem *pOptItem);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	//
	void					ShowUVOrient();

protected:
	kiInputEntityTool		*pPickFaceTool_;
	kiOptionSet			aOptionSet_;

	kcPreviewCurve		*pUIsoPreviewCurve_;
	kcPreviewCurve		*pVIsoPreviewCurve_;
	kcDispSolidArrow		*pUIsoCrvArrow_;
	kcDispSolidArrow		*pVIsoCrvArrow_;
};

class kcmFaceNormalAnalysis : public kiCommand
{
public:
	kcmFaceNormalAnalysis(void);
	virtual ~kcmFaceNormalAnalysis(void);

	// 表明命令可以完成,主要和Apply函数结合使用.
	virtual  BOOL		CanFinish();//命令是否可以完成

	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
	//
	virtual  int			OnInputFinished(kiInputTool *pTool);

	//选项被选中了
	virtual void			OnOptionSelected(kiOptionItem *pOptItem);

protected:
	virtual int			OnExecute();
	virtual int			OnEnd(int nCode);

protected:
	// 创建必要的输入工具
	virtual	BOOL			CreateInputTools();
	virtual BOOL			DestroyInputTools();
	virtual BOOL			InitInputToolQueue();

protected:
	BOOL					DisplayNormal();

protected:
	kiInputEntityTool		*m_pInputEntity;
	kiOptionSet			m_optionSet;
	bool					m_bTopoNormal;

	kcDispSolidArrow		*pNormalArrow_;//预览箭头
};

////曲线曲率显示
//class kcmCurveCurvature : public kiCommand{
//public:
//	kcmCurveCurvature();
//	~kcmCurveCurvature();
//
//	// 表明命令可以完成,主要和Apply函数结合使用.
//	virtual  BOOL			CanFinish();//命令是否可以完成
//
//	// 当输入工具获取结果,结束时,会回调对应命令的该函数.
//	// 用于继续执行命令或切换输入工具.也可能返回KSTA_DONE,表明命令执行结束.
//	//
//	virtual  int			OnInputFinished(kiInputTool *pTool);
//
//protected:
//	virtual int			OnExecute();
//	virtual int			OnEnd(int nCode);
//
//protected:
//	// 创建必要的输入工具
//	virtual	BOOL			CreateInputTools();
//	virtual BOOL			DestroyInputTools();
//	virtual BOOL			InitInputToolQueue();
//
//	bool					DoShowCurvature();
//
//protected:
//	kiInputEntityTool		*pPickEntTool_;
//	kcPreviewSegments		*pPreviewSegs_;
//
//	kiOptionSet			aOptionSet_;
//
//	int					nbSamp_;//采样点个数
//	double				dScale_;//缩放比例
//};

#endif