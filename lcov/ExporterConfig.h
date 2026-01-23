#pragma once

#include <filesystem>
#include <optional>


namespace YAML {
	class Node;
}

class ExporterConfigLog {
public:
	enum class MsgLevel {
		Info,
		Error
	};

	void AddMsg(MsgLevel level, const std::string& message);
	void LogMessages();
	bool HasErrors() const;

	std::vector<std::pair<MsgLevel, std::string>> messages;
};

class ExporterConfig {
public:
	ExporterConfigLog Log;
	/**
	 * Walks up from the given directory to find the ".covlcov" configuration file.
	 *
	 * @param startDir The directory to start searching for ".covlcov" from (inclusive)
	 */
	explicit ExporterConfig(std::filesystem::path startDir);

	bool IsLoaded() const noexcept;
	std::optional<std::filesystem::path> ConfigPath() const;

	// If true, only include files under baseDir and emit SF paths relative to baseDir.
	bool IncludeByBaseDir() const noexcept;

	// Helper used by exporter when iterating files.
	bool ShouldIncludeInReportByPath(const std::filesystem::path& path) const;
	// Resolved base directory (empty if baseDir not set or config not loaded)
	std::filesystem::path GetResolvedBaseDir() const;

	// Returns the path that should be written to LCOV "SF:" line.
	// If IncludeByBaseDir() is true, returns path relative to resolved baseDir (when possible).
	std::filesystem::path MakeSFPath(const std::filesystem::path& path) const;
	void LoadFromYaml(YAML::Node root);
	void LoadFromFile(const std::filesystem::path& covlcovPath);

private:
	static std::optional<std::filesystem::path> FindCovLcovUpwards(const std::filesystem::path& startDir);

private:
	bool loaded_{false};
	std::filesystem::path covlcovPath_;

	std::optional<std::filesystem::path> baseDir_; // raw from yaml
	bool includeByBaseDir_ = false;
	bool isYamlValid = false;
};
