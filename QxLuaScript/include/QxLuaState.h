#pragma once

#include "lua.hpp"
#include "QxLibDef.h"

//
class QXLIB_API QxLuaState {
public:
	QxLuaState();
	~QxLuaState();

	// 编译脚本，检查是否有错误.并不执行脚本
	bool				Compile(const char *pszScript);

	//执行脚本,脚本以文本文件的形式传入
	bool				Execute(const char *pszScript);

	//执行一个文件中的脚本.szFile为文件名称
	bool				ExecuteFile(const std::string &szFile);

	const char*			GetErrorMsg() const;

public:
	//注册一个自定义模型
	static bool			RegisterModule(const char *modname, lua_CFunction openf, const char *desc);
	
protected:
	bool				Init();
	void				Close();

	void				DoGetErrorMsg();

protected:
	lua_State			*luaState_;
	std::string			szError_; //错误信息
};