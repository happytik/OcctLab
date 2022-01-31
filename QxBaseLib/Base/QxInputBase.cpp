//
#include "stdafx.h"
#include "QxInputBase.h"

//////////////////////////////////////////////////////////////////////////
kuiMouseInput::kuiMouseInput(void)
{
	m_nFlags = 0;
	m_x = m_y = 0;
}

kuiMouseInput::kuiMouseInput(UINT nFlags, LONG x,LONG y,const Handle(V3d_View) &aView)
{
	m_nFlags = nFlags;
	m_x = x;m_y = y;
	aView_ = aView;
}

kuiMouseInput::kuiMouseInput(const kuiMouseInput &other)
{
	Assign(other);
}

kuiMouseInput& kuiMouseInput::operator=(const kuiMouseInput &other)
{
	Assign(other);
	return *this;
}

void kuiMouseInput::Assign(const kuiMouseInput &other)
{
	m_nFlags = other.m_nFlags;
	m_x = other.m_x;
	m_y = other.m_y;
	aView_ = other.aView_;
}

//////////////////////////////////////////////////////////////////////////
//
kuiKeyInput::kuiKeyInput()
{
	nChar_ = 0;
	nRepCnt_ = 0;
	nFlags_ = 0;
}

kuiKeyInput::kuiKeyInput(UINT nChar,UINT nRepCnt,UINT nFlags)
{
	nChar_ = nChar;
	nRepCnt_ = nRepCnt;
	nFlags_ = nFlags;
}

//////////////////////////////////////////////////////////////////////////
//
void		kGetAxisDir(eStdAxis axis,double v[3])
{
	v[0] = v[1] = v[2] = 0.0;
	switch(axis){
	case Z_POS:	v[2] = 1.0; break;
	case Z_NEG:	v[2] = -1.0; break;
	case X_POS:	v[0] = 1.0; break;
	case X_NEG:	v[0] = -1.0; break;
	case Y_POS:	v[1] = 1.0; break;
	case Y_NEG:	v[1] = -1.0; break;
	default:
		break;
	}
}