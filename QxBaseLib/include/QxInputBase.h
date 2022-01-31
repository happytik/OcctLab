//
#ifndef _QX_INPUT_BASE_H_
#define _QX_INPUT_BASE_H_

#include <math.h>
#include "V3d_View.hxx"
#include "QxLibDef.h"

//判断t是否在[a,b]区间内，带容差
#ifndef OLB_IN_RANGE
#define OLB_IN_RANGE(t,a,b,tol) ((t > a - tol) && (t < b + tol))
#endif

// 封装鼠标和键盘的输入信息。
class QXLIB_API kuiMouseInput
{
public:
	kuiMouseInput(void);
	kuiMouseInput(UINT nFlags,LONG x,LONG y,const Handle(V3d_View) &aView);
	kuiMouseInput(const kuiMouseInput &other);

	kuiMouseInput& operator=(const kuiMouseInput &other);

protected:
	void				Assign(const kuiMouseInput &other);

public:
	UINT		m_nFlags;
	LONG		m_x,m_y;//
	Handle(V3d_View) aView_;//所在的view类
};

class QXLIB_API kuiKeyInput
{
public:
	kuiKeyInput();
	kuiKeyInput(UINT nChar,UINT nRepCnt,UINT nFlags);

public:
	UINT		nChar_;
	UINT		nRepCnt_;
	UINT		nFlags_; 
};

// 标准坐标系轴
typedef enum e_std_axis{
	Z_POS = 0,
	Z_NEG,
	X_POS,
	X_NEG,
	Y_POS,
	Y_NEG
}eStdAxis;

QXLIB_API void  kGetAxisDir(eStdAxis axis,double v[3]);



#endif