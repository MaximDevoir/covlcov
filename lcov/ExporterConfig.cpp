#include "pch.h"
#include "ExporterConfig.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

void ExporterConfigLog::AddMsg(MsgLevel level, const std::string& message) {
	messages.emplace_back(level, message);
}

void ExporterConfigLog::LogMessages() {
	for (const auto& [level, msg] : messages) {
		std::cout << "LCOV EXPORTER [" << (level == MsgLevel::Info ? "INFO" : "ERROR") << "] " << msg << '\n';
	}
}

bool ExporterConfigLog::HasErrors() const {
	for (const auto& [level, msg] : messages) {
		if (level == MsgLevel::Error)
			return true;
	}
	return false;
}

ExporterConfig::ExporterConfig(std::filesystem::path startDir) {
	if (startDir.empty())
		startDir = std::filesystem::current_path();

	std::error_code ec;
	startDir = std::filesystem::weakly_canonical(startDir, ec);
	if (ec)
		startDir = std::filesystem::absolute(startDir, ec);

	if (auto found = FindCovLcovUpwards(startDir)) {
		LoadFromFile(*found);
		loaded_ = true;
	}
}

// Whether the configuration file is loaded
bool ExporterConfig::IsLoaded() const noexcept {
	return loaded_;
}

std::optional<std::filesystem::path> ExporterConfig::ConfigPath() const {
	if (!loaded_)
		return std::nullopt;
	return covlcovPath_;
}

bool ExporterConfig::IncludeByBaseDir() const noexcept {
	return includeByBaseDir_ && baseDir_.has_value();
}

std::optional<std::filesystem::path> ExporterConfig::FindCovLcovUpwards(const std::filesystem::path& startDir) {
	std::filesystem::path cur = startDir;

	while (true) {
		auto candidate = cur / L".covlcov";
		std::error_code ec;
		if (std::filesystem::exists(candidate, ec) && !ec)
			return candidate;

		auto parent = cur.parent_path();
		if (parent == cur || parent.empty())
			break;

		cur = std::move(parent);
	}

	return std::nullopt;
}

void ExporterConfig::LoadFromFile(const std::filesystem::path& covlcovPath) {
	covlcovPath_ = covlcovPath;
	Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "Loading .covlcov configuration from .covlcov at: " + covlcovPath_.string());
	std::ifstream ifs(covlcovPath_, std::ios::binary);
	if (!ifs) {
		isYamlValid = false;
		Log.AddMsg(ExporterConfigLog::MsgLevel::Error, "Failed to open .covlcov for reading.");
		return;
	}

	YAML::Node root;
	try {
		isYamlValid = true;
		root = YAML::Load(ifs);
		if (!root.IsMap()) {
			isYamlValid = false;
			Log.AddMsg(ExporterConfigLog::MsgLevel::Error, "Invalid .covlcov: unable to parse.");
			return;
		}
	} catch (const YAML::Exception& e) {
		isYamlValid = false;
		Log.AddMsg(ExporterConfigLog::MsgLevel::Error, "Failed to parse .covlcov:" + std::string(e.what()));
		return;
	}

	LoadFromYaml(root);
}

void ExporterConfig::LoadFromYaml(YAML::Node root) {
	if (root["baseDir"]) {
		baseDir_ = root["baseDir"].as<std::string>();
		includeByBaseDir_ = true;
	} else {
		baseDir_ = std::filesystem::path(".");
	}
	if (root["includeByBaseDir"]) {
		includeByBaseDir_ = root["includeByBaseDir"].as<bool>();
	}
	Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "Loaded .covlcov configuration from .covlcov at: " + covlcovPath_.string());
	Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "baseDir: " + (baseDir_.has_value() ? baseDir_.value().string() : "none"));
	Log.AddMsg(ExporterConfigLog::MsgLevel::Info, "includeByBaseDir: " + std::to_string(includeByBaseDir_));
}

std::filesystem::path ExporterConfig::GetResolvedBaseDir() const {
	if (!loaded_ || !baseDir_.has_value())
		return {};

	const auto cfgDir = covlcovPath_.has_parent_path()
		                    ? covlcovPath_.parent_path()
		                    : std::filesystem::current_path();


	return baseDir_->is_absolute() ? *baseDir_ : ((*baseDir_ == std::filesystem::path(".")) ? cfgDir : (cfgDir / *baseDir_));
}

/**
 * Takes an absolute path provided by OpenCppCoverage and converts it to an SF path, depending on configuration in .covlcov
 *
 * @param path An absolute or relative path
 * @return The SF path
 */
std::filesystem::path ExporterConfig::MakeSFPath(const std::filesystem::path& path) const {
	if (!isYamlValid) {
		return path;
	}
	if (!IncludeByBaseDir())
		return path;

	auto baseDir = GetResolvedBaseDir();
	if (baseDir.empty())
		return path;

	std::error_code ec1, ec2;
	auto absFile = std::filesystem::weakly_canonical(path, ec1);
	if (ec1) absFile = path;

	auto absBase = std::filesystem::weakly_canonical(baseDir, ec2);
	if (ec2) absBase = baseDir;

	auto rel = absFile.lexically_relative(absBase);
	if (rel.empty())
		return path;

	auto it = rel.begin();
	if (it != rel.end() && *it == L"..")
		return path;

	return rel;
}

/**
 * Determines whether the given file path should be included in the lcov report.
 *
 * If no .covlcov file is loaded, all files are included.
 *
 * If a .covlcov file is loaded, only files within matching within the baseDir are included.
 *
 * If no baseDir
 *
 * @param path The path of the file to check
 * @return Whether the file should be included in the lcov report
 */
bool ExporterConfig::ShouldIncludeInReportByPath(const std::filesystem::path& path) const {
	if (!IncludeByBaseDir())
		return true;

	const auto baseDir = GetResolvedBaseDir();
	if (baseDir.empty())
		return true;

	std::error_code ec1, ec2;
	auto absFile = std::filesystem::weakly_canonical(path, ec1);
	if (ec1) absFile = path;

	auto absBase = std::filesystem::weakly_canonical(baseDir, ec2);
	if (ec2) absBase = baseDir;

	// If not under baseDir, the relative path will start with ".." (or be empty).
	auto rel = absFile.lexically_relative(absBase);
	if (rel.empty())
		return false;

	auto it = rel.begin();
	if (it != rel.end() && *it == L"..")
		return false;

	return true;
}
