//
#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include "QxLuaState.h"

class QxModuleReg;
class QxLuaRegistry;
typedef std::vector<QxModuleReg *> QxModuleRegArray;

//管理注册的模块
class QxModuleReg {
public:
	QxModuleReg(const char *modname, lua_CFunction openFunc, const char *desc);
	~QxModuleReg();

public:
	std::string			sModuleName_;//模块名称
	lua_CFunction		pOpenFunc_;
	std::string			sDesc_;//模块简单描述
};

QxModuleReg::QxModuleReg(const char *modname, lua_CFunction openFunc, const char *desc)
{
	ASSERT(modname && openFunc && desc);
	sModuleName_ = modname;
	pOpenFunc_ = openFunc;
	sDesc_ = desc;
}

QxModuleReg::~QxModuleReg()
{

}

///////////////////////////////////////////////////////////////////////////////
//
class QxLuaRegistry {
public:
	QxLuaRegistry();
	~QxLuaRegistry();

	//注册一个扩展模块,该函数不依赖lua环境。可以在lua_State对象不存在情况下，调用
	//信息记录在该对象内，后面在创建lua_State时，再添加到lua_State对象中
	bool					RegisterModule(const char *modname, lua_CFunction openf, const char *desc);

	// 将记录的module，添加到lua_State中。
	void					InitExtModules(lua_State *L);

protected:
	void					Clear();

protected:
	QxModuleRegArray		aModuleRegArray_;
};

QxLuaRegistry::QxLuaRegistry()
{
}

QxLuaRegistry::~QxLuaRegistry()
{
	Clear();
}

//注册一个扩展模块,该函数不依赖lua环境。可以在lua_State对象不存在情况下，调用
//信息记录在该对象内，后面在创建lua_State时，再添加到lua_State对象中
bool QxLuaRegistry::RegisterModule(const char *modname, lua_CFunction openf, const char *desc)
{
	if (!modname || !openf || !desc) {
		ASSERT(FALSE);
		return false;
	}
	
	//判断模型名称是否已经存在了
	std::string sModName = modname;
	QxModuleRegArray::size_type ix, isize = aModuleRegArray_.size();

	for (ix = 0; ix < isize; ix++) {
		//这里不考虑大小写，避免出现例如：Os和os这样容易混淆的模块名称
		std::string::size_type slen = aModuleRegArray_[ix]->sModuleName_.size();
		if (sModName.size() == slen) {
			//这里忽略大小写，以防止类似：Sys、sys这样的模块同时存在
			if (_stricmp(modname, aModuleRegArray_[ix]->sModuleName_.c_str()) == 0) {
				TRACE("\n module name %s has existed.", modname);
				ASSERT(FALSE);
				return false;
			}
		}
	}

	QxModuleReg *pModReg = new QxModuleReg(modname, openf, desc);
	aModuleRegArray_.push_back(pModReg);

	return true;
}

// 将记录的module，添加到lua_State中。
void QxLuaRegistry::InitExtModules(lua_State *L)
{
	ASSERT(L);
	if (aModuleRegArray_.empty())
		return;

	//注册扩展模块
	QxModuleRegArray::size_type ix, isize = aModuleRegArray_.size();
	for (ix = 0; ix < isize; ix++) {
		luaL_requiref(L, aModuleRegArray_[ix]->sModuleName_.c_str(),
			aModuleRegArray_[ix]->pOpenFunc_, 1);
		//module table is on top
		lua_pop(L, 1);//pop module object
	}
}

void QxLuaRegistry::Clear()
{
	QxModuleRegArray::size_type ix, isize = aModuleRegArray_.size();
	for (ix = 0; ix < isize; ix++) {
		delete aModuleRegArray_[ix];
	}
	aModuleRegArray_.clear();
}

///////////////////////////////////////////////////////////////////////////////
// 全局变量，保持
static QxLuaRegistry s_luaReqistry;

///////////////////////////////////////////////////////////////////////////////
//

QxLuaState::QxLuaState()
{
	luaState_ = NULL;

	Init();
}

QxLuaState::~QxLuaState()
{
	Close();
}

bool QxLuaState::Init()
{
	if (luaState_ != NULL)
		return true;

	luaState_ = luaL_newstate();
	if (luaState_ == NULL) {
		szError_ = "new state failed.";
		return false;
	}
	//
	luaL_openlibs(luaState_);
	s_luaReqistry.InitExtModules(luaState_);

	return true;
}

void QxLuaState::Close()
{
	if (luaState_ != NULL) {
		lua_close(luaState_);
		luaState_ = NULL;
	}
	szError_.clear();
}

// 编译脚本，检查是否有错误.并不执行脚本
bool QxLuaState::Compile(const char *pszScript)
{
	ASSERT(luaState_ != NULL);
	if (!pszScript || pszScript[0] == '\0') {
		return false;
	}

	// Loads a Lua chunk without running it. 
	// If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack. 
	// Otherwise, it pushes an error message.
	int rc = luaL_loadstring(luaState_, pszScript);
	if (rc != LUA_OK) {
		DoGetErrorMsg();
		return false;
	}
	lua_pop(luaState_, 1);//pop compiled chunk

	return true;
}

// 执行脚本
//
bool QxLuaState::Execute(const char *pszScript)
{
	ASSERT(luaState_ != NULL);
	if (!pszScript || pszScript[0] == '\0') {
		ASSERT(FALSE);
		return false;
	}

	//编译
	int rc = luaL_loadstring(luaState_, pszScript);
	if (rc != LUA_OK) {
		DoGetErrorMsg();
		return false;
	}

	//执行
	rc = lua_pcall(luaState_, 0, LUA_MULTRET, 0);
	if (rc != LUA_OK) {
		DoGetErrorMsg();
		return false;
	}

	return true;
}

// 执行一个脚本文件
//
bool QxLuaState::ExecuteFile(const std::string &szFile)
{
	ASSERT(luaState_ != NULL);
	if (szFile.empty()) {
		ASSERT(FALSE);
		return false;
	}

	//加载
	std::string strEmtpy("");
	int rc = luaL_loadfilex(luaState_, szFile.c_str(), NULL);
	if (rc != LUA_OK) {
		DoGetErrorMsg();
		return false;
	}

	//执行
	rc = lua_pcall(luaState_, 0, LUA_MULTRET, 0);
	if (rc != LUA_OK) {
		DoGetErrorMsg();
		return false;
	}

	return true;
}

const char* QxLuaState::GetErrorMsg() const
{
	if (szError_.empty()) return NULL;
	return szError_.c_str();
}

void QxLuaState::DoGetErrorMsg()
{
	//具体的错误信息
	if (lua_isstring(luaState_, -1)) {
		const char *pszMsg = lua_tostring(luaState_, -1);
		if (pszMsg == NULL) {
			szError_ = "Null error";
		}else {
			szError_ = pszMsg;
		}
	}else{
		szError_ = "Unknown error.";
	}
	lua_pop(luaState_, 1);//pop err message
}

//注册一个自定义模型
bool QxLuaState::RegisterModule(const char *modname, lua_CFunction openf, const char *desc)
{
	return s_luaReqistry.RegisterModule(modname, openf, desc);
}

