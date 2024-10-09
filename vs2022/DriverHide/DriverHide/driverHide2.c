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
//	__in_opt PACCESS_STATE AccessState,// ����Ȩ�޿���д0 д0��ȫ���ʲ��ܿ���FILE_ALL_ACCESS,
//	__in_opt ACCESS_MASK DesiredAccess,
//	__in POBJECT_TYPE ObjectType,//��������
//	__in KPROCESSOR_MODE AccessMode,// �ں�ģʽ ������ģʽ enum ����
//	__inout_opt PVOID ParseContext,
//	__out PVOID* Object //������� ����Ҫ�õ�����������
//);
//
//
//extern POBJECT_TYPE* IoDriverObjectType;
//
//VOID DriverUnload(PDRIVER_OBJECT pDriver)
//{
//	DbgPrint("END\r\n");//ж��
//}
//
//NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg)
//{
//
//	PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)pDriver->DriverSection;
//	PKLDR_DATA_TABLE_ENTRY pre = (PKLDR_DATA_TABLE_ENTRY)ldr->InLoadOrderLinks.Flink;//����Ľڵ�
//	PKLDR_DATA_TABLE_ENTRY next = (PKLDR_DATA_TABLE_ENTRY)pre->InLoadOrderLinks.Flink;//��һ�������Ľڵ�
//
//
//	UNICODE_STRING driverName1 = { 0 };
//	RtlInitUnicodeString(&driverName1, L"\\driver\\HTTP");
//
//	UNICODE_STRING driverName = { 0 };
//	RtlInitUnicodeString(&driverName, L"HTTP.sys");//��ȡ����
//
//	while (next != pre)//����һ��
//	{
//
//		if (next->BaseDllName.Length != 0 && RtlCompareUnicodeString(&driverName, &next->BaseDllName, TRUE) == 0)//�ж�����ǲ��ǿյģ�Ȼ�������Ƿ�Ե���
//		{
//
//			PDRIVER_OBJECT Driver = NULL;
//			NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);//ͨ�������ĵ������Ķ���
//
//			if (NT_SUCCESS(status))
//			{
//				RemoveEntryList(&next->InLoadOrderLinks);
//				Driver->DriverInit = NULL;
//				Driver->DriverSection = NULL;//��˫������ȥ����ͬʱ��һЩ�����Ķ���Ҳ����0
//			}
//			DbgPrintEx(77, 0, "[db]:driver name = %wZ\r\n", &next->FullDllName);
//
//			break;
//		}
//
//		next = (PKLDR_DATA_TABLE_ENTRY)next->InLoadOrderLinks.Flink;//������һ��
//	}
//
//	pDriver->DriverUnload = DriverUnload;
//	return STATUS_SUCCESS;
//}
