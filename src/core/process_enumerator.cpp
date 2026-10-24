#include "process_enumerator.h"
#include "../util/json_util.h"
#include <windows.h>
#include <tlhelp32.h>
#include <sddl.h>
#include <winternl.h>

// 动态加载 NtQueryInformationProcess 获取命令行
typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
    HANDLE ProcessHandle, ULONG ProcessInformationClass,
    PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength
);
#define ProcessCommandLineInformation 60

namespace wph { namespace core {

std::wstring GetProcessPath(HANDLE hProcess) {
    wchar_t buffer[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameW(hProcess, 0, buffer, &size)) {
        return buffer;
    }
    return L"<Unknown>";
}

std::wstring GetProcessCommandLine(HANDLE hProcess) {
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtdll) return L"<Unknown>";

    pfnNtQueryInformationProcess NtQueryInformationProcess = 
        (pfnNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
    if (!NtQueryInformationProcess) return L"<Unknown>";

    ULONG returnLength = 0;
    NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessCommandLineInformation, NULL, 0, &returnLength);
    
    if (status == STATUS_INFO_LENGTH_MISMATCH || returnLength > 0) {
        std::vector<BYTE> buffer(returnLength);
        status = NtQueryInformationProcess(hProcess, ProcessCommandLineInformation, buffer.data(), returnLength, &returnLength);
        if (status == 0) { // STATUS_SUCCESS
            UNICODE_STRING* ustr = (UNICODE_STRING*)buffer.data();
            if (ustr->Length > 0 && ustr->Buffer) {
                return std::wstring(ustr->Buffer, ustr->Length / sizeof(wchar_t));
            }
        }
    }
    return L""; // 可能是系统进程，没有命令行
}

std::wstring GetProcessIntegrityLevel(HANDLE hProcess) {
    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) return L"<Unknown>";

    DWORD dwLength = 0;
    GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwLength);
    if (dwLength == 0) {
        CloseHandle(hToken);
        return L"<Unknown>";
    }

    std::vector<BYTE> buffer(dwLength);
    std::wstring il = L"Unknown";
    if (GetTokenInformation(hToken, TokenIntegrityLevel, buffer.data(), dwLength, &dwLength)) {
        TOKEN_MANDATORY_LABEL* pTIL = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(buffer.data());
        DWORD dwIL = *GetSidSubAuthority(pTIL->Label.Sid, 0);
        
        if (dwIL >= SECURITY_MANDATORY_SYSTEM_RID) il = L"System";
        else if (dwIL >= SECURITY_MANDATORY_HIGH_RID) il = L"High";
        else if (dwIL >= SECURITY_MANDATORY_MEDIUM_RID) il = L"Medium";
        else if (dwIL >= SECURITY_MANDATORY_LOW_RID) il = L"Low";
        else il = L"Untrusted";
    }
    CloseHandle(hToken);
    return il;
}

std::vector<ProcessInfo> EnumerateProcesses() {
    std::vector<ProcessInfo> result;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return result;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            ProcessInfo info;
            info.pid = pe32.th32ProcessID;
            info.ppid = pe32.th32ParentProcessID;
            info.name = pe32.szExeFile;
            
            // 处理特殊系统进程
            if (info.pid == 0) {
                info.name = L"[Idle Process]";
                info.path = L"<System>";
                info.commandLine = L"";
                info.integrityLevel = L"System";
            } else if (info.pid == 4) {
                info.name = L"System";
                info.path = L"<System>";
                info.commandLine = L"";
                info.integrityLevel = L"System";
            } else {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, info.pid);
                if (hProcess) {
                    info.path = GetProcessPath(hProcess);
                    info.commandLine = GetProcessCommandLine(hProcess);
                    info.integrityLevel = GetProcessIntegrityLevel(hProcess);
                    CloseHandle(hProcess);
                } else {
                    info.path = L"<Access Denied>";
                    info.commandLine = L"<Access Denied>";
                    info.integrityLevel = L"<Access Denied>";
                }
            }
            result.push_back(info);
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return result;
}

std::string GenerateProcessesJson(const std::vector<ProcessInfo>& processes) {
    std::string json = "[";
    bool first = true;
    for (const auto& p : processes) {
        if (!first) json += ",";
        first = false;
        
        json += "{";
        json += "\"pid\":" + std::to_string(p.pid) + ",";
        json += "\"ppid\":" + std::to_string(p.ppid) + ",";
        json += "\"name\":\"" + util::EscapeJson(p.name) + "\",";
        json += "\"path\":\"" + util::EscapeJson(p.path) + "\",";
        json += "\"cmd\":\"" + util::EscapeJson(p.commandLine) + "\",";
        json += "\"il\":\"" + util::EscapeJson(p.integrityLevel) + "\"";
        json += "}";
    }
    json += "]";
    return json;
}

}}