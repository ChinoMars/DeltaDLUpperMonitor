#pragma once


// CHelpDoc �Ի���

class CHelpDoc : public CDialog
{
	DECLARE_DYNAMIC(CHelpDoc)

public:
	CHelpDoc(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CHelpDoc();

public:
	virtual BOOL OnInitDialog();

// �Ի�������
	enum { IDD = IDD_HELP_DOC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
