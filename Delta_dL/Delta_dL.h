// Delta_dL.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDelta_dLApp:
// �йش����ʵ�֣������ Delta_dL.cpp
//

class CDelta_dLApp : public CWinApp
{
public:
	CDelta_dLApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CDelta_dLApp theApp;