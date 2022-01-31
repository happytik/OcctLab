#include "StdAfx.h"
#include "kiSnapMngr.h"

//////////////////////////////////////////////////////////////////////////
//
kiSnapPoint::kiSnapPoint()
{
	_scx = _scy = -1000;
}

kiSnapPoint::kiSnapPoint(double x,double y,double z)
{
	_scx = _scy = -1000;
	_point.set(x,y,z);
}

kiSnapPoint::kiSnapPoint(const kiSnapPoint& other)
{
	*this = other;
}

kiSnapPoint&	kiSnapPoint::operator=(const kiSnapPoint& other)
{
	_point = other._point;
	_scx = other._scx;
	_scy = other._scy;

	return *this;
}

