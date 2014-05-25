// SerialPort.cpp: implementation of the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "serialas.h"
#include "SerialPort.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialPort::CSerialPort()
{
   m_hCom = INVALID_HANDLE_VALUE;
   m_sError.Empty();
   m_dwError = 0;
}

CSerialPort::~CSerialPort()
{

}

HANDLE CSerialPort::OpenPort(LPCTSTR sPort,DWORD dwBaudRate,BYTE byDataBits,
				BYTE byParity,BYTE byStopBits,BOOL bSynchronization)
{
	bool bFlagTry = FALSE; //指示是否正常退出了try块
	TCHAR szPort[10] = {0};
	_tcscpy(szPort,_T("\\\\.\\"));
	_tcscat(szPort,sPort);

	DWORD dwFlag = 0;
	if(bSynchronization)
	{
		dwFlag = 0;
	}
	else
	{
		dwFlag = FILE_FLAG_OVERLAPPED;
	}
	if(m_hCom!=INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hCom);
		m_hCom=INVALID_HANDLE_VALUE;
	}
	__try
	{
	m_hCom = CreateFile(szPort,
							GENERIC_READ|GENERIC_WRITE,
							0,						//共享模式
							NULL,
							OPEN_EXISTING,
							dwFlag,						
							NULL);

	if(m_hCom==INVALID_HANDLE_VALUE)
	{
		__leave;
	}

	if(SetupComm(m_hCom,2048,1024)==0)
	{
		__leave;
	}

	if(PurgeComm( m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR 

		| PURGE_RXCLEAR )==0)
	{
		__leave;
	}
		
	DCB dcb;                         //定义dcb对象

	if(!GetCommState(m_hCom,&dcb))
	{
		__leave;
	}
	
	COMMTIMEOUTS ct;
	GetCommTimeouts(m_hCom, &ct);
	ct.ReadIntervalTimeout = 500;
	ct.ReadTotalTimeoutConstant = 1000;
	ct.ReadTotalTimeoutMultiplier = 1000;
	ct.WriteTotalTimeoutConstant = 0;
	ct.WriteTotalTimeoutMultiplier = 0;

	if(!SetCommTimeouts(m_hCom, &ct))
	{
		__leave;
	}

	dcb.Parity = byParity;				//获得校验方式
	if(byParity!=NOPARITY)
	{
		dcb.fBinary = TRUE;				//允许校验
	}
	dcb.BaudRate = dwBaudRate;			//获得通信速率
	dcb.ByteSize = byDataBits;			//数据位数
	dcb.StopBits = byStopBits;			//停止位

	dcb.fOutxCtsFlow = FALSE;			//流量控制
	dcb.fOutxDsrFlow = FALSE;
	dcb.fOutX = FALSE;
	dcb.fInX = FALSE;

	// 将设定的参数值用于该串口
	if(!SetCommState(m_hCom,&dcb))
	{
		__leave;
	}
	bFlagTry = TRUE;
	}//end try
	__finally
	{
//		if(AbnormalTermination())
		if(!bFlagTry)
		{
			m_dwError = GetLastError();
			if(m_hCom!=INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_hCom);
				m_hCom=INVALID_HANDLE_VALUE;
			}
		}
	}
	return m_hCom;
}


BOOL  CSerialPort::ClosePort()
{
	if(CloseHandle(m_hCom))
	{
		m_hCom = INVALID_HANDLE_VALUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

CString CSerialPort::GetError()
{
	return GetErrorDesc(m_dwError);
}

DWORD CSerialPort::GetErrorCode() const
{
	return m_dwError;
}

CString CSerialPort::GetErrorDesc(DWORD dwError)
{
	CString strResult;
	HLOCAL hlocal = NULL; //malloc a result buffer
	BOOL fOk = FormatMessage
	(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
	NULL, dwError, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), 
	(PTSTR) &hlocal, 0, NULL);
	if (!fOk) 
	{
		//check if it is netError
		//remember do not load the dll or module the dll loaded
		HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (hDll != NULL) 
		{
			fOk =FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
			hDll, dwError, 
			MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), 
			(PTSTR) &hlocal, 0, NULL);
			FreeLibrary(hDll);
		}
	}
//	if(!fOk)
//	return GetSocketErrorDesc(dwError);
	strResult.Format(_T("错误代码 : %ld\n错误消息: %s"),dwError,hlocal);
	LocalFree(hlocal);
	return strResult;
}
