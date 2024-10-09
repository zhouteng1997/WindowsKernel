//#include <ntifs.h>
//
//typedef struct _KLDR_DATA_TABLE_ENTRY {
//	LIST_ENTRY InLoadOrderLinks;
//	LIST_ENTRY exp;
//	ULONG un;
//	ULONG NonPagedDebugInfo;
//	ULONG DllBase;
//	ULONG EntryPoint;
//	ULONG SizeOfImage;
//	UNICODE_STRING FullDllName;
//	UNICODE_STRING BaseDllName;
//	ULONG Flags;
//	USHORT LoadCount;
//	USHORT __Undefined5;
//	ULONG  __Undefined6;
//	ULONG  CheckSum;
//	ULONG  TimeDateStamp;
//} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;
//
//NTKERNELAPI NTSTATUS ObReferenceObjectByName(
//	__in PUNICODE_STRING ObjectName,
//	__in ULONG Attributes,
//	__in_opt PACCESS_STATE AccessState,// 访问权限可以写0 写0完全访问不受控制FILE_ALL_ACCESS,
//	__in_opt ACCESS_MASK DesiredAccess,
//	__in POBJECT_TYPE ObjectType,//对象类型
//	__in KPROCESSOR_MODE AccessMode,// 内核模式 有三种模式 enum 类型
//	__inout_opt PVOID ParseContext,
//	__out PVOID* Object //输出对象 我们要得到的驱动对象
//);
//
//
//extern POBJECT_TYPE* IoDriverObjectType;
//
//VOID DriverUnload(PDRIVER_OBJECT pDriver)
//{
//	DbgPrint("END\r\n");//卸载
//}
//
//NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
//{
//
//	PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
//	PKLDR_DATA_TABLE_ENTRY pre = (PKLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink;//本身的节点
//	PKLDR_DATA_TABLE_ENTRY next = (PKLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink;//下一个驱动的节点
//
//
//	UNICODE_STRING driverName1 = { 0 };
//	RtlInitUnicodeString(&driverName1, L"\\driver\\HTTP");
//
//	UNICODE_STRING driverName = { 0 };
//	RtlInitUnicodeString(&driverName, L"HTTP.sys");//获取名字
//
//	while (next != pre)//遍历一遍
//	{
//
//		if (next->BaseDllName.Length != 0 && RtlCompareUnicodeString(&driverName, &next->BaseDllName, TRUE) == 0)//判断这个是不是空的，然后名字是否对的上
//		{
//
//			PDRIVER_OBJECT Driver = NULL;
//			NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);//通过驱动的到驱动的对象
//
//			if (NT_SUCCESS(status))
//			{
//				RemoveEntryList(&next->InLoadOrderLinks);
//				Driver->DriverInit = NULL;
//				Driver->DriverSection = NULL;//从双链表中去除，同时把一些其他的东西也都置0
//			}
//			DbgPrintEx(77, 0, "[db]:driver name = %wZ\r\n", &next->FullDllName);
//
//			break;
//		}
//
//		next = (PKLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink;//遍历下一个
//	}
//
//	pDriver->DriverUnload = DriverUnload;
//	return STATUS_SUCCESS;
//}
