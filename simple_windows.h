//
// Created by Benedikt Krimmel on 1/6/2023.
//

#ifndef INJECTOR_SIMPLE_WINDOWS_H
#define INJECTOR_SIMPLE_WINDOWS_H

#include <string>
#include <stdexcept>

#include <Windows.h>
#include <psapi.h>


void simple_inject(DWORD pid, const std::string &lib) {
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD
                                 | PROCESS_QUERY_INFORMATION
                                 | PROCESS_VM_OPERATION
                                 | PROCESS_VM_WRITE
                                 | PROCESS_VM_READ,
                                 FALSE, pid);

    DWORD thread_id;
    auto func = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    auto mem = VirtualAllocEx(process, nullptr, lib.size() + 1, MEM_COMMIT, PAGE_READWRITE);
    SIZE_T written;
    WriteProcessMemory(process, mem, lib.c_str(), lib.size() + 1, &written);

    auto thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)mem, 0, &thread_id);
    if (!thread)
        throw std::runtime_error { "Couldn't create remote thread." };

    WaitForSingleObject(thread, 2000);
    DWORD exit_code;
    if (!GetExitCodeThread(thread, &exit_code))
        throw std::runtime_error { "Couldn't retrieve exit code." };

    VirtualFreeEx(process, mem, 0, MEM_RELEASE);

    if (!exit_code) {
        std::cout << "LoadLibraryA failed :(" << std::endl;
    }

    CloseHandle(thread);
    CloseHandle(process);
}

#endif //INJECTOR_SIMPLE_WINDOWS_H
