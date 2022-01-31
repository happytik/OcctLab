//
#include "stdafx.h"
#include "sciModelLib.h"
#include "kcDocContext.h"
#include "sciTopolLib.h"
#include "QxLuaScript.h"

//记录外部传入的主要对象
static kcDocContext *s_pDocContext = NULL;

bool LuaScriptInit(kcDocContext *pDocCtx)
{
	s_pDocContext = pDocCtx;

	InitLuaExtModuleLib();
	InitLuaExtTopolLib();

	return true;
}

kcDocContext* GetDocContext()
{
	return s_pDocContext;
}