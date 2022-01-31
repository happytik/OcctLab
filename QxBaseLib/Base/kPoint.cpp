#include "StdAfx.h"
#include "kPoint.h"

//////////////////////////////////////////////////////////////////////////
//
void	kPoint2::set(double *pv)
{
	ASSERT(pv);
	_a[0] = pv[0];_a[1] = pv[1];
}

//////////////////////////////////////////////////////////////////////////
//
void	kPoint3::set(double *pv)
{
	ASSERT(pv);
	_a[0] = pv[0];_a[1] = pv[1];_a[2] = pv[2];
}

void	kPoint3::get(double *pv)
{
	ASSERT(pv);
	pv[0] = _a[0];pv[1] = _a[1];pv[2] = _a[2];
}

//////////////////////////////////////////////////////////////////////////
//
kPoint::kPoint(void)
{
	kPoint2 p2(3,4);
	kPoint2 p22 = p2;
	p22[0] = 4;

	kPoint3 p3(5.0);
	kPoint3 p31 = p3;
	p31[1] = 6.0;
	kPoint3 p32 = p31;
	p32[2] = 7.0;
}

kPoint::~kPoint(void)
{
}
