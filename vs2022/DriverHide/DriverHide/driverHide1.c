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
	__in_opt PACCESS_STATE AccessState,// ����Ȩ�޿���д0 д0��ȫ���ʲ��ܿ���FILE_ALL_ACCESS,
	__in_opt ACCESS_MASK DesiredAccess,
	__in POBJECT_TYPE ObjectType,//��������
	__in KPROCESSOR_MODE AccessMode, //�ں�ģʽ ������ģʽ enum ����
	__inout_opt PVOID ParseContext,
	__out PVOID* Object //������� ����Ҫ�õ�����������
);


extern POBJECT_TYPE* IoDriverObjectType;


void DriverHide(PWCH ObjName)
{
	LARGE_INTEGER in = { 0 };
	in.QuadPart = -10000 * 5000;
	KeDelayExecutionThread(KernelMode, FALSE, &in);//�ȴ�
	UNICODE_STRING driverName1 = { 0 };
	RtlInitUnicodeString(&driverName1, ObjName);//��������������
	PDRIVER_OBJECT Driver = NULL;//��ʼ����������
	NTSTATUS status = ObReferenceObjectByName(&driverName1, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &Driver);//ͨ�������ĵ������Ķ���

	if (NT_SUCCESS(status))//�ж�
	{
		PKLDR_DATA_TABLE_ENTRY ldr = (PKLDR_DATA_TABLE_ENTRY)Driver->DriverSection;//��������ṹ.���Խ���Ϊ_LDR_DATA_TABLE_ENTRY��һ������洢����һ����������  
		DbgPrintEx(77, 0, "[db]: driver name = %wZ\r\n", &ldr->FullDllName);//�������
		Driver->DriverSection = ldr->InLoadOrderLinks.Flink;
		RemoveEntryList(&ldr->InLoadOrderLinks);//ȥ��˫�������е�ֵ
		Driver->DriverInit = NULL;
		ObDereferenceObject(Driver);//ɾ���ոյĵ�����������
	}
	return;
}


VOID DriverUnload(PDRIVER_OBJECT pDriver)
{
	pDriver;
	DbgPrint("END\r\n");//ж��
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
