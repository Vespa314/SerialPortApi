#pragma once

#include "SerialPort.h"
#include <afxmt.h>
#include <deque>

#define PORT_DEBUG_MODE 0

#define MAX_PORT_NUM 256

#define OPEN_PORT_SUCCESS 1
#define OPEN_PORT_FAIL -1

#define CLOSE_PORT_SUCCESS 1
#define CLOSR_PORT_FAIL 0

#define PORT_NUM_INVALID -2
#define BAUDRATE_INVALID -3
#define DATABITS_INVALID -4
#define PARITY_INVALID -5
#define STOPBIT_INVALID -6

#define BUFSIZE 65536	

#define uchar unsigned char

static const DWORD BAUDRATE[] = 
{CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,CBR_4800,
CBR_9600,CBR_14400,CBR_19200,CBR_38400,CBR_56000 ,
CBR_57600,CBR_115200 ,CBR_128000 ,CBR_256000 
};

static const DWORD PARITY[] = {NOPARITY,ODDPARITY,EVENPARITY,MARKPARITY,SPACEPARITY};

static const DWORD STOPBITS[]={ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS};


class CSerialPortApi
{
public:
	CSerialPortApi(void);
	virtual ~CSerialPortApi(void);

	
	//有效端口
 	CString PortList[MAX_PORT_NUM];
	//获取有效端口数目
 	size_t GetPortNum();

	//打开串口
	INT OpenPort(CString sPort,DWORD dwBaudRate,BYTE byDataBits,BYTE byParity,BYTE byStopBits);
	//关闭串口
	INT ClosePort();
	//发送数据
	void Send(CString str);
	void Send(uchar str[], size_t SendLength);
	//是否有接受数据未读取
	BOOL ReceiveFlag;
	//读取接受的数据  二选一
	CString ReadRecv();
	deque<BYTE> ReadRecvByte();
	vector<CString> ReadRecvByteSplite(char ch);
	
	CString ErrorMsg();
private:
	//************************************
	// Method:    getExistPort
	// FullName:  CSerialPortApi::getExistPort
	// Access:    public 
	// Returns:   void
	// 获取所有的端口号，放在PortList中;
	// 并且设置有效端口数目PortNum
	//************************************
 	void getExistPort();

	HANDLE m_hCom;
 	size_t PortNum;
	CSerialPort m_port;
	BOOL b_portIsOpen;
	HANDLE m_hSendEvent;

	static UINT SendThreadProc(LPVOID pParam);
	static UINT RevThreadProc(LPVOID pParam);
	uchar* Str4Send;
	size_t mSendLength;

	CCriticalSection m_bRevCS;
	deque<BYTE> m_dequeRevData;

	BOOL IsPortValid(CString sCom);
	BOOL IsBaudrateValid(DWORD dwBaudRate);
	BOOL IsDataBitValid(BYTE byDataBits);
	BOOL IsParityValid(BYTE byParity);
	BOOL IsStopbitValid(BYTE byStopBits);
};


typedef struct _SEND
{
	BOOL bFlag;
	DOUBLE value;
}SEND;