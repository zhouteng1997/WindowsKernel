#include "Ssdt.h"
#include "Tool.h"
#include <intrin.h>

//这一节只能处理32位的问题，64的暂且不知道如何处理，暂时跳过

PUCHAR gMapNtdll = NULL;

//导出服务表
//extern PSsdtTable KeServiceDescriptorTable;

#ifdef _WIN64
 // 适用于 Windows 10 获取 SSDT 的方式
 //extern "C" NTKERNELAPI PVOID KeServiceDescriptorTableShadow;
#else
 // 适用于 Windows 7 或其他老系统的方式
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
		// 现在可以使用 pKeServiceDescriptorTableShadow 进行进一步操作
	}
	else {
		// 处理符号获取失败的情况
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