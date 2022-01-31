//////////////////////////////////////////////////////////////////////////
//
#include "StdAfx.h"
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>
#include <Transfer_TransientProcess.hxx>
#include <StepData_StepModel.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <TDF_Tool.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDataStd_Name.hxx>
#include <NCollection_UtfIterator.hxx>
#include <STEPConstruct_Styles.hxx>
#include <StepVisual_StyledItem.hxx>
#include <StepRepr_Representation.hxx>
#include <StepShape_AdvancedFace.hxx>
#include <XSControl_TransferReader.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TransferBRep.hxx>
#include <TopTools_MapOfShape.hxx>
#include <StepVisual_Colour.hxx>
#include <time.h>
#include "resource.h"
#include "QxBaseDef.h"
#include "kcLayer.h"
#include "kcModel.h"
#include "kcEntity.h"
#include "XCAFDocDialog.h"
#include "kcEntityFactory.h"
#include "kcShapeAttribMgr.h"
#include "kcStepLayerMgr.h"
#include "kcSTEPReader.h"

static void uxDumpStepModel(const STEPControl_Reader &aReader);

kcSTEPReader::kcSTEPReader(kcModel *pModel)
	:kcBaseReader(pModel)
{
	pShapeAttribMgr_ = NULL;
}

kcSTEPReader::~kcSTEPReader()
{
	KC_SAFE_DELETE(pShapeAttribMgr_);
}

//导入
BOOL	kcSTEPReader::DoImport()
{
	if(!m_pModel)
		return false;

	CFileDialog dlg(TRUE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"STEP Files (*.step , *.stp)|*.step; *.stp||", 
		NULL );

	if(dlg.DoModal() == IDOK){
		CString szFile = dlg.GetPathName();
		m_aShapeList.Clear();

		CWaitCursor wc;
		//char utf8Char[1024];
		//EncodeUtf8FilePath(szFile,utf8Char,1024);
		TCollection_AsciiString ascStr;
		EncodeFilePath(szFile,ascStr);

		//ReadSTEP((const char *)szFile);
		//ReadSTEP(utf8Char);
		ReadSTEP(ascStr.ToCString());

		this->AddShapeToModel();
	}

	return TRUE;
}

//读取一个step文件
bool  kcSTEPReader::ReadStepFile(const char *pszFile)
{
	if(pszFile == NULL){
		return false;
	}

	TCollection_AsciiString ascStr;
	EncodeFilePath(pszFile,ascStr);

	IFSelect_ReturnStatus rs = ReadSTEP(ascStr.ToCString());
	if(rs != IFSelect_RetDone)
		return false;
	return true;
}


// 读取一个step文件
IFSelect_ReturnStatus  kcSTEPReader::ReadSTEP(const Standard_CString& aFileName)
{
	// create additional log file
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(aFileName);
	if (status != IFSelect_RetDone)
		return status;

	aReader.WS()->MapReader()->SetTraceLevel(2); // increase default trace level

	Standard_Boolean failsonly = Standard_False;
	aReader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

	// Root transfers
	Standard_Integer nbr = aReader.NbRootsForTransfer();
	aReader.PrintCheckTransfer (failsonly, IFSelect_ItemsByEntity);
	for ( Standard_Integer n = 1; n<=nbr; n++) {
		Standard_Boolean ok = aReader.TransferRoot(n);
	}

	// Collecting resulting entities
	Standard_Integer nbs = aReader.NbShapes();
	if (nbs == 0) {
		return IFSelect_RetVoid;
	}
	for (Standard_Integer i=1; i<=nbs; i++) {
		m_aShapeList.Append(aReader.Shape(i));
	}

	//获取颜色信息
	if(pShapeAttribMgr_ == NULL){
		pShapeAttribMgr_ = new kcShapeAttribMgr;
	}else{
		pShapeAttribMgr_->Clear();
	}
	pShapeAttribMgr_->ReadShapeColors(aReader);

#ifdef _DEBUG

#endif

	return status;
}

////////////////////////////////////////////////////////
kcXCAFSTEPReader::kcXCAFSTEPReader(kcModel *pModel)
	:kcBaseReader(pModel)
{
	m_nLevel = 0;
}

kcXCAFSTEPReader::~kcXCAFSTEPReader()
{
}

int EncodeTextOfStepFile(const char *pszFileName,std::string& szConvFile);
int EncodeStepFileToUtf8(const char *pszFileName,std::string& szConvFile);
int GetGBKTextName(const char *pszFileName,std::vector<std::string> &aNameVec);

//导入
BOOL  kcXCAFSTEPReader::DoImport()
{
	if(!m_pModel)
		return false;

	CFileDialog dlg(TRUE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"STEP Files (*.step , *.stp)|*.step; *.stp||", 
		NULL );

	if(dlg.DoModal() == IDOK){
		CString szFile = dlg.GetPathName();
		
		m_aShapeList.Clear();
		aFaceMap_.Clear();

		CWaitCursor wc;

		//// 将STEP文件进行编码，转成UTF8编码的unicode
		//// 这个已经证明不可行
		//std::string sConvFile;
		////EncodeStepFileToUtf8(szFile,sConvFile);
		////EncodeTextOfStepFile(szFile,sConvFile);
		//sConvFile = szFile;
		
		//读取多自己字符串
		aStepMbStrMgr_.Clear();
		aStepMbStrMgr_.Read(szFile);
		
		//读取step文件内容
		TCollection_AsciiString ascStr;

		EncodeFilePath(szFile,ascStr);
		ReadSTEP(ascStr.ToCString());

		if(!m_aShapeList.IsEmpty()){
			this->AddShapeToModel();
		}
	}

	return TRUE;
}

#define MAXUNICODE	0x10FFFF

/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is invalid.
*/
static const char *utf8_decode (const char *o, int *val) {
  static const unsigned int limits[] = {0xFF, 0x7F, 0x7FF, 0xFFFF};
  const unsigned char *s = (const unsigned char *)o;
  unsigned int c = s[0];
  unsigned int res = 0;  /* final result */
  if (c < 0x80)  /* ascii? */
    res = c;
  else {
    int count = 0;  /* to count number of continuation bytes */
    while (c & 0x40) {  /* still have continuation bytes? */
      int cc = s[++count];  /* read next byte */
      if ((cc & 0xC0) != 0x80)  /* not a continuation byte? */
        return NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
      c <<= 1;  /* to test next bit */
    }
    res |= ((c & 0x7F) << (count * 5));  /* add first byte */
    if (count > 3 || res > MAXUNICODE || res <= limits[count])
      return NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  if (val) *val = res;
  return (const char *)s + 1;  /* +1 to include first byte */
}

int DecodeAsciiString(const TCollection_ExtendedString& extString,std::string& str)
{
	TCollection_AsciiString ascString(extString);
	const char *pStr = ascString.ToCString();
	if(!pStr)
		return 0;

	size_t ix = 0,slen = strlen(pStr);
	const char *pStrEnd = pStr + slen;
	int code = 0;

	while(ix < slen){
		pStr = utf8_decode(pStr,&code);
		if(pStr == NULL || code > 256){//非utf8编码
			str = ascString.ToCString();//取原始的字符串
			return 0;
		}

		str += (char)code;
		if(pStr == pStrEnd)
			break;
	}

	return 1;
}

// 将extString中保存的unicode字符串，转成多字节
int DecodeUnicodeString(const TCollection_ExtendedString& extString,std::string& str)
{
	if(extString.IsAscii()){
		int ix,n = extString.Length();
		for(ix = 1;ix <= n;ix ++){
			Standard_ExtCharacter ec = extString.Value(ix);
			str += (char)(ec & 0xff);
		}
	}else{
		const wchar_t *pWStr = extString.ToWideString();
		int slen = ::WideCharToMultiByte(CP_ACP,0,pWStr,-1,NULL,0,NULL,NULL);
		char *pszBuf = new char[slen + 1];
		WideCharToMultiByte(CP_ACP,0,pWStr,-1,pszBuf,slen,NULL,NULL);
		str = pszBuf;
		delete pszBuf;
	}

	return 1;
}

std::string DecodeNameExtString(const TCollection_ExtendedString& extString,stdStringVector &aStringVec)
{
	std::string szName;
	if(extString.IsAscii()){
		int ix,n = extString.Length();
		for(ix = 1;ix <= n;ix ++){
			Standard_ExtCharacter ec = extString.Value(ix);
			szName += (char)(ec & 0xff);
		}
		return szName;
	}

	//非asc，可能是被错误的当作utf8，解码到了unicode
	stdStringVector::size_type ix,isize = aStringVec.size();
	for(ix = 0;ix < isize;ix ++){
		std::string ss = aStringVec.at(ix);

		TCollection_AsciiString ascStr(ss.c_str());
		TCollection_ExtendedString extStr(ascStr);
		if(extStr == extString){
			szName = ss;
			break;
		}
	}
	//const wchar_t *pWStr = extString.ToWideString();
	//int slen = ::WideCharToMultiByte(CP_UTF8,0,pWStr,-1,NULL,0,NULL,NULL);
	//if(slen > 0){
	//	char *pszBuf = new char[slen + 1];
	//	WideCharToMultiByte(CP_UTF8,0,pWStr,-1,pszBuf,slen,NULL,NULL);
	//	szName = pszBuf;
	//	delete pszBuf;
	//}

	return szName;
}

#if 0
int DecodeUtf8String(const TCollection_ExtendedString& extString,std::string& str)
{
	if(extString.IsAscii()){
		int ix,n = extString.Length();
		for(ix = 1;ix <= n;ix ++){
			Standard_ExtCharacter ec = extString.Value(ix);
			str += (char)(ec & 0xff);
		}
	}else{
		str = "xxxxxx";
		//int ix,n = extString.Length();
		//char *pBuf = new char[n * 4];

		//NCollection_Utf16Iter uit(extString.ToExtString());
		//char *pStr = pBuf;
		//if(*uit == 0){
		//	str = "xxxxxx";
		//}else{
		//	for(;*uit != 0; ++uit){
		//		pStr = uit.GetUtf(pStr);
		//	}
		//	str = pBuf;
		//}
		
	}
	//TCollection_AsciiString ascString(extString);
	//const char *pStr = ascString.ToCString();
	//if(!pStr)
	//	return 0;

	////unicode utf8 code
	//wchar_t *pWStr = NULL;
	//int wclen = ::MultiByteToWideChar(CP_UTF8,0,pStr,-1,NULL,0);
	//pWStr = new wchar_t[wclen];
	//::MultiByteToWideChar(CP_UTF8,0,pStr,-1,pWStr,wclen);

	////
	//int slen = ::WideCharToMultiByte(CP_ACP,0,pWStr,-1,NULL,0,NULL,NULL);
	//char *pszBuf = new char[slen];
	//WideCharToMultiByte(CP_ACP,0,pWStr,-1,pszBuf,slen,NULL,NULL);

	//str = pszBuf;

	return 1;
}

#endif

void Test()
{
	TCollection_AsciiString tas("装配体1");
	TRACE("\n ---%s---\n",tas.ToCString());
	TCollection_ExtendedString eas(tas);
	TCollection_AsciiString tas1(eas);
	//EncodeFilePath("零件1",tas);
	TRACE("\n ---%s---\n",tas1.ToCString());
}

#if 0
int uxWideCharToMultByte(const TCollection_ExtendedString &extString,std::string& str)
{
	TCollection_AsciiString tas("装配体1");
	TRACE("\n ---%s---\n",tas.ToCString());
	TCollection_ExtendedString eas(tas);
	TCollection_AsciiString tas1(eas);
	//EncodeFilePath("零件1",tas);
	TRACE("\n ---%s---\n",tas1.ToCString());
	std::string tss;
	DecodeAsciiString(tas1,tss);
	TRACE("\n ---%s    \n",tss.c_str());

	TCollection_AsciiString ascString(extString);
	DecodeAsciiString(ascString,str);

	//const char *pUtf8Str = ascString.ToCString();
	//if(pUtf8Str){
	//	TRACE("\n    %s----",pUtf8Str);
	//	int nbWC = MultiByteToWideChar(CP_UTF8,0,pUtf8Str,-1,NULL,0);
	//	if(nbWC > 0){
	//		wchar_t *pWCBuf = new wchar_t[nbWC + 1];
	//		MultiByteToWideChar(CP_UTF8,0,pUtf8Str,-1,pWCBuf,nbWC);
	//		pWCBuf[nbWC] = 0;

	//		int num = WideCharToMultiByte(936,0,pWCBuf,-1,NULL,0,NULL,NULL);
	//		if(num > 0){
	//			char *pszBuf = new char[num+1];
	//			WideCharToMultiByte(936,0,pWCBuf,-1,pszBuf,num,"_",NULL);
	//			pszBuf[num] = '\0';
	//			str = pszBuf;

	//			delete[] pszBuf;
	//			delete[] pWCBuf;

	//			return 1;
	//		}else{
	//			delete[] pWCBuf;
	//		}
	//	}
	//}	

	return 0;
}
#endif

void  kcXCAFSTEPReader::DecodeNameString(const TCollection_ExtendedString &aExtStr,std::string &szName)
{
	szName.clear();
	if(aExtStr.IsEmpty())
		return;

	if(aExtStr.IsAscii()){
		int ix,n = aExtStr.Length();
		for(ix = 1;ix <= n;ix ++){
			Standard_ExtCharacter ec = aExtStr.Value(ix);
			szName += (char)(ec & 0xff);
		}
		return;
	}

	//非asc，可能是被错误的当作utf8，解码到了unicode
	if(!aStepMbStrMgr_.Find(aExtStr,szName)){
		TRACE("\n ###---???--- find ext name faild.");
	}
}

void  kcXCAFSTEPReader::AddLabel(TDF_Label& aLabel,const TopLoc_Location& tloc,Handle(XCAFDoc_ColorTool) &aColTool)
{
	std::string ssp;
	int ix;

	//Test();

	m_nLevel ++;
	for(ix = 0;ix < m_nLevel;ix ++) ssp += "--------";

	//entity
	TCollection_AsciiString aEntry;
	TDF_Tool::Entry(aLabel,aEntry);
	TRACE("\n%s entry : %s",ssp.c_str(),aEntry.ToCString());

	//name
	Handle(TDataStd_Name) Name;
	if (aLabel.FindAttribute(TDataStd_Name::GetID(),Name)){
		TCollection_ExtendedString aeNam = Name->Get();
		std::string ssNam;
		
		DecodeNameString(aeNam,ssNam);
		TRACE("\n%s name : %s",ssp.c_str(),ssNam.c_str());
	}

	//color
	int hasColor = 0;
	Quantity_Color col;
	if(aColTool->GetColor(aLabel,col) ||
		aColTool->GetColor(aLabel,XCAFDoc_ColorSurf,col))
	{
		hasColor = 1;
		TRACE("\n%s color : %.4f,%.4f,%.4f",ssp.c_str(),col.Red(),col.Green(),col.Blue());
	}else{
		TRACE("\n%s color : nil",ssp.c_str());
	}

	if(XCAFDoc_ShapeTool::IsAssembly(aLabel)){
		TRACE("\n%s type : ASSEMBLY ",ssp.c_str());
		TopLoc_Location refLoc = XCAFDoc_ShapeTool::GetLocation(aLabel);
		if(!refLoc.IsIdentity()){
			TRACE("\n%s refloc : not null.",ssp.c_str());
		}

		int nbc = XCAFDoc_ShapeTool::NbComponents(aLabel);
		if(nbc > 0){
			TDF_LabelSequence aSeq;
			XCAFDoc_ShapeTool::GetComponents(aLabel,aSeq);

			int ix,num = aSeq.Length();
			for(ix = 1;ix <= num;ix ++){
				TDF_Label aL = aSeq.Value(ix);
				AddLabel(aL,refLoc * tloc,aColTool);
			}
		}
	}else if(XCAFDoc_ShapeTool::IsReference(aLabel)){
		TRACE("\n%s type : Reference ",ssp.c_str());
		TDF_Label aRef;
		if(XCAFDoc_ShapeTool::GetReferredShape(aLabel,aRef)){
			TopLoc_Location refLoc = XCAFDoc_ShapeTool::GetLocation(aLabel);
			if(!refLoc.IsIdentity()){
				TRACE("\n%s refloc : not null.",ssp.c_str());
			}
			AddLabel(aRef,refLoc*tloc,aColTool);

		}else{
			TRACE("\n get referred shape failed.");
		}
	}else{
		if (XCAFDoc_ShapeTool::IsSimpleShape(aLabel)){
			if(aLabel.Father().Father().Father().IsRoot())
				TRACE("\n%s type : PART ",ssp.c_str());
			else{
				TRACE("\n%s type : Simple shape...",ssp.c_str());
			}
		}else{
			TRACE("\n%s type : INSTANCE ",ssp.c_str());
		}  

		//
		TopoDS_Shape aS;
		XCAFDoc_ShapeTool::GetShape(aLabel,aS);
		if(aS.IsNull()){
			TRACE("\n get shape failed.shape is NULL.");
		}else{
			if(!tloc.IsIdentity())
				aS.Location(tloc);

			//if(hasColor){
			//	AddEntity(aS,col);
			//}else{
			//	m_aShapeList.Append(aS);
			//}
			m_aShapeList.Append(aS);
			//添加到Model中
			if(hasColor){
				m_pModel->GetShapeAttribMgr()->AddColorAttrib(aS,col);
			}

			MapFace(aS);
		}
	}
	
	m_nLevel --;
}

void  kcXCAFSTEPReader::MapFace(const TopoDS_Shape &aS)
{
	if(aS.IsNull() || aS.ShapeType() >= TopAbs_FACE)
		return;

	TopExp_Explorer exp;
	for(exp.Init(aS,TopAbs_FACE);exp.More();exp.Next()){
		const TopoDS_Shape &aF = exp.Current();
		//
		aFaceMap_.Add(aF);
	}
}

static void uxDumpDocStruct(const Handle(TDocStd_Document) &aDoc);

IFSelect_ReturnStatus  kcXCAFSTEPReader::ReadSTEP(const Standard_CString& aFileName)
{
	STEPCAFControl_Reader reader;

	reader.SetColorMode(true);
	reader.SetNameMode(true);

	IFSelect_ReturnStatus status = reader.ReadFile(aFileName);
	if (status != IFSelect_RetDone)
		return status;

	Handle(TDocStd_Document) aDoc;
	Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
	TCollection_ExtendedString docnam("MDTV-XCAF");
	app->NewDocument(docnam,aDoc);

	if(reader.Transfer(aDoc)){
		//
		int ix,num = 0;
		BOOL bAssembly = FALSE;
		TDF_LabelSequence seqLabel;
		TopLoc_Location loc;
		Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
		Handle(XCAFDoc_ColorTool) aColTool = XCAFDoc_DocumentTool::ColorTool(aDoc->Main());
		Handle(XCAFDoc_LayerTool) aLayerTool = XCAFDoc_DocumentTool::LayerTool(aDoc->Main());

		m_nLevel = 0;
		aShapeTool->GetFreeShapes(seqLabel);
		num = seqLabel.Length();
		ASSERT(num == 1);
		for(ix = 1;ix <= num;ix ++){
			TDF_Label aLabel = seqLabel.Value(ix);
			if(XCAFDoc_ShapeTool::IsAssembly(aLabel))
				bAssembly = TRUE;

			AddLabel(aLabel,loc,aColTool);
		}

		if(bAssembly) TRACE("\n has assembly.\n");
		else TRACE("\n has no assembly.\n");

		TRACE("\n\n");

		ReadFaceColors(reader);

		kcStepLayerMgr lyMgr(&this->aStepMbStrMgr_);
		lyMgr.ReadLayers(aDoc);

	}else{
#ifdef _DEBUG
		uxDumpStepModel(reader.Reader());
	//uxDumpDocStruct(aDoc);
#endif
		return IFSelect_RetFail;
	}

#ifdef _DEBUG
	uxDumpStepModel(reader.Reader());
	//uxDumpDocStruct(aDoc);
#endif

#define _XCAF_DOC_DLG
#ifdef _XCAF_DOC_DLG
	{
		XCAFDocDialog dlg(aDoc);
		dlg.DoModal();
	}
#endif

	return IFSelect_RetDone;
}

void kcXCAFSTEPReader::ReadFaceColors(STEPCAFControl_Reader &aCAFReader)
{
	STEPControl_Reader &aReader = aCAFReader.ChangeReader();
	Handle(XSControl_WorkSession) aWS = aReader.WS();
	Handle(Transfer_TransientProcess) aTP = aWS->TransferReader()->TransientProcess();

	STEPConstruct_Styles aStyles ( aWS );
	if ( ! aStyles.LoadStyles() ) {
		TRACE("\n load styles failed.");
		return;
	}

	int ix,jx,nb = aStyles.NbStyles();
	for(ix = 1;ix <= nb;ix ++){
		Handle(StepVisual_StyledItem) aStyleItem = aStyles.Style(ix);
		if(aStyleItem.IsNull())
			continue;

		Handle(StepVisual_Colour) aSurfCol, aBoundCol, aCurveCol, aRenderCol;
		// check if it is component style
		bool bComponent = false;
		double transp = 0.0;
		if (!aStyles.GetColors(aStyleItem,aSurfCol,aBoundCol,aCurveCol,aRenderCol, transp, bComponent))
			continue;

		if(aSurfCol.IsNull())
			continue;

		// find shape
		std::vector<Handle(Standard_Transient)> anItems;
		if (!aStyleItem->Item().IsNull()) {
			anItems.push_back(aStyleItem->Item());
		}
		else if (!aStyleItem->ItemAP242().Representation().IsNull()){
			//special case for AP242: item can be Reprsentation
			Handle(StepRepr_Representation) aRepr = aStyleItem->ItemAP242().Representation();
			for (jx = 1; jx <= aRepr->Items()->Length(); jx++)
				anItems.push_back(aRepr->Items()->Value(jx));
		}
		//
		if(anItems.empty())
			continue;

		std::vector<Handle(Standard_Transient)>::size_type iix,iisz = anItems.size();
		for(iix = 0;iix < iisz;iix ++){
			Handle(Standard_Transient) aEnt = anItems.at(iix);
			Handle(Standard_Type) aType = aEnt->DynamicType();
			//TRACE("\n ---- ent type is %s.",aType->Name());
			if(aEnt->IsKind(STANDARD_TYPE(StepShape_AdvancedFace))){
				TopoDS_Shape aS = TransferBRep::ShapeResult(aTP,aEnt);
				if(!aS.IsNull()){
					//TRACE("\n --- get shape.");
					if(aFaceMap_.Contains(aS)){
						//TRACE("\n ------face in face map.");
						if(m_pModel){
							Quantity_Color aCol;
							if(aStyles.DecodeColor(aSurfCol,aCol)){
								m_pModel->GetShapeAttribMgr()->AddColorAttrib(aS,aCol);
								//TRACE("\n ------ add face color to manager.");
							}
						}
					}
				}
			}
		}
	}
}

static BOOL uxEncodeStepText(const std::string &szText,std::string &szEncText)
{
	//空字符串
	if(szText.empty()){
		szEncText = "";
		return TRUE;
	}

	int slen = 0,wclen = 0,uslen = 0;
		
	//获取编码长度
	slen = (int)szText.size();
	wclen = ::MultiByteToWideChar(CP_ACP,0,szText.c_str(),-1,NULL,0);//长度含0
	if(wclen <= 0){//return 0,failed
		szEncText = szText;//保留原来的字符串
		return FALSE;
	}

	//进行编码
	if(wclen == slen + 1){//ascii字符串,不用编码
		szEncText = szText;
	}else{
		char szBuf[1024];
		char *pszStr = NULL;
		wchar_t wcBuf[1024];
		wchar_t *pwStr = NULL;

		pwStr = wcBuf;
		if(wclen > 1024){
			pwStr = new wchar_t[wclen + 1];
		}
		
		//转unicode
		::MultiByteToWideChar(CP_ACP,0,szText.c_str(),-1,pwStr,wclen);//自动会写入结尾的0

		//进行UTF-8编码
		uslen = ::WideCharToMultiByte(CP_UTF8,0,pwStr,-1,NULL,0,NULL,NULL);

		pszStr = szBuf;
		if(uslen > 1024){
			pszStr = new char[uslen + 1];
		}

		::WideCharToMultiByte(CP_UTF8,0,pwStr,-1,pszStr,uslen,NULL,NULL);//自动写入结尾的0
		szEncText = pszStr;

		if(pwStr != wcBuf) delete[] pwStr;
		if(pszStr != szBuf) delete[] pszStr;
	}

	return TRUE;
}

//获取step文件中所有的中文名称
int GetGBKTextName(const char *pszFileName,std::vector<std::string> &aNameVec)
{
	FILE *fp = NULL;
	fopen_s(&fp,pszFileName,"r");
	if(fp == NULL){
		return 0;
	}

	int maxSize = 4000;
	char *pszLine = new char[maxSize];
	int slen = 0,wclen = 0;

	while(fgets(pszLine,maxSize,fp)){
		slen = strnlen_s(pszLine,maxSize); //长度不含0，pszLine包含了\n
		//不会为空，即使空行，也会包含一个\n
		if(slen == 0)
			continue;

		//判断是否包含非ASC字符串，需要进行转换
		wclen = ::MultiByteToWideChar(CP_ACP,0,pszLine,-1,NULL,0);//长度含0
		if(wclen == slen + 1){//ascii 字符串，不用转换
			continue;
		}

		// 找出字符串
		int icx = 0,icxStart = -1;
		while(icx < slen){
			if(pszLine[icx] == '\''){
				if(icxStart < 0){
					icxStart = icx + 1;//字符串从'后开始
				}else{
					//找到一个字符串,进行编码,将从icxStart＋1开始的内容，进行编码转换
					if(icxStart < icx){//判断''空字符串的情形，这种情形很多
						int kx;
						std::string szText;
						
						szText.reserve(icx - icxStart + 1);
						for(kx = icxStart;kx < icx;kx ++){
							szText += pszLine[kx];
						}

						TRACE("\n #---- text : %s",szText.c_str());
						aNameVec.push_back(szText);
					}
					//
					icxStart = -1;//重新开始
				}
			}

			icx ++;
		}//end while
	}

	fclose(fp);

	return 1;
}

// 对STEP文件中的名称进行unicode的utf8编码，以便OCC获取正确的名称
int EncodeTextOfStepFile(const char *pszFileName,std::string& szConvFile)
{
	szConvFile = "d:\\DbgDump\\step_unicode_utf8.step";
	FILE *fp = NULL,*ofp = NULL;

	fopen_s(&fp,pszFileName,"r");
	fopen_s(&ofp,szConvFile.c_str(),"w");
	if(fp == NULL || ofp == NULL){
		if(fp) fclose(fp);
		if(ofp) fclose(ofp);

		return 0;
	}

	int maxSize = 4000,encBufSize = 4000;
	char *pszLine = new char[maxSize];
	char *pszEncLine = new char[encBufSize];
	int slen = 0,wclen = 0;

#define GROW_ENCBUF(n,ix) \
	encBufSize += n; \
	char *pszTemp = new char[encBufSize];\
	memcpy(pszTemp,pszEncLine,ix);\
	delete[] pszEncLine;\
	pszEncLine = pszTemp;

	while(fgets(pszLine,maxSize,fp)){
		slen = strnlen_s(pszLine,maxSize); //长度不含0，pszLine包含了\n
		//不会为空，即使空行，也会包含一个\n
		if(slen == 0)
			continue;

		//判断是否包含非ASC字符串，需要进行转换
		wclen = ::MultiByteToWideChar(CP_ACP,0,pszLine,-1,NULL,0);//长度含0
		if(wclen == slen + 1){//ascii 字符串，不用转换
			fwrite(pszLine,slen,1,ofp);
			continue;
		}

		// 找出字符串
		int icx = 0,itx = 0,icxStart = -1;
		while(icx < slen){
			if(pszLine[icx] == '\''){
				if(icxStart < 0){
					icxStart = icx + 1;//字符串从'后开始
				}else{
					//找到一个字符串,进行编码,将从icxStart＋1开始的内容，进行编码转换
					if(icxStart < icx){//判断''空字符串的情形，这种情形很多
						int kx,cnt = 0;
						std::string szText,szEncText;
						std::string::size_type stix;

						szText.reserve(icx - icxStart + 1);
						for(kx = icxStart;kx < icx;kx ++){
							szText += pszLine[kx];
						}

						//为了避免对汉字进行utf8编码出现非法字符，统一添加空格
						//获取名称时，去掉
						szText += "      ";

						//进行编码
						uxEncodeStepText(szText,szEncText);

						//如果行太长，需要重新分配内存
						if(itx + (int)szEncText.size() >= encBufSize){
							GROW_ENCBUF(1000,itx);
						}

						for(stix = 0;stix < szEncText.size();stix ++){
							pszEncLine[itx++] = szEncText.at(stix);
						}
					}
					//
					icxStart = -1;//重新开始
				}
				//记录 '号
				pszEncLine[itx++] = pszLine[icx++];
			}else{
				//不是字符串内部，直接拷贝
				if(icxStart < 0){
					//空间不够了，需要扩充
					if(itx >= encBufSize){
						GROW_ENCBUF(1000,itx);
					}
					pszEncLine[itx++] = pszLine[icx++];
				}else{
					icx++;
				}
			}
		}//end while

		//如果没有找到配对引号，需要将剩余部分读入
		if(icxStart >= 0){
			//
			if(itx + slen - icxStart >= encBufSize){
				GROW_ENCBUF(1000,itx);
			}
			int kx;
			for(kx = icxStart;kx < slen;kx ++){
				pszEncLine[itx++] = pszLine[kx];
			}
		}

		//
		pszEncLine[itx] = '\0';

		//写入
		fwrite(pszEncLine,strnlen_s(pszEncLine,encBufSize),1,ofp);
	}

	fclose(fp);
	fclose(ofp);

#undef GROW_ENCBUF

	delete[] pszLine;
	delete[] pszEncLine;

	return 1;
}

//将step文件编码为unicode的utf8编码，以方便occ的内部处理
int EncodeStepFileToUtf8(const char *pszFileName,std::string& szConvFile)
{
	szConvFile = "d:\\DbgDump\\~stp_utf8.step";
	char *ptf = "d:\\DbgDump\\~stp_utf8 b.step";
	FILE *fp = NULL;
	fopen_s(&fp,pszFileName,"r");
	if(!fp){
		return 0;
	}

	FILE *ofp = NULL;
	fopen_s(&ofp,szConvFile.c_str(),"w");
	if(!ofp){
		fclose(fp);
		return 0;
	}

	FILE *tfp = NULL;
	fopen_s(&tfp,ptf,"wb");

	int maxCnt = 4000;
	char *pszLine = new char[maxCnt + 1];
	char *pszBuf = new char[maxCnt + 1];
	wchar_t *pwcBuf = new wchar_t[maxCnt + 1];
	int slen = 0,uslen = 0,wclen = 0;

	while(fgets(pszLine,maxCnt,fp)){
		slen = strnlen_s(pszLine,maxCnt); //长度不含0，pszLine包含了\n
		//不会为空，即使空行，也会包含一个\n
		if(slen == 0)
			continue;
		//
		wclen = ::MultiByteToWideChar(CP_ACP,0,pszLine,-1,NULL,0);//长度含0
		if(wclen == slen + 1){//ascii字符串
			fwrite(pszLine,slen,1,ofp);
		}else{
		//{
			wchar_t *pWStr = pwcBuf;
			if(wclen >= maxCnt){
				pWStr = new wchar_t[maxCnt + 1];
			}
			::MultiByteToWideChar(936,0,pszLine,-1,pWStr,wclen);//自动会写入结尾的0

			//
			char *pUStr = pszBuf;
			uslen = ::WideCharToMultiByte(CP_UTF8,0,pWStr,-1,NULL,0,NULL,NULL);
			if(uslen >= maxCnt){
				pUStr = new char[uslen + 1];
			}
			::WideCharToMultiByte(CP_UTF8,0,pWStr,-1,pUStr,uslen,NULL,NULL);//自动写入结尾的0

			fwrite(pUStr,strnlen_s(pUStr,uslen),1,ofp);
			fwrite(pUStr,strnlen_s(pUStr,uslen),1,tfp);

			if(pWStr != pwcBuf) delete[] pWStr;
			if(pUStr != pszBuf) delete[] pUStr;
		}
	}

	fclose(fp);
	fclose(ofp);
	fclose(tfp);

	delete[] pszLine;
	delete[] pszBuf;
	delete[] pwcBuf;

	return 1;
}

static void uxDumpStepModel(const STEPControl_Reader &aReader)
{
	const char *pszFile = "d:\\DbgDump\\StepReaderModel.txt";
	FILE *fp = NULL;

	fopen_s(&fp,pszFile,"w");
	if(fp == NULL)
		return;

	Handle(Standard_Type) aType;
	Handle(StepData_StepModel) aStepModel = aReader.StepModel();
	int ix,nbEnt = aStepModel->NbEntities();
	for(ix = 1;ix <= nbEnt;ix ++){
		Handle(Standard_Transient) aEnt = aStepModel->Entity(ix);
		//
		aType = aEnt->DynamicType();
		fprintf_s(fp,"%d type name is : %s\n",ix,aType->Name());
	}

	fclose(fp);
}

static char *ss_aShapeType[] = {
		"COMPOUND",
		"COMPSOLID",
		"SOLID",
		"SHELL",
		"FACE",
		"WIRE",
		"EDGE",
		"VERTEX",
		"SHAPE"
	};

static void uxDumpLabel(FILE *fp,TDF_Label aLabel,int nsp)
{
	TCollection_AsciiString strTags;
	TDF_Tool::Entry(aLabel,strTags);
	std::string ssp(nsp,' ');
	TopoDS_Shape aShape;
	int ix,nbAttr = 0;

	nbAttr = aLabel.NbAttributes();
	fprintf_s(fp,"%s[%s] : Attrib num = %d\n",ssp.c_str(),strTags.ToCString(),nbAttr);
	if(XCAFDoc_ShapeTool::GetShape(aLabel,aShape)){
		fprintf_s(fp,"%s  --has shape, type is %s\n",ssp.c_str(),ss_aShapeType[(int)aShape.ShapeType()]);
	}
	if(nbAttr > 0){
		TDF_AttributeIterator attrIte(aLabel,false);
		ix = 0;
		for(;attrIte.More();attrIte.Next()){
			Handle(TDF_Attribute) aAttr = attrIte.Value();
			//
			Handle(Standard_Type) aType = aAttr->DynamicType();
			fprintf_s(fp,"%s    attr %d : type %s\n",ssp.c_str(),ix,aType->Name());
			ix ++;
		}
	}


	TDF_ChildIterator iter(aLabel,false);
	for(;iter.More();iter.Next()){
		TDF_Label aSubLab = iter.Value();
		//
		uxDumpLabel(fp,aSubLab,nsp + 2);
	}
}

static void uxDumpDocStruct(const Handle(TDocStd_Document) &aDoc)
{
	const char *pszFile = "d:\\DbgDump\\DocStd_Struct.txt";
	FILE *fp = NULL;
	std::string ssp,ssp2("  "),ssp4("    "),ssp8("        ");

	fopen_s(&fp,pszFile,"w");
	if(fp == NULL)
		return;

	int nbChild = 0,nbAttr = 0;
	TDF_Label aMainLab = aDoc->Main();
	TCollection_AsciiString strTagList;
	
	TDF_Tool::Entry(aMainLab,strTagList);
	fprintf_s(fp,"Main Label : %s, attrib num = %d.\n",
		strTagList.ToCString(),TDF_Tool::NbAttributes(aMainLab));

	TDF_ChildIterator iter(aMainLab,false);
	for(;iter.More();iter.Next()){
		TDF_Label aLab = iter.Value();
		//
		uxDumpLabel(fp,aLab,2);
	}

	fclose(fp);
}