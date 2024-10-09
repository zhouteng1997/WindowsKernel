#include "Ssdt.h"
#include "Tool.h"
#include <intrin.h>

//��һ��ֻ�ܴ���32λ�����⣬64�����Ҳ�֪����δ�����ʱ����

PUCHAR gMapNtdll = NULL;

//���������
//extern PSsdtTable KeServiceDescriptorTable;

#ifdef _WIN64
 // ������ Windows 10 ��ȡ SSDT �ķ�ʽ
 //extern "C" NTKERNELAPI PVOID KeServiceDescriptorTableShadow;
#else
 // ������ Windows 7 ��������ϵͳ�ķ�ʽ
 //extern "C" NTKERNELAPI PVOID KeServiceDescriptorTable;
#endif

extern  NTKERNELAPI PVOID KeServiceDescriptorTableShadow;


UINT64 wpOff()
{
	UINT64 cr0 = __readcr0();
	_disable();
	__writecr0(cr0 & (~0x10000));
	return cr0;
}

VOID wpOn(UINT64 value)
{
	__writecr0(value);
	_enable();
}


BOOLEAN SsdtInit()
{
	if (gMapNtdll) return TRUE;
	PWCH path = GetSystemRootNtdllPath();
	gMapNtdll = MapOfViewFile(path);
	ExFreePool(path);
	return TRUE;
}

VOID SsdtDestory()
{
	if (!gMapNtdll) return;
	UmMapOfViewFile(gMapNtdll);
	gMapNtdll = NULL;
}

PSsdtTable SsdtGet()
{
	UNICODE_STRING functionName;
	RtlInitUnicodeString(&functionName, L"KeServiceDescriptorTableShadow");
	PVOID pKeServiceDescriptorTableShadow = MmGetSystemRoutineAddress(&functionName);

	UNICODE_STRING functionName;
	RtlInitUnicodeString(&functionName, L"KeServiceDescriptorTableShadow");
	PVOID pKeServiceDescriptorTableShadow = MmGetSystemRoutineAddress(&functionName);

	if (pKeServiceDescriptorTableShadow) {
		// ���ڿ���ʹ�� pKeServiceDescriptorTableShadow ���н�һ������
	}
	else {
		// ������Ż�ȡʧ�ܵ����
	}

	return (PSsdtTable)((PUCHAR)KeServiceDescriptorTableShadow + 0x40);
}

ULONG SsdtGetFunctionIndex(char* funName)
{
	PUCHAR func = (PUCHAR)ExportTableFuncByName((char*)gMapNtdll, funName);
	if (!func) return (ULONG) -1;
	return *(PULONG)(func + 1);
}

ULONG_PTR SsdtSetHook(char* funName, ULONG_PTR newFunction)
{
	PSsdtTable ssdtTable = SsdtGet();

	ULONG index = SsdtGetFunctionIndex(funName);

	if (index == -1) return 0;

	DbgBreakPoint();
#ifdef _WIN64
	ULONG function = ((ssdtTable->ssdt.funcTable[index] >> 4) + ssdtTable->ssdt.funcTable);
#else
	ULONG function = ssdtTable->ssdt.funcTable[index];
#endif

	UINT64 _cr0 = wpOff();

	ssdtTable->ssdt.funcTable[index] = newFunction;

	wpOn(_cr0);

	return function;

}