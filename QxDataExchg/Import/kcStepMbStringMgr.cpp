#include "stdafx.h"
#include <TCollection_AsciiString.hxx>
#include "kcStepMbStringMgr.h"

//记录一对字符串
kcAscExtStringPair::kcAscExtStringPair()
{}

kcAscExtStringPair::kcAscExtStringPair(const std::string &aStr)
{
	if(!aStr.empty()){
		aStr_ = aStr;
	}
}

bool  kcAscExtStringPair::IsValid() const
{
	return (aStr_.empty() ? false : true);
}

bool  kcAscExtStringPair::IsSame(const TCollection_ExtendedString &extStr)
{
	if(aStr_.empty()) return false;
	//
	if(aExtStr_.IsEmpty()){
		TCollection_AsciiString astr(aStr_.c_str());
		TCollection_ExtendedString es(astr);
		aExtStr_ = es;
	}

	return ((aExtStr_ == extStr) ? true : false);
}

/////////////////////////////////////////////////////////////////
//
kcStepMbStringMgr::kcStepMbStringMgr(void)
{}

kcStepMbStringMgr::~kcStepMbStringMgr(void)
{
	Clear();
}

// 从STEP文件中，读取所有多字节的字符串
//
void  kcStepMbStringMgr::Read(const char *pszStepFile)
{
	FILE *fp = NULL;
	fopen_s(&fp,pszStepFile,"r");
	if(fp == NULL){
		TRACE("\n #- kcStepMbStringMgr::Read (%s) failed.",pszStepFile);
		return;
	}

	int maxSize = 4000;
	char *pszLine = new char[maxSize];
	int slen = 0,wclen = 0;

	//逐行读取和解析
	while(fgets(pszLine,maxSize,fp)){
		slen = strnlen_s(pszLine,maxSize); //长度不含0，pszLine包含了\n
		//不会为空，即使空行，也会包含一个\n
		if(slen == 0)
			continue;

		//判断是否包含包含了多字节字符
		wclen = ::MultiByteToWideChar(CP_ACP,0,pszLine,-1,NULL,0);//长度含0
		if(wclen == slen + 1){//ascii 字符串，不用处理
			continue;
		}

		// 找出字符串
		int icx = 0,icxStart = -1;
		while(icx < slen){
			if(pszLine[icx] == '\''){
				if(icxStart < 0){
					icxStart = icx + 1;//字符串从'后开始
				}else{
					//找到一个字符串,记录
					if(icxStart < icx){//判断''空字符串的情形，这种情形很多
						int kx;
						std::string szText;
						
						szText.reserve(icx - icxStart + 1);
						for(kx = icxStart;kx < icx;kx ++){
							szText += pszLine[kx];
						}

						//判断是否包含多字节字符
						wclen = ::MultiByteToWideChar(CP_ACP,0,szText.c_str(),-1,NULL,0);//长度含0
						if(wclen != (int)szText.size() + 1){//包含了多字节字符
							TRACE("\n #---- text : %s",szText.c_str());
							Add(szText);
						}
					}
					//
					icxStart = -1;//重新开始
				}
			}

			icx ++;
		}//end while
	}

	fclose(fp);
}

bool  kcStepMbStringMgr::Add(const std::string &ss)
{
	stdAscExtStringMap::iterator ite = aStringMap_.find(ss);
	if(ite != aStringMap_.end()){
		return true;
	}

	kcAscExtStringPair *pStrPair = new kcAscExtStringPair(ss);
	aStringMap_.insert(stdAscExtStringMap::value_type(ss,pStrPair));
	aStrPairVec_.push_back(pStrPair);

	return true;
}

void  kcStepMbStringMgr::Clear()
{
	stdAscExtStringPairVector::size_type ix,isize = aStrPairVec_.size();
	for(ix = 0;ix < isize;ix ++){
		delete aStrPairVec_[ix];
	}
	
	aStringMap_.clear();
	aStrPairVec_.clear();
}

//解码一个ExtString字符串
bool  kcStepMbStringMgr::DecodeExtString(const TCollection_ExtendedString &aExtStr,std::string &szStr)
{
	szStr.clear();
	if(aExtStr.IsEmpty())
		return true;

	//是ascii字符串，直接获取
	if(aExtStr.IsAscii()){
		int ix,n = aExtStr.Length();
		for(ix = 1;ix <= n;ix ++){
			Standard_ExtCharacter ec = aExtStr.Value(ix);
			szStr += (char)(ec & 0xff);
		}
		return true;
	}

	//非asc，可能是被错误的当作utf8，解码到了unicode
	//通过查表获取
	return Find(aExtStr,szStr);
}

bool  kcStepMbStringMgr::Find(const TCollection_ExtendedString &aExtStr,std::string &aStr)
{
	kcAscExtStringPair *pPair = NULL;
	stdAscExtStringPairVector::size_type ix,isize = aStrPairVec_.size();
	for(ix = 0;ix < isize;ix ++){
		pPair = aStrPairVec_.at(ix);

		if(pPair->IsSame(aExtStr)){
			aStr = pPair->aStr_;
			return true;
		}
	}
	return false;
}

