// HelpDoc.cpp : 实现文件
//

#include "stdafx.h"
#include "Delta_dL.h"
#include "HelpDoc.h"


// CHelpDoc 对话框

IMPLEMENT_DYNAMIC(CHelpDoc, CDialog)

CHelpDoc::CHelpDoc(CWnd* pParent /*=NULL*/)
	: CDialog(CHelpDoc::IDD, pParent)
{

}

CHelpDoc::~CHelpDoc()
{
}

void CHelpDoc::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHelpDoc, CDialog)
END_MESSAGE_MAP()


// CHelpDoc 消息处理程序


BOOL CHelpDoc::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_HELP_CONTENT,_T("（1）光纤长度精密测量仪属于精密类光纤产品，使用与搬运时注意防震与跌落。\r\n\r\n（2）本设备仅适用于普通单模光纤，目前暂不支持特种光纤如保偏光纤等偏振敏感光纤干涉仪的臂长差测试。\r\n\r\n（3）本设备测量范围0-400米之间粗分为四挡量程。对于有些待测光纤臂长差介于两档之间的光纤链路，虽然两档都可以得到测量结果的情况，应当选择量程使得测量的波数落在6-36之间，波数越接近此值，测量结果越准确。\r\n\r\n（4）点击“保存数据”按钮，数据保存位置在硬盘：C：/ DeltaDLData目录下。子目录名称为：输入的型号+产品编号名，保存的文件名称为测量日期时间格式命名的文件名，文件形式为*.txt格式。\r\n\r\n（5）由于光纤存在色散效应，不同测试光波长对应有不同的色散引起的光信号延迟效应，本设备测试光波长对应于C 波段，因此测量结果对应于C 波段光纤长度。\r\n\r\n（6）不同光纤适配器连接时由于存在回波引起的干扰以及匹配不善引入的间隙不确定，不同光纤适配器连接时需要匹配转接。并且尽量减少连接器数量。\r\n\r\n（7）光纤接口连接不同待测元件连接器时，由于连接器接触其附带的灰尘将引起光路连接不良，具体表现链路损耗增加。因此需要及时确保待测元件接头清洁无灰尘。\r\n\r\n（8）光纤接口经使用不善将引起光路连接不良、损耗明显增加情况下，需要清洁面板内的光纤接头，具体步骤打开机箱上盖板后，退出面板内部光纤连接器接头螺纹，按照清洁光纤接头做法清洗光纤连接头后，重新装入面板内连接器并恢复机箱原始状态。"));

	return TRUE;
}
