#include <ntifs.h>
#include "Tool.h"
#include "Ssdt.h"


ULONG_PTR goldFunc = 0;

typedef NTSTATUS(NTAPI* OpenProcessProc)(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
	);

NTSTATUS NTAPI MyOpenProcess(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId
)
{
	DbgPrintEx(77, 0, "[db]:%s\r\n", __FUNCTION__);
	OpenProcessProc func = (OpenProcessProc)goldFunc;

	return func(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
}

VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
	SsdtSetHook("ZwOpenProcess", goldFunc);
	SsdtDestory();

	//ÑÓÊ±
	LARGE_INTEGER inTime = { 0 };
	inTime.QuadPart = -10000 * 3000;
	KeDelayExecutionThread(KernelMode, FALSE, &inTime);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pDriver;
	pReg;

	DbgBreakPoint();
	SsdtInit();
	goldFunc = SsdtSetHook("ZwOpenProcess", (ULONG_PTR)MyOpenProcess);

	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
