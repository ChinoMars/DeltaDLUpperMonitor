#pragma once


// CStaticHelp_Dlg �Ի���

class CStaticHelp_Dlg : public CDialog
{
	DECLARE_DYNAMIC(CStaticHelp_Dlg)

public:
	CStaticHelp_Dlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CStaticHelp_Dlg();

// �Ի�������
	enum { IDD = IDD_STATIC_HELP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult);
};
