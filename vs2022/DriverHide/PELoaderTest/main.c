#include <ntifs.h>


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pDriver;
	pReg;
	DbgPrint("----------PELoaderTest_���ǵڶ���-----------\r\n");//ж��
	return STATUS_SUCCESS;;
}
