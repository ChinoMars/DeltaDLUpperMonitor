// StaticHelp_Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Delta_dL.h"
#include "StaticHelp_Dlg.h"


// CStaticHelp_Dlg 对话框

IMPLEMENT_DYNAMIC(CStaticHelp_Dlg, CDialog)

CStaticHelp_Dlg::CStaticHelp_Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStaticHelp_Dlg::IDD, pParent)
{

}

CStaticHelp_Dlg::~CStaticHelp_Dlg()
{
}

void CStaticHelp_Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStaticHelp_Dlg, CDialog)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR1, &CStaticHelp_Dlg::OnNMThemeChangedScrollbar1)
END_MESSAGE_MAP()


// CStaticHelp_Dlg 消息处理程序

void CStaticHelp_Dlg::OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
