//
// Created by Benedikt Krimmel on 1/6/2023.
//

#ifndef INJECTOR_FIND_PROCESS_WINDOWS_H
#define INJECTOR_FIND_PROCESS_WINDOWS_H

#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include <Windows.h>
#include <psapi.h>

struct ProcessInfo {
    DWORD pid;
    std::string process_name;
};

ProcessInfo get_process_info(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                 FALSE, pid);
    TCHAR process_name[MAX_PATH] = TEXT("<unknown>");

    if (process != nullptr) {
        HMODULE mod;
        DWORD returned;
        if (EnumProcessModules(process, &mod, sizeof(mod), &returned)) { // NOLINT(bugprone-sizeof-expression)
            GetModuleBaseName(process, mod, process_name,
                              sizeof(process_name) / sizeof(TCHAR));
        }
    }

    CloseHandle(process);

    return ProcessInfo {
        pid, process_name
    };
}

class ProcessNotFoundException : public std::exception {};

ProcessInfo find_process(const std::string &process_name) {
    DWORD process_ids[1024];
    DWORD returned_processes_bytes;

    std::vector<ProcessInfo> matching_processes;

    if (!EnumProcesses(process_ids, sizeof(process_ids), &returned_processes_bytes)) {
        throw std::runtime_error { "Couldn't enumerate processes." };
    }

    DWORD returnedProcesses = returned_processes_bytes / sizeof(DWORD);
    for (int i = 0; i < returnedProcesses; i++) {
        if (process_ids[i] == 0)
            continue;

        auto info = get_process_info(process_ids[i]);
        if (info.process_name.find(process_name) != std::string::npos)
            matching_processes.push_back(info);
    }

    if (matching_processes.empty())
        throw std::logic_error { "Found no matching processes." };

    for (auto &process : matching_processes) {
        std::cout << std::setfill(' ') << std::setw(5)
                  << process.pid << ": " << process.process_name << std::endl;
    }

    if (matching_processes.size() > 1) {
        throw std::logic_error { "Found multiple matching processes." };
    }

    // there should only be one process in here
    return matching_processes[0];
}

#endif //INJECTOR_FIND_PROCESS_WINDOWS_H
