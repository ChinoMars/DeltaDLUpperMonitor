#pragma once

#include <math.h>
class CCurveLine
{
public:
	CCurveLine(void);
	CCurveLine(CString FileName);
	CCurveLine(unsigned short *pBuf,int length);
	CCurveLine(int i,CString str);
	~CCurveLine(void);

	double ReadWaveLength(CString str,CString Pattern);
	int InititalCurv(char* data);

	double Begin_WaveLength;
	double End_WaveLength;
	int PointNum;
	double* PointData;

	int InitSuccessFlag;
	CString condition_str;
};

char* GetSubString(char* str,int Begin_Idx,char EndFlag);
//×ª»»¡°-3.88518405E+001¡±³ÉÊý×Ö
double StringDecode(char* str,int power_bias = 0);