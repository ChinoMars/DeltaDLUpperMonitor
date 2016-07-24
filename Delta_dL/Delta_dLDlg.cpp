// Delta_dLDlg.cpp : 实现文件
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


// 根目录
//CString ROOT_DIR = _T("C:\\Users\\Chino\\Desktop\\DeltaDLData");
//CString ROOT_DIR = _T("C:\\Users\\Administrator\\Desktop\\DeltaDLData");
CString ROOT_DIR = _T("D:\\Delta_dL\\DeltaDLData");



//用来保存找到的设备路径
CString MyDevPathName=_T("");
//定义变量用来保存VID、PID、版本号
DWORD MyVid,MyPid,MyPvn;
//用来保存读数据的设备句柄
HANDLE hReadHandle=INVALID_HANDLE_VALUE;
//用来保存写数据的设备句柄
HANDLE hWriteHandle=INVALID_HANDLE_VALUE;

//用来保存设备是否已经找到
BOOL MyDevFound=FALSE;

/////////////////////////////////////
HIDP_CAPS					Capabilities;
PHIDP_PREPARSED_DATA		HidParsedData;

//发送报告用的OVERLAPPED。
OVERLAPPED WriteOverlapped;
//接收报告用的OVERLAPPED。
OVERLAPPED ReadOverlapped;

//指向读报告线程的指针
CWinThread * pReadReportThread;
//指向写报告线程的指针
CWinThread * pWriteReportThread;

//发送报告的缓冲区，1字节报告ID+8字节报告数据。
UCHAR WriteReportBuffer[512]={0};

//接收报告的缓冲区，1字节报告ID+8字节报告数据。
UCHAR ReadReportBuffer[512]={0};

//标记当前缓冲区的数据是否传输
UCHAR Send_flag = 0;

unsigned short raw_data[DATA_LENGTH];


UINT WriteReportThread(LPVOID pParam);
UINT ReadReportThread(LPVOID pParam);
void Data_process(LPVOID pParam);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CDelta_dLDlg 对话框




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
	//ON_WM_DEVICECHANGE() //此消息就是处理设备添加删除  
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


// CDelta_dLDlg 消息处理程序

BOOL CDelta_dLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_Measure_Comb.AddString(_T("long (>50m)"));
	m_Measure_Comb.AddString(_T("mid 1 (5m--50m)"));
	m_Measure_Comb.AddString(_T("mid 2 (0.5m--5m)"));
	m_Measure_Comb.AddString(_T("short (<0.5m)"));
	m_Measure_Comb.SetCurSel(1);

	m_ctllHIDdevices.ShowWindow(SW_HIDE);
	m_Measure_btn.EnableWindow(TRUE);
	//m_Save_btn.EnableWindow(FALSE);

	// 隐藏FSR项
	m_FSR.ShowWindow(SW_HIDE);

	m_Zheshelv.SetWindowText(_T("1.45"));
	m_Pid.SetWindowText(_T("yes"));
	m_Connect_state.SetWindowText(_T("未检测到设备"));

	m_PRO.SetWindowText(_T("测量完成:"));
	m_Progress.SetPos(100);

	// 设置标题字体字号
	m_font.CreatePointFont(300,_T("宋体"),NULL);

	// 用于调整窗口大小时自动调整控件大小
	INIT_EASYSIZE;

	// 创建数据存储目录
	if (!PathIsDirectory(ROOT_DIR))
	{
		if (!CreateDirectory(ROOT_DIR, NULL))
		{
			MessageBox(_T("创建数据目录失败"));
		}
	}


	//初始化写报告时用的Overlapped结构体
	//偏移量设置为0
	WriteOverlapped.Offset=0;
	WriteOverlapped.OffsetHigh=0;
	//创建一个事件，提供给WriteFile使用，当WriteFile完成时，
	//会设置该事件为触发状态。手工重置事件，初始化为无信号
	WriteOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);


	//初始化读报告时用的Overlapped结构体
	//偏移量设置为0
	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	//创建一个事件，提供给ReadFile使用，当ReadFile完成时，
	//会设置该事件为触发状态。
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//创建写报告的线程（处于挂起状态）
	pWriteReportThread=AfxBeginThread(WriteReportThread,
		                                this,
										THREAD_PRIORITY_NORMAL,
										0,
										CREATE_SUSPENDED,
										NULL);
	//如果创建成功，则恢复该线程的运行
	if(pWriteReportThread!=NULL)
	{
		pWriteReportThread->ResumeThread();
	}

	//创建一个读报告的线程（处于挂起状态）
	pReadReportThread=AfxBeginThread(ReadReportThread,
		                                this,
									THREAD_PRIORITY_NORMAL,
									0,
									CREATE_SUSPENDED,
									NULL);
	//如果创建成功，则恢复该线程的运行
	if(pReadReportThread!=NULL)
	{
		pReadReportThread->ResumeThread();
	}

	//注册监听USB插拔事件
	// 注意  
	GUID myHID_GUID = {0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED }};


	HidD_GetHidGuid(&myHID_GUID);
	DEV_BROADCAST_DEVICEINTERFACE Filter;  
	ZeroMemory(&Filter,sizeof(Filter));  
	Filter.dbcc_size = sizeof(Filter);   // size gets set to 29 with 1-byte packing or 32 with 4- or 8-byte packing  
	Filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;  
	Filter.dbcc_classguid = myHID_GUID;  
	//DEVICE_NOTIFY_ALL_INTERFACE_CLASSES //关注所有设备事件  
	if(NULL == RegisterDeviceNotification(this->GetSafeHwnd(),&Filter,DEVICE_NOTIFY_WINDOW_HANDLE))
		TRACE("RegisterDeviceNotification failed!!");  



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDelta_dLDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDelta_dLDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDelta_dLDlg::OnSearch(void)
{
	m_ctllHIDdevices.ResetContent(); //list控件，用来显示所有的HID设备
	UpdateData(FALSE); //更新界面

	CString temp;
	int Count = 0; //Total number of devices found
	DWORD strSize=0,requiredSize=0;
	BOOL result1,result2;
	ULONG DeviceInterfaceDetailDataSize;
	//定义一些变量，以后会用到
	SP_DEVINFO_DATA DeviceInfoData;
	SP_DEVICE_INTERFACE_DATA  DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

	//第一步：获取deviceID
	GUID deviceId;
	HidD_GetHidGuid(&deviceId);
		//第二步：获取设备信息
	HDEVINFO handle;
	handle = SetupDiGetClassDevs(&deviceId, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT); //Get only HID devices
	//第三步：对所有的设备进行枚举
	//SetupDiEnumDeviceInterfaces();
	result1=false; //定义一些变量
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
		//获得设备详细数据（初步）
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
		//再次获得详细数据
		result2=SetupDiGetDeviceInterfaceDetail(handle,
												&DeviceInterfaceData,
												DeviceInterfaceDetailData,
												strSize,
												&requiredSize,
												&DeviceInfoData);

		//获得设备路径（最重要的部分）
		temp=DeviceInterfaceDetailData->DevicePath;
		UpdateData(FALSE);
		m_ctllHIDdevices.AddString(temp);
		Count++;
	} while (result1);
}


void CDelta_dLDlg::GetMyIDs()
{
 ////从文本框中获取VID号，保存在MyVid中。
	//GetInputData(IDC_VID,"厂商ID(VID)输入格式错误。请重新输入。",MyVid);
 ////从文本框中获取PID号，保存在MyPid中。
 //GetInputData(IDC_PID,"产品ID(PID)输入格式错误。请重新输入。",MyPid);
	////从文本框中获取PVN号，保存在MyPvn中。
	//GetInputData(IDC_PVN,"产品版本号(PVN)输入格式错误。请重新输入。",MyPvn);
	MyVid = 0x0483;
	MyPid = 0x5720;
	MyPvn = 0x0200;
}

void CDelta_dLDlg::GetInputData(int nID, CString ErrorMsg, DWORD &Value)
{
	//CString InText;

	//GetDlgItemText(nID,InText);	//获取输入的文本

	//if((InText.GetLength()<=4)&&(sscanf(InText,"%x",&Value)))//判断是否输入正确并转换成十进制数
	//{
	//	SetDlgItemText(nID,itos(Value,16));	//设置文本
	//	return TRUE;
	//}
	//else	//输入错误，提示重新输入，并聚焦到错误的输入框
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
	// TODO: 在此添加控件通知处理程序代码
	int i;
	CString str_temp;
	CString DevicePath;
	HANDLE hCom;

	OnSearch();
	int device_num = m_ctllHIDdevices.GetCount();

	GetMyIDs();
	MyDevFound=FALSE;

	//搜索目标设备
	for( i = 0;i < device_num;i++)
	{
		m_ctllHIDdevices.GetText(i,str_temp);
		DevicePath=str_temp;
		//CreateFile是非常重要的一步，用来建立于HID通信的句柄
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

		//找到对应的设备
		if (devAttr.VendorID == MyVid && devAttr.ProductID == MyPid) 
		{
			//读方式打开设备
			hReadHandle=CreateFile(DevicePath, 
									GENERIC_READ,
									 FILE_SHARE_READ|FILE_SHARE_WRITE, 
									 NULL,
									 OPEN_EXISTING,
									 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
									 NULL);
						if(hReadHandle!=INVALID_HANDLE_VALUE)
							//MessageBox(_T("读访问打开设备成功"));

						//写方式打开设备
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
							MessageBox(_T("写访问打开设备成功"));
							break;
						}

		}

	}
	if (i==device_num)
	{
		MessageBox(_T("There is no such HID device..."));
	}

	//如果设备已经找到，那么应该使能各操作按钮，并同时禁止打开设备按钮
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

		m_Connect_state.SetWindowText(_T("已连接"));
		SetEvent(ReadOverlapped.hEvent);
		//开启一个计时器与下位机实时交互数据
		SetTimer(1,100,NULL);
	}
}

//写报告的线程，该线程比较简单，只是简单地等待事件被触发，
//然后清除数据正在发送的标志。
UINT WriteReportThread(LPVOID pParam)
{
	while(1)
	{
		//设置事件为无效状态
		 ResetEvent(WriteOverlapped.hEvent);

		//等待事件触发
		 WaitForSingleObject(WriteOverlapped.hEvent,INFINITE);

		//清除数据正在发送标志
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

	//将参数pParam取出，并转换为CMyUsbHidTestAppDlg型指针，
	//以供下面调用其成员函数。
	pAppDlg=( CDelta_dLDlg*)pParam;
    
    
	//该线程是个死循环，直到程序退出时，它才退出
	while(1)
	{
		//设置事件为无效状态
		ResetEvent(ReadOverlapped.hEvent);
        
		//如果设备已经找到
		if(MyDevFound==TRUE)
		{
			if(hReadHandle==INVALID_HANDLE_VALUE) //如果读句柄无效
			{
				
			}
			else  //否则，句柄有效
			{
				//则调用ReadFile函数请求9字节的报告数据
				ReadFile(hReadHandle,
						ReadReportBuffer,
						Capabilities.InputReportByteLength,//23字节=1字节报告ID+22字节数据
						NULL,
						&ReadOverlapped);
			}

			//等待事件触发
			WaitForSingleObject(ReadOverlapped.hEvent,INFINITE);
             	
			//如果等待过程中设备被拔出，也会导致事件触发，但此时MyDevFound
			//被设置为假，因此在这里判断MyDevFound为假的话就进入下一轮循环。
		   if(MyDevFound==FALSE) continue;

			//如果设备没有被拔下，则是ReadFile函数正常操作完成。
			//通过GetOverlappedResult函数来获取实际读取到的字节数。
			GetOverlappedResult(hReadHandle,&ReadOverlapped,&Length,TRUE);

			//如果字节数不为0，则将读到的数据显示到信息框中
			if(Length!=0)
			{
				Data_process(pParam);
			}
		}
		else
		{
			//阻塞线程，直到下次事件被触发
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
	//将参数pParam取出，并转换为CMyUsbHidTestAppDlg型指针，
	//以供下面调用其成员函数。
	pAppDlg=( CDelta_dLDlg*)pParam;


	if (ReadReportBuffer[1] == 0xa0)
	{
		CString str = _T("");
		str.Format(_T("%d"),ReadReportBuffer[3]);
		pAppDlg->m_Vid.SetWindowText(str);
		if(ReadReportBuffer[2] == 0x80)
		{
			pAppDlg->m_PRO.SetWindowText(_T("测量中："));
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
		pAppDlg->m_PRO.SetWindowText(_T("测量完成："));
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
	// TODO: 在此添加控件通知处理程序代码
	DWORD numBytesReturned;
	BOOL result1;
	if(!MyDevFound)
	{
		MessageBox(_T("设备不存在"));
		return;
	}
	//如果句柄无效，则说明打开设备失败
	if(hWriteHandle==INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("无效的写报告句柄，可能是打开设备时失败"));
		return ;
	}
	if (1 == measure_done)
	{
		//发送测量数据帧
		WriteReportBuffer[0] = 0;
		WriteReportBuffer[1] = 1;	//功能代码，1：表示测量
		WriteReportBuffer[2] = m_Measure_Comb.GetCurSel();	//测量范围，1：mid
		WriteReportBuffer[3] = 0;
		WriteReportBuffer[4] = 0;
		Send_flag = 1;
		measure_done=0;
	}
}

//添加一个消息函数负责与下位机进行数据交互
void CDelta_dLDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(1 == nIDEvent)
	{
		//如果设备是关闭的,该定时器关闭
		if( !MyDevFound)
		{KillTimer(1);}

		
		if (1 == Send_flag)
		{
			BOOL result1;
			//将数据发送出去
			result1 = WriteFile(hWriteHandle,
								&WriteReportBuffer[0],
								Capabilities.OutputReportByteLength,
								NULL,
								&WriteOverlapped);
			Send_flag = 0;
			//判断结果看是否误报
			if(!result1)
			{
				//获取最后错误代码
				UINT LastError;
				LastError=GetLastError();
				//看是否是真的IO挂起
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
			//实时的将数据画出
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

			// TODO 该部分取消自动保存，改为手动保存
			////将数据保存到文件
			//char file_name_str[200];
			//CEdit* pEdit= (CEdit *) GetDlgItem(IDC_EDIT_FILENAME);
			//CString cond_str,temp_str;
			//pEdit->GetWindowText(cond_str);

			//CTime tm; tm=CTime::GetCurrentTime();
			//CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));


			//CString f_n;
			//f_n.Format(_T("%s_%s.txt"),cond_str,time_str);
			////sprintf(file_name_str,_T("%s_%s.txt"),cond_str.GetBuffer(),time_str.GetBuffer());

			////创建文件
			//ofstream ofs(f_n.GetBuffer());
			//
			//	
			//	

			//ofs<<"测量时间："<<CT2A(time_str)<<endl; 
			//ofs.precision(6);
			//ofs<<"插损："<<Loss<<"dB"<<endl;
			//ofs<<"长度差："<<dL<<"m"<<endl;
			//ofs<<"折射率："<<n_zheshelv<<endl;
			//ofs.precision(4);
			//for (int i = 0;i < DATA_LENGTH;i++)
			//{
			//	ofs<<raw_data[i]/800.0<<endl;
			//}
			//ofs.close();

			draw_flag = 0;
			measure_done = 1;
		}

		// 测量未完成禁止保存
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
	// TODO: 在此添加控件通知处理程序代码
	//当选择发生改变时将变化的状态传给下位机
	WriteReportBuffer[0] = 0;
	WriteReportBuffer[1] = 2;	//功能代码，2：表示界面发生变化
	WriteReportBuffer[2] = m_Measure_Comb.GetCurSel();	//测量范围，1：mid
	WriteReportBuffer[3] = 0;
	WriteReportBuffer[4] = 0;
	Send_flag = 1;
}

//设备状态改变时的处理函数
afx_msg LRESULT  CDelta_dLDlg::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_DEVICEINTERFACE pdbi;
	CString DevPathName;

	//dwData是一个指向DEV_BROADCAST_DEVICEINTERFACE结构体的指针，
	//在该结构体中保存了设备的类型、路径名等参数。通过跟我们指定设备
	//的路径名比较，即可以判断是否是我们指定的设备拔下或者插入了。
	pdbi=(PDEV_BROADCAST_DEVICEINTERFACE)lParam;

	switch(wParam) //参数nEventType中保存着事件的类型
	{
	//设备连接事件
		case DBT_DEVICEARRIVAL:
			if (!MyDevFound)
			{
				OnSearch();
				OnBnClickedConnectbtn();
			}
		return TRUE;

  //设备拔出事件
		case DBT_DEVICEREMOVECOMPLETE: 
			if(pdbi->dbcc_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
			{
				DevPathName=pdbi->dbcc_name; //保存发生状态改变的设备的路径名
				//比较是否是我们指定的设备
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
	//设备有设备发生变化
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
	// TODO: 在此添加控件通知处理程序代码
	//如果读数据的句柄不是无效句柄，则关闭之
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
	//设置设备状态为未找到
	MyDevFound=FALSE;
	//修改按键使能情况
	m_Measure_btn.EnableWindow(TRUE);

	m_Connect_state.SetWindowText(_T("未检测到设备"));
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
		MessageBox(_T("折射率错误"));
	}
	n_zheshelv = Tx_n/10000.0;
	WriteReportBuffer[0] = 0;
	WriteReportBuffer[1] = 3;	//功能代码，2：表示界面发生变化
	WriteReportBuffer[2] = Tx_n & 0xff;	//测量范围，1：mid
	WriteReportBuffer[3] = (Tx_n >> 8) & 0xff;;
	WriteReportBuffer[4] = 0;
	Send_flag = 1;
}



//画图函数
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
	CPen pen_line(PS_SOLID,1,RGB(255,0,0));//画线颜色
	memDC.SelectObject(&pen_line);

	//自定义绘图函数
	memDC.FillSolidRect(CRect(0,0,rect.Width(),rect.Height()),RGB(180,180,180));//背景

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

	//显示坐标
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
	// 检查是否已设置文件名
	CEdit* pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_PRODID);
	CString prodId_str;
	pEditTmp->GetWindowText(prodId_str);

	if (prodId_str.IsEmpty())
	{
		MessageBox(_T("请输入产品型号"));
		return;
	}

	CString prodCode_str;
	pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_PRODCODE);
	pEditTmp->GetWindowText(prodCode_str);

	if (prodCode_str.IsEmpty())
	{
		MessageBox(_T("请输入产品编号"));
		return;
	}

	CString operatorName;
	pEditTmp = (CEdit*) GetDlgItem(IDC_EDIT_OPERATOR);
	pEditTmp->GetWindowText(operatorName);
	
	// 目录名格式：ProdId+prodCode
	CString dirName;
	dirName.Format(_T("%s_%s"), prodId_str, prodCode_str);

	// 检测目录是否存在，若不存在则创建
	CString fullDir;
	fullDir.Format(_T("%s\\%s"), ROOT_DIR, dirName);
	if (!PathIsDirectory(fullDir))
	{
		if (!CreateDirectory(fullDir,NULL))
		{
			MessageBox(_T("创建数据目录失败，请检查产品型号和编号是否合法"));
		}
	}
	
	// 数据文件名格式：ProdId+ProdCode()+Time
	CTime tm = CTime::GetCurrentTime();
	CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));
	CString fileName;
	//fileName.Format(_T("%s\\%s%s_%s.txt"),fullDir,prodId_str,prodCode_str.Right(3),time_str);
	fileName.Format(_T("%s\\%s.txt"),fullDir,time_str);

	// 创建文件
	ofstream ofs(fileName.GetBuffer());

	ofs<<"测量时间："<<CT2A(time_str)<<endl; 
	ofs<<"操作人："<<CT2A(operatorName)<<endl;
	ofs.precision(6);
	ofs<<"插损："<<Loss<<"dB"<<endl;
	ofs<<"长度差："<<dL<<"m"<<endl;
	ofs<<"折射率："<<n_zheshelv<<endl;
	ofs.precision(4);
	for (int i = 0;i < DATA_LENGTH;i++)
	{
		ofs << raw_data[i] << endl; // for test
		//ofs<<raw_data[i]/800.0<<endl;
	}
	ofs.close();

	

	

	////将数据保存到文件
	//char file_name_str[200];
	//CEdit* pEdit= (CEdit *) GetDlgItem(IDC_EDIT_PRODID);
	//CString cond_str,temp_str;
	//pEdit->GetWindowText(cond_str);

	//CTime tm; tm=CTime::GetCurrentTime();
	//CString time_str = tm.Format(_T("%Y%m%d_%H_%M_%S"));


	//CString f_n;
	//f_n.Format(_T("%s_%s.txt"),cond_str,time_str);
	////sprintf(file_name_str,_T("%s_%s.txt"),cond_str.GetBuffer(),time_str.GetBuffer());

	////创建文件
	//ofstream ofs(f_n.GetBuffer());




	//ofs<<"测量时间："<<CT2A(time_str)<<endl; 
	//ofs.precision(6);
	//ofs<<"插损："<<Loss<<"dB"<<endl;
	//ofs<<"长度差："<<dL<<"m"<<endl;
	//ofs<<"折射率："<<n_zheshelv<<endl;
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

	// TODO:  在此更改 DC 的任何属性
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_TITLE:
			pDC->SetTextColor(RGB(0x00,0x32,0x73));
			pDC->SelectObject(&m_font);
			break;
		default:
			break;
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}



void CDelta_dLDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	UPDATE_EASYSIZE;

	/**
	// TODO: 在此处添加消息处理程序代码
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
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
