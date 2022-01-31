//////////////////////////////////////////////////////////////////////////
// 记录保存数据的一个块
#ifndef _KSTG_BLOCK_H_
#define _KSTG_BLOCK_H_

#include <vector>
#include <string>
#include <map>
#include "QxLibDef.h"

// Code Value的类型
typedef unsigned short vcode_t;

class CArchive;
class TopoDS_Shape;
class QxStgCodeValue;
class QxStgBlock;

typedef std::vector<QxStgCodeValue *>		QxStgCodeValueArray;
typedef std::map<vcode_t,QxStgCodeValue *>	QxStgCodeValueMap;



//是否unsigned类型的掩码，占据第4bite
#define KSTG_BITS_MASK_UNSIGNED	(0x1 << 3)
//code是否short类型（需要使用2字节）的掩码，使用7bit
#define KSTG_BITS_MASK_SHORT_CODE	(0x1 << 6)	
//是否数组的掩码,占据第8bit
#define KSTG_BITS_MASK_ARRAY		(0x1 << 7)
// 数组大小的bit掩码,5,6个bit
#define KSTG_BITS_MASK_ARRAY_NUM	0x30



// 类型定义
#define KSTG_TYPE_NULL		0    //未知的类型
#define KSTG_TYPE_CHAR		0x01
#define KSTG_TYPE_BYTE		(KSTG_TYPE_CHAR | KSTG_BITS_MASK_UNSIGNED)
#define KSTG_TYPE_SHORT		0x02
#define KSTG_TYPE_USHORT		(KSTG_TYPE_SHORT | KSTG_BITS_MASK_UNSIGNED)
#define KSTG_TYPE_INT			0x03
#define KSTG_TYPE_UINT		(KSTG_TYPE_INT | KSTG_BITS_MASK_UNSIGNED)
#define KSTG_TYPE_INT64		0x04
#define KSTG_TYPE_UINT64		(KSTG_TYPE_INT64 | KSTG_BITS_MASK_UNSIGNED)
#define KSTG_TYPE_BOOL		0x05
#define KSTG_TYPE_FLOAT		0x06
#define KSTG_TYPE_DOUBLE		0x07
#define KSTG_TYPE_BLOCK		0x08 //这个是一个特殊的类型，记录一个QxStgBlock对象

#define KSTG_INVALID_CODE		0xFFFF //将该值定义为无效值，不能使用

#define KSTG_IS_VALID_CODE(code) (code != KSTG_INVALID_CODE)

//判断是否无符号数
#define KSTG_IS_UNSIGNED(b)	((b & KSTG_BITS_MASK_UNSIGNED) != 0)
//判断是否数组
#define KSTG_IS_ARRAY(b)		((b & KSTG_BITS_MASK_ARRAY) != 0)
//判断code是否使用2byte
#define KSTG_IS_SHORT_CODE(b)  ((b & KSTG_BITS_MASK_SHORT_CODE) != 0)

// 记录一个带code的值，格式如下：
// typeattr(1BYTE) code(1BYTE) [array num(1-4BYTE) or blocksize] data(变长数组)
// 这里：typeattr: 1-4bit表明的是类型，其中3bit表明是否无符号数。
//                 5-6bit记录记录数组个数或block大小的byte数，0-3对应1-4个BYTE
//                 7bit:表示code是否单字节，1：code使用2个字节
//                 8bit:表明是否数组
// 数组个数或块大小，使用变长数组（1-4个BYTE）记录。
// data的大小，并不记录下来，会根据typeattr来计算其大小。
class QXLIB_API QxStgCodeValue{
public:
	QxStgCodeValue();
	QxStgCodeValue(vcode_t code,int ival);
	QxStgCodeValue(vcode_t code,unsigned int ival);
	QxStgCodeValue(vcode_t code,double dval);
	QxStgCodeValue(vcode_t code,double da[3]);
	QxStgCodeValue(vcode_t code,bool bval);
	QxStgCodeValue(vcode_t code,const char *pszBuf);
	QxStgCodeValue(vcode_t code,BYTE* pbuf,int n,bool bCopy=false);
	QxStgCodeValue(vcode_t code,QxStgBlock *pBlock,bool bFree=true);
	~QxStgCodeValue();

	bool				IsValid() const;
	vcode_t				GetCode() const { return code_; }
	bool				IsUnsigned() const;
	bool				IsBlock() const;
	bool				IsArray() const;

	bool				Set(vcode_t code,int ival);
	bool				Set(vcode_t code,unsigned int ival);
	bool				Set(vcode_t code,double dval);
	bool				Set(vcode_t code,double da[3]);
	bool				Set(vcode_t code,bool bval);
	bool				Set(vcode_t code,const char *pszBuf);
	bool				Set(vcode_t code,BYTE* pbuf,int n,bool bCopy=false);
	bool				Set(vcode_t code,QxStgBlock *pBlock,bool bFree=true);

	void				Clear();

	// 返回整个数据的大小
	int				GetFullSize();

public:
	bool				GetValue(int& iv) const;
	bool				GetValue(unsigned int& uiv) const;
	bool				GetValue(double& dv) const;
	bool				GetValue(double da[3]) const;
	bool				GetValue(bool& bv) const;
	bool				GetValue(std::string& str) const;
	bool				GetValue(BYTE *& pbuf,int &size) const;
	QxStgBlock*		GetBlock() const;

public:
	// 返回读取的字节数，失败返回0
	int				ReadData(BYTE *pBuf,bool bCopy=false);
	// 返回写入的字节数，失败返回0
	int				WriteData(BYTE *pBuf);

protected:
	void				Reset();
	void				SetCT(vcode_t code,BYTE type,bool bAry=false);

	int				CalcHeaderSize();
	void				CalcDataSize();
	void				CalcFullSize();

	BYTE				BuildTypeAttr();

	void				WriteValueToBuf(BYTE *pBuf);
	bool				ReadValueFromBuf(BYTE *pBuf,bool bCopy=false);

protected:
	BYTE				type_;//类型,低4bit
	BYTE				attrib_;//属性，高4bit
	vcode_t			code_;//编号
	bool				bArray_;//是否数组
	int				nArrayNum_;//数组个数
	int				nFullSize_;//整个数据的大小
	int				nDataSize_;//数据的大小，byte数
	BYTE				*pValueBuf_;//整个数据的数组，包括了类型、code等所有数据

	//记录各种值
	union{
		char						cval;
		BYTE						ucval;
		short					sval;
		unsigned short			usval;
		int						ival;
		unsigned int				uival;
		int64_t					ival64;
		uint16_t					uival64;
		bool						bval;
		float					fval;
		double					dval;
		BYTE						*ary;//记录数组数据类型是根据具体类型变动的
	}value_;
	bool				bCopyData_; //是否拷贝的数据，拷贝的数据需要释放

	//记录一个block对象，这样可以实现block内部包含block的嵌套包含
	QxStgBlock		*pBlock_;
	bool				bFreeBlock_;//是否释放该对象
};

///////////////////////////////////////////////////////////////////////////////
//

//块的类型
#define KSTG_BLOCK_TYPE_UNKNOWN	0x0		//未知类型

#define KSTG_IS_VALID_BLOCK_TYPE(t) (t != KSTG_BLOCK_TYPE_UNKNOWN)

///////////////////////////////////////////////////////////////////////////////
// 重新实现的块

#define KSTG_CODE_MAP_SIZE 32

// 块的结构：
// type(short) version(short) attrib(short) valueNum(1BYTE) datasize(1-4BYTE) 一系列CodeValue对象
// valueNum:是CodeValue的个数,考虑到code是BYTE类型，最多255个，因此可以使用1byte
// attrib: 0-1bit是记录datasize需要使用的byte数
//
class QXLIB_API QxStgBlockBase{
public:
	QxStgBlockBase();
	virtual ~QxStgBlockBase();

	virtual bool					IsValid() const;
	short						GetType() const;
	short						GetVersion() const;

	virtual void					Clear();

	int							GetCodeValueCount() const;
	QxStgCodeValue*				GetCodeValueByIndex(int idx) const;

	//获取整个块的大小,方便块嵌套使用
	int							GetBlockSize();
	
protected:
	//
	void							Reset();
	// 
	bool							DoAddCodeValue(QxStgCodeValue *pCV);

	// 计算整个块数据大小，不包括块头
	int							CalcBlockDataSize();
	// 计算块头的大小
	int							CalcBlockHeaderSize();

protected:
	bool							CodeExisted(vcode_t code) const;

protected:
	short						type_; //块类型
	short						version_; //版本，对一个块来说，应当够用了
	short						attrib_;//属性，低2bit记录datasize使用的byte数,34bit记录cv的个数
	int							nBlockSize_;//整个块大小，单位byte
	int							nDataSize_; //数据块大小，仅包含codevalue的值，不包含上面3个
	BYTE							*pDataBuf_; //数据块，大小为nDataSize_
	unsigned int					nEndMagic_; //块末尾标识

protected:
	QxStgCodeValueArray			aCodeValueArray_;//
	QxStgCodeValueMap				aCodeValueMap_;//
};

class QXLIB_API QxStgBlock : public QxStgBlockBase{
public:
	QxStgBlock();
	QxStgBlock(short type);
	~QxStgBlock();


public://写入部分接口函数
	bool							Initialize(short type);
	
	bool							AddCodeValue(QxStgCodeValue *pCodeValue);
	bool							AddCodeValue(vcode_t code,int ival);
	bool							AddCodeValue(vcode_t code,unsigned int ival);
	bool							AddCodeValue(vcode_t code,double dval);
	bool							AddCodeValue(vcode_t code,double da[3]);
	bool							AddCodeValue(vcode_t code,bool bval);
	bool							AddCodeValue(vcode_t code,BYTE *pb,int size,bool bcopy=true);
	bool							AddCodeValue(vcode_t code,const char *pszBuf);
	// 加入一个子块，子块指针会记录到内部，bFreeBlk:true,则由内部负责释放该对象
	bool							AddCodeValue(vcode_t code,QxStgBlock *pBlk,bool bFreeBlk=true);

	bool							Write(CArchive &ar);
	//写入到内存中，返回写入byte数目
	int							WriteToBuf(BYTE *pBuf);

protected://写入部分辅助函数
	//
	bool							BuildDataBuffer(BYTE *pBuf);
	// 构建头部块,返回buf中有效byte个数
	int							BuildHeaderBuf(BYTE *pBuf);				

public: //读取部分函数
	// 从流中读取
	bool							Read(CArchive &ar);
	// 带类型的读取，如果类型g不匹配，返回失败
	bool							Read(CArchive &ar,short type);
	// 从内存中读取，返回读取的字节数，失败，返回0
	int							ReadFromBuf(BYTE *pBuf,bool bCopy);

	//根据code获取值对
	QxStgCodeValue*				GetCodeValueByCode(vcode_t code);
	bool							GetValueByCode(vcode_t code,int &ival);
	bool							GetValueByCode(vcode_t code,unsigned int &ival);
	bool							GetValueByCode(vcode_t code,double &dval);
	bool							GetValueByCode(vcode_t code,double da[3]);
	bool							GetValueByCode(vcode_t code,bool &bval);
	bool							GetValueByCode(vcode_t code,BYTE *&pb,int &size);
	bool							GetValueByCode(vcode_t code,std::string& str);
	QxStgBlock*					GetSubBlock(vcode_t code); //获取内嵌的子块
	
protected://读取辅助函数
	bool							ReadHeaderData(CArchive &ar);
	// 读取头部数据，返回读取个数
	int							ReadHeaderData(BYTE *pBuf);
	// 夺取所有的CV
	bool							ReadCodeValueArray(BYTE *pBuf,bool bCopy);

protected:
	int							nReadCVNum_; //读取的CV对象的个数
};

//////////////////////////////////////////////////////////////////////////
// 一个shape相关的数据
// 保存一个shape的数据，采用BRep结构，并对生成字符串进行压缩。
// 最终结果保存在一个块中，块结构：原始BRep数据长度 + 压缩后的BRep数据
class QXLIB_API kstgShapeData{
public:
	kstgShapeData();
	~kstgShapeData();

	// 生成数据
	// nSize:数据块的大小
	// pData:块中的数据,头一个int记录压缩前的数据大小，后面是压缩后的数据
	BOOL							Build(const TopoDS_Shape& aShape,BYTE *&pData,int &nSize);
	// 解析
	TopoDS_Shape					Parse(BYTE *pData,int nSize);
};


#endif