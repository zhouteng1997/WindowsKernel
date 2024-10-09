#include "loader.h"  // �����Զ���ͷ�ļ� loader.h�����п��ܶ����� ExAllocatePoolZero �������Զ��庯��
#include "tool.h"
#include <ntimage.h>  // ���� Windows NT �ں˵�ͼ������ص�ͷ�ļ�

typedef NTSTATUS(NTAPI* DriverEntryProc)(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg);

// ���� IMAGE_RELOC �ṹ�壬���ڱ�ʾ��ַ�ض�λ��
typedef struct _IMAGE_RELOC {
	UINT16 Offset : 12;  // ��ʾ�ض�λ����ҳ���е�ƫ������ռ 12 λ
	UINT16 Type : 4;     // ��ʾ�ض�λ�����ͣ�ռ 4 λ
} IMAGE_RELOC, * PIMAGE_RELOC;

// ���ļ�������ת��Ϊ�ڴ�ӳ��
PUCHAR FileToImage(char* fileBuffer) {
	if (!fileBuffer) return NULL;  // ����ļ�������Ϊ�գ�ֱ�ӷ��� NULL

	// ��ȡ DOS ͷ��ָ�룬�Է��� DOS ͷ�е� e_lfanew �ֶ�
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)fileBuffer;

	// ���� DOS ͷ�е� e_lfanew �ֶλ�ȡ NT ͷ��ָ��
	PIMAGE_NT_HEADERS pNts = (PIMAGE_NT_HEADERS)(fileBuffer + pDos->e_lfanew);

	// ��ȡ���� PE ӳ�����ڴ��еĴ�С
	ULONG sizeOfImage = pNts->OptionalHeader.SizeOfImage;

	// �����ڴ����ڴ洢 PE �ļ����ڴ�ӳ��
	PUCHAR imageBuff = ExAllocatePoolZero(NonPagedPool, sizeOfImage, 'load');

	// ��������ڴ��ʼ��Ϊ 0
	memset(imageBuff, 0, sizeOfImage);

	// �� PE ͷ�����Ƶ��ڴ�ӳ����
	memcpy(imageBuff, fileBuffer, pNts->OptionalHeader.SizeOfHeaders);

	// ��ȡ����������
	ULONG numberOfSections = pNts->FileHeader.NumberOfSections;

	// ��ȡ��һ������ͷ��ָ��
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNts);

	// ��ÿ�����������ݴ��ļ����������Ƶ��ڴ�ӳ���е���Ӧλ��
	for (ULONG i = 0; i < numberOfSections; i++) {
		memcpy(imageBuff + pSection->VirtualAddress, fileBuffer + pSection->PointerToRawData, pSection->SizeOfRawData);
		pSection++;  // �ƶ�����һ������ͷ
	}

	// ����ָ���ڴ�ӳ���ָ��
	return imageBuff;
}

// �����ڴ�ӳ����ض�λ��
BOOLEAN UpdateRelocation(char* imageBuffer) {
	// ��ȡ NT ͷ��ָ��
	PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBuffer);
	if (!pNts) return FALSE;  // ��� NT ͷ�����ڣ����� FALSE

	// ��ȡ�ض�λ���Ŀ¼��ָ��
	PIMAGE_DATA_DIRECTORY iRelocation = &pNts->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	// ��ȡ��ַ�ض�λ��Ļ�ַ
	PIMAGE_BASE_RELOCATION pBase = (PIMAGE_BASE_RELOCATION)(imageBuffer + iRelocation->VirtualAddress);

	// �����ض�λ���е�ÿһ����
	while (pBase->SizeOfBlock && pBase->VirtualAddress) {
		// ��ȡ�ض�λ���ָ�룬ָ���ض�λ���еľ���ƫ����������
		/*PIMAGE_RELOC RelocationBlock = (PIMAGE_RELOC)(pBase->VirtualAddress + imageBuffer + sizeof(IMAGE_BASE_RELOCATION));*/
		PIMAGE_RELOC RelocationBlock = (PIMAGE_RELOC)((PUCHAR)pBase+ sizeof(IMAGE_BASE_RELOCATION));

		// ���㵱ǰ�����ض�λ�������
		UINT32 NumberOfRelocation = (pBase->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(UINT16);

		// ����ÿ���ض�λ��
		for (UINT32 i = 0; i < NumberOfRelocation; i++) {
			if (RelocationBlock[i].Type == IMAGE_REL_BASED_DIR64) {
				// ����ض�λ����Ϊ 64 λ���� x64 �ܹ��£��������µ�ַ������
				PUINT64 address = (PUINT64)((PUINT8)imageBuffer + pBase->VirtualAddress + RelocationBlock[i].Offset);
				UINT64 delta = (UINT64)(*address - pNts->OptionalHeader.ImageBase + (PUINT8)imageBuffer);
				*address = delta;
			}
			if (RelocationBlock[i].Type == IMAGE_REL_BASED_HIGHLOW) {
				// ����ض�λ����Ϊ 32 λ���� x86 �ܹ��£��������µ�ַ������
				PUINT32 address = (PUINT32)((PUINT8)imageBuffer + pBase->VirtualAddress + RelocationBlock[i].Offset);
				UINT32 delta = (UINT32)(*address - pNts->OptionalHeader.ImageBase + (PUINT8)imageBuffer);
				*address = delta;
			}
		}

		// �ƶ�����һ����ַ�ض�λ��
		pBase = (PIMAGE_BASE_RELOCATION)((PUCHAR)pBase + pBase->SizeOfBlock);
	}

	return TRUE;  // ����ض�λ�ɹ������� TRUE
}

BOOLEAN UpdataIAT(char* imageBuffer) {
	if (!imageBuffer) return FALSE;
	// ��ȡ NT ͷ��ָ��
	PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBuffer);
	if (!pNts) return FALSE;  // ��� NT ͷ�����ڣ����� FALSE

	PIMAGE_DATA_DIRECTORY pimportDir = &pNts->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR import = (PIMAGE_IMPORT_DESCRIPTOR)(imageBuffer + pimportDir->VirtualAddress);

	BOOLEAN isSuccess = TRUE;
	for (; import->Name; import++)
	{
		PUCHAR libName = (imageBuffer + import->Name);
		DbgBreakPoint();
		ULONG_PTR base = QueryModule(libName, NULL);
		if (!base) {
			isSuccess = FALSE;
			break;
		}

		PIMAGE_THUNK_DATA pThuckName = (PIMAGE_THUNK_DATA)(imageBuffer + import->OriginalFirstThunk);
		PIMAGE_THUNK_DATA pThuckFunc = (PIMAGE_THUNK_DATA)(imageBuffer + import->FirstThunk);
		for (; pThuckName->u1.ForwarderString; ++pThuckName, ++pThuckFunc)
		{
			PIMAGE_IMPORT_BY_NAME FuncName = (PIMAGE_IMPORT_BY_NAME)(imageBuffer + pThuckName->u1.AddressOfData);
			//ULONG_PTR func = ExportTableFuncByName((char*)base, FuncName->Name);
			ULONG_PTR func = RtlFindExportedRoutineByName((char*)base, FuncName->Name);
			if (func)
			{
				pThuckFunc->u1.Function = (ULONG_PTR)func;
			}
			else
			{
				isSuccess = FALSE;
				break;
			}

		}
		if (!isSuccess)break;

	}
	return isSuccess;
}

VOID UpdateCookie(char* imageBuffer)
{
	DbgBreakPoint();
	if (!imageBuffer) return FALSE;
	PIMAGE_NT_HEADERS pNtS = RtlImageNtHeader(imageBuffer);
	if (!pNtS) return FALSE;
	PIMAGE_DATA_DIRECTORY pConfigDir = &pNtS->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
	PIMAGE_LOAD_CONFIG_DIRECTORY config = (PIMAGE_LOAD_CONFIG_DIRECTORY)(pConfigDir->VirtualAddress + imageBuffer);
	*(PULONG)(config->SecurityCookie) += 10;
}

BOOLEAN LoadDriver(PUCHAR fileBuffer)
{

	DbgBreakPoint();

	PUCHAR imageBase = FileToImage(fileBuffer);
	if (!imageBase)return FALSE;
	BOOLEAN isSuccess = FALSE;

	do
	{
		isSuccess = UpdateRelocation(imageBase);
		if (!isSuccess)break;
		isSuccess = UpdataIAT(imageBase);
		if (!isSuccess)break;

		//�޸�cookie
		UpdateCookie(imageBase);

		//ca11 ��ڵ�
		PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBase);
		ULONG_PTR entry = pNts->OptionalHeader.AddressOfEntryPoint;
		DriverEntryProc EntryPointFunc = imageBase + entry;
		NTSTATUS status = EntryPointFunc(NULL, NULL);
		if (!NT_SUCCESS(status))
		{
			isSuccess = FALSE;
			break;
		}
		//���PEͷ
		memset(imageBase, 0, PAGE_SIZE);
	} while (0);
	if (!isSuccess)
	{
		ExFreePool(imageBase);
	}
	return isSuccess;
}