#pragma once

#include <vector>
#include <string>

typedef std::string Level;

class LevelManager {
public:
	const std::vector<Level>& GetLevels() const {
		return {};
	};
	const int GetCurrentLevel() const {
		return 0;
	};
	bool Initialize(const std::vector<std::string>& levels) {
		return true;
	};
	bool Initialize() {
		return true;
	};

protected:
	std::vector<Level> m_Levels;
	int m_CurrentLevel;
};