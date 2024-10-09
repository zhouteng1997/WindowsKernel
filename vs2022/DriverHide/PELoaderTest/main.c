#include <ntifs.h>


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pDriver;
	pReg;
	DbgPrint("----------PELoaderTest_这是第二次-----------\r\n");//卸载
	return STATUS_SUCCESS;;
}
