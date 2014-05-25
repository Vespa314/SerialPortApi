#include "StdAfx.h"
#include "SerialPortApi.h"

CSerialPortApi::CSerialPortApi(void)
{
	m_hCom = INVALID_HANDLE_VALUE;
	PortNum = 0;
	b_portIsOpen = FALSE;
	ReceiveFlag = FALSE;
	m_hSendEvent = CreateEvent( 
		NULL,         // no security attributes
		TRUE,         // manual-reset event
		FALSE,        // initial state is nosignaled
		NULL		  // object name
		); 
// 	if(m_hSendEvent==NULL)
// 		return false;

	getExistPort();
}

CSerialPortApi::~CSerialPortApi(void)
{
}

void CSerialPortApi::getExistPort()
{
	PortNum = 0;
	CString str;
	HKEY hKey;
	str=_T("HARDWARE\\DEVICEMAP\\SERIALCOMM");
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, str,0,KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) 
	{
		RegCloseKey( hKey );
		return;
	}

	TCHAR    achClass[MAX_PATH] = _T("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys;                 // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	TCHAR  achValue[MAX_PATH]; 
	DWORD cchValue = MAX_PATH; 
	BYTE  achBuff[80]; 
	DWORD chValue = 60; 
	DWORD type = REG_SZ;
	// Get the class name and the value count. 
	if(RegQueryInfoKey(hKey,        // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime)!=ERROR_SUCCESS)      // last write time 
	{
		RegCloseKey( hKey );
		return;
	}
	if (cValues) 
	{
		for (DWORD j = 0, retValue = ERROR_SUCCESS; 
			j < cValues; j++) 
		{ 
			chValue = 60;
			cchValue = 60;
			retValue = RegEnumValue(hKey, j, achValue, 
				&cchValue, 
				NULL, 
				&type,    // &dwType, 
				achBuff, // &bData, 
				&chValue);   // &bcData 
			if(retValue == ERROR_SUCCESS)
			{
				PortList[PortNum++] = (LPTSTR)(achBuff);
			}
		}
	}
	RegCloseKey( hKey );
	return;
}

INT CSerialPortApi::OpenPort( CString sPort,DWORD dwBaudRate,BYTE byDataBits,BYTE byParity,BYTE byStopBits )
{
	if(!IsPortValid(sPort))  return PORT_NUM_INVALID;
	if(!IsBaudrateValid(dwBaudRate))  return BAUDRATE_INVALID;
	if(!IsDataBitValid(byDataBits))  return DATABITS_INVALID;
	if(!IsParityValid(byParity))  return PARITY_INVALID;
	if(!IsStopbitValid(byStopBits))  return STOPBIT_INVALID;

 	m_hCom = m_port.OpenPort(sPort,dwBaudRate,byDataBits,byParity,byStopBits);
  	if(m_hCom != INVALID_HANDLE_VALUE)
  	{
		b_portIsOpen = TRUE;
		AfxBeginThread(SendThreadProc,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
		AfxBeginThread(RevThreadProc,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
  		return OPEN_PORT_SUCCESS;
  	}
  	else
  	{
		//CString str = m_port.GetError();
		//AfxMessageBox(str);
  		return OPEN_PORT_FAIL;
  	}
}


BOOL CSerialPortApi::IsPortValid( CString sCom )
{
	for(size_t i = 0;i < PortNum;i++)
	{
		if(PortList[i].Compare(sCom)==0)
			return TRUE;
	}
	if(PORT_DEBUG_MODE)
	{
		CString str;
		str.Format("Port Num(%s) is Invalid!",sCom);
		AfxMessageBox(str);
	}
		
	return FALSE;
}

size_t CSerialPortApi::GetPortNum()
{
	return PortNum;
}

BOOL CSerialPortApi::IsBaudrateValid( DWORD dwBaudRate )
{
	for(size_t i = 0;i < sizeof BAUDRATE;i++)
	{
		if(BAUDRATE[i] == dwBaudRate)
			return TRUE;
	}
	if(PORT_DEBUG_MODE)
	{
		CString str;
		str.Format("Baud rate(%d) is Invalid!",dwBaudRate);
		AfxMessageBox(str);
	}

	return FALSE;
}

BOOL CSerialPortApi::IsDataBitValid( BYTE byDataBits )
{
	if(byDataBits == 6 || byDataBits == 7 || byDataBits == 8)
		return TRUE;
	if(PORT_DEBUG_MODE)
	{
		CString str;
		str.Format("Data bits(%d) is Invalid!",byDataBits);
		AfxMessageBox(str);
	}
	return FALSE;
}

BOOL CSerialPortApi::IsParityValid( BYTE byParity )
{
	for(size_t i = 0;i < sizeof PARITY;i++)
	{
		if(PARITY[i] == byParity)
			return TRUE;
	}
	if(PORT_DEBUG_MODE)
	{
		CString str;
		str.Format("Parity bit(%d) is Invalid!",byParity);
		AfxMessageBox(str);
	}
	return FALSE;
}

BOOL CSerialPortApi::IsStopbitValid( BYTE byStopBits )
{
	for(size_t i = 0;i < sizeof STOPBITS;i++)
	{
		if(STOPBITS[i] == byStopBits)
			return TRUE;
	}
	if(PORT_DEBUG_MODE)
	{
		CString str;
		str.Format("Stop bit(%d) is Invalid!",byStopBits);
		AfxMessageBox(str);
	}
	return FALSE;
}

void CSerialPortApi::Send( CString str )
{
	Str4Send = str;
	SetEvent(m_hSendEvent);
}


UINT CSerialPortApi::SendThreadProc(LPVOID pParam)
{
	CSerialPortApi *capi = (CSerialPortApi *)pParam;
	HANDLE hEvent = 0;
	OVERLAPPED overlapped;
	memset(&overlapped,0,sizeof(OVERLAPPED));
	DWORD dwWrite = 0;
	static _SEND send;

	hEvent = CreateEvent( 
		NULL,         // no security attributes
		TRUE,         // manual-reset event
		FALSE,        // initial state is nosignaled
		NULL		  // object name
		); 
	overlapped.hEvent = hEvent;
	while(capi->b_portIsOpen)
	{
		::WaitForSingleObject(capi->m_hSendEvent,0xFFFFFFFF);
		::ResetEvent(capi->m_hSendEvent);
		if(capi->m_hCom==INVALID_HANDLE_VALUE)
			return 0;

		DWORD len = capi->Str4Send.GetLength();
		WriteFile(capi->m_hCom,capi->Str4Send.GetBuffer(len),len,&dwWrite,&overlapped);
	}
	return 0;
}

UINT CSerialPortApi::RevThreadProc(LPVOID pParam)
{
 	HANDLE			h_gEvent;
 	DWORD			dwBytesRead;
 	OVERLAPPED		Overlapped;
 
 	CSerialPortApi *capi = (CSerialPortApi *)pParam;
 
 	BOOL bReadStatus = FALSE;
 	dwBytesRead  = 0;
 	memset(&Overlapped,0,sizeof(OVERLAPPED));
 	h_gEvent = NULL;
 	h_gEvent = CreateEvent( 
 		NULL,				
 		TRUE,				//手工设置事件有无信号
 		FALSE,				//初始化事件为无信号状态
 		NULL				
 		); 
 
 	if(h_gEvent == NULL) return 0;
 
 	Overlapped.hEvent = h_gEvent;

 	BYTE data[BUFSIZE];
 	ZeroMemory(data,BUFSIZE);
 	DWORD dwEvtMask=0;
 	GetCommMask(capi->m_hCom,&dwEvtMask);
 	dwEvtMask |= EV_RXCHAR;
 	SetCommMask(capi->m_hCom,dwEvtMask);
 	while(capi->b_portIsOpen)
 	{
 		WaitCommEvent(capi->m_hCom,&dwEvtMask,NULL);
 		if(capi->m_hCom==INVALID_HANDLE_VALUE)
 			return 0;
 		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR){ 
 
 			COMSTAT ComStat ; 
 			DWORD dwLength,dwErrorFlags; 
 
 			ClearCommError(capi->m_hCom, &dwErrorFlags, &ComStat ) ; 
 			dwLength = ComStat.cbInQue ; //输入缓冲区有多少数据？ 
 
 			if (dwLength > 0){
 
 				bReadStatus = ReadFile( capi->m_hCom, data,dwLength, &dwBytesRead, &Overlapped); //读数据
 				if(!bReadStatus)
 				{
 					if(GetLastError()==ERROR_IO_PENDING)
 					{
 
 						while(!GetOverlappedResult(capi->m_hCom, &Overlapped, &dwBytesRead, TRUE ))
 						{ 
 							if(GetLastError() == ERROR_IO_INCOMPLETE) 
 								continue;
 						}
 						capi->m_bRevCS.Lock();
 						for(int i = 0;i<dwBytesRead;i++)
 						{
 							capi->m_dequeRevData.push_back(data[i]);
 						}
						capi->ReceiveFlag = TRUE;
 						capi->m_bRevCS.Unlock();
 					}
 				}
 				else
 				{
 					capi->m_bRevCS.Lock();
 					for(int i = 0;i<dwLength;i++)
 					{
 						capi->m_dequeRevData.push_back(data[i]);
 					}
					capi->ReceiveFlag = TRUE;
 					capi->m_bRevCS.Unlock();
 				}
 			}
 		}
 	}
	return 0;
}

CString CSerialPortApi::ReadRecv()
{
	if(!ReceiveFlag)
		return "";
 	CString str,strTemp;
 	str.Empty();
 	m_bRevCS.Lock();
 	int size = m_dequeRevData.size();
 	for(int i=0;i<size;i++)
 	{
 
 		strTemp.Format(_T("%c"),m_dequeRevData[0]);
 		m_dequeRevData.pop_front();
 		str += strTemp;
 	}
	ReceiveFlag = FALSE;
 	m_bRevCS.Unlock();
 	return str;
}

INT CSerialPortApi::ClosePort()
{
	if(m_hCom == INVALID_HANDLE_VALUE)
		return CLOSE_PORT_SUCCESS;
	b_portIsOpen = FALSE;
	if(m_port.ClosePort())
	{
		m_hCom = INVALID_HANDLE_VALUE;
		::SetEvent(m_hSendEvent);
		/*调用串口记得KillTimer*/
		return CLOSE_PORT_SUCCESS;
	}
	return CLOSR_PORT_FAIL;
}

CString CSerialPortApi::ErrorMsg()
{
	return m_port.GetError();
}
