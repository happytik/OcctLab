//////////////////////////////////////////////////////////////////////////
// 一个简单的color类，方便记录颜色
#ifndef _K_COLOR_H_
#define _K_COLOR_H_

#include "QxLibDef.h"

class QXLIB_API kColor
{
public:
	kColor(void);
	kColor(int icolor);
	kColor(unsigned char a);
	kColor(unsigned char r,unsigned char g,unsigned char b);
	kColor(double a);
	kColor(double r,double g,double b);
	kColor(const kColor& other);
	~kColor(void);

	kColor&				operator=(const kColor& other);
	bool					operator==(const kColor& other);
	bool					operator!=(const kColor& other);

	void					set(int icolor);
	void					set(unsigned char a);
	void					set(unsigned char r,unsigned char g,unsigned char b);
	void					set(double a);
	void					set(double r,double g,double b);

	double					r() const { return _r;}
	double					g() const { return _g;}
	double					b() const { return _b;}
	double					a() const { return _a;}
	int						rgb() const;

protected:
	double					_r,_g,_b,_a;
};

#endif