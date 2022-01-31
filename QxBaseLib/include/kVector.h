//////////////////////////////////////////////////////////////////////////
//
#ifndef _K_VECTOR_H_
#define _K_VECTOR_H_

#include "QxLibDef.h"
#include "kPoint.h"

//////////////////////////////////////////////////////////////////////////
//
class QXLIB_API kVector2{
public:
	kVector2();
	kVector2(double x,double y);

	void				set(double x,double y);
	
	const double		x() const;
	const double		y() const;

	const double		operator[](int i) const;
	double&				operator[](int i);

private:
	double				_a[2];
};

//////////////////////////////////////////////////////////////////////////
// 
class QXLIB_API kVector3{
public:
	kVector3();
	kVector3(double a);
	kVector3(double x,double y,double z);
	kVector3(double v[3]);
	kVector3(const kPoint3& p1,const kPoint3& p2);//p2 - p1。

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

	kVector3&		operator  = (const kVector3& v);
	kVector3&		operator  = (double a);
	kVector3&		operator += (const kVector3& v);
	kVector3&		operator += (double a);
	kVector3&		operator -= (const kVector3& v);
	kVector3&		operator -= (double a);
	kVector3&		operator *= (double a);
	kVector3&		operator /= (double a);
	bool			operator == (const kVector3& v) const;
	bool			operator != (const kVector3& v) const;

	double			operator *  (const kVector3& v) const;// 点乘
	kVector3		operator ^  (const kVector3& v) const;// 叉乘

	bool			iszero() const;
	double			length() const;
	bool			normalize();
	void			reverse();

	//计算角度,需要都是单位向量,角度在-PI到PI之间.
	double			AngleWithRef(const kVector3& todir,const kVector3& refdir);

private:
	double				_a[3];
};

kVector3 operator - (const kVector3& v);			    // -v1
kVector3 operator + (const kVector3& v1, const kVector3& v2);	    // v1 + v2
kVector3 operator - (const kVector3& v1, const kVector3& v2);	    // v1 - v2
kVector3 operator * (const kVector3& v, double a);	    // v1 * 3.0
kVector3 operator * (double a, const kVector3& v);	    // 3.0 * v1
kVector3 operator / (const kVector3& v, double a);	    // v1 / 3.0
kVector3  vecmin(const kVector3& v1, const kVector3& v2);
kVector3  vecmax(const kVector3& v1, const kVector3& v2);

kPoint3	 operator + (const kPoint3& p,const kVector3& v);
kPoint3	 operator - (const kPoint3& p,const kVector3& v);
kVector3 operator - (const kPoint3& p1,const kPoint3& p2);//p1 - p2。

inline kVector3::kVector3()
{
	_a[0] = _a[1] = _a[2] = 0.0;
}

inline kVector3::kVector3(double a)
{
	_a[0] = _a[1] = _a[2] = a;
}

inline kVector3::kVector3(double x,double y,double z)
{
	_a[0] = x;_a[1] = y;_a[2] = z;
}

inline kVector3::kVector3(double v[3])
{
	_a[0] = v[0];_a[1] = v[1];_a[2] = v[2];
}

inline kVector3::kVector3(const kPoint3& p1,const kPoint3& p2)//p2 - p1。
{
	_a[0] = p2[0] - p1[0];
	_a[1] = p2[1] - p1[1];
	_a[2] = p2[2] - p1[2];
}

inline void	kVector3::set(double a)
{
	_a[0] = _a[1] = _a[2] = a;
}

inline void	kVector3::set(double x,double y,double z)
{
	_a[0] = x;_a[1] = y;_a[2] = z;
}

inline void	kVector3::get(double& x,double& y,double& z)
{
	x = _a[0];y = _a[1];z = _a[2];
}

inline const double	 kVector3::x() const
{
	return _a[0];
}

inline const double  kVector3::y() const
{
	return _a[1];
}

inline const double  kVector3::z() const
{
	return _a[2];
}

inline const double	kVector3::operator[](int i) const
{
	ASSERT(i >= 0 && i < 3);
	return _a[i];
}

inline double&	kVector3::operator[](int i)
{
	ASSERT(i >= 0 && i < 3);
	return _a[i];
}

inline kVector3&	kVector3::operator  = (const kVector3& v)
{
	if(this == &v)
		return *this;

	_a[0] = v[0];_a[1] = v[1];_a[2] = v[2];
	return *this;
}

inline kVector3&	kVector3::operator  = (double a)
{
	_a[0] = _a[1] = _a[2] = 0.0;
	return *this;
}

inline kVector3&	kVector3::operator += (const kVector3& v)
{
	_a[0] += v[0];_a[1] += v[1];_a[2] += v[2];
	return *this;
}

inline kVector3&	kVector3::operator += (double a)
{
	_a[0] += a;_a[1] += a;_a[2] += a;
	return *this;
}

inline kVector3&	kVector3::operator -= (const kVector3& v)
{
	_a[0] -= v[0];_a[1] -= v[1];_a[2] -= v[2];
	return *this;
}

inline kVector3&	kVector3::operator -= (double a)
{
	_a[0] -= a;_a[1] -= a;_a[2] -= a;
	return *this;
}

inline kVector3&	kVector3::operator *= (double a)
{
	_a[0] *= a;_a[1] *= a;_a[2] *= a;
	return *this;
}

inline kVector3&	kVector3::operator /= (double a)
{
	if(fabs(a) < KDBL_EPSILON)
	{
		ASSERT(0);
		return *this;
	}
	_a[0] /= a;_a[1] /= a;_a[2] /= a;
	return *this;
}

inline bool		kVector3::operator == (const kVector3& v) const
{
	double dx = fabs(_a[0] - v[0]);
	double dy = fabs(_a[1] - v[1]);
	double dz = fabs(_a[2] - v[2]);
	double eps = KDBL_EPSILON * 10.0;
	if(dx > eps || dy > eps || dz > eps)
		return false;

	double diff = ::sqrt(dx * dx + dy * dy + dz * dz);
	if(diff < KDBL_EPSILON)
		return true;
	return false;
}

inline bool		kVector3::operator != (const kVector3& v) const
{
	double dx = fabs(_a[0] - v[0]);
	double dy = fabs(_a[1] - v[1]);
	double dz = fabs(_a[2] - v[2]);
	double eps = KDBL_EPSILON * 10.0;
	if(dx > eps || dy > eps || dz > eps)
		return true;

	double diff = ::sqrt(dx * dx + dy * dy + dz * dz);
	if(diff < KDBL_EPSILON)
		return false;
	return true;
}

// 点乘
inline double		kVector3::operator *  (const kVector3& v) const
{
	return _a[0] * v[0] + _a[1] * v[1] + _a[2] * v[2];
}

// 叉乘
inline kVector3		kVector3::operator ^  (const kVector3& v) const
{
	return kVector3(_a[1] * v[2] - _a[2] * v[1],
					_a[2] * v[0] - _a[0] * v[2],
					_a[0] * v[1] - _a[1] * v[0]);
}

inline bool		kVector3::iszero() const
{
	double eps = KDBL_EPSILON * 2.0;
	if(fabs(_a[0]) > eps || fabs(_a[1]) > eps ||  fabs(_a[2]) > eps)
	   return false;
	double sv = ::sqrt(_a[0] * _a[0] + _a[1] * _a[1] + _a[2] * _a[2]);
	if(sv < KDBL_EPSILON)
		return true;
	return false;
}

inline double	kVector3::length() const
{
	return ::sqrt(_a[0] * _a[0] + _a[1] * _a[1] + _a[2] * _a[2]);
}

inline void		kVector3::reverse()
{
	_a[0] = -_a[0];_a[1] = -_a[1];_a[2] = -_a[2];
}

//////////////////////////////////////////////////////////////////////////
inline kVector3 operator - (const kVector3& v)
{
	return kVector3(-v[0],-v[1],-v[2]);
}

inline kVector3 operator + (const kVector3& v1, const kVector3& v2)	    // v1 + v2
{
	return kVector3(v1[0] + v2[0],v1[1] + v2[1],v1[2] + v2[2]);
}

inline kVector3 operator - (const kVector3& v1, const kVector3& v2)	    // v1 - v2
{
	return kVector3(v1[0] - v2[0],v1[1] - v2[1],v1[2] - v2[2]);
}

inline kVector3 operator * (const kVector3& v, double a)	    // v1 * 3.0
{
	return kVector3(v[0] * a,v[1] * a,v[2] * a);
}

inline kVector3 operator * (double a, const kVector3& v)	    // 3.0 * v1
{
	return kVector3(v[0] * a,v[1] * a,v[2] * a);
}

inline kVector3 operator / (const kVector3& v, double a)	    // v1 / 3.0
{
	if(fabs(a) < KDBL_EPSILON)
	{
		ASSERT(0);
		return kVector3(KDBL_MAX,KDBL_MAX,KDBL_MAX);
	}
	return kVector3(v[0] / a,v[1] / a,v[2] / a);
}

inline kVector3  vecmin(const kVector3& v1, const kVector3& v2)
{
	return kVector3(v1[0] < v2[0] ? v1[0] : v2[0],
					v1[1] < v2[1] ? v1[1] : v2[1],
					v1[2] < v2[2] ? v1[2] : v2[2]);
}

inline kVector3  vecmax(const kVector3& v1, const kVector3& v2)
{
	return kVector3(v1[0] > v2[0] ? v1[0] : v2[0],
					v1[1] > v2[1] ? v1[1] : v2[1],
					v1[2] > v2[2] ? v1[2] : v2[2]);
}

inline kPoint3	 operator + (const kPoint3& p,const kVector3& v)
{
	return kPoint3(p[0] + v[0],p[1] + v[1],p[2] + v[2]);
}

inline kPoint3	 operator - (const kPoint3& p,const kVector3& v)
{
	return kPoint3(p[0] - v[0],p[1] - v[1],p[2] - v[2]);
}

inline kVector3  operator - (const kPoint3& p1,const kPoint3& p2)//p1 - p2。
{
	return kVector3(p2,p1);
}

//////////////////////////////////////////////////////////////////////////

class kVector
{
public:
	kVector(void);
	~kVector(void);
};

#endif
