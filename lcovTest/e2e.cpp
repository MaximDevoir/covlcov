#include "pch.h"
#include <windows.h>
#include <filesystem>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

#include "gtest/gtest.h"


bool FilesAreIdentical(const std::filesystem::path& a,
                       const std::filesystem::path& b) {
	namespace fs = std::filesystem;

	if (!fs::exists(a) || !fs::exists(b))
		return false;

	if (fs::file_size(a) != fs::file_size(b))
		return false;

	std::ifstream fa(a, std::ios::binary);
	std::ifstream fb(b, std::ios::binary);

	if (!fa || !fb)
		return false;

	return std::equal(
		std::istreambuf_iterator<char>(fa),
		std::istreambuf_iterator<char>(),
		std::istreambuf_iterator<char>(fb)
	);
}

// Runs `exePath` with `args` (both wide) and captures stdout/stderr.
// Returns exit code and captured output.
struct ProcessResult {
	DWORD exitCode = static_cast<DWORD>(-1);
	std::string stdoutStr;
	std::string stderrStr;
};

static std::string ReadFromPipe(HANDLE pipe) {
	std::string out;
	constexpr DWORD bufSize = 4096;
	char buffer[bufSize];
	DWORD read = 0;
	while (ReadFile(pipe, buffer, bufSize, &read, nullptr) && read > 0) {
		out.append(buffer, buffer + read);
	}
	return out;
}

ProcessResult RunProcessCapture(const std::wstring& exePath, const std::wstring& args) {
	ProcessResult result;
	SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

	HANDLE outRead = nullptr, outWrite = nullptr;
	HANDLE errRead = nullptr, errWrite = nullptr;

	if (!CreatePipe(&outRead, &outWrite, &sa, 0)) return result;
	if (!SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0)) {
		CloseHandle(outRead);
		CloseHandle(outWrite);
		return result;
	}

	if (!CreatePipe(&errRead, &errWrite, &sa, 0)) {
		CloseHandle(outRead);
		CloseHandle(outWrite);
		return result;
	}
	if (!SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, 0)) {
		CloseHandle(outRead);
		CloseHandle(outWrite);
		CloseHandle(errRead);
		CloseHandle(errWrite);
		return result;
	}

	STARTUPINFOW si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = outWrite;
	si.hStdError = errWrite;
	si.hStdInput = nullptr;

	PROCESS_INFORMATION pi{};

	std::wstring cmd = L"\"" + exePath + L"\" " + args;
	// CreateProcessW needs a mutable LPWSTR
	std::vector<wchar_t> cmdLine(cmd.begin(), cmd.end());
	cmdLine.push_back(0);

	BOOL ok = CreateProcessW(nullptr, cmdLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si,
	                         &pi);

	// Parent doesn't need the write ends
	CloseHandle(outWrite);
	CloseHandle(errWrite);

	if (!ok) {
		CloseHandle(outRead);
		CloseHandle(errRead);
		return result;
	}

	// Read output while process runs (simple approach)
	result.stdoutStr = ReadFromPipe(outRead);
	result.stderrStr = ReadFromPipe(errRead);

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	result.exitCode = exitCode;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(outRead);
	CloseHandle(errRead);

	return result;
}

// Note: We are using cwd as base for paths. You must be running tasks from within the lcovTest folder.
TEST(E2E, RunExeWithString) {
	WCHAR cwdBuf[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, cwdBuf);
	std::wcout << L"cwd: " << cwdBuf << L"\n";

	std::filesystem::path cwdPath(cwdBuf);
	std::filesystem::path thisFile = cwdPath / L"e2e.cpp";
	ASSERT_TRUE(std::filesystem::exists(thisFile)) << "Must run from script from within lcovTest folder. CWD: "
	    << cwdPath.string();

	std::filesystem::path releasePath = std::filesystem::path(cwdBuf)
		/ L".."
		/ L"OpenCppCoverage"
		/ L"x64"
		/ L"Release";
	std::filesystem::path exePath = releasePath
		/ L"OpenCppCoverage.exe";
	std::filesystem::path pluginPath = releasePath
		/ L"Plugins"
		/ L"Exporter"
		/ L"lcov.dll";
	std::filesystem::path lcovTestE2EMockPath = std::filesystem::path(cwdBuf)
		/ L".."
		/ L"x64"
		/ L"Release"
		/ L"lcovTestE2EMock.exe";

	// get a std::wstring for CreateProcessW
	std::wstring exePathW = exePath.wstring();

	ASSERT_TRUE(std::filesystem::exists(exePath)) << "Executable not found at: " << exePath.string();
	ASSERT_TRUE(std::filesystem::exists(pluginPath)) << "lcov.dll not found: " << pluginPath.string();

	const std::wstring mockPathQuoted = L"\"" + lcovTestE2EMockPath.wstring() + L"\"";
	const std::wstring args = std::wstring(
			LR"(--export_type=html:codecov/coverage_report_html --sources lcovTestE2EMock --export_type=lcov:codecov/coverage_report.lcov -- )")
		+ mockPathQuoted;
	std::wcout << L"exePath: " << exePathW << L"\nargs: " << args << L"\n";
	ProcessResult r = RunProcessCapture(exePath, args);
	std::cout << "stdout:\n" << r.stdoutStr << "\n";
	std::cout << "stderr:\n" << r.stderrStr << "\n";
	std::cerr << "stderr:\n" << r.stderrStr << "\n";
	ASSERT_EQ(r.exitCode, 0u);

	std::filesystem::path codecovPath = std::filesystem::path(cwdBuf)
		/ L"codecov"
		/ L"coverage_report.lcov";

	// std::filesystem::path snapshotPath = std::filesystem::path(cwdBuf)
	// 	/ L"snapshot.lcov";
	WCHAR exeBuf[MAX_PATH];
	GetModuleFileNameW(nullptr, exeBuf, MAX_PATH);
	std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();
	std::filesystem::path snapshotPath = exeDir / L"snapshot.lcov";
	// Check if file was created
	ASSERT_TRUE(std::filesystem::exists(codecovPath)) << "LCOV output file not found at: " << codecovPath.string();
	ASSERT_TRUE(FilesAreIdentical(snapshotPath, codecovPath));
}
