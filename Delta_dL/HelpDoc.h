#pragma once


// CHelpDoc 对话框

class CHelpDoc : public CDialog
{
	DECLARE_DYNAMIC(CHelpDoc)

public:
	CHelpDoc(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHelpDoc();

public:
	virtual BOOL OnInitDialog();

// 对话框数据
	enum { IDD = IDD_HELP_DOC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
