#include "StdAfx.h"
#include <math.h>
#include "QxBaseDef.h"
#include "kColor.h"

inline static double axRound01(double a)
{
	if(a < 0.0) return 0.0;
	if(a > 1.0) return 1.0;
	return a;
}

kColor::kColor(void)
{
	_r = _g = _b = 0.0;
	_a = 1.0;
}

kColor::kColor(int icolor)
{
	set(icolor);
}

kColor::kColor(unsigned char a)
{
	set(a);
}

kColor::kColor(unsigned char r,unsigned char g,unsigned char b)
{
	set(r,g,b);
}

kColor::kColor(double a)
{
	set(a);
}

kColor::kColor(double r,double g,double b)
{
	set(r,g,b);
}

kColor::kColor(const kColor& other)
{
	_r = other._r;_g = other._g;_b = other._b;_a = other._a;
}

kColor::~kColor(void)
{
}

kColor&		kColor::operator=(const kColor& other)
{
	_r = other._r;_g = other._g;_b = other._b;_a = other._a;
	return *this;
}

bool  kColor::operator==(const kColor& other)
{
	if(fabs(_r - other._r) > KDBL_EPSILON ||
		fabs(_g - other._g) > KDBL_EPSILON ||
		fabs(_b - other._b) > KDBL_EPSILON ||
		fabs(_a - other._a) > KDBL_EPSILON)
		return false;
	return true;
}

bool  kColor::operator!=(const kColor& other)
{
	if(fabs(_r - other._r) > KDBL_EPSILON ||
		fabs(_g - other._g) > KDBL_EPSILON ||
		fabs(_b - other._b) > KDBL_EPSILON ||
		fabs(_a - other._a) > KDBL_EPSILON)
		return true;
	return false;
}

void  kColor::set(int icolor)
{
	_r = GetRValue(icolor) / 255.0;
	_g = GetGValue(icolor) / 255.0;
	_b = GetBValue(icolor) / 255.0;
	_a = 1.0;
}

void	kColor::set(unsigned char a)
{
	_r = _g = _b = a / 255.0;
	_a = 1.0;
}

void	kColor::set(unsigned char r,unsigned char g,unsigned char b)
{
	_r = r / 255.0;_g = g / 255.0;_b = b / 255.0;
	_a = 1.0;
}

void	kColor::set(double a)
{
	_r = _g = _b = axRound01(a);
	_a = 1.0;
}

void	kColor::set(double r,double g,double b)
{
	_r = axRound01(r);_g = axRound01(g);_b = axRound01(b);
	_a = 1.0;
}

int		kColor::rgb() const
{
	unsigned char r,g,b;
	r = (unsigned char)(_r * 255);
	g = (unsigned char)(_g * 255);
	b = (unsigned char)(_b * 255);
	return RGB(r,g,b);
}

