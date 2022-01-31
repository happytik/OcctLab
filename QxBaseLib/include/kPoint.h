//////////////////////////////////////////////////////////////////////////
// 点
#ifndef _K_POINT_H_
#define _K_POINT_H_

#include "QxLibDef.h"
#include "QxBaseDef.h"

//2D点
class QXLIB_API kPoint2{
public:
	kPoint2();
	kPoint2(double x,double y);
	kPoint2(double *pv);

	void			set(double x,double y);
	void			set(double *pv);

	const double	x() const;
	const double    y() const;

	const double	operator[](int i) const;
	double&			operator[](int i);

	double			distance(const kPoint2& p) const;

private:
	double			_a[2];
};

inline kPoint2::kPoint2()
{
	_a[0] = _a[1] = 0.0;
}

inline kPoint2::kPoint2(double x,double y)
{
	set(x,y);
}

inline kPoint2::kPoint2(double *pv)
{
	set(pv);
}

inline void	kPoint2::set(double x,double y)
{
	_a[0] = x;_a[1] = y;
}

inline const double kPoint2::x() const
{
	return _a[0];
}

inline const double kPoint2::y() const
{
	return _a[1];
}

inline const double	kPoint2::operator[](int i) const
{
	ASSERT(i >= 0 && i < 2);
	return _a[i];
}

inline double&	 kPoint2::operator[](int i)
{
	ASSERT(i >= 0 && i < 2);
	return _a[i];
}

inline double	 kPoint2::distance(const kPoint2& p) const
{
	double dist = (_a[0] - p[0]) * (_a[0] - p[0]) + (_a[1] - p[1]) * (_a[1] - p[1]);
	return ::sqrt(dist);
}

//////////////////////////////////////////////////////////////////////////
//

class QXLIB_API kPoint3{
public:
	kPoint3();
	kPoint3(double v);
	kPoint3(double x,double y,double z);
	kPoint3(double *pv);

	void			set(double a);
	void			set(double x,double y,double z);
	void			get(double& x,double& y,double& z);

	void			set(double *pv);
	void			get(double *pv);

	const double	x() const;
	const double    y() const;
	const double    z() const;

	const double	operator[](int i) const;
	double&			operator[](int i);

	const kPoint3	operator+(const kPoint3& p) const;
	void			operator+=(const kPoint3& p);
	const kPoint3	operator*(double a) const;
	void			operator*=(double a);
	const kPoint3	operator/(double a) const;
	void			operator/=(double a);
	double			distance(const kPoint3& p) const;

protected:
	double			_a[3];
};

inline kPoint3::kPoint3()
{
	_a[0] = _a[1] = _a[2] = 0.0;
}

inline kPoint3::kPoint3(double v)
{
	_a[0] = _a[1] = _a[2] = v;
}

inline kPoint3::kPoint3(double x,double y,double z)
{
	_a[0] = x;_a[1] = y;_a[2] = z;
}

inline kPoint3::kPoint3(double *pv)
{
	set(pv);
}

inline void		kPoint3::set(double a)
{
	_a[0] = _a[1] = _a[2] = a;
}

inline void	kPoint3::set(double x,double y,double z)
{
	_a[0] = x;_a[1] = y;_a[2] = z;
}

inline void	kPoint3::get(double& x,double& y,double& z)
{
	x = _a[0];y = _a[1];z = _a[2];
}

inline const double	kPoint3::x() const
{
	return _a[0];
}

inline const double    kPoint3::y() const
{
	return _a[1];
}

inline const double    kPoint3::z() const
{
	return _a[2];
}

inline const double	kPoint3::operator[](int i) const
{
	ASSERT(i >= 0 && i < 3);
	return _a[i];
}

inline double&	kPoint3::operator[](int i)
{
	ASSERT(i >= 0 && i < 3);
	return _a[i];
}

inline const kPoint3  kPoint3::operator+(const kPoint3& p) const
{
	kPoint3 pt;
	pt[0] = _a[0] + p[0];
	pt[1] = _a[1] + p[1];
	pt[2] = _a[2] + p[2];

	return pt;
}

inline void	kPoint3::operator+=(const kPoint3& p) 
{
	_a[0] += p[0];_a[1] += p[1];_a[2] += p[2];
}

inline const kPoint3  kPoint3::operator*(double a) const
{
	kPoint3 pt;
	pt[0] = _a[0] * a;
	pt[1] = _a[1] * a;
	pt[2] = _a[2] * a;
	return pt;
}

inline void		kPoint3::operator*=(double a)
{
	_a[0] *= a;_a[1] *= a;_a[2] *= a;
}

inline const kPoint3 	kPoint3::operator/(double a) const
{
	kPoint3 pt;
	if(fabs(a) < KDBL_MIN)
	{
		ASSERT(0);
		return pt;
	}

	double ar = 1.0 / a;
	pt[0] = _a[0] * ar;
	pt[1] = _a[1] * ar;
	pt[2] = _a[2] * ar;
	return pt;
}

inline void		kPoint3::operator/=(double a)
{
	if(fabs(a) < KDBL_MIN)
	{
		ASSERT(0);
		return;
	}
	double ar = 1.0 / a;
	_a[0] *= ar;_a[1] *= ar;_a[2] *= ar;
}
inline double	kPoint3::distance(const kPoint3& p) const
{
	double dist = (_a[0] - p[0]) * (_a[0] - p[0]);
	dist += (_a[1] - p[1]) * (_a[1] - p[1]);
	dist += (_a[2] - p[2]) * (_a[2] - p[2]);
	return ::sqrt(dist);
}

//////////////////////////////////////////////////////////////////////////
// 先不添加。
//class kPoint4{
//
//};

class kPoint
{
public:
	kPoint(void);
	~kPoint(void);
};


#endif