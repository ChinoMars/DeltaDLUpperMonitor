// Delta_dLDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "CurveLine.h"

#include "EasySize.h"

// CDelta_dLDlg �Ի���
class CDelta_dLDlg : public CDialog
{
	// ���� EasySize
	DECLARE_EASYSIZE
// ����
public:
	CDelta_dLDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DELTA_DL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

//���������
	CCurveLine* DiffCurv;
	CCurveLine* DestinateCurv;
	CCurveLine* CurvFromOSA;

	void DrawCurve( CCurveLine& Curve,int ID );
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	void GetInputData(int nID, CString ErrorMsg, DWORD &Value);
	void GetMyIDs(void);
	CString UcharToUnicode(UCHAR *puchar, UINT length);
public:
	void OnSearch(void);
	bool bFoundDevice;
	CString strLog;
	CString strPath;

	CListBox m_ctllHIDdevices;
	afx_msg void OnBnClickedButton1();
	CButton m_Measure_btn;
	afx_msg void OnBnClickedConnectbtn();
	CEdit m_Vid;
	CEdit m_Pid;
	afx_msg void OnBnClickedMeasurebtn();
	CComboBox m_Measure_Comb;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeMeasureCombo();
	afx_msg LRESULT OnMyDeviceChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedClosebtn();
	CEdit m_Connect_state;
	CEdit m_FSR;
	CEdit m_DeltL;
	CProgressCtrl m_Progress;
	CStatic m_PRO;
	CEdit m_Zheshelv;
	afx_msg void OnEnChangeEdit2();
	float m_nReflect;
	CEdit m_Loss;
	afx_msg void OnEnChangeEditFilename();
	CFont m_font;
	CButton m_Save_btn;
	afx_msg void OnBnClickedSavebtn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CList<CRect,CRect> m_listRect;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeEditProdid();
};
