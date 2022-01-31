// XInputEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "kiInputTool.h"
#include "XInputEdit.h"

////////////////////////////////////////////////////////////
//
XInputRequest::XInputRequest()
{
	_ssPromptText = "";
	_pInputEditCallBack = NULL;
	_pOptionSet = NULL;
}

XInputRequest::XInputRequest(const char *pszPrompt,kiInputEditCallBack *pCallBack,kiOptionSet *pOptSet)
{
	_ssPromptText = pszPrompt;
	_pInputEditCallBack = pCallBack;
	_pOptionSet = pOptSet;
}

XInputRequest::XInputRequest(const XInputRequest& other)
{
	*this = other;
}

XInputRequest& XInputRequest::operator=(const XInputRequest& other)
{
	_ssPromptText = other._ssPromptText;
	_pInputEditCallBack = other._pInputEditCallBack;
	_pOptionSet = other._pOptionSet;

	return *this;
}

////////////////////////////////////////////////////////////
// CXInputEdit

IMPLEMENT_DYNAMIC(CXInputEdit, CWnd)
CXInputEdit::CXInputEdit()
{
	m_szPromptText = "";
	m_szFullPromptText = "";
	m_szInputText = "";
	m_szFullText = "";
	m_promptRect.SetRectEmpty();
	m_bHasFocus = FALSE;

	m_selOptionColor = RGB(0,0,255);
	m_nCaretHeight = 0;
	m_nMargin = -1;

	_pInputEditCallBack = NULL;
	m_pOptionSet = NULL;
}

CXInputEdit::~CXInputEdit()
{
}

// 必要的初始化
//
BOOL	CXInputEdit::Initialize()
{
	if(m_nMargin >= 0)//已经初始化过了。
		return TRUE;

	//计算必要的参数信息
	CRect rcClient;
	GetClientRect(&rcClient);
	if(rcClient.Width() <= 0)
		return FALSE;

	TEXTMETRIC tm;
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject(&m_textFont);
	dc.GetTextMetrics(&tm);
	dc.SelectObject(pOldFont);

	//计算文字的上下间隙和光标的高度
	m_nMargin = (rcClient.Height() - tm.tmHeight) / 2;
	m_nCaretHeight = tm.tmHeight + 2;
	
	//必要的初始化
	m_pOptionSet = NULL;
	m_promptRect.SetRectEmpty();

	return TRUE;
}

//显示一行提示信息，等待用户输入数据
// 信息中可以包含多个选项。一行文本分为如下三部分：
//  提示信息 多个选项 : 输入文本
//  如果pOptionSet为空，则没有选项文本
//
int		CXInputEdit::RequestInput(const char *pPromptText,
					 			  kiInputEditCallBack *pEditCallBack,
								  kiOptionSet *pOptionSet)
{
	ASSERT(pPromptText && pEditCallBack);
	ASSERT(m_nMargin > 0);//已经初始化了

	//清除旧的
	m_szInputText = _T("");
	m_szFullText = _T("");
	m_chrPosArray.clear();
	m_optPosArray.clear();

	//重新计算
	m_szPromptText = pPromptText;
	_pInputEditCallBack = pEditCallBack;
	m_pOptionSet = pOptionSet;
	if(m_pOptionSet){
		m_pOptionSet->SetInputEdit(this);//必要的关联
	}

	CalcPromptTextInfo();
	m_szFullText = m_szFullPromptText;

	m_nCaretIdx = 0;
	Invalidate(FALSE);

	return 1;
}

// 提示一行文本，不要求输入
//
void  CXInputEdit::PromptText(const char *lpszText)
{
	ASSERT(lpszText);
	//清除旧的
	m_szInputText = _T("");
	m_szFullText = _T("");
	m_chrPosArray.clear();
	m_optPosArray.clear();

	m_szPromptText = lpszText;
	_pInputEditCallBack = NULL;
	m_pOptionSet = NULL;

	//计算输入文本的大小。定位caret。
	CalcPromptTextInfo();
	m_szFullText = m_szFullPromptText;

	m_nCaretIdx = 0;

	//重画。
	Invalidate(FALSE);
}

// 更新所有,主要更新文字的显示和选项的值
//
void  CXInputEdit::Update()
{
	if(!_pInputEditCallBack){
		ASSERT(FALSE);
		return;
	}

	//不能清除输入文本信息
	m_szFullPromptText = "";
	
	//重新计算
	CalcPromptTextInfo();
	//重新构造完整文本行
	m_szFullText = m_szFullPromptText + m_szInputText;

	Invalidate(FALSE);
}

//获取输入的文本
std::string  CXInputEdit::GetInputText() const
{
	std::string szText = "";
	if(!m_szInputText.IsEmpty()){
		int ix,len = m_szInputText.GetLength();
		for(ix = 0;ix < len;ix ++){
			szText += m_szInputText.GetAt(ix);
		}
	}
	return szText;
}

// 根据提示文本和选项，计算提示文本的信息，主要是：
//  在Edit中所占的区域范围
//  选项的位置信息等
//
BOOL  CXInputEdit::CalcPromptTextInfo()
{
	ASSERT(m_nMargin > 0);
	ASSERT(!m_szPromptText.IsEmpty());
	//清除旧的
	m_optPosArray.clear();
	m_promptRect.SetRectEmpty();

	CSize sz;
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject(&m_textFont);

	if(!m_pOptionSet){//没有选项
		m_szFullPromptText = m_szPromptText + " : ";//加上分割符
	}else{
		std::string ssOptText;
		int ix,nbOpt = m_pOptionSet->GetCount();

		m_szFullPromptText = m_szPromptText;
		m_szFullPromptText += " ( ";//开始字符

		for(ix = 0;ix < nbOpt;ix ++){
			kiOptionItem  *pItem = m_pOptionSet->GetOption(ix);
			XOptionPos optPos;

			sz = dc.GetTextExtent(m_szFullPromptText);
			//记录option的范围
			optPos._rect.left = sz.cx + m_nMargin;
			optPos._rect.top = m_nMargin;
			optPos._rect.bottom = m_nMargin + sz.cy;

			//添加上选项文本后，再次计算范围
			pItem->GetPromptText(ssOptText);
			m_szFullPromptText += ssOptText.c_str();
			sz = dc.GetTextExtent(m_szFullPromptText);
			optPos._rect.right = m_nMargin + sz.cx;

			if(ix < nbOpt - 1)
				m_szFullPromptText += "  ";

			m_optPosArray.push_back(optPos);
		}
		m_szFullPromptText += ") : ";
	}

	//记录包含选项的提示文本的完整范围
	sz = dc.GetTextExtent(m_szFullPromptText);
	m_promptRect.left = m_nMargin;
	m_promptRect.top = m_nMargin;
	m_promptRect.right = m_nMargin + sz.cx;
	m_promptRect.bottom = m_nMargin + sz.cy;
	
	dc.SelectObject(pOldFont);

	return TRUE;
}

//将一个输入请求压入栈中，以便后面恢复该请求
//
int  CXInputEdit::PushRequest()
{
	if(!m_szPromptText.IsEmpty()){
		XInputRequest inputReq(m_szPromptText,_pInputEditCallBack,m_pOptionSet);
		m_inputReqStack.push(inputReq);

		return 1;
	}

	return 0;
}

// 从栈中恢复一个前面的输入请求
//
int  CXInputEdit::PopRequest()
{
	if(!m_inputReqStack.empty()){
		XInputRequest inputReq = m_inputReqStack.top();
		//重新请求
		this->RequestInput(inputReq._ssPromptText.c_str(),inputReq._pInputEditCallBack,inputReq._pOptionSet);
		
		m_inputReqStack.pop();

		return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////
//

BEGIN_MESSAGE_MAP(CXInputEdit, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_CHAR()
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CXInputEdit 消息处理程序
int  CXInputEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 创建特定的字体
	CFont font;
	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));
	lf.lfHeight = 12;
	strcpy_s(lf.lfFaceName,32,"宋体");
	m_textFont.CreateFontIndirect(&lf);

	return 0;
}

void CXInputEdit::OnDestroy()
{
	CWnd::OnDestroy();

	if(m_textFont.GetSafeHandle())
		m_textFont.DeleteObject();
}

// 绘制Edit的区域，高亮显示当前选项
//
void CXInputEdit::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect cliRect;
	GetClientRect(&cliRect);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	ASSERT(memDC.GetSafeHdc());

	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc,cliRect.Width(),cliRect.Height());
	ASSERT(memBmp.GetSafeHandle());

	CBitmap *pOldBmp = memDC.SelectObject(&memBmp);
	CFont *pOldFont = memDC.SelectObject(&m_textFont);

	//填充
	memDC.FillSolidRect(&cliRect,RGB(255,255,255));

	//绘制文本,绘制完整的文本，包括：提示信息和选项。
	if(!m_szFullText.IsEmpty())
		memDC.TextOut(m_nMargin,m_nMargin,m_szFullText);

	//绘制选中的选项，在上面绘制基础上重新绘制一次。
	if(!m_optPosArray.empty()){
		CString szName;
		std::string str;
		int ix,isize = (int)m_optPosArray.size();
		COLORREF textcol = memDC.GetTextColor();

		for(ix = 0;ix < isize;ix ++){
			XOptionPos& optPos = m_optPosArray.at(ix);

			//高亮显示当前选项
			if(optPos._bHilight){
				kiOptionItem *pItem = m_pOptionSet->GetOption(ix);

				pItem->GetPromptText(str);
				szName = str.c_str();

				memDC.SetTextColor(m_selOptionColor);
				memDC.TextOut(optPos._rect.left,optPos._rect.top,szName);
				//绘制一条线。
				CPen pen(PS_SOLID,1,m_selOptionColor);
				CPen *pOldPen = memDC.SelectObject(&pen);

				memDC.MoveTo(optPos._rect.left,optPos._rect.bottom);
				memDC.LineTo(optPos._rect.right,optPos._rect.bottom);
				memDC.SelectObject(pOldPen);

				break;
			}	
		}
		memDC.SetTextColor(textcol);
	}

	//绘制边框
	CBrush bsh(RGB(0,64,128));//GetSysColor(COLOR_3DSHADOW));
	memDC.FrameRect(&cliRect,&bsh);

	dc.BitBlt(0,0,cliRect.Width(),cliRect.Height(),&memDC,0,0,SRCCOPY);

	memDC.SelectObject(pOldFont);
	memDC.SelectObject(pOldBmp);
	memBmp.DeleteObject();
	memDC.DeleteDC();
}

// 通常仅仅宽度变换，不要特别处理。
//
void CXInputEdit::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);


}

void CXInputEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_DELETE){
		DeleteChar(FALSE);
	}else if(nChar == VK_BACK){
		DeleteChar(TRUE);
	}else if(nChar == VK_SPACE || nChar == VK_RETURN){
		//调用回调函数,还可以适当的检查结果.
		// 输入可以为空.
		//调用回调函数
		if(_pInputEditCallBack){
			//判断是不是选项字符
			if(m_pOptionSet && m_szInputText.GetLength() == 1){
				int iSel = m_pOptionSet->FindOption(m_szInputText.GetAt(0));
				if(iSel >= 0){
					kiOptionItem *pOptionItem = m_pOptionSet->GetOption(iSel);
					pOptionItem->OnEditOptionSelected(iSel,pOptionItem);
					//_pInputEditCallBack->OnEditOptionSelected(iSel,pOptionItem);
				}else{
					_pInputEditCallBack->OnEditInputFinished(m_szInputText);
				}
			}else{
				_pInputEditCallBack->OnEditInputFinished(m_szInputText);
			}
		}
	}else if(nChar == VK_LEFT){
		//左移光标
		if(m_nCaretIdx > 0){
			m_nCaretIdx --;
			UpdateCaret();
		}
	}else if(nChar == VK_RIGHT){
		//右移光标
		if(m_nCaretIdx < (int)m_chrPosArray.size()){
			m_nCaretIdx ++;
			UpdateCaret();
		}
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

// 删除当前光标前或后一个字符。
//
int  CXInputEdit::DeleteChar(BOOL bBack)
{
	if(m_szInputText.IsEmpty())
		return 0;

	int nsize = (int)m_chrPosArray.size();
	int nLen = m_szInputText.GetLength();
	ASSERT(nsize == nLen);

	//没有什么可删了
	if(bBack && m_nCaretIdx == 0 ||
	   !bBack && m_nCaretIdx == nsize)
	   return 0;

	if(!bBack && m_nCaretIdx == nsize - 1 ||
	   bBack && m_nCaretIdx == nsize)
	{//删除最后一个
		m_szInputText.Delete(nLen - 1,1);
		nLen = m_szFullText.GetLength();
		m_szFullText.Delete(nLen - 1);

		m_chrPosArray.pop_back();
		if(bBack)
			m_nCaretIdx --;
	}else{
		//删除中间的。
		CClientDC dc(this);
		CSize sz,sz2;
		CFont *pOldFont = dc.SelectObject(&m_textFont);
		int nIdx = 0,ndf = 0,dx = 0;
		int ix,nL,nR,xShift = 0;
		CString szL,szR,szLL;

		//分离两个字符串
		if(bBack){
			nIdx = m_nCaretIdx - 1;//被删除字符下标
		}else{
			nIdx = m_nCaretIdx;
		}
		nL = nIdx + 1;//保证总是删除左边字符串的最后一个
		nR = nsize - nL;
		szL = m_szInputText.Left(nL);
		szR = m_szInputText.Right(nR);
		
		//
		sz = dc.GetTextExtent(szL);
		if(nIdx > 0){
			szLL = szL.Left(nIdx);
			sz2 = dc.GetTextExtent(szLL);
			xShift = sz.cx - sz2.cx;// > 0
		}else{//就删除开始一个字符
			szLL = "";
			xShift = sz.cx;
		}
		//调整后面对应项。
		for(ix = nIdx + 1;ix < nsize;ix ++){
			m_chrPosArray[ix]._nStart -= xShift;
			m_chrPosArray[ix]._nEnd -= xShift;
		}
		//删除
		m_chrPosArray.erase(m_chrPosArray.begin() + nIdx);
		
		//重构字符串
		m_szInputText = szLL + szR;
		m_szFullText = m_szFullPromptText + m_szInputText;
				
		if(bBack)
			m_nCaretIdx --;

		dc.SelectObject(pOldFont);
	}

	UpdateCaret();
	Invalidate(FALSE);

	return 0;
}

void CXInputEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CXInputEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TRACE("\n onchar %d.",nChar);
	char c = (char)nChar;
	if((c >= '0' && c <= '9') || 
		(c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z'))
	{
		AcceptChar(c);
	}
	else if(c == '.' || c == ',' || c == '+' || c == '-')
	{
		AcceptChar(c);
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

// 接受一个字符,可能是插入，也可能是在后面追加。
//
int  CXInputEdit::AcceptChar(char c)
{
	CClientDC dc(this);
	CSize sz;
	int ix,nsize = (int)m_chrPosArray.size();
	CFont *pOldFont = dc.SelectObject(&m_textFont);

	if(m_szInputText.IsEmpty()){
		ASSERT(m_nCaretIdx == 0);
	}

	ASSERT(m_szInputText.GetLength() == nsize);//两个数组个数始终相同
	if(m_nCaretIdx == nsize){//末尾追加字符
	
		m_szInputText += (char)c;
		m_szFullText += (char)c;
		//
		sz = dc.GetTextExtent(m_szInputText);
		//
		XCharPos cpos;
		if(nsize == 0){//第一个输入
			cpos._nStart = m_promptRect.right;
		}else{
			cpos._nStart = m_chrPosArray[nsize - 1]._nEnd;
		}
		cpos._nEnd = m_promptRect.right + sz.cx;
		m_chrPosArray.push_back(cpos);

		//更新光标位置
		m_nCaretIdx ++;
		//
	}else{//插入字符

		CString szL,szR;
		int nl = m_nCaretIdx;
		int nr = nsize - nl; 
		int xShift = 0;//偏移的距离

		//获取光标左右的字符串。
		if(nl > 0){
			szL = m_szInputText.Left(nl);
			szR = m_szInputText.Right(nr);
		}else{
			szL.Empty();
			szR = m_szInputText;
		}
		//
		szL += c;
		sz = dc.GetTextExtent(szL);

		XCharPos cpos;
		cpos._nStart = m_chrPosArray[nl]._nStart;//原来字符的起始位置
		cpos._nEnd = m_promptRect.right + sz.cx;//
		xShift = cpos._nEnd - cpos._nStart;
		//调整后面的
		for(ix = m_nCaretIdx;ix < nsize;ix ++){
			m_chrPosArray[ix]._nStart += xShift;
			m_chrPosArray[ix]._nEnd += xShift;
		}
		//插入新的
		m_chrPosArray.insert(m_chrPosArray.begin() + m_nCaretIdx,cpos);

		//重构文本
		m_szInputText = szL + szR;
		m_szFullText = m_szFullPromptText + m_szInputText;
		
		// 光标更新。
		m_nCaretIdx ++;
	}

	dc.SelectObject(pOldFont);

	UpdateCaret();

	Invalidate(FALSE);
	return 0;
}

void  CXInputEdit::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	CreateSolidCaret(1,m_nCaretHeight);
	UpdateCaret();

	m_bHasFocus = TRUE;
}

void CXInputEdit::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	HideCaret();
	DestroyCaret();

	m_bHasFocus = FALSE;
}

void CXInputEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CXInputEdit::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(!m_bHasFocus){
		CRect rc;
		GetClientRect(&rc);
		if(rc.PtInRect(point)){
			CalcCaretPos(point);
			SetFocus();
		}
	}else{
		//计算光标的位置。
		CalcCaretPos(point);
		UpdateCaret();
	}

	//计算是否点击到选项上.
	int isel = HitOptionTest(point);
	if(isel >= 0){
		//调用回调函数,这里调用Item的回调函数
		if(_pInputEditCallBack){
			kiOptionItem *pItem = m_pOptionSet->GetOption(isel);
			pItem->OnEditOptionSelected(isel,pItem);
			//_pInputEditCallBack->OnEditOptionSelected(isel,pItem);//应当改为clicked
		}
	}

	CWnd::OnLButtonUp(nFlags, point);
}

// 根据点计算对应的caret的位置.
//
void		CXInputEdit::CalcCaretPos(CPoint point)
{
	if(point.x < m_nMargin || m_szFullText.IsEmpty()){//起始位置
		m_nCaretIdx = 0;
		return;
	}

	if(point.x < m_promptRect.right || m_chrPosArray.empty()){
		m_nCaretIdx = 0;
		return;
	}

	//从数组中进行查找
	int ix,isize = (int)m_chrPosArray.size();
	if(point.x >= m_chrPosArray[isize - 1]._nEnd)
		m_nCaretIdx = isize;//最后
	else if(point.x <= m_chrPosArray[0]._nStart)
		m_nCaretIdx = 0;
	else{
		int nm = 0;
		for(ix = 0;ix < isize;ix ++){
			XCharPos& cpos = m_chrPosArray.at(ix);
			nm = (cpos._nStart + cpos._nEnd) / 2;
			if(point.x >= cpos._nStart && point.x < nm){
				m_nCaretIdx = ix;
				break;
			}else if(point.x >= nm && point.x < cpos._nEnd){
				m_nCaretIdx = ix + 1;
				break;
			}
		}
	}
}

void	CXInputEdit::UpdateCaret()
{
	POINT pos;

	pos.y = m_nMargin - 1;
	if(m_szFullText.IsEmpty())
		pos.x = m_nMargin;
	else if(m_chrPosArray.empty()){
		ASSERT(m_szInputText.IsEmpty());
		pos.x = m_promptRect.right;
	}else{
		int nSize = (int)m_chrPosArray.size();
		ASSERT(m_nCaretIdx >= 0 && m_nCaretIdx <= nSize);
		if(m_nCaretIdx == nSize)
			pos.x = m_chrPosArray[nSize - 1]._nEnd;
		else
			pos.x = m_chrPosArray[m_nCaretIdx]._nStart;
	}

	if(m_bHasFocus)
		HideCaret();
	SetCaretPos(pos);
	ShowCaret();
}

//设定标记，允许onchar接收所有的键值
//
UINT CXInputEdit::OnGetDlgCode()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	UINT nFlag = CWnd::OnGetDlgCode();
	nFlag |= DLGC_WANTALLKEYS;
	return nFlag;
}

void CXInputEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if((nFlags & MK_LBUTTON) || (nFlags & MK_RBUTTON)){
		//do nothing
	}else{
		HitOptionTest(point);
		Invalidate(FALSE);
	}

	CWnd::OnMouseMove(nFlags, point);
}

// 是否掠过或点击了选项
//
int	CXInputEdit::HitOptionTest(CPoint point)
{
	int ix,isel = -1,isize = (int)m_optPosArray.size();

	for(ix = 0;ix < isize;ix ++){
		XOptionPos& optPos = m_optPosArray.at(ix);
		CRect rect(optPos._rect);
		if(rect.PtInRect(point)){
			isel = ix;
			optPos._bHilight = true;
		}else{
			optPos._bHilight = false;
		}
	}

	return isel;
}