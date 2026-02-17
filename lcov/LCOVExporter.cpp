#include "pch.h"
#include "LCOVExporter.h"

#include "Plugin/Exporter/ModuleCoverage.hpp"
#include "Plugin/Exporter/FileCoverage.hpp"
#include "Plugin/Exporter/LineCoverage.hpp"
#include "Plugin/OptionsParserException.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "ExporterConfig.h"

std::optional<std::filesystem::path> LCOVExporter::Export(const Plugin::CoverageData& coverageData,
                                                          const std::optional<std::wstring>& argument) {
	std::filesystem::path outputPath = argument ? *argument : L"lcov.info";
	std::wofstream ofs{outputPath, std::ios::binary};
	if (!ofs) {
		cfg.Log.AddMsg(ExporterConfigLog::MsgLevel::Error,
		               "LCOV Exporter: Cannot create the output file for LCOV export: " +
		               outputPath.string());
		cfg.Log.LogMessages();
		return outputPath;
	}

	if (cfg.Log.HasErrors()) {
		cfg.Log.LogMessages();
		return outputPath;
	}

	if (coverageData.GetModules().empty()) {
		return outputPath;
	}

	if (cfg.GetResolvedBaseDir() != std::filesystem::path{}) {
		cfg.Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "Base directory resolved to: " + cfg.GetResolvedBaseDir().string());
		cfg.Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "Only files within base directory will be included in report");
	}

	for (const auto& mod : coverageData.GetModules()) {
		for (const auto& file : mod->GetFiles()) {
			const auto sfPath = cfg.MakeSFPath(file->GetPath());
			if (!cfg.ShouldIncludeInReportByPath(file->GetPath())) {
				cfg.Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "Excluding file from report. Not within configured include path: " + sfPath.string());
				continue;
			}
			ofs << "TN:" << '\n';
			// Source file path
			ofs << "SF:" << sfPath.generic_wstring() << '\n';
			const auto& lines = file->GetLines();

			// DA entries: one per line, hit count is 1 (executed) or 0 (not)
			for (const auto& line : lines) {
				if (line.HasBeenExecuted()) {
					ofs << "DA:" << line.GetLineNumber() << ",1" << '\n';
				} else {
					// TODO: Should the inclusion of zero-hits be an option?
					ofs << "DA:" << line.GetLineNumber() << ",0" << '\n';
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

	cfg.Log.LogMessages();
	return outputPath;
}

void LCOVExporter::CheckArgument(const std::optional<std::wstring>& argument) {
	std::wcout << std::wstring(5, '\n');
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
	return L" lcov exporter plugin help\n"
		L"  LCOV format export (optional output file)\n"
		L" --export_type=lcov:reports/coverage.info\n"
		L"If omitted, defaults to lcov.info\n";
}

int LCOVExporter::GetExportPluginVersion() const { return 1; }

Plugin::IExportPlugin* CreatePlugin() {
	return new LCOVExporter();
}
