#pragma once

#include <string>
#include <vector>

// Simple key=value configuration system for saving/loading all cheat settings.
// Configs are stored as plain-text .cfg files in the lynette config directory
// (next to the game executable, under "lynette_configs").
class CConfig
{
public:
	// Initialise the config directory. Safe to call multiple times.
	void Init();

	// Persist the current Vars state to <name>.cfg. Returns true on success.
	bool Save(const std::string& name);

	// Load <name>.cfg into the current Vars state. Returns true on success.
	bool Load(const std::string& name);

	// Refresh the cached list of available config files on disk.
	void RefreshList();

	// Cached list of config names (without extension) found on disk.
	const std::vector<std::string>& GetConfigs() const { return m_Configs; }

	// Absolute path to the directory configs are stored in.
	const std::string& GetDirectory() const { return m_Directory; }

private:
	std::string              m_Directory;
	std::vector<std::string> m_Configs;
};

namespace G { inline CConfig Config; }
