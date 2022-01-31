//////////////////////////////////////////////////////////////////////////
// 文本相关的功能。
//
#ifndef _KCG_TEXTLIB_H_
#define _KCG_TEXTLIB_H_

#include <string>
#include <vector>
#include "kPoint.h"
#include "QxLibDef.h"

class CFont;

// 获取文字轮廓类,根据字体信息，获取对应的文字轮廓信息。
// 仅仅获取轮廓信息，不进行格式化，也不根据轮廓进行进一步的操作。
// 每个文字返回的都是一系列的Wire。
//
class QXLIB_API kcGlyphOutline{
public:
	kcGlyphOutline(HDC hDC,LPLOGFONT lf);
	~kcGlyphOutline();

	BOOL						IsValid() const;

	// 返回一个组合对象,每个对象是Compound对象。
	//
	bool						GetStringOutLine(LPCTSTR lpszStr,TopTools_SequenceOfShape& seqOfShape);

	// 获取一个字符的轮廓对象。
	TopoDS_Compound				GetCharOutLine(char c);

	//获取一个字符的轮廓对象
	TopoDS_Compound				GetCharOutLine(WCHAR wc);

	CFont*						GetFont() const { return m_pFont; }

protected:
	TopoDS_Edge					BuildLine(const POINTFX& pfx);
	TopoDS_Edge					BuildQSpline(double x1,double y1,double x2,double y2);
	TopoDS_Edge					BuildCSpline(double x1,double y1,double x2,double y2,double x3,double y3);
	void						AddPoint(double x,double y);

	//将fixed值转为double
	double						FixedToDouble(const FIXED& fv);

protected:
	HDC							m_hDC;//
	LPLOGFONT					m_pLogFont;
	CFont						*m_pFont;
	HGDIOBJ						m_hOldFont;
	WCHAR						m_wcText;	
	std::vector<kPoint2>		m_pntArray;
};

//////////////////////////////////////////////////////////////////////////
//

class QXLIB_API kcFontText{
public:
	kcFontText(LOGFONT& lf);
	~kcFontText();

	// 修改字体设置
	void						SetLogFont(LOGFONT& lf);

	// 获取字符串对应的文字
	// 传入：
	//   hDC:设备局部 lpszText：字符串 dHeight：文字高度 dGap：文字间空白 
	//   bWire:获取轮廓还是曲面
	// 传出：aList：对象的列表。
	//       对象是格式化好的文字，包围盒左下角在0,0,0处.
	//
	BOOL						GetText(HDC hDC,LPCTSTR lpszText,double dHeight,double dGap,BOOL bWire,TopTools_ListOfShape& aList);							

protected:
	//
	TopoDS_Compound				CreateFaceFromWire(const TopoDS_Compound& aOutLine);

protected:
	LOGFONT						m_logFont;
};

class kcText{
public:
	kcText(HDC hDC,LPLOGFONT lf);
	~kcText();

	//主要的方法，获取字符串格式化后的轮廓或face
	//bool						GetText(LPCTSTR lpszText,)

protected:
	kcGlyphOutline				m_aGlyphOutline;
};

//////////////////////////////////////////////////////////////////////////
//
class QXLIB_API kxCharFontText{
public:
	kxCharFontText(unsigned char b);
	~kxCharFontText();

public:
	unsigned char				m_uChar;
	TopoDS_Compound				m_aOutLine;
	TopoDS_Compound				m_aFace;
	Bnd_Box						m_aBndBox;//包围盒
	double						m_dWidth;
	double						m_dHeight;
};

typedef std::vector<kxCharFontText *>	TCharFontTextList;

class QXLIB_API kcCharText{
public:
	kcCharText();
	~kcCharText();

	bool						Initialize(HDC hDC,LPLOGFONT lf);
	void						Destroy();
	bool						IsValid() const;

	//获取单个字符的轮廓
	TopoDS_Compound				GetOutline(char c,double dHeight);
	//获取单个字符的face
	TopoDS_Compound				GetFace(char c,double dHeight);

	//获取单个字符的轮廓或face
	bool						GetText(const char *pStr,double dHeight,double dGap,bool bFace,TopTools_ListOfShape& aList);

protected:
	//
	TopoDS_Compound				CreateFaceFromWire(const TopoDS_Compound& aOutLine);

protected:
	kxCharFontText				*m_pCharText[256];//记录每个字符对应的文本信息
};

QXLIB_API kcCharText&		glb_GetArialCharText();
QXLIB_API kcCharText&		glb_GetCourierCharText();

QXLIB_API bool			glb_InitArialCharText(HDC hDC);
QXLIB_API bool			glb_InitCourierCharText(HDC hDC);



#endif