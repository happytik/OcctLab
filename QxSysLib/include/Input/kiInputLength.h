//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_INPUT_LENGTH_H_
#define _KI_INPUT_LENGTH_H_

#include "kPoint.h"
#include "kVector.h"
#include "kiInputTool.h"

class kcPreviewLine;
class kcPreviewText;

class QXLIB_API kiInputLength : public kiInputTool
{
public:
	// 用于命令中的构造函数。
	kiInputLength(kiCommand *pCommand,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	// 用于InputTool中的构造函数。
	kiInputLength(kiInputTool *pParent,const char *pszPrompt,kiOptionSet *pOptionSet = NULL);
	virtual ~kiInputLength(void);

	void						Reset();

	// bRealtimeUpdate为true时，可以及时获取长度值，并更新预览
	// 当不设定方向时，取的是基准面上的方向
	void						Set(double basePnt[3],bool bRealtimeUpdate = false);
	void						Set(const  kPoint3& basePnt,bool bRealtimeUpdate = false);

	// 设定基准点和方向，计算该方向上的长度，并显示预览
	void						Set(double basePnt[3],double baseDir[3],bool bRealtimeUpdate = false);
	void						Set(const kPoint3& aBasePnt,const kVector3& aBaseDir,bool bRealtimeUpdate = false);

	// 对baseDir进行反向
	void						ReverseBaseDir();

	// 返回长度值，可能是负值.
	double					GetLength() const { return dLength_; };
	void						SetDefault(double dLen) { dDefaultLength_ = dLength_ = dLen; }
	kPoint3					GetEndPoint() const { return _endPoint; }

protected:
	//
	virtual int				OnBegin();
	virtual int				OnEnd();

protected:
	// 获取提示字符串
	virtual void				DoGetPromptText(std::string& str);
	// 出入文本完成，通常命令可以结束了
	virtual int				DoTextInputFinished(const char *pInputText);

public:
	//鼠标消息
	virtual int				OnLButtonUp(kuiMouseInput& mouseInput); 
	virtual int				OnMouseMove(kuiMouseInput& mouseInput);
	virtual int				OnRButtonUp(kuiMouseInput& mouseInput);

protected:
	//
	void					CalcCurrPoint(kuiMouseInput& mouseInput,kPoint3& kpnt);

protected:
	double					dLength_;//长度
	double					dDefaultLength_;//
	kPoint3					_endPoint;
	double					aBasePoint_[3];//基准点
	double					aBaseDir_[3];//基准方向，沿此方向的距离
	bool					bHasBaseDir_;//是否沿基准方向
	bool					bRealtimeUpdate_;//是否实时更新关联变量的值。

protected:
	kcPreviewLine			*pPreviewLine_;
	kcPreviewText			*pPreviewText_;
};

#endif