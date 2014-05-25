// SerialPort.h: interface for the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALPORT_H__5C32C8F3_BFC5_424C_B94D_1AF2B93B9000__INCLUDED_)
#define AFX_SERIALPORT_H__5C32C8F3_BFC5_424C_B94D_1AF2B93B9000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "SerialPort.h"
using namespace std;

class CSerialPort : public CObject  
{
public:
	CSerialPort();
	virtual ~CSerialPort();
private:
	HANDLE m_hCom;
	DWORD m_dwError;
	CString m_sError;
	CString GetErrorDesc(DWORD dwError);
public:
	HANDLE OpenPort(LPCTSTR sPort,DWORD dwBaudRate,
		BYTE byDataBits,BYTE byParity,BYTE byStopBits,BOOL bSynchronization = FALSE);
	BOOL  ClosePort();
	CString GetError();
	DWORD GetErrorCode() const;
};

#endif // !defined(AFX_SERIALPORT_H__5C32C8F3_BFC5_424C_B94D_1AF2B93B9000__INCLUDED_)
