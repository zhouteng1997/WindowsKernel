#include <ntifs.h>
#include "tool.h"
#include "dll.h"
#include "loader.h" 

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
	DbgPrint("END\r\n");//п╤ть
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	DbgBreakPoint();
	ULONG dwImageSize = sizeof(sysData);
	unsigned char* pMemory = (unsigned char*)ExAllocatePool(NonPagedPool, dwImageSize);
	memcpy(pMemory, sysData, dwImageSize);
	for (ULONG i = 0; i < dwImageSize; i++)
	{
		pMemory[i] ^= 0xd8;
		pMemory[i] ^= 0xcd;
	}
	DbgBreakPoint();
	QueryModule(" ", NULL);
	DbgBreakPoint();
	LoadDriver(pMemory);
	ExFreePool(pMemory);
	pDriver->DriverUnload = DriverUnload;
	return STATUS_UNSUCCESSFUL;
}