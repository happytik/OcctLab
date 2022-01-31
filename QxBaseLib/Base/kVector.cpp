#include "StdAfx.h"
#include "kVector.h"

//////////////////////////////////////////////////////////////////////////
//
void	kVector3::set(double *pv)
{
	ASSERT(pv);
	_a[0] = pv[0];_a[1] = pv[1];_a[2] = pv[2];
}

void	kVector3::get(double *pv)
{
	ASSERT(pv);
	pv[0] = _a[0];pv[1] = _a[1];pv[2] = _a[2];
}

bool	 kVector3::normalize()
{
	double len = length(),dinv = 0.0;
	if(len < KDBL_EPSILON)
	{
		_a[0] = _a[1] = _a[2] = 0.0;
		return false;
	}
	dinv = 1.0 / len;
	_a[0] *= dinv;
	_a[1] *= dinv;
	_a[2] *= dinv;

	return true;
}

//¼ÆËã½Ç¶È
double	kVector3::AngleWithRef(const kVector3& todir,const kVector3& refdir)
{
	double angle = 0.0;
	kVector3 crv = *this ^ todir;
	double cosinus = *this * todir;
	double sinus = crv.length();
	if (cosinus > -0.70710678118655 && cosinus < 0.70710678118655)
		angle = acos(cosinus);
	else
	{
		if(cosinus < 0.0)  angle = K_PI - asin (sinus);
		else               angle = asin (sinus);
	}
	if (crv * refdir >= 0.0)  
		return  angle;
	else
		return -angle;
}

//////////////////////////////////////////////////////////////////////////
//
kVector::kVector(void)
{
}

kVector::~kVector(void)
{
}
