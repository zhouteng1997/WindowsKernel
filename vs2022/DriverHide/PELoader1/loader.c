#include "loader.h"  // 包含自定义头文件 loader.h，其中可能定义了 ExAllocatePoolZero 和其他自定义函数
#include "tool.h"
#include <ntimage.h>  // 包含 Windows NT 内核的图像处理相关的头文件

typedef NTSTATUS(NTAPI* DriverEntryProc)(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg);

// 定义 IMAGE_RELOC 结构体，用于表示基址重定位项
typedef struct _IMAGE_RELOC {
	UINT16 Offset : 12;  // 表示重定位项在页面中的偏移量，占 12 位
	UINT16 Type : 4;     // 表示重定位的类型，占 4 位
} IMAGE_RELOC, * PIMAGE_RELOC;

// 将文件缓冲区转换为内存映像
PUCHAR FileToImage(char* fileBuffer) {
	if (!fileBuffer) return NULL;  // 如果文件缓冲区为空，直接返回 NULL

	// 获取 DOS 头的指针，以访问 DOS 头中的 e_lfanew 字段
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)fileBuffer;

	// 根据 DOS 头中的 e_lfanew 字段获取 NT 头的指针
	PIMAGE_NT_HEADERS pNts = (PIMAGE_NT_HEADERS)(fileBuffer + pDos->e_lfanew);

	// 获取整个 PE 映像在内存中的大小
	ULONG sizeOfImage = pNts->OptionalHeader.SizeOfImage;

	// 分配内存用于存储 PE 文件的内存映像
	PUCHAR imageBuff = ExAllocatePoolZero(NonPagedPool, sizeOfImage, 'load');

	// 将分配的内存初始化为 0
	memset(imageBuff, 0, sizeOfImage);

	// 将 PE 头部复制到内存映像中
	memcpy(imageBuff, fileBuffer, pNts->OptionalHeader.SizeOfHeaders);

	// 获取节区的数量
	ULONG numberOfSections = pNts->FileHeader.NumberOfSections;

	// 获取第一个节区头的指针
	PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNts);

	// 将每个节区的数据从文件缓冲区复制到内存映像中的相应位置
	for (ULONG i = 0; i < numberOfSections; i++) {
		memcpy(imageBuff + pSection->VirtualAddress, fileBuffer + pSection->PointerToRawData, pSection->SizeOfRawData);
		pSection++;  // 移动到下一个节区头
	}

	// 返回指向内存映像的指针
	return imageBuff;
}

// 更新内存映像的重定位表
BOOLEAN UpdateRelocation(char* imageBuffer) {
	// 获取 NT 头的指针
	PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBuffer);
	if (!pNts) return FALSE;  // 如果 NT 头不存在，返回 FALSE

	// 获取重定位表的目录项指针
	PIMAGE_DATA_DIRECTORY iRelocation = &pNts->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	// 获取基址重定位表的基址
	PIMAGE_BASE_RELOCATION pBase = (PIMAGE_BASE_RELOCATION)(imageBuffer + iRelocation->VirtualAddress);

	// 遍历重定位表中的每一个块
	while (pBase->SizeOfBlock && pBase->VirtualAddress) {
		// 获取重定位项的指针，指向重定位表中的具体偏移量和类型
		/*PIMAGE_RELOC RelocationBlock = (PIMAGE_RELOC)(pBase->VirtualAddress + imageBuffer + sizeof(IMAGE_BASE_RELOCATION));*/
		PIMAGE_RELOC RelocationBlock = (PIMAGE_RELOC)((PUCHAR)pBase+ sizeof(IMAGE_BASE_RELOCATION));

		// 计算当前块中重定位项的数量
		UINT32 NumberOfRelocation = (pBase->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(UINT16);

		// 处理每个重定位项
		for (UINT32 i = 0; i < NumberOfRelocation; i++) {
			if (RelocationBlock[i].Type == IMAGE_REL_BASED_DIR64) {
				// 如果重定位类型为 64 位（如 x64 架构下），计算新地址并更新
				PUINT64 address = (PUINT64)((PUINT8)imageBuffer + pBase->VirtualAddress + RelocationBlock[i].Offset);
				UINT64 delta = (UINT64)(*address - pNts->OptionalHeader.ImageBase + (PUINT8)imageBuffer);
				*address = delta;
			}
			if (RelocationBlock[i].Type == IMAGE_REL_BASED_HIGHLOW) {
				// 如果重定位类型为 32 位（如 x86 架构下），计算新地址并更新
				PUINT32 address = (PUINT32)((PUINT8)imageBuffer + pBase->VirtualAddress + RelocationBlock[i].Offset);
				UINT32 delta = (UINT32)(*address - pNts->OptionalHeader.ImageBase + (PUINT8)imageBuffer);
				*address = delta;
			}
		}

		// 移动到下一个基址重定位块
		pBase = (PIMAGE_BASE_RELOCATION)((PUCHAR)pBase + pBase->SizeOfBlock);
	}

	return TRUE;  // 如果重定位成功，返回 TRUE
}

BOOLEAN UpdataIAT(char* imageBuffer) {
	if (!imageBuffer) return FALSE;
	// 获取 NT 头的指针
	PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBuffer);
	if (!pNts) return FALSE;  // 如果 NT 头不存在，返回 FALSE

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

		//修复cookie
		UpdateCookie(imageBase);

		//ca11 入口点
		PIMAGE_NT_HEADERS pNts = RtlImageNtHeader(imageBase);
		ULONG_PTR entry = pNts->OptionalHeader.AddressOfEntryPoint;
		DriverEntryProc EntryPointFunc = imageBase + entry;
		NTSTATUS status = EntryPointFunc(NULL, NULL);
		if (!NT_SUCCESS(status))
		{
			isSuccess = FALSE;
			break;
		}
		//清空PE头
		memset(imageBase, 0, PAGE_SIZE);
	} while (0);
	if (!isSuccess)
	{
		ExFreePool(imageBase);
	}
	return isSuccess;
}