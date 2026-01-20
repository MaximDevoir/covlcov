#pragma once

#include "Plugin/Exporter/IExportPlugin.hpp"
#include "Plugin/Exporter/CoverageData.hpp"

class LCOVExporter : public Plugin::IExportPlugin {
public:
	void CheckArgument(const std::optional<std::wstring>& argument) override;
	std::optional<std::filesystem::path> Export(const Plugin::CoverageData& coverageData,
	                                            const std::optional<std::wstring>& argument) override;
	std::wstring GetArgumentHelpDescription() override;
	[[nodiscard]] int GetExportPluginVersion() const override;
};

extern "C" {
inline __declspec(dllexport) Plugin::IExportPlugin* CreatePlugin() {
	return new LCOVExporter();
}
}
