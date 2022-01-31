#include "stdafx.h"
#include "kBase.h"
#include "QxModelLib.h"
#include "QxLuaState.h"
#include "sciTopolLib.h"

class LuaTopoShape{
public:
	LuaTopoShape(TopoDS_Shape ashape){
		_shape = ashape;
	}

public:
	TopoDS_Shape	_shape;
};

int topol_interp_spline(lua_State *L)
{
	return 0;
}

static const struct luaL_Reg topollib[] = {
	{"interp_spline",topol_interp_spline},
	{NULL,NULL}
};

int luaopen_topol(lua_State *L)
{
	luaL_newlib(L,topollib);
	return 1;
}

void InitLuaExtTopolLib()
{
	QxLuaState::RegisterModule("Topol",luaopen_topol,"topol module");
}