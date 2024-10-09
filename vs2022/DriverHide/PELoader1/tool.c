#include "tool.h"

ULONG_PTR QueryModule(PUCHAR moduleName, ULONG_PTR* moduleSize) {
	if (moduleName == NULL) return 0;

	RTL_PROCESS_MODULES rtlModule = { 0 };
	PRTL_PROCESS_MODULES Modules = &rtlModule;
	BOOLEAN isAllocate = FALSE;

	ULONG* retLen = 0;
	NTSTATUS status = NtQuerySystemInformation(SystemModuleInformation, (void*)Modules, sizeof(RTL_PROCESS_MODULES), &retLen);

	if (status == STATUS_INFO_LENGTH_MISMATCH) {
		Modules = ExAllocatePoolZero(PagedPool, retLen + sizeof(RTL_PROCESS_MODULES), 'tool');
		if (!Modules) return 0;
		memset(Modules, 0, retLen + sizeof(RTL_PROCESS_MODULES));

		status = NtQuerySystemInformation(SystemModuleInformation, Modules, retLen + sizeof(RTL_PROCESS_MODULES), &retLen);
		if (!NT_SUCCESS(status))
		{
			ExFreePool(Modules);
			return 0;
		}
		isAllocate = TRUE;
	}

	ULONG_PTR moduleBase = 0;
	PUCHAR kernelModuleName = NULL;
	do {

		if (_stricmp(moduleName, "ntoskrnl.exe") == 0 || _stricmp(moduleName, "ntkrnlpa.exe") == 0)
		{
			PRTL_PROCESS_MODULE_INFORMATION moduleInfo = &Modules->Modules[0];
			moduleBase = moduleInfo->ImageBase;
			if (moduleSize) *moduleSize = moduleInfo->ImageSize;
			break;
		}
		kernelModuleName = ExAllocatePoolZero(PagedPool, strlen(moduleName) + 1, 'tool');
		memset(kernelModuleName, 0, strlen(moduleName) + 1);
		memcpy(kernelModuleName, moduleName, strlen(moduleName));
		_strupr(kernelModuleName);

		for (int i = 0; i < Modules->NumberOfModules; i++) {
			PRTL_PROCESS_MODULE_INFORMATION moduleInfo = &Modules->Modules[i];

			DbgPrintEx(77, 0, "baseName=%s,fullpath=%s \r\n", moduleInfo->FullPathName + moduleInfo->OffsetToFileName, moduleInfo->FullPathName);

			PUCHAR pathName = _strupr(moduleInfo->FullPathName);
			if (strstr(pathName, kernelModuleName)) {
				moduleBase = moduleInfo->ImageBase;
				if (moduleSize) *moduleSize = moduleInfo->ImageSize;
				break;
			}
		}

	} while (0);

	if (kernelModuleName)
		ExFreePool(kernelModuleName);
	if (isAllocate)
		ExFreePool(Modules);
	return moduleBase;

}
