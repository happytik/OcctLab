//
#ifndef _QX_LUA_SCRIPT_H_
#define _QX_LUA_SCRIPT_H_

#include "QxLibDef.h"

class kcDocContext;

//初始化
QXLIB_API bool LuaScriptInit(kcDocContext *pDocCtx);

// 显示脚本对话框
QXLIB_API bool ShowLuaScriptDialog(bool bShow);

kcDocContext* GetDocContext();

#endif