#include "StdAfx.h"
#include <string>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include "kcLayer.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "kcShapeAttribMgr.h"
#include "kcEntityFactory.h"
#include "kcBaseReader.h"

kcBaseReader::kcBaseReader(kcModel *pModel)
:m_pModel(pModel)
{
	//ASSERT(pModel);
}

kcBaseReader::~kcBaseReader(void)
{
	m_aShapeList.Clear();
}


// 将导入对象加入model
BOOL	kcBaseReader::AddShapeToModel()
{
	if(!m_pModel || m_aShapeList.IsEmpty()){
		ASSERT(FALSE);
		return FALSE;
	}

	//创建一个新图层，将新建对象添加到新图层中
	kcLayer *pLayer = m_pModel->CreateNewLayer();
	if(!pLayer)
		return FALSE;

	TopTools_ListIteratorOfListOfShape lls;
	for(lls.Initialize(m_aShapeList);lls.More();lls.Next()){
		TopoDS_Shape aS = lls.Value();
		AddEntity(aS,pLayer);
	}

	return TRUE;
}

// 将导入对象添加到model中。
bool	kcBaseReader::AddEntity(const TopoDS_Shape& aShape,kcLayer *pLayer)
{
	ASSERT(m_pModel != NULL);
	if(aShape.IsNull())
		return false;

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aShape);
	if(!pEntity)
		return false;

	Quantity_Color aCol;
	if(m_pModel->GetShapeAttribMgr()->FindColorAttrib(aShape,aCol)){
		pEntity->SetColor(aCol.Red(),aCol.Green(),aCol.Blue(),TRUE);
	}

	m_pModel->AddEntity(pEntity,pLayer);

	return true;
}

bool  kcBaseReader::AddEntity(const TopoDS_Shape& aShape,Quantity_Color &aCol)
{
	ASSERT(m_pModel != NULL);
	if(aShape.IsNull())
		return false;

	kcEntityFactory efac;
	kcEntity *pEntity = efac.Create(aShape);
	if(!pEntity)
		return false;

	pEntity->SetColor(aCol.Red(),aCol.Green(),aCol.Blue(),TRUE);
	m_pModel->AddEntity(pEntity,TRUE);

	return true;
}

//将路径名称重新编码为utf-8，OCCT680需要这样
int   EncodeUtf8FilePath(const char *pFileName,char *pwcFileName,int nSize)
{
	wchar_t wcFileName[1024];
	int wLen = 0,nLen;

	wLen = MultiByteToWideChar(936,0,pFileName,-1,NULL,0);
	if(wLen > 0 && wLen < 1024){
		MultiByteToWideChar(936,0,pFileName,-1,wcFileName,wLen + 1);//转为unicode
		nLen = WideCharToMultiByte(CP_UTF8,0,wcFileName,wLen,NULL,0,NULL,NULL);
		if(nLen < nSize){
			WideCharToMultiByte(CP_UTF8,0,wcFileName,wLen,pwcFileName,nLen,NULL,NULL);
			return 1;
		}
	}
	return 0;
}

int   EncodeFilePath(const char *pFileName,TCollection_AsciiString& ascStr)
{
	wchar_t wcFileName[1024];
	int wLen = 0,nLen;

	wLen = MultiByteToWideChar(CP_ACP,0,pFileName,-1,NULL,0);
	if(wLen > 0 && wLen < 1024){
		MultiByteToWideChar(CP_ACP,0,pFileName,-1,wcFileName,wLen + 1);//转为unicode
		TCollection_AsciiString tmpStr((const Standard_WideChar*)wcFileName);
		ascStr.Copy(tmpStr);

		return 1;
	}

	return 0;
}