#include "StdAfx.h"
#include "kcEntity.h"
#include "kcLayer.h"
#include "kcBasePlane.h"
#include "QxStgBlock.h"
#include "kcHandleMgr.h"

////////////////////////////////////////////////////////
//
kcHandleMap::kcHandleMap()
{
	_pHandleMap = NULL;
	_nMapSize = 0;
	_nMaxHandle = 0;
	_nIdleHandle = 0;
}

kcHandleMap::~kcHandleMap()
{
	Clear();
}

bool  kcHandleMap::Initialize(int capacity)
{
	//清除旧的
	if(_pHandleMap){
		Clear();
	}

	_nMapSize = capacity;
	if(capacity <= 0){
		_nMapSize = 256;
	}

	int ix;
	_pHandleMap = new BYTE[_nMapSize];
	for(ix = 0;ix < _nMapSize;ix ++)
		_pHandleMap[ix] = 0;
	
	_pHandleMap[0] = 1;//0是无效的Handle，不能分配
	_nIdleHandle = _nMapSize * 8 - 1;//去除0

	return true;
}

void  kcHandleMap::Clear()
{
	if(_pHandleMap){
		delete [] _pHandleMap;
		_pHandleMap = NULL;
	}

	_nMapSize = 0;
	_nMaxHandle = 0;
	_nIdleHandle = 0;
}

// 对n个byte来说，handle对应范围为[0,n*8-1]
// 例如：对mapsize为1，handle为0-255共256个.
//
int  kcHandleMap::MapMaxHandle()
{
	ASSERT(_nMapSize > 0);
	return (_nMapSize * 8 - 1);
}

// 分配一个可用的Handle.
// 分配策略，依次向后分配，如果没有了，再看前面是否有空闲，如果有从
// 空闲中分配。
//
int  kcHandleMap::AllocHandle()
{
	int nHandle = 0;
	int ix = 0;
	BYTE bv = 0;

	//如果没有可用handle，再分配
	if(_nIdleHandle <= 0){
		//分配完了，需要再次分配内存。
		BYTE *pBuf = new BYTE[_nMapSize * 2];
		if(pBuf){
			for(ix = 0;ix < _nMapSize;ix ++)
				pBuf[ix] = _pHandleMap[ix];
			for(ix = 0;ix <_nMapSize;ix ++)
				pBuf[_nMapSize + ix] = 0;

			delete[] _pHandleMap;
			_pHandleMap = pBuf;
			_nIdleHandle = _nMapSize << 8;
			_nMapSize *= 2;
		}
	}

	if(_nIdleHandle > 0){
		if(_nMaxHandle < MapMaxHandle()){
			_nMaxHandle ++;//先加
			nHandle = _nMaxHandle;
			_nIdleHandle --;
			
			//标记
			ix = _nMaxHandle >> 3;// 
			bv = 0x01 << (_nMaxHandle % 8);
			_pHandleMap[ix] |= bv;
		}else{
			//寻找释放的handle。
			for(ix = 0;ix < _nMapSize;ix ++){
				if(_pHandleMap[ix] != 0xFF)
					break;
			}
			nHandle = ix * 8;

			bv = 0x01;
			for(ix = 0;ix < 8;ix ++){
				if((_pHandleMap[ix] & bv) == 0){
					_pHandleMap[ix] |= bv;
					break;
				}
				bv = bv << 1;
			}
			nHandle += ix;
		}
	}

	return nHandle;
}

// 释放一个可用的Handle
void  kcHandleMap::FreeHandle(int handle)
{
	if(!KC_IS_VALID_HANDLE(handle) || handle > _nMaxHandle)
		return;

	int ix = handle >> 3;
	BYTE bv = 0x01 << (handle % 8);

	_pHandleMap[ix] &= ~bv;

	_nIdleHandle ++;
	if(_nMaxHandle == handle)
		_nMaxHandle --;
}

// 标记一个handle已经使用了,要保证内存已经分配足够了。
// 内部不会自动分配内存.
bool  kcHandleMap::MaskHandle(int nHandle)
{
	//这些需要保证
	ASSERT(_nMapSize > 0 && nHandle >0 && nHandle <= MapMaxHandle());

	int ix = nHandle >> 3;// 
	BYTE bv = 0x01 << (nHandle % 8);
	ASSERT(ix < _nMapSize);
	if((_pHandleMap[ix] & bv) == 0){//尚未标记
		_pHandleMap[ix] |= bv;

		if(_nMaxHandle < nHandle)	_nMaxHandle = nHandle;
		_nIdleHandle --;
	}else{
		ASSERT(FALSE);
	}

	return true;
}

bool  kcHandleMap::Read(CArchive& ar)
{
	QxStgBlock blk;

	if(!blk.Read(ar) || blk.GetType() != 0x1123){
		ASSERT(FALSE);
		return false;
	}

	int nMapSize = 0;
	if(blk.GetValueByCode(0,nMapSize)){
		Initialize(nMapSize);
		return true;
	}else{
		ASSERT(FALSE);
	}

	return false;
}

bool  kcHandleMap::Write(CArchive& ar)
{
	QxStgBlock blk;

	blk.Initialize(0x1123);//类型为了验证
	blk.AddCodeValue(0,_nMapSize);
	
	return (blk.Write(ar) ? true : false);
}

////////////////////////////////////////////////////////
//
kcHandleMgr::kcHandleMgr(void)
{
}

kcHandleMgr::~kcHandleMgr(void)
{
	Clear();
}

bool  kcHandleMgr::Initialize()
{
	_layerHandleMap.Initialize();
	_pointHandleMap.Initialize();
	_curveHandleMap.Initialize();
	_wireHandleMap.Initialize();
	_faceHandleMap.Initialize();
	_shellHandleMap.Initialize();
	_solidHandleMap.Initialize();
	_compHandleMap.Initialize();
	_blockHandleMap.Initialize();
	aDiamDimHandleMap_.Initialize();
	aRadDimHandleMap_.Initialize();
	aLenDimHandleMap_.Initialize();
	aAngDimHandleMap_.Initialize();
	_basePlaneHandleMap.Initialize();

	InitHandleMapMap();
	
	return true;
}

// 这里的顺序和保存和读取的应当一致
void  kcHandleMgr::InitHandleMapMap()
{
	_handleMapMap.clear();
	_handleMapArray.clear();

	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_POINT,&_pointHandleMap));
	_handleMapArray.push_back(KCT_ENT_POINT);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_EDGE,&_curveHandleMap));
	_handleMapArray.push_back(KCT_ENT_EDGE);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_WIRE,&_wireHandleMap));
	_handleMapArray.push_back(KCT_ENT_WIRE);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_FACE,&_faceHandleMap));
	_handleMapArray.push_back(KCT_ENT_FACE);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_SHELL,&_shellHandleMap));
	_handleMapArray.push_back(KCT_ENT_SHELL);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_SOLID,&_solidHandleMap));
	_handleMapArray.push_back(KCT_ENT_SOLID);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_COMPOUND,&_compHandleMap));
	_handleMapArray.push_back(KCT_ENT_COMPOUND);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_BLOCK,&_blockHandleMap));
	_handleMapArray.push_back(KCT_ENT_BLOCK);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_DIM_DIAMETER, &aDiamDimHandleMap_));
	_handleMapArray.push_back(KCT_ENT_DIM_DIAMETER);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_DIM_RADIUS, &aRadDimHandleMap_));
	_handleMapArray.push_back(KCT_ENT_DIM_RADIUS);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_DIM_LENGTH, &aLenDimHandleMap_));
	_handleMapArray.push_back(KCT_ENT_DIM_LENGTH);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_DIM_ANGLE, &aAngDimHandleMap_));
	_handleMapArray.push_back(KCT_ENT_DIM_ANGLE);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_ENT_LAYER,&_layerHandleMap));
	_handleMapArray.push_back(KCT_ENT_LAYER);
	_handleMapMap.insert(stdHandleMapMap::value_type(KCT_BASE_PLANE,&_basePlaneHandleMap));
	_handleMapArray.push_back(KCT_BASE_PLANE);
}

void  kcHandleMgr::Clear()
{
	kcHandleMap *pHandleMap = NULL;
	stdHandleMapMap::iterator ite;
	for(ite = _handleMapMap.begin();ite != _handleMapMap.end();ite ++){
		pHandleMap = (*ite).second;
		pHandleMap->Clear();
	}

	_handleMapArray.clear();
	_handleMapMap.clear();

	//重新初始化
	Initialize();
}

kcHandleMap*  kcHandleMgr::FindHandleMap(int entType)
{
	stdHandleMapMap::iterator ite = _handleMapMap.find(entType);
	if(ite != _handleMapMap.end())
		return (*ite).second;
	return NULL;
}

//根据类型，分配和释放Handle
int  kcHandleMgr::AllocHandle(int entType)
{
	int nHandle = 0;
	kcHandleMap *pHandleMap = FindHandleMap(entType);
	ASSERT(pHandleMap);
	if(pHandleMap){
		nHandle = pHandleMap->AllocHandle();
	}

	return nHandle;
}

void  kcHandleMgr::FreeHandle(int entType,int handle)
{
	kcHandleMap *pHandleMap = FindHandleMap(entType);
	ASSERT(pHandleMap);
	if(pHandleMap){
		pHandleMap->FreeHandle(handle);
	}
}

// 标记一个handle已经使用了
bool  kcHandleMgr::MaskHandle(int entType,int handle)
{
	kcHandleMap *pHandleMap = FindHandleMap(entType);
	ASSERT(pHandleMap);
	if(pHandleMap){
		pHandleMap->MaskHandle(handle);
		return true;
	}
	return false;
}

//注意：这里，保存和读取，都要保证顺序
// 改变顺序，增删map，都会导致文件格式改变
bool  kcHandleMgr::Read(CArchive& ar)
{
	std::vector<int>::size_type ix,isize = _handleMapArray.size();
	kcHandleMap *pHandleMap = NULL;
	
	for(ix = 0;ix < isize;ix ++){
		pHandleMap = FindHandleMap(_handleMapArray[ix]);
		ASSERT(pHandleMap);
		pHandleMap->Read(ar);
	}

	return true;
}

bool  kcHandleMgr::Write(CArchive& ar)
{
	std::vector<int>::size_type ix,isize = _handleMapArray.size();
	kcHandleMap *pHandleMap = NULL;
	
	for(ix = 0;ix < isize;ix ++){
		pHandleMap = FindHandleMap(_handleMapArray[ix]);
		ASSERT(pHandleMap);
		pHandleMap->Write(ar);
	}

	return true;
}