#include "StdAfx.h"
#include <fstream>
#include <BRepTools_ShapeSet.hxx>
#include "kcgBrepFile.h"

kcgBRepFile::kcgBRepFile(void)
{
}

kcgBRepFile::~kcgBRepFile(void)
{
}

// 写入对象到文件中,可以选择是否保存三角面片
// 和BRepTools::Write一致，但允许指定是否保存三角面片信息。
BOOL	kcgBRepFile::Write(const TopoDS_Shape& aShape,const char *lpszFile,BOOL bWithTriangles)
{
	std::filebuf fbuf;
	std::ostream aStream(&fbuf);
	if(!fbuf.open(lpszFile,std::ios::out)){
		return FALSE;
	}
	
	BRepTools::Write(aShape,aStream);
	
	return TRUE;
}

// 从文件中读取对象
BOOL	kcgBRepFile::Read(TopoDS_Shape& aShape,const char *lpszFile,BOOL bWithTriangles)
{
	std::filebuf fic;
	std::istream in(&fic);
	if (!fic.open(lpszFile, std::ios::in)) 
		return FALSE;

	BRep_Builder B;
	BRepTools::Read(aShape,in,B);
    
	return TRUE;
}