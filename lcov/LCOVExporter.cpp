#include "pch.h"
#include "LCOVExporter.h"

#include "Plugin/Exporter/ModuleCoverage.hpp"
#include "Plugin/Exporter/FileCoverage.hpp"
#include "Plugin/Exporter/LineCoverage.hpp"
#include "Plugin/OptionsParserException.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

std::optional<std::filesystem::path> LCOVExporter::Export(const Plugin::CoverageData& coverageData,
                                                          const std::optional<std::wstring>& argument) {
	std::filesystem::path output = argument ? *argument : L"lcov.info";
	std::wofstream ofs{output, std::ios::binary};

	if (!ofs)
		throw std::runtime_error("LCOV Exporter: Cannot create the output file for LCOV export.");

	if (coverageData.GetModules().empty())
		return output;


	for (const auto& mod : coverageData.GetModules()) {
		for (const auto& file : mod->GetFiles()) {
			ofs << "TN:" << '\n';
			// Source file path
			ofs << "SF:" << file->GetPath().wstring() << '\n';

			const auto& lines = file->GetLines();

			// DA entries: one per line, hit count is 1 (executed) or 0 (not)
			for (const auto& line : lines) {
				if (line.HasBeenExecuted()) {
					ofs << "DA:" << line.GetLineNumber() << ",1" << '\n';
				}
			}

			// LF: lines found, LH: lines hit
			auto coveredCount = std::ranges::count_if(lines, [](const auto& line) {
				return line.HasBeenExecuted();
			});
			ofs << "LF:" << lines.size() << '\n';
			ofs << "LH:" << coveredCount << '\n';
			ofs << "end_of_record\n";
		}
	}


	std::wcout << "LCOVExporter: coverage data exported to \"" << output.wstring() << "\"\n";
	return output;
}

void LCOVExporter::CheckArgument(const std::optional<std::wstring>& argument) {
	std::wcout << std::wstring(5, '\n');
	// simple log argument
	if (argument) {
		std::wcout << "LCOVExporter::CheckArgument: argument=\"" << *argument << "\"\n";
	} else {
		std::wcout << "LCOVExporter::CheckArgument: no argument\n";
	}
	std::wcout << "LCOVExporter::CheckArgument: current working directory=\""
		<< std::filesystem::current_path().wstring() << "\"\n";

	// Try to check if the argument is a file.
	if (argument && !std::filesystem::path{*argument}.has_filename()) {
		throw Plugin::OptionsParserException("Invalid argument for LCOV export.");
	}

	std::wcout << std::wstring(5, '\n');
}

std::wstring LCOVExporter::GetArgumentHelpDescription() {
	return L"output file (optional)";
}

int LCOVExporter::GetExportPluginVersion() const { return 1; }
