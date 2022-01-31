#pragma once

#include <vector>
#include <map>
#include <TCollection_ExtendedString.hxx>

class kcAscExtStringPair;
typedef std::map<std::string,kcAscExtStringPair *> stdAscExtStringMap;
typedef std::vector<kcAscExtStringPair *>  stdAscExtStringPairVector;

//记录一对字符串
class kcAscExtStringPair{
public:
	kcAscExtStringPair();
	kcAscExtStringPair(const std::string &aStr);
	
	bool							IsValid() const;
	bool							IsSame(const TCollection_ExtendedString &extStr);

public:
	std::string					aStr_;
	TCollection_ExtendedString		aExtStr_;
};

class kcStepMbStringMgr
{
public:
	kcStepMbStringMgr(void);
	~kcStepMbStringMgr(void);

	void							Read(const char *pszStepFile);
	void							Clear();

	//解码一个ExtString字符串
	bool							DecodeExtString(const TCollection_ExtendedString &aExtStr,std::string &szStr);

	bool							Find(const TCollection_ExtendedString &aExtStr,std::string &aStr);

protected:
	bool							Add(const std::string &ss);

protected:
	stdAscExtStringMap			aStringMap_;
	stdAscExtStringPairVector		aStrPairVec_;
};

