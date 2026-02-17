#include "pch.h"
#include "LCOVExporter.h"
#include <filesystem>
#include <fstream>
#include <sstream>

#include <yaml-cpp/yaml.h>

#include "Plugin/OptionsParserException.hpp"
#include "Plugin/Exporter/CoverageData.hpp"
#include "Plugin/Exporter/ModuleCoverage.hpp"
#include "Plugin/Exporter/FileCoverage.hpp"
#include "Plugin/Exporter/LineCoverage.hpp"

#include <gtest/gtest.h>

namespace fs = std::filesystem;

TEST(TestCaseName, TestName) {
	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}

TEST(Case2, TestName) {
	Plugin::IExportPlugin* exporter = CreatePlugin();
	ASSERT_NE(exporter, nullptr);
	exporter->CheckArgument(std::nullopt);
	auto version = exporter->GetExportPluginVersion();
	std::cout << "Export plugin version: " << version << std::endl;
	delete exporter;
}

TEST(LCOVExporterTest, ExportBasicCoverage) {
	Plugin::CoverageData data{L"TestRun", 0};
	auto& module = data.AddModule(L"TestModule.exe");
	fs::path cwd = fs::current_path();
	auto filePath = cwd / L"Test\\TestFile.cpp";
	auto& file = module.AddFile(filePath.wstring());
	file.AddLine(1, true); // executed
	file.AddLine(2, false); // not executed
	file.AddLine(3, true); // executed

	Plugin::IExportPlugin* exporter = CreatePlugin();
	fs::path outputPath = L"test_basic.info";
	auto result = exporter->Export(data, outputPath.wstring());
	ASSERT_TRUE(result.has_value());
	ASSERT_TRUE(fs::exists(*result));

	// Verify content
	std::wstring content;
	{
		std::wifstream ifs(*result);
		std::wstringstream buffer;
		buffer << ifs.rdbuf();
		content = buffer.str();
	} // ifs closed here

	ASSERT_NE(content.find(L"TN:"), std::wstring::npos);
	// Check if SF: line exists with something filled out for it.
	{
		auto pos = content.find(L"SF:");
		ASSERT_NE(pos, std::wstring::npos);
		auto eol = content.find(L'\n', pos);
		if (eol == std::wstring::npos) eol = content.size();
		// ensure at least 5 characters after "SF:" on the same line
		ASSERT_GE(static_cast<int>(eol - (pos + 3)), 5);
	}
	ASSERT_NE(content.find(L"DA:1,1"), std::wstring::npos);
	ASSERT_NE(content.find(L"DA:3,1"), std::wstring::npos);
	ASSERT_NE(content.find(L"LF:3"), std::wstring::npos); // 3 lines found
	ASSERT_NE(content.find(L"LH:2"), std::wstring::npos); // 2 lines hit
	ASSERT_NE(content.find(L"end_of_record"), std::wstring::npos);

	// Cleanup
	fs::remove(*result);

	delete exporter;
}

TEST(LCOVExporterTest, ExportWithDefaultFilename) {
	Plugin::CoverageData data{L"TestRun", 0};

	auto& module = data.AddModule(L"Module.exe");
	auto& file = module.AddFile(L"Test.cpp");
	file.AddLine(1, true);

	Plugin::IExportPlugin* exporter = CreatePlugin();

	// Act - no argument, should use default
	auto result = exporter->Export(data, std::nullopt);

	// Assert
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(L"lcov.info", result->filename().wstring());
	ASSERT_TRUE(fs::exists(*result));

	// Cleanup
	fs::remove(*result);
}

TEST(LCOVExporterTest, ExportEmptyData) {
	// Arrange
	Plugin::CoverageData data{L"Empty", 0};

	Plugin::IExportPlugin* exporter = CreatePlugin();
	fs::path outputPath = L"test_empty.info";

	// Act
	auto result = exporter->Export(data, outputPath.wstring());

	// Assert - should still create file even if empty
	ASSERT_TRUE(result.has_value());
	ASSERT_TRUE(fs::exists(*result));

	// Cleanup
	fs::remove(*result);
}

TEST(LCOVExporterTest, CheckArgument_InvalidDirectory) {
	Plugin::IExportPlugin* exporter = CreatePlugin();

	// Directory path without filename should throw
	ASSERT_THROW(exporter->CheckArgument(L"C:\\some\\directory\\"),
	             Plugin::OptionsParserException);
}

TEST(LCOVExporterTest, GetArgumentHelpDescription) {
	Plugin::IExportPlugin* exporter = CreatePlugin();
	auto help = exporter->GetArgumentHelpDescription();
	ASSERT_FALSE(help.empty());
	ASSERT_NE(help.find(L"lcov exporter plugin help"), std::wstring::npos);
}
