//////////////////////////////////////////////////////////////////////////
// 将对象保存到brep文件中，或充brep文件中读取对象
#ifndef _KCG_BREP_FILE_H_
#define _KCG_BREP_FILE_H_

class kcgBRepFile
{
public:
	kcgBRepFile(void);
	~kcgBRepFile(void);

	// 写入对象到文件中,可以选择是否保存三角面片
	BOOL					Write(const TopoDS_Shape& aShape,const char *lpszFile,BOOL bWithTriangles);

	// 从文件中读取对象
	BOOL					Read(TopoDS_Shape& aShape,const char *lpszFile,BOOL bWithTriangles);
};

#endif