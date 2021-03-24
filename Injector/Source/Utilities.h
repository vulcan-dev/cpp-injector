#pragma once

#include <tlhelp32.h>
#include <cstdint>

namespace Utilities {
	auto fetchPID = [](LPCSTR pname) -> DWORD {
		if (pname == NULL || pname[0] == NULL) return 1;

		PROCESSENTRY32 pEntry;
		pEntry.dwSize = sizeof(PROCESSENTRY32);
		
		HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (Process32First(hsnap, &pEntry)) {
			do {
				if (!lstrcmpi(pEntry.szExeFile, pname)) {
					CloseHandle(hsnap);
					return pEntry.th32ProcessID;
				}
			} while (Process32Next(hsnap, &pEntry));
		}

		CloseHandle(hsnap);
		return 0;
	};

	bool Inject_RemoteThread(const std::string& dll, const DWORD pid) {
		if (pid == NULL || dll.size() == 0) {
			return 1;
		}

		try {
			void* pHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);

			LPVOID LoadLib = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
			LPVOID mem = (LPVOID)VirtualAllocEx(pHandle, NULL, strlen(dll.c_str()) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			WriteProcessMemory(pHandle, mem, dll.c_str(), strlen(dll.c_str()) + 1, NULL);
			
			CreateRemoteThread(pHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, (LPVOID)mem, NULL, NULL);
			CloseHandle(pHandle);
		} catch (std::exception e) {
			MessageBoxA(NULL, e.what(), "Injection Exception", MB_OK);
		}

		spdlog::info("Sucessfully Injected into Process");

		return 0;
	}
}