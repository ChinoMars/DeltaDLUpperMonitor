// StaticHelp_Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Delta_dL.h"
#include "StaticHelp_Dlg.h"


// CStaticHelp_Dlg �Ի���

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


// CStaticHelp_Dlg ��Ϣ�������

void CStaticHelp_Dlg::OnNMThemeChangedScrollbar1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// �ù���Ҫ��ʹ�� Windows XP ����߰汾��
	// ���� _WIN32_WINNT ���� >= 0x0501��
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}
