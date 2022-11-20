//////////////////////////////////////////////////////////////////////////
//
#ifndef _KI_OPTIONSET_H_
#define _KI_OPTIONSET_H_

#include <string>
#include <vector>
#include "QxLibDef.h"
#include "kiInputEdit.h"

#define KOPT_INPUT_BOOL			1 //bool��
#define KOPT_INPUT_INT				2
#define KOPT_INPUT_DOUBLE			3
#define KOPT_INPUT_ENUM			4 //�����ѡ��
#define KOPT_INPUT_BUTTON			5 //û��ֵ������һ��ѡ�������
#define KOPT_INPUT_STDAXIS			6 //��׼����ϵ��

class kiOptionItem;
	class kiOptionVoid;
	class kiOptionBool;
	class kiOptionInt;
	class kiOptionDouble;
	class kiOptionEnum;
	class kiOptionButton;

class kiOptionSet;
class kiInputTool;
class kiInputEdit;
class kiInputEditCallBack;

typedef std::vector<kiOptionItem *> stdOptionItemArray;

// ���ﶨ��ϵͳʹ��button��ID�����������Id��ͻ����������������
#define QX_SYS_BUTTON_ID_BASE			0x0F0000
#define QX_SYS_BUTTON_APPLY			(QX_SYS_BUTTON_ID_BASE + 1) //��Ӧ�á�ѡ�ť
#define QX_SYS_BUTTON_OK				(QX_SYS_BUTTON_ID_BASE + 2) //��ȷ����ѡ�ť
#define QX_SYS_BUTTON_CANCEL			(QX_SYS_BUTTON_ID_BASE + 3) //���˳���ѡ�ť

// ѡ��ı��ѡ�еĻص��ӿ�,���øýӿڵĻص�������֪ͨ��ѡ���Ѿ��ı��ѡ����������
// Ŀǰ��Command��InputToolʵ��.
//
class kiOptionCallBack{
public:
	// ѡ��ֵ���ı��ˣ���������ֵ��ѡ��
	virtual void				OnOptionChanged(kiOptionItem *pOptItem) = 0;
	// ѡ�ѡ���ˣ�Ŀǰ��Ҫ��OptionButton�����ʱ������øú���֪ͨ
	virtual void				OnOptionSelected(kiOptionItem *pOptItem) = 0;
};

//////////////////////////////////////////////////////////////////////////
// һ��ѡ������Ƕ�������
// ����:name(key)<default value>
// ����:����(D)<20>,���о���Ϊname,DΪkey,20ΪĬ��ֵ��ʾ(�����).
// ��ͬ��Item���ݲ�ͬ��ID�����֡�
//
class QXLIB_API kiOptionItem : public kiInputEditCallBack
{
public:
	//pname:ѡ������ key:ѡ���Ӧ�Ŀ�ݼ���
	kiOptionItem(const char *pname,char key);
	kiOptionItem(const char *pname,char key,int id);
	virtual ~kiOptionItem();

	virtual int				GetType() const = 0;
	int						GetID() const { return m_nID;}
	void						SetID(int nID) { m_nID = nID; }
	void						EnablePopup(bool bEnable) { m_bPopupItem = bEnable; }
	//�Ƿ���Ч
	virtual bool				IsValid() const;

	//������ݵ���Ч��,���û��ڸ�item������ֵ,�����Ч��,��ȡ���.
	virtual bool				ParseInputText(const char *lpszText) = 0;

	// ���ڹ�����������ʾʱ,ʹ��,��ʾ����:����(D)<20>:�ľ���,�ȴ��û�����.
	// �������ӱ������ʾ,�����sub optionSet,�������optionSet.
	virtual bool				GetPromptText(std::string& strPrompt) = 0;

	// ����bool��enumʱ��Ҫ�����Ӧ��enumֵ��
	virtual kiOptionSet*		GetSubOptionSet() = 0;
	// ��Ҫ����bool��enumʱ,���ڼ�¼�����ȡ��ֵ��ѡ��.
	virtual bool				SetCurrentOption(int idx) = 0;

public://CallBack�ӿ�
	//��InputEdit�����ı��ı�ʱ
	virtual int				OnEditInputChanged(const char *pszText);

	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	// ��ʾѡ���Ӧ��ֵ�Ѿ����ı���,
	// ������ѡ��ֵ�����˸ı�ʱ��ͨ��������comand��inputtool������д���
	//
	void						NotifyOptionChanged();

public:
	std::string				m_strName;//����
	char						m_cKey;//��д���档��ݼ��ַ�.
	int						m_nID;//Ψһ��ʶѡ���id,������Ϊ-1.
	bool						m_bPopupItem;//�Ƿ񵯳������true���������ڣ���������ֵ.

public:
	virtual void				SetParentItem(kiOptionItem *pParentItem) { pParentItem_ = pParentItem; }
	
	// �趨�ص�����
	virtual void				SetOptionCallBack(kiOptionCallBack *pCallBack);
	bool						HasOptionCallBack() const;

protected:
	virtual void				SetInputEdit(kiInputEdit *pInputEdit);
	

	friend class kiOptionSet;
	
protected:
	kiOptionItem				*pParentItem_;//��Item��
	//����Edit�ӿ�.���ڽ���������ѡ���Ĺ���
	kiInputEdit				*pInputEdit_;

	// �ص��ӿ�,Ŀǰ��Command��InputToolʵ�֣���˿��Թ�����Ӧ�Ķ���
	// ��ѡ��任ʱ��֪ͨ��Щ����,Ŀǰ������Ҫ��OptionSet��������
	//   ���Ե����趨�ص����󣬻�����OptionSetͳһ�趨�ص������ṩ�㹻�������
	//   ��Item���趨ʱ������OptionSet�趨��
	//   ������߶����趨������Ҫ�ص�����,����Ϊ��
	kiOptionCallBack			*pOptionCallBack_;
};

// ����:name(key),����:��ɫ(R) ��ɫ(Y).û��ֵ.
// ��Ҫ����bool��enum��,Ϊ����.��ʾһ��ѡ��
// Ҳ���Ա�ʾһ����������ID���֡�
// 
class QXLIB_API kiOptionButton : public kiOptionItem{
public:
	kiOptionButton(const char *pname,char key);
	kiOptionButton(const char *pname,char key,int id);

	virtual int				GetType() const { return KOPT_INPUT_BUTTON; }

	// ������ݵ���Ч��,����Ӧ���ǿ�ݼ�key.
	virtual bool				ParseInputText(const char *lpszText);
	// 
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);
};

// ����:�Ƿ񿽱�(C)<��>
class QXLIB_API kiOptionBool : public kiOptionItem{
public:
	kiOptionBool(const char *pname,char key,bool& bVar);//bVar�Ǳ��������ã�����ᱣ�浽�ñ����С�
	kiOptionBool(const char *pname,char key,int id,bool& bVar);
	~kiOptionBool();

	virtual int				GetType() const { return KOPT_INPUT_BOOL; }

	//��תֵ.
	void						NotValue();

	//������ݵ���Ч��
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL;}
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	bool&					m_boolVar;//�����������á�
};

// ö��,����:����<����>(����(S) ����(M))
//
class QXLIB_API kiOptionEnum : public kiOptionItem{
public:
	kiOptionEnum(const char *pname,char key,int& eVal);
	kiOptionEnum(const char *pname,char key,int id,int& eVal);
	virtual ~kiOptionEnum();

	virtual int				GetType() const { return KOPT_INPUT_ENUM; }

	//���һ��ѡ��.
	bool						AddEnumItem(const char *pname,char key);

	//������ݵ���Ч��
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return m_pOptionSet;}
	virtual bool				SetCurrentOption(int idx);

	int						GetPrevEnumValue() const { return m_oldEnumVal; }

public:
	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	int&						m_enumVal;

protected:
	kiOptionSet				*m_pOptionSet;
	int						m_oldEnumVal;//�л�ǰ��ֵ
};

//��׼����ϵ��ѡ��
class QXLIB_API kiOptionStdAxis : public kiOptionEnum{
public:
	kiOptionStdAxis(const char *pname,char key,int& eVal);
	kiOptionStdAxis(const char *pname,char key,int id,int& eVal);

	int						Initialize(bool bUseWCS,bool bOnlyZ);

protected:
	bool						_bUseWCS;
	bool						_bOnlyZ;
};

// ����:����(D)<3>
//
class QXLIB_API kiOptionInt : public kiOptionItem{
public:
	kiOptionInt(const char *pname,char key,int& iVal);
	kiOptionInt(const char *pname,char key,int id,int& iVal);

	virtual int				GetType() const { return KOPT_INPUT_INT; }

	//������ݵ���Ч��
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	int&						m_intVar;//
};

// ����:����(D)<20.000>
//
class QXLIB_API kiOptionDouble : public kiOptionItem{
public:
	kiOptionDouble(const char *pname,char key,double& dval);
	kiOptionDouble(const char *pname,char key,int id,double& dval);

	virtual int				GetType() const { return KOPT_INPUT_DOUBLE; }

	//������ݵ���Ч��
	virtual bool				ParseInputText(const char *lpszText);
	virtual bool				GetPromptText(std::string& strPrompt);
	virtual kiOptionSet*		GetSubOptionSet() { return NULL; }
	virtual bool				SetCurrentOption(int idx) { return true;}

public:
	//��InputEdit�ı��������ʱ
	virtual int				OnEditInputFinished(const char *pszText);

	//��һ��ѡ�ѡ��ʱ�����磺ͨ������ַ������ͨ�����
	virtual int				OnEditOptionSelected(int idx,kiOptionItem *pItem);

public:
	double&					m_doubleVal;
};

/// һ��ѡ��ϣ����Ա�command��inputtoolʹ�ã���ѡ��ı�򱻵��ʱ��
//  �ᴥ����Ӧ����Ļص�������
//  ������InputToolʱ�����OptionSetû���趨IOptionCallBack������趨ΪInputTool��
//  ����ʹ��OptionSet�趨��IOptionCallBack�ӿ�

class QXLIB_API kiOptionSet{
public:
	kiOptionSet();
	kiOptionSet(kiOptionCallBack *pOptCallBack);
	~kiOptionSet();

	// �������������Ǳ����
	void						SetInputEdit(kiInputEdit *pInputEdit);

	// ����command��inputTool�������ϣ��ѡ��ı�ʱ����Ӧ����õ�֪ͨ����Ҫ
	// �趨�ù���
	void						SetOptionCallBack(kiOptionCallBack *pCallBack);
	bool						HasOptionCallBack() const;

	// ���һ��item��
	// Item����������������䣬������OptionSet�����ͷ�
	bool						AddOption(kiOptionItem* pItem);
	kiOptionItem*				AddBoolOption(const char *pname,char key,bool& bVar);
	kiOptionItem*				AddIntOption(const char *pname,char key,int& iVal);
	kiOptionItem*				AddDoubleOption(const char *pname,char key,double& dval);
	kiOptionItem*				AddButtonOption(const char *pname,char key,int nID);
	kiOptionItem*				AddSysButtonOption(int nID);
	void						AddApplyButtonOption();
	void						AddQuitButtonOption();

	//����һ��ѡ��
	bool						InsertOptionItem(size_t idx,kiOptionItem *pItem);
	bool						RemoveOptionItem(kiOptionItem *pItem);

	void						Clear();
	bool						IsEmpty() const;

	int						GetCount() const;
	kiOptionItem*				GetOption(int ix);

	//���ݼ���ȡ��Ӧ�±ꡣ
	int						FindOption(char c);
	int						IndexOfOption(char c);
	int						IndexOfOption(kiOptionItem *pItem);
	bool						HasOptionItem(kiOptionItem *pItem);

protected:
	void						DoAddItem(kiOptionItem *pItem);

protected:
	kiOptionCallBack			*pOptionCallBack_;  //�����Ķ���ѡ���޸�ʱ�����ö�Ӧ�Ļص�����
	stdOptionItemArray		aItemArray_;
	kiInputEdit				*pInputEdit_;//����������༭�ӿ�
};


#endif