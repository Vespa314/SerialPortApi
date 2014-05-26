SerialPortApi
=============

###应用环境：
**MFC**
`VS2008下测试通过`

###需要添加文件
* SerialPort.h
* SerialPort.cpp
* SerialPortApi.h
* SerialPortApi.cpp

### 使用：
设置头文件：
    #include "SerialPortApi.h"
添加API变量：
```
CSerialPortApi COMPort;
```
打开串口并设置定时器:
```
if(OPEN_PORT_SUCCESS == COMPort.OpenPort("COM3",57600,8,NOPARITY,ONESTOPBIT))
{
	SetTimer(RECV_TIMER_ID,50,NULL);
}
else
{
	AfxMessageBox(COMPort.ErrorMsg());
}
```
定时器内部读取数据：
```
if(COMPort.ReceiveFlag)
{
	CString str += COMPort.ReadRecv();
    //Todo
}
```
发送数据：`COMPort.Send(str);`
关闭串口：`COMPort.ClosePort();`



### 可用API：
##### 1.打开串口：
`INT CSerialPortApi::OpenPort(CString sPort,DWORD dwBaudRate,BYTE byDataBits,BYTE byParity,BYTE byStopBits);`
输入参数：
* **sPort**：端口号，一定要属于`PortList`元素，否则会返回错误；
* **dwBaudRate**：波特率，使用宏`BAUDRATE`中的元素，否则会返回错误；
```
static const DWORD BAUDRATE[] = 
{CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,
CBR_4800,CBR_9600,CBR_14400,CBR_19200,
CBR_38400,CBR_56000 ,CBR_57600,CBR_115200,
CBR_128000 ,CBR_256000 };
```
* **byDataBits**：数据位，可选值只有`6`,`7`,`8`，否则返回错误；
* **byParity**：校验位，使用宏`PARITY`中的元素(分别表示`无校验`,`奇校验`,`偶校验`,`标记`,`空格`)，否则会返回错误；
```
static const DWORD PARITY[] = {NOPARITY,ODDPARITY,EVENPARITY,MARKPARITY,SPACEPARITY};
```
* **byStopBits**：停止位，使用宏`STOPBITS`中的元素(分别表示`1`,`1.5`,`2`)，否则会返回错误；
```
static const DWORD STOPBITS[]={ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS};
```

返回值：
* 串口名有误：`PORT_NUM_INVALID`
* 波特率有误：`BAUDRATE_INVALID`
* 数据位有误：`DATABITS_INVALID`
* 校验位有误：`PARITY_INVALID`
* 停止位有误：`STOPBIT_INVALID`
* 成功：`OPEN_PORT_SUCCESS`
* 失败：`OPEN_PORT_FAIL`

> `OPEN_PORT_FAIL`的详细错误信息可以用`CString CSerialPortApi::ErrorMsg()` 获取，另外如果打开串口失败又找不到原因的话，可以将`SerialPortApi.h`中的：
```
#define PORT_DEBUG_MODE 0
```
改成：
```
#define PORT_DEBUG_MODE 1
```
这样出错了会以弹窗显示错误原因；

##### 2.关闭串口：
`INT CSerialPortApi::ClosePort();`
返回值:
* 成功：`CLOSE_PORT_SUCCESS`
* 失败：`CLOSR_PORT_FAIL`

##### 3.发送数据：
`void CSerialPortApi::Send(CString str);`
输入参数：
* **str**：待发送数据

##### 4.读取数据：
`CString CSerialPortApi::ReadRecv();`
返回值:
* 读取到的数据

>**说明：**读取数据之前请先判断`ReceiveFlag`是否为`TRUE`,否则读到的是空字符串；

##### 5.读取错误信息：
`CString CSerialPortApi::ErrorMsg();`
返回值:
* 错误代码和错误消息


### 可用变量和方法：
##### 1.可用的端口：`CString PortList[MAX_PORT_NUM];`

##### 2.获取可用的端口数目：`size_t GetPortNum();`

##### 3.缓冲区是否有接受数据未读取：`BOOL ReceiveFlag;`


### Private函数说明：
> 说明：这一部分说明使用者原则上无需知晓，可作为原理了解；

##### 1.获取所有可用的端口：`void getExistPort()`

基于读取注册表项`HARDWARE\DEVICEMAP\SERIALCOMM`，理论上和设备管理器看到的一样。

调用后将会初始化`CSerialPortApi`内的两个变量:`PortNum`,`PortList`,分别表示读取到的串口的数目(`size_t`)和列表(`CString`)

`CSerialPortApi`类变量外部读取`PortNum`要使用`size_t GetPortNum();`函数；

该函数放在了`CSerialPortApi`的构造函数中，所以定义变量后即可读取`PortList`;

##### 2.发送数据线程:`static UINT SendThreadProc(LPVOID pParam);`
线程开启后，只要`b_portIsOpen`为`TRUE`，即一直在一个`while`循环中等待，由`m_hSendEvent`变量激活发送模块，激活方法是:`SetEvent(m_hSendEvent);`,激活后即会发送`Str4Send`中的信息；

##### 3.接受数据线程:`static UINT RevThreadProc(LPVOID pParam);`
该线程如果接收到数据，会把数据放到`deque<BYTE> m_dequeRevData;`中的尾部，并且设置`ReceiveFlag`为`TRUE`;

如果检测到`ReceiveFlag`为`TRUE`，可以调用`ReadRecv()`读取数据，转为为`CString`变量返回，并且清空队列`m_dequeRevData`,设置`ReceiveFlag`为`FALSE`；

由于有锁机制，所以不会产生因为`RevThreadProc`对`m_dequeRevData`写信息而同时`ReadRecv()`对`m_dequeRevData`读信息这种事情；

### 其他：
如果想像串口助手等辅助软件一样添加若干个CComboBox,然后供用户选择的话，可以设置如下五个`CComboBox`变量关联相关控件：
```
CComboBox m_port;
CComboBox m_baudrate;
CComboBox m_parity;
CComboBox m_databits;
CComboBox m_stopbits;
```
然后使用下面代码初始化即可:
```
	m_port.ResetContent();

	//Set Port
	for(size_t i = 0;i < COMPort.GetPortNum();i++)
	{
		m_port.AddString(COMPort.PortList[i]);
	}
	m_port.SetCurSel(0);

	//Set Baudrate
	CString str = "";
	m_baudrate.ResetContent();
	int i = 0;
	for(i = 0;i<sizeof(BAUDRATE)/sizeof(DWORD);i++)
	{
		str.Format(_T("%d"),BAUDRATE[i]);
		m_baudrate.AddString(str);
	}
	m_baudrate.SelectString(-1,"57600");

	m_parity.ResetContent();
	m_parity.AddString(_T("无校验"));
	m_parity.AddString(_T("奇校验"));
	m_parity.AddString(_T("偶校验"));
	m_parity.AddString(_T("标记"));
	m_parity.AddString(_T("空格"));
	m_parity.SetCurSel(0);

	m_databits.ResetContent();
	m_databits.AddString(_T("6"));
	m_databits.AddString(_T("7"));
	m_databits.AddString(_T("8"));
	m_databits.SelectString(-1,"8");

	m_stopbits.ResetContent();
	m_stopbits.AddString(_T("1"));
	m_stopbits.AddString(_T("1.5"));
	m_stopbits.AddString(_T("2"));
	m_stopbits.SetCurSel(0);
```

