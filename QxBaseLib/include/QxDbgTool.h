//
#ifndef _QX_DBG_TOOL_H_
#define _QX_DBG_TOOL_H_

#include "QxLibDef.h"

#ifndef KTRACE
#ifdef _DEBUG
#define KTRACE		glb_Trace
#else
#define KTRACE
#endif
#endif

QXLIB_API void	glb_Trace(char *szfmt,...);

#endif