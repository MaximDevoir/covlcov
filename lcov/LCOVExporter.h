#pragma once

#include "ExporterConfig.h"

#include <filesystem>

#ifdef LCOV_EXPORTS
#  define LCOV_API __declspec(dllexport)
#else
#  define LCOV_API __declspec(dllimport)
#endif

#include "Plugin/Exporter/IExportPlugin.hpp"
#include "Plugin/Exporter/CoverageData.hpp"

class LCOVExporter : public Plugin::IExportPlugin {
public:
	ExporterConfig cfg{std::filesystem::current_path()};

	void CheckArgument(const std::optional<std::wstring>& argument) override;
	std::optional<std::filesystem::path> Export(const Plugin::CoverageData& coverageData,
	                                            const std::optional<std::wstring>& argument) override;
	std::wstring GetArgumentHelpDescription() override;
	[[nodiscard]] int GetExportPluginVersion() const override;
};

extern "C" LCOV_API Plugin::IExportPlugin* CreatePlugin();
