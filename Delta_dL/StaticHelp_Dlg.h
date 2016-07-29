#pragma once


// CStaticHelp_Dlg 对话框

class CStaticHelp_Dlg : public CDialog
{
	DECLARE_DYNAMIC(CStaticHelp_Dlg)

public:
	CStaticHelp_Dlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStaticHelp_Dlg();

// 对话框数据
	enum { IDD = IDD_STATIC_HELP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult);
};
