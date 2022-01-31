//
#include "stdafx.h"
#include "QxDbgTool.h"

void	glb_Trace(char *szfmt,...)
{
	char sbuf[4096];//×ã¹»´ó
	va_list va;

	va_start(va,szfmt);
	int len = vsprintf_s(sbuf,szfmt,va);
	va_end(va);
	sbuf[len] = '\0';

	::OutputDebugString(sbuf);
}
