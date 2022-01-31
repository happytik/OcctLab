#include "stdafx.h"
#include <gp_Pnt.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include "QxBaseDef.h"
#include "QxModelLib.h"
#include "kcDocContext.h"
#include "QxLuaScript.h"
#include "QxLuaState.h"
#include "sciModelLib.h"

//获取当前的model对象
static kcModel* GetCurrentModel()
{
	kcDocContext *pDocCtx = GetDocContext();
	if (pDocCtx) {
		return pDocCtx->GetModel();
	}
	return NULL;
}

// 向模型添加一条直线
static bool sciAddLine(double p1[3],double p2[3])
{
	kPoint3 pnt1(p1),pnt2(p2);
	double dist = pnt1.distance(pnt2);
	if(dist < 1e-8)
		return false;

	kcModel *pModel = GetCurrentModel();
	if (!pModel)
		return false;

	gp_Pnt gp1(p1[0],p1[1],p1[2]),gp2(p2[0],p2[1],p2[2]);
	BRepBuilderAPI_MakeEdge me(gp1,gp2);
	if(me.IsDone()){
		TopoDS_Edge aEdge = me.Edge();

		kcEntityFactory ef;
		kcEntity *pEntity = ef.Create(aEdge);
		if(pEntity){
			pModel->AddEntity(pEntity);
		}
		return true;
	}

	return false;
}

// 传入两个点，添加一条直线对象到model中。
// 返回bool值
static int model_add_line(lua_State *L)
{
	double p1[3],p2[3];
	int ix;
	int n = lua_gettop(L);
	if(n == 2){
		luaL_checktype(L,-2,LUA_TTABLE);
		luaL_checktype(L,-1,LUA_TTABLE);
		for(ix = 0;ix < 3;ix ++){
			lua_rawgeti(L,-2,ix + 1);
			p1[ix] = luaL_checknumber(L,-1);
			lua_pop(L,1);

			lua_rawgeti(L,-1,ix + 1);
			p2[ix] = luaL_checknumber(L,-1);
			lua_pop(L,1);
		}
		//使用两个点
		if (sciAddLine(p1, p2)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}

	lua_pushboolean(L, 0);
	return 1;
}

// 传入一系列点,以一个tab表示
static int model_poly_line(lua_State *L)
{
	gp_Pnt *paPnt = NULL;
	int ix,jx,nps = 0;
	double pnt[3];
	int top = lua_gettop(L);
	if(top == 1){
		luaL_checktype(L,-1,LUA_TTABLE);
		nps = (int)luaL_len(L,-1);
		if(nps >= 2){
			paPnt = new gp_Pnt[nps];
			for(ix = 0;ix < nps;ix ++){
				lua_rawgeti(L,-1,ix + 1);
				luaL_checktype(L,-1,LUA_TTABLE);
				for(jx = 0;jx < 3;jx ++){
					lua_rawgeti(L,-1,jx+1);
					pnt[jx] = luaL_checknumber(L,-1);
					lua_pop(L,1);
				}
				lua_pop(L,1);
				paPnt[ix].SetX(pnt[0]);
				paPnt[ix].SetY(pnt[1]);
				paPnt[ix].SetZ(pnt[2]);
			}
		}
		//创建poly
		BRepBuilderAPI_MakeWire mw;
		for(ix = 1;ix < nps;ix ++){
			BRepBuilderAPI_MakeEdge me(paPnt[ix-1],paPnt[ix]);
			if(me.IsDone()){
				TopoDS_Edge aEdge = me.Edge();
				mw.Add(aEdge);
			}
		}
		if(mw.IsDone()){
			TopoDS_Wire aW = mw.Wire();
			kcEntityFactory kef;
			kcModel *pModel = GetCurrentModel();
			kcEntity *pEntity = kef.Create(aW);
			if(pEntity && pModel){
				pModel->AddEntity(pEntity);
				lua_pushboolean(L, 1);
				return 1;
			}
		}
	}
	lua_pushboolean(L, 0);
	return 1;
}

///传入:tab[,tan,atan]
// tab是点的列表，tan是首尾切矢，atan是每个点的切矢
int model_interp_spline(lua_State *L)
{
	return 0;
}

static int g_undo_flag = 0;

int model_begin_undo(lua_State *L)
{
	if(g_undo_flag)
		luaL_error(L,"has in undo state");

	int top = lua_gettop(L);
	const char *pszName = NULL;
	//kcModel *pModel = glb_GetContext().GetModel();

	//if(top == 1 && lua_type(L,-1) == LUA_TSTRING){
	//	pszName = luaL_checkstring(L,-1);
	//}else{
	//	pszName = "undo1";
	//}

	//pModel->BeginUndo(pszName);
	//g_undo_flag = 1;

	return 0;
}

int model_end_undo(lua_State *L)
{
	if(!g_undo_flag)
		luaL_error(L,"not in undo state");

	//kcModel *pModel = glb_GetContext().GetModel();
	//pModel->EndUndo();

	g_undo_flag = 0;

	return 0;
}

static const struct luaL_Reg modellib[] = {
	{"add_line",model_add_line},
	{"poly_line",model_poly_line},
	//{"interp_spline",model_interp_spline},
	//{"begin_undo",model_begin_undo},
	//{"end_undo",model_end_undo},
	{NULL,NULL}
};

//模块的lua接口函数
int luaopen_model(lua_State *L)
{
	luaL_newlib(L,modellib);
	return 1;
}

//模块的初始化函数
void InitLuaExtModuleLib()
{
	//注册
	QxLuaState::RegisterModule("Model",luaopen_model,"model module");
}