// Delta_dLDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Delta_dL.h"
#include "Delta_dLDlg.h"

#include "EasySize.h"

#include "fstream"
#include "iostream"

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

using std::endl;
using std::ofstream;
using std::cin;
using std::endl;
using std::cout;



#include "dbt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"hid.lib")
#pragma comment(lib,"hidparse.lib")
#pragma comment(lib,"hidclass.lib")
#pragma comment(lib,"setupapi.lib")

extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}



//
#define DATA_LENGTH 4000
#define PACKAGE_NUM 133
#define PACKAGE_DATA_LEN 30


// ��Ŀ¼
//CString ROOT_DIR = _T("C:\\Users\\Chino\\Desktop\\DeltaDLData");
//CString ROOT_DIR = _T("C:\\Users\\Administrator\\Desktop\\DeltaDLData");
CString ROOT_DIR = _T("D:\\Delta_dL\\DeltaDLData");



//���������ҵ����豸·��
CString MyDevPathName=_T("");
//���������������VID��PID���汾��
DWORD MyVid,MyPid,MyPvn;
//������������ݵ��豸���
HANDLE hReadHandle=INVALID_HANDLE_VALUE;
//��������д���ݵ��豸���
HANDLE hWriteHandle=INVALID_HANDLE_VALUE;

//���������豸�Ƿ��Ѿ��ҵ�
BOOL MyDevFound=FALSE;

/////////////////////////////////////
HIDP_CAPS					Capabilities;
PHIDP_PREPARSED_DATA		HidParsedData;

//���ͱ����õ�OVERLAPPED��
OVERLAPPED WriteOverlapped;
//���ձ����õ�OVERLAPPED��
OVERLAPPED ReadOverlapped;

//ָ��������̵߳�ָ��
CWinThread * pReadReportThread;
//ָ��д�����̵߳�ָ��
CWinThread * pWriteReportThread;

//���ͱ���Ļ�������1�ֽڱ���ID+8�ֽڱ������ݡ�
UCHAR WriteReportBuffer[512]={0};

//���ձ���Ļ�������1�ֽڱ���ID+8�ֽڱ������ݡ�
UCHAR ReadReportBuffer[512]={0};

//��ǵ�ǰ�������������Ƿ���
UCHAR Send_flag = 0;

unsigned short raw_data[DATA_LENGTH];


UINT WriteReportThread(LPVOID pParam);
UINT ReadReportThread(LPVOID pParam);
void Data_process(LPVOID pParam);

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDelta_dLDlg �Ի���




CDelta_dLDlg::CDelta_dLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDelta_dLDlg::IDD, pParent)
	, m_nReflect(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDelta_dLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ctllHIDdevices);
	DDX_Control(pDX, IDC_MEASURE_btn, m_Measure_btn);
	DDX_Control(pDX, IDC_SAVE_btn, m_Save_btn);
	DDX_Control(pDX, IDC_VID, m_Vid);
	DDX_Control(pDX, IDC_PID, m_Pid);
	DDX_Control(pDX, IDC_MEASURE_COMBO, m_Measure_Comb);
	DDX_Control(pDX, IDC_CONNECT_STATE, m_Connect_state);
	DDX_Control(pDX, IDC_FSR, m_FSR);
	DDX_Control(pDX, IDC_DELTL, m_DeltL);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Control(pDX, IDC_PRO, m_PRO);
	DDX_Control(pDX, IDC_EDIT2, m_Zheshelv);
	DDX_Control(pDX, IDC_LOSS, m_Loss);
}

BEGIN_MESSAGE_MAP(CDelta_dLDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONNECT_btn, &CDelta_dLDlg::OnBnClickedConnectbtn)
	ON_BN_CLICKED(IDC_MEASURE_btn, &CDelta_dLDlg::OnBnClickedMeasurebtn)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_MEASURE_COMBO, &CDelta_dLDlg::OnCbnSelchangeMeasureCombo)
	ON_MESSAGE(WM_DEVICECHANGE, OnMyDeviceChange)
	//ON_WM_DEVICECHANGE() //����Ϣ���Ǵ����豸���ɾ��  
	ON_BN_CLICKED(IDC_CLOSE_btn, &CDelta_dLDlg::OnBnClickedClosebtn)
	ON_EN_CHANGE(IDC_EDIT2, &CDelta_dLDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_SAVE_btn, &CDelta_dLDlg::OnBnClickedSavebtn)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT_PRODID, &CDelta_dLDlg::OnEnChangeEditProdid)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CDelta_dLDlg)
	EASYSIZE(IDC_STATIC_TITLEGROUP,ES_BORDER,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_STATIC_TITLE,IDC_STATIC_SUBTITLE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_HCENTER)
	EASYSIZE(IDC_STATIC,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_STATIC_SUBTITLE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_CONNECT_STATE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_EDIT2,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_DELTL,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_LOSS,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_FSR,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_MEASURE_COMBO,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_PRO,ES_BORDER,IDC_STATIC_RESTGROUP,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_PROGRESS1,IDC_PRO,IDC_STATIC_RESTGROUP,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDC_STATIC_RESTGROUP,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)

	EASYSIZE(IDC_MEASURE_btn,ES_BORDER,ES_KEEPSIZE,IDC_CONNECT_btn,ES_BORDER,ES_HCENTER)
	EASYSIZE(IDC_CONNECT_btn,IDC_MEASURE_btn,ES_KEEPSIZE,IDC_CLOSE_btn2,ES_BORDER,0)
	EASYSIZE(IDC_CLOSE_btn2,IDC_CONNECT_btn,ES_KEEPSIZE,IDC_SAVE_btn,ES_BORDER,0)
	EASYSIZE(IDC_SAVE_btn,IDC_CLOSE_btn2,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	
	EASYSIZE(IDC_STATIC_INFOGROUP,ES_BORDER,IDC_MEASURE_btn,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_STATIC_PRODID,ES_BORDER,IDC_STATIC_INFOGROUP,ES_KEEPSIZE,IDC_STATIC_INFOGROUP,ES_VCENTER)
	EASYSIZE(IDC_STATIC_PRODCODE,ES_BORDER,IDC_STATIC_INFOGROUP,ES_KEEPSIZE,IDC_STATIC_INFOGROUP,ES_VCENTER)
	EASYSIZE(IDC_STATIC_OPERATOR,ES_BORDER,IDC_STATIC_INFOGROUP,ES_KEEPSIZE,IDC_STATIC_INFOGROUP,ES_VCENTER)
	EASYSIZE(IDC_EDIT_PRODID,ES_BORDER,IDC_STATIC_PRODID,ES_KEEPSIZE,IDC_STATIC_PRODID,ES_VCENTER)
	EASYSIZE(IDC_EDIT_PRODCODE,ES_BORDER,IDC_STATIC_PRODID,ES_KEEPSIZE,IDC_STATIC_PRODID,ES_VCENTER)
	EASYSIZE(IDC_EDIT_OPERATOR,ES_BORDER,IDC_STATIC_PRODID,ES_KEEPSIZE,IDC_STATIC_PRODID,ES_VCENTER)
	
	EASYSIZE(MUBIAO_CURV_ID,IDC_MEASURE_COMBO,ES_BORDER,ES_BORDER,IDC_STATIC_INFOGROUP,0)


END_EASYSIZE_MAP


// CDelta_dLDlg ��Ϣ�������

BOOL CDelta_dLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_Measure_Comb.AddString(_T("long (>50m)"));
	m_Measure_Comb.AddString(_T("mid 1 (5m--50m)"));
	m_Measure_Comb.AddString(_T("mid 2 (0.5m--5m)"));
	m_Measure_Comb.AddString(_T("short (<0.5m)"));
	m_Measure_Comb.SetCurSel(1);

	m_ctllHIDdevices.ShowWindow(SW_HIDE);
	m_Measure_btn.EnableWindow(TRUE);
	//m_Save_btn.EnableWindow(FALSE);

	// ����FSR��
	m_FSR.ShowWindow(SW_HIDE);

	m_Zheshelv.SetWindowText(_T("1.45"));
	m_Pid.SetWindowText(_T("yes"));
	m_Connect_state.SetWindowText(_T("δ��⵽�豸"));

	m_PRO.SetWindowText(_T("�������:"));
	m_Progress.SetPos(100);

	// ���ñ��������ֺ�
	m_font.CreatePointFont(300,_T("����"),NULL);

	// ���ڵ������ڴ�Сʱ�Զ������ؼ���С
	INIT_EASYSIZE;

	// �������ݴ洢Ŀ¼
	if (!PathIsDirectory(ROOT_DIR))
	{
		if (!CreateDirectory(ROOT_DIR, NULL))
		{
			MessageBox(_T("��������Ŀ¼ʧ��"));
		}
	}


	//��ʼ��д����ʱ�õ�Overlapped�ṹ��
	//ƫ��������Ϊ0
	WriteOverlapped.Offset=0;
	WriteOverlapped.OffsetHigh=0;
	//����һ���¼����ṩ��WriteFileʹ�ã���WriteFile���ʱ��
	//�����ø��¼�Ϊ����״̬���ֹ������¼�����ʼ��Ϊ���ź�
	WriteOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);


	//��ʼ��������ʱ�õ�Overlapped�ṹ��
	//ƫ��������Ϊ0
	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	//����һ���¼����ṩ��ReadFileʹ�ã���ReadFile���ʱ��
	//�����ø��¼�Ϊ����״̬��
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//����д������̣߳����ڹ���״̬��
	pWriteReportThread=AfxBeginThread(WriteReportThread,
		                                this,
										THREAD_PRIORITY_NORMAL,
										0,
										CREATE_SUSPENDED,
										NULL);
	//��������ɹ�����ָ����̵߳�����
	if(pWriteReportThread!=NULL)
	{
		pWriteReportThread->ResumeThread();
	}

	//����һ����������̣߳����ڹ���״̬��
	pReadReportThread=AfxBeginThread(ReadReportThread,
		                                this,
									THREAD_PRIORITY_NORMAL,
									0,
									CREATE_SUSPENDED,
									NULL);
	//��������ɹ�����ָ����̵߳�����
	if(pReadReportThread!=NULL)
	{
		pReadReportThread->ResumeThread();
	}

	//ע�����USB����¼�
	// ע��  
	GUID myHID_GUID = {0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED }};


	HidD_GetHidGuid(&myHID_GUID);
	DEV_BROADCAST_DEVICEINTERFACE Filter;  
	ZeroMemory(&Filter,sizeof(Filter));  
	Filter.dbcc_size = sizeof(Filter);   // size gets set to 29 with 1-byte packing or 32 with 4- or 8-byte packing  
	Filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;  
	Filter.dbcc_classguid = myHID_GUID;  
	//DEVICE_NOTIFY_ALL_INTERFACE_CLASSES //��ע�����豸�¼�  
	if(NULL == RegisterDeviceNotification(this->GetSafeHwnd(),&Filter,DEVICE_NOTIFY_WINDOW_HANDLE))
		TRACE("RegisterDeviceNotification failed!!");  



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}




void CDelta_dLDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDelta_dLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CDelta_dLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDelta_dLDlg::OnSearch(void)
{
	m_ctllHIDdevices.ResetContent(); //list�ؼ���������ʾ���е�HID�豸
	UpdateData(FALSE); //���½���

	CString temp;
	int Count = 0; //Total number of devices found
	DWORD strSize=0,requiredSize=0;
	BOOL result1,result2;
	ULONG DeviceInterfaceDetailDataSize;
	//����һЩ�������Ժ���õ�
	SP_DEVINFO_DATA DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA  DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

	//��һ������ȡdeviceID
	GUID deviceId;
	HidD_GetHidGuid(&deviceId);
		//�ڶ�������ȡ�豸��Ϣ
	HDEVINFO handle;
	handle = SetupDiGetClassDevs(&deviceId, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT); //Get only HID devices
	//�������������е��豸����ö��
	//SetupDiEnumDeviceInterfaces();
	result1=false; //����һЩ����
	result2=false;
	CString temp11;
	do
	{
		DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		result1 = SetupDiEnumDeviceInterfaces(	handle,
												NULL, // IN PSP_DEVINFO_DATA  DeviceInfoData,  OPTIONAL
												&deviceId,
												Count,
												&DeviceInterfaceData
												);
		//����豸��ϸ���ݣ�������
		SetupDiGetDeviceInterfaceDetail(handle,
										&DeviceInterfaceData,
										NULL,
										0,
										&strSize,
										NULL);
		requiredSize=strSize;
		DeviceInterfaceDetailData=(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
		DeviceInterfaceDetailData->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA);
		//�ٴλ����ϸ����
		result2=SetupDiGetDeviceInterfaceDetail(handle,
												&DeviceInterfaceData,
												DeviceInterfaceDetailData,
												strSize,
												&requiredSize,
												&DeviceInfoData);

		//����豸·��������Ҫ�Ĳ��֣�
		temp=DeviceInterfaceDetailData->DevicePath;
		UpdateData(FALSE);
		m_ctllHIDdevices.AddString(temp);
		Count++;
	} while (result1);
}


void CDelta_dLDlg::GetMyIDs()
{
 ////���ı����л�ȡVID�ţ�������MyVid�С�
	//GetInputData(IDC_VID,"����ID(VID)�����ʽ�������������롣",MyVid);
 ////���ı����л�ȡPID�ţ�������MyPid�С�
 //GetInputData(IDC_PID,"��ƷID(PID)�����ʽ�������������롣",MyPid);
	////���ı����л�ȡPVN�ţ�������MyPvn�С�
	//GetInputData(IDC_PVN,"��Ʒ�汾��(PVN)�����ʽ�������������롣",MyPvn);
	MyVid = 0x0483;
	MyPid = 0x5720;
	MyPvn = 0x0200;
}

void CDelta_dLDlg::GetInputData(int nID, CString ErrorMsg, DWORD &Value)
{
	//CString InText;

	//GetDlgItemText(nID,InText);	//��ȡ������ı�

	//if((InText.GetLength()<=4)&&(sscanf(InText,"%x",&Value)))//�ж��Ƿ�������ȷ��ת����ʮ������
	//{
	//	SetDlgItemText(nID,itos(Value,16));	//�����ı�
	//	return TRUE;
	//}
	//else	//���������ʾ�������룬���۽�������������
	//{
	//	MessageBox(ErrorMsg,NULL,MB_OK | MB_ICONEXCLAMATION);
	//	GetDlgItem(nID)->SetFocus();
	//	((CEdit*)GetDlgItem(nID))->SetSel(0,-1);
	//	return FALSE;
	//}
}


CString CDelta_dLDlg::UcharToUnicode(UCHAR *puchar, UINT length)
{
	UINT i=0;
	UCHAR *phan;
	WCHAR *pchina;
	CString str;
	
	phan=new UCHAR[length+2];
	for(i=0;i<length;i++)
		phan[i]=puchar[i];
	phan[length]=0;
	phan[length+1]=0;
	pchina=(WCHAR*)phan;
	str=pchina;
	return str;
	delete []phan;
}





void CDelta_dLDlg::OnBnClickedConnectbtn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int i;
	CString str_temp;
	CString DevicePath;
	HANDLE hCom;

	OnSearch();
	int device_num = m_ctllHIDdevices.GetCount();

	GetMyIDs();
	MyDevFound=FALSE;

	//����Ŀ���豸
	for( i = 0;i < device_num;i++)
	{
		m_ctllHIDdevices.GetText(i,str_temp);
		DevicePath=str_temp;
		//CreateFile�Ƿǳ���Ҫ��һ��������������HIDͨ�ŵľ��
		hCom = CreateFile (
									DevicePath,
									NULL,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL,
									OPEN_EXISTING, 0,
									NULL
								);
		HIDD_ATTRIBUTES devAttr;
		devAttr.Size=sizeof(HIDD_ATTRIBUTES);
		if (!HidD_GetAttributes(hCom,&devAttr))
		{
			CloseHandle(hCom);
			continue;
		}

		//�ҵ���Ӧ���豸
		if (devAttr.VendorID == MyVid && devAttr.ProductID == MyPid) 
		{
			//����ʽ���豸
			hReadHandle=CreateFile(DevicePath, 
									GENERIC_READ,
									 FILE_SHARE_READ|FILE_SHARE_WRITE, 
									 NULL,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
									 NULL);
						if(hReadHandle!=INVALID_HANDLE_VALUE)
							//MessageBox(_T("�����ʴ��豸�ɹ�"));

						//д��ʽ���豸
			hWriteHandle=CreateFile(DevicePath, 
							  GENERIC_WRITE,
							 FILE_SHARE_READ|FILE_SHARE_WRITE, 
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
							 NULL);
						if(hWriteHandle!=INVALID_HANDLE_VALUE)
						{
							m_Measure_btn.EnableWindow(TRUE);
							MyDevFound=TRUE;
							MyDevPathName = DevicePath;
							MessageBox(_T("д���ʴ��豸�ɹ�"));
							break;
						}

		}

	}
	if (i==device_num)
	{
		MessageBox(_T("There is no such HID device..."));
	}

	//����豸�Ѿ��ҵ�����ôӦ��ʹ�ܸ�������ť����ͬʱ��ֹ���豸��ť
	if(MyDevFound)
	{
		SetEvent(ReadOverlapped.hEvent);

		HidD_GetPreparsedData(hReadHandle, &HidParsedData);
		
		/* extract the capabilities info */
		HidP_GetCaps( HidParsedData ,&Capabilities);
		HidD_FreePreparsedData(HidParsedData);	

		unsigned char buffer[126]={0};
		if(HidD_GetManufacturerString(hCom,buffer,126))//CustomHID_StringSerial
		{
			CString str;
			str=UcharToUnicode(buffer,126);//(UCHAR *puchar, CString);
			str+=_T("\n");
			//MessageBox(str);
		}

		m_Connect_state.SetWindowText(_T("������"));
		SetEvent(ReadOverlapped.hEvent);
		//����һ����ʱ������λ��ʵʱ��������
		SetTimer(1,100,NULL);
	}
}

//д������̣߳����̱߳Ƚϼ򵥣�ֻ�Ǽ򵥵صȴ��¼���������
//Ȼ������������ڷ��͵ı�־��
UINT WriteReportThread(LPVOID pParam)
{
	while(1)
	{
		//�����¼�Ϊ��Ч״̬
		 ResetEvent(WriteOverlapped.hEvent);

		//�ȴ��¼�����
		 WaitForSingleObject(WriteOverlapped.hEvent,INFINITE);

		//����������ڷ��ͱ�־
		//DataInSending=FALSE;
	}
	return 0;
}

UINT ReadReportThread(LPVOID pParam)
{
	CDelta_dLDlg *pAppDlg;
	DWORD Length;
	UINT i;
	CString Str,strtemp;

	//������pParamȡ������ת��ΪCMyUsbHidTestAppDlg��ָ�룬
	//�Թ�����������Ա������
	pAppDlg=( CDelta_dLDlg*)pParam;
    
    
	//���߳��Ǹ���ѭ����ֱ�������˳�ʱ�������˳�
	while(1)
	{
		//�����¼�Ϊ��Ч״̬
		ResetEvent(ReadOverlapped.hEvent);
        
		//����豸�Ѿ��ҵ�
		if(MyDevFound==TRUE)
		{
			if(hReadHandle==INVALID_HANDLE_VALUE) //����������Ч
			{
				
			}
			else  //���򣬾����Ч
			{
				//�����ReadFile��������9�ֽڵı�������
				ReadFile(hReadHandle,
						ReadReportBuffer,
						Capabilities.InputReportByteLength,//23�ֽ�=1�ֽڱ���ID+22�ֽ�����
						NULL,
						&ReadOverlapped);
			}

			//�ȴ��¼�����
			WaitForSingleObject(ReadOverlapped.hEvent,INFINITE);
             	
			//����ȴ��������豸���γ���Ҳ�ᵼ���¼�����������ʱMyDevFound
			//������Ϊ�٣�����������ж�MyDevFoundΪ�ٵĻ��ͽ�����һ��ѭ����
		   if(MyDevFound==FALSE) continue;

			//����豸û�б����£�����ReadFile��������������ɡ�
			//ͨ��GetOverlappedResult��������ȡʵ�ʶ�ȡ�����ֽ�����
			GetOverlappedResult(hReadHandle,&ReadOverlapped,&Length,TRUE);

			//����ֽ�����Ϊ0���򽫶�����������ʾ����Ϣ����
			if(Length!=0)
			{
				Data_process(pParam);
			}
		}
		else
		{
			//�����̣߳�ֱ���´��¼�������
			WaitForSingleObject(ReadOverlapped.hEvent,INFINITE);
		}
	}
	return 0;
}

char draw_flag = 0;
char  measure_done = 1;
float dL,Loss,n_zheshelv;
char save_flag = 0;
void Data_process(LPVOID pParam)
{
	CDelta_dLDlg *pAppDlg;
	//������pParamȡ������ת��ΪCMyUsbHidTestAppDlg��ָ�룬
	//�Թ�����������Ա������
	pAppDlg=( CDelta_dLDlg*)pParam;


	if (ReadReportBuffer[1] == 0xa0)
	{
		CString str = _T("");
		str.Format(_T("%d"),ReadReportBuffer[3]);
		pAppDlg->m_Vid.SetWindowText(str);
		if(ReadReportBuffer[2] == 0x80)
		{
			pAppDlg->m_PRO.SetWindowText(_T("�����У�"));
			pAppDlg->m_Progress.SetPos(ReadReportBuffer[4]);
			save_flag = 0;
		}
	}
	else if (ReadReportBuffer[1] == 0x40)
	{
		if (0x01 & ReadReportBuffer[2])
		{
			CString str = _T("");
			str.Format(_T("%f"),*((float *)&ReadReportBuffer[3]));
			pAppDlg->m_Zheshelv.SetWindowText(str);
		}

		if (0x02 & ReadReportBuffer[2])
		{
			pAppDlg->m_Measure_Comb.SetCurSel(ReadReportBuffer[7]);
		}
		
	}
	else if (ReadReportBuffer[1] == 0xf0)
	{
		CString str = _T("");
		str.Format(_T("%f"),*((float *)&ReadReportBuffer[3]));
		pAppDlg->m_FSR.SetWindowText(str);
		str.Format(_T("%f"),*((float *)&ReadReportBuffer[7]));
		dL = *((float *)&ReadReportBuffer[7]);
		pAppDlg->m_DeltL.SetWindowText(str);
		str.Format(_T("%f"),*((float *)&ReadReportBuffer[11]));
		Loss = *((float *)&ReadReportBuffer[11]);
		pAppDlg->m_Loss.SetWindowText(str);
		pAppDlg->m_PRO.SetWindowText(_T("������ɣ�"));
		pAppDlg->m_Progress.SetPos(100);
	}
	else if (ReadReportBuffer[1] == 0x5a)
	{
		int temp;
		temp = (ReadReportBuffer[3]<<8)+ReadReportBuffer[4];
		for (int i= 0;i<PACKAGE_DATA_LEN;i++)
		{
			raw_data[temp*PACKAGE_DATA_LEN+i] = (ReadReportBuffer[i*2+5]<<8)+ReadReportBuffer[i*2+6];
		}
		if (PACKAGE_NUM == temp)
		{
			//ofstream ofs("C:\\Users\\lv\\Desktop\\test.txt");
			//for (int i = 0;i < 3990;i++)
			//{
			//	ofs<<raw_data[i]<<endl;
			//}
			//ofs.close();
			draw_flag = 1;
			save_flag = 1;
		}
	}

}



void CDelta_dLDlg::OnBnClickedMeasurebtn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD numBytesReturned;
	BOOL result1;
	if(!MyDevFound)
	{
		MessageBox(_T("�豸������"));
		return;
	}
	//��������Ч����˵�����豸ʧ��
	if(hWriteHandle==INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("��Ч��д�������������Ǵ��豸ʱʧ��"));
		return ;
	}
	if (1 == measure_done)
	{
		//���Ͳ�������֡
		WriteReportBuffer[0] = 0;
		WriteReportBuffer[1] = 1;	//���ܴ��룬1����ʾ����
		WriteReportBuffer[2] = m_Measure_Comb.GetCurSel();	//������Χ��1��mid
		WriteReportBuffer[3] = 0;
		WriteReportBuffer[4] = 0;
		Send_flag = 1;
		measure_done=0;
	}
}

//���һ����Ϣ������������λ���������ݽ���
void CDelta_dLDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(1 == nIDEvent)
	{
		//����豸�ǹرյ�,�ö�ʱ���ر�
		if( !MyDevFound)
		{KillTimer(1);}

		
		if (1 == Send_flag)
		{
			BOOL result1;
			//�����ݷ��ͳ�ȥ
			result1 = WriteFile(hWriteHandle,
								&WriteReportBuffer[0],
								Capabilities.OutputReportByteLength,
								NULL,
								&WriteOverlapped);
			Send_flag = 0;
			//�жϽ�����Ƿ���
			if(!result1)
			{
				//��ȡ���������
				UINT LastError;
				LastError=GetLastError();
				//���Ƿ������IO����
				if((LastError==ERROR_IO_PENDING)||(LastError==ERROR_SUCCESS))
				{
					return ;
				}
				else
				MessageBox(_T("wrong"));
			}
		}

		if (draw_flag)
		{
			//ʵʱ�Ľ����ݻ���
			CCurveLine* drawline = new CCurveLine(raw_data,DATA_LENGTH);
			if(drawline->InitSuccessFlag)
			{
				CRect rect;
				GetDlgItem(MUBIAO_CURV_ID)->GetWindowRect(&rect);
				GetDlgItem(MUBIAO_CURV_ID)->GetParent()->ScreenToClient(rect);
				//InvalidateRect(rect,FALSE);

				DrawCurve(*drawline,MUBIAO_CURV_ID);
			}

			m_Save_btn.EnableWindow(TRUE);

			// TODO �ò���ȡ���Զ����棬��Ϊ�ֶ�����
			////�����ݱ��浽�ļ�
			//char file_name_str[200];
			//CEdit* pEdit= (CEdit *) GetDlgItem(IDC_EDIT_FILENAME);
			//CString cond_str,temp_str;
			//pEdit->GetWindowText(cond_str);

			//CTime tm; tm=CTime::GetCurrentTime();
			//CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));


			//CString f_n;
			//f_n.Format(_T("%s_%s.txt"),cond_str,time_str);
			////sprintf(file_name_str,_T("%s_%s.txt"),cond_str.GetBuffer(),time_str.GetBuffer());

			////�����ļ�
			//ofstream ofs(f_n.GetBuffer());
			//
			//	
			//	

			//ofs<<"����ʱ�䣺"<<CT2A(time_str)<<endl; 
			//ofs.precision(6);
			//ofs<<"����"<<Loss<<"dB"<<endl;
			//ofs<<"���Ȳ"<<dL<<"m"<<endl;
			//ofs<<"�����ʣ�"<<n_zheshelv<<endl;
			//ofs.precision(4);
			//for (int i = 0;i < DATA_LENGTH;i++)
			//{
			//	ofs<<raw_data[i]/800.0<<endl;
			//}
			//ofs.close();

			draw_flag = 0;
			measure_done = 1;
		}

		// ����δ��ɽ�ֹ����
		if (save_flag)
		{
			m_Measure_btn.EnableWindow(TRUE);
		}
		else
		{
			m_Measure_btn.EnableWindow(TRUE);
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CDelta_dLDlg::OnCbnSelchangeMeasureCombo()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//��ѡ�����ı�ʱ���仯��״̬������λ��
	WriteReportBuffer[0] = 0;
	WriteReportBuffer[1] = 2;	//���ܴ��룬2����ʾ���淢���仯
	WriteReportBuffer[2] = m_Measure_Comb.GetCurSel();	//������Χ��1��mid
	WriteReportBuffer[3] = 0;
	WriteReportBuffer[4] = 0;
	Send_flag = 1;
}

//�豸״̬�ı�ʱ�Ĵ�����
afx_msg LRESULT  CDelta_dLDlg::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_DEVICEINTERFACE pdbi;
	CString DevPathName;

	//dwData��һ��ָ��DEV_BROADCAST_DEVICEINTERFACE�ṹ���ָ�룬
	//�ڸýṹ���б������豸�����͡�·�����Ȳ�����ͨ��������ָ���豸
	//��·�����Ƚϣ��������ж��Ƿ�������ָ�����豸���»��߲����ˡ�
	pdbi=(PDEV_BROADCAST_DEVICEINTERFACE)lParam;

	switch(wParam) //����nEventType�б������¼�������
	{
	//�豸�����¼�
		case DBT_DEVICEARRIVAL:
			if (!MyDevFound)
			{
				OnSearch();
				OnBnClickedConnectbtn();
			}
		return TRUE;

  //�豸�γ��¼�
		case DBT_DEVICEREMOVECOMPLETE: 
			if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
			{
				DevPathName=pdbi->dbcc_name; //���淢��״̬�ı���豸��·����
				//�Ƚ��Ƿ�������ָ�����豸
				if(MyDevPathName.CompareNoCase(DevPathName)==0)
				{
					if(MyDevFound==TRUE)
					{ 
						MyDevFound=FALSE;
						OnBnClickedClosebtn();
					}
				}
			}
			return TRUE;
	//�豸���豸�����仯
		//case DBT_DEVNODES_CHANGED:
		//	OnBnClickedConnectbtn();
		//	if(MyDevFound==FALSE)OnBnClickedClosebtn();

		//	return TRUE;
		default:
			return TRUE;
	} 
}
void CDelta_dLDlg::OnBnClickedClosebtn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//��������ݵľ��������Ч�������ر�֮
	KillTimer(1);
	if(hReadHandle!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hReadHandle);
		hReadHandle=INVALID_HANDLE_VALUE;
	}
	if(hWriteHandle!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hWriteHandle);
		hWriteHandle=INVALID_HANDLE_VALUE;
	}
	//�����豸״̬Ϊδ�ҵ�
	MyDevFound=FALSE;
	//�޸İ���ʹ�����
	m_Measure_btn.EnableWindow(TRUE);

	m_Connect_state.SetWindowText(_T("δ��⵽�豸"));
//	GetDlgItem(IDC_BUTTON_Open)->EnableWindow(TRUE);
//	GetDlgItem(IDC_BUTTON_Close)->EnableWindow(FALSE);
//	GetDlgItem(IDC_BUTTON_Send)->EnableWindow(FALSE);
}

void CDelta_dLDlg::OnEnChangeEdit2()
{
	int Tx_n;
	float n_r;
	CString temp;
	m_Zheshelv.GetWindowText(temp);

	n_r = _wtof(temp.GetBuffer());

	if(n_r<2 && n_r>0.9)
	{
		Tx_n = n_r * 10000000.0;
		Tx_n = Tx_n  / 1000;
	}
	else
	{
		Tx_n = 14500;
		MessageBox(_T("�����ʴ���"));
	}
	n_zheshelv = Tx_n/10000.0;
	WriteReportBuffer[0] = 0;
	WriteReportBuffer[1] = 3;	//���ܴ��룬2����ʾ���淢���仯
	WriteReportBuffer[2] = Tx_n & 0xff;	//������Χ��1��mid
	WriteReportBuffer[3] = (Tx_n >> 8) & 0xff;;
	WriteReportBuffer[4] = 0;
	Send_flag = 1;
}



//��ͼ����
void CDelta_dLDlg::DrawCurve( CCurveLine& Curve,int ID )
{
	double DataMax = -99999999;
	double DataMin = 99999999;

	int i = 0;
	while(i < Curve.PointNum)
	{
		if(DataMax < Curve.PointData[i] )//&& Curve.PointData[i] < 10)
			DataMax = Curve.PointData[i];
		if(DataMin > Curve.PointData[i] )//&& Curve.PointData[i] > -10)
			DataMin = Curve.PointData[i];
		i++;
	}

	int Left = 0;
	double ReferenceLevel = DataMax + Left;
	double PlotRange = DataMax-DataMin + Left*2;

	CRect rect;
	CDC *pDc; 
	CDC memDC; 
	GetDlgItem(ID)->GetWindowRect(&rect);
	GetDlgItem(ID)->GetParent()->ScreenToClient(rect);
	pDc = this->GetDC();

	CBitmap memBitmap;
	memDC.CreateCompatibleDC(NULL);              
	memBitmap.CreateCompatibleBitmap(pDc,rect.Width(),rect.Height());
	memDC.SelectObject(&memBitmap);
	CPen pen_line(PS_SOLID,1,RGB(255,0,0));//������ɫ
	memDC.SelectObject(&pen_line);

	//�Զ����ͼ����
	memDC.FillSolidRect(CRect(0,0,rect.Width(),rect.Height()),RGB(180,180,180));//����

	CPoint LastPoint,CurPoint;
	LastPoint.x = 0;
	LastPoint.y = long((ReferenceLevel - Curve.PointData[0])/PlotRange*rect.Height());
	memDC.MoveTo(LastPoint);
	for(int i = 1;i < Curve.PointNum;i++)
	{
		CurPoint.x = i*rect.Width()/(Curve.PointNum-1);
		CurPoint.y = long((ReferenceLevel - Curve.PointData[i])/PlotRange*rect.Height());
		memDC.LineTo(CurPoint);
		CurPoint = LastPoint;
	}

	pen_line.DeleteObject();
	pen_line.CreatePen(PS_SOLID,1,RGB(0,0,0));
	memDC.SelectObject(&pen_line);
	memDC.MoveTo(0,rect.Height()/2);
	memDC.LineTo(rect.Width(),rect.Height()/2);

	//��ʾ����
	CString str;
	memDC.SetBkMode(TRANSPARENT);
	str.Format(_T("%.2f "),ReferenceLevel);
	memDC.TextOut(0,0,str);
	str.Format(_T("%.2f "),ReferenceLevel-PlotRange/2);
	memDC.TextOut(0,rect.Height()/2,str);
	str.Format(_T("%.2f "),ReferenceLevel-PlotRange);
	CSize text_size = memDC.GetTextExtent(str);
	memDC.TextOut(0,rect.Height()-text_size.cy,str);


	//////////////////////////////////////////////////////////////
	pDc->BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&memDC,0,0,SRCCOPY);
	this->ReleaseDC(pDc);
	memDC.DeleteDC();
	memBitmap.DeleteObject();
}

void CDelta_dLDlg::OnBnClickedSavebtn()
{
	// ����Ƿ��������ļ���
	CEdit* pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_PRODID);
	CString prodId_str;
	pEditTmp->GetWindowText(prodId_str);

	if (prodId_str.IsEmpty())
	{
		MessageBox(_T("�������Ʒ�ͺ�"));
		return;
	}

	CString prodCode_str;
	pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_PRODCODE);
	pEditTmp->GetWindowText(prodCode_str);

	if (prodCode_str.IsEmpty())
	{
		MessageBox(_T("�������Ʒ���"));
		return;
	}

	CString operatorName;
	pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_OPERATOR);
	pEditTmp->GetWindowText(operatorName);
	
	// Ŀ¼����ʽ��ProdId+prodCode
	CString dirName;
	dirName.Format(_T("%s_%s"), prodId_str, prodCode_str);

	// ���Ŀ¼�Ƿ���ڣ����������򴴽�
	CString fullDir;
	fullDir.Format(_T("%s\\%s"), ROOT_DIR, dirName);
	if (!PathIsDirectory(fullDir))
	{
		if (!CreateDirectory(fullDir,NULL))
		{
			MessageBox(_T("��������Ŀ¼ʧ�ܣ������Ʒ�ͺźͱ���Ƿ�Ϸ�"));
		}
	}
	
	// �����ļ�����ʽ��ProdId+ProdCode()+Time
	CTime tm = CTime::GetCurrentTime();
	CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));
	CString fileName;
	//fileName.Format(_T("%s\\%s%s_%s.txt"),fullDir,prodId_str,prodCode_str.Right(3),time_str);
	fileName.Format(_T("%s\\%s.txt"),fullDir,time_str);

	// �����ļ�
	ofstream ofs(fileName.GetBuffer());

	ofs<<"����ʱ�䣺"<<CT2A(time_str)<<endl; 
	ofs<<"�����ˣ�"<<CT2A(operatorName)<<endl;
	ofs.precision(6);
	ofs<<"����"<<Loss<<"dB"<<endl;
	ofs<<"���Ȳ"<<dL<<"m"<<endl;
	ofs<<"�����ʣ�"<<n_zheshelv<<endl;
	ofs.precision(4);
	for (int i = 0;i < DATA_LENGTH;i++)
	{
		ofs << raw_data[i] << endl; // for test
		//ofs<<raw_data[i]/800.0<<endl;
	}
	ofs.close();

	

	

	////�����ݱ��浽�ļ�
	//char file_name_str[200];
	//CEdit* pEdit= (CEdit *) GetDlgItem(IDC_EDIT_PRODID);
	//CString cond_str,temp_str;
	//pEdit->GetWindowText(cond_str);

	//CTime tm; tm=CTime::GetCurrentTime();
	//CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));


	//CString f_n;
	//f_n.Format(_T("%s_%s.txt"),cond_str,time_str);
	////sprintf(file_name_str,_T("%s_%s.txt"),cond_str.GetBuffer(),time_str.GetBuffer());

	////�����ļ�
	//ofstream ofs(f_n.GetBuffer());




	//ofs<<"����ʱ�䣺"<<CT2A(time_str)<<endl; 
	//ofs.precision(6);
	//ofs<<"����"<<Loss<<"dB"<<endl;
	//ofs<<"���Ȳ"<<dL<<"m"<<endl;
	//ofs<<"�����ʣ�"<<n_zheshelv<<endl;
	//ofs.precision(4);
	//for (int i = 0;i < DATA_LENGTH;i++)
	//{
	//	ofs<<raw_data[i]/800.0<<endl;
	//}
	//ofs.close();
	
}

HBRUSH CDelta_dLDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_TITLE:
			pDC->SetTextColor(RGB(0x00,0x32,0x73));
			pDC->SelectObject(&m_font);
			break;
		default:
			break;
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}



void CDelta_dLDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;

	/**
	// TODO: �ڴ˴������Ϣ����������
	if(m_listRect.GetCount() > 0)
	{
		CRect dlgNow;
		GetWindowRect(&dlgNow);
		POSITION pos = m_listRect.GetHeadPosition();

		CRect dlgSaved;
		dlgSaved = m_listRect.GetNext(pos);

		float x = dlgNow.Width() * 1.0 / dlgSaved.Width();
		float y = dlgNow.Height() * 1.0 / dlgSaved.Height();
		ClientToScreen(dlgNow);

		CRect childSaved;

		CWnd* pWnd = GetWindow(GW_CHILD);
		while(pWnd)
		{
			childSaved = m_listRect.GetNext(pos);
			childSaved.left = dlgNow.left + (childSaved.left - dlgSaved.left) * x;
			childSaved.right = dlgNow.right + (childSaved.right - dlgSaved.right) * x;
			childSaved.top = dlgNow.top + (childSaved.top - dlgSaved.top) * y;
			childSaved.bottom = dlgNow.bottom + (childSaved.bottom - dlgSaved.bottom) * y;
			ScreenToClient(childSaved);
			pWnd->MoveWindow(childSaved);
			pWnd = pWnd->GetNextWindow();
		}
	}
	*/

}

void CDelta_dLDlg::OnEnChangeEditProdid()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
