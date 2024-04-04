#pragma once

#include "Scene.h"
#include "Scene_GalaxyImpact.h"

class Scene_Menu : public Scene
{
private:
	std::vector<std::string>	m_menuStrings;
	sf::Text					m_menuText;
	std::vector<std::string>	m_levelPaths;
	int							m_menuIndex{0};
	std::string					m_title;
	

	void init();
	void onEnd() override;
public:
	
	friend class Scene_GalaxyImpact;

	Scene_Menu(GameEngine* gameEngine);

	void update(sf::Time dt) override;
	void loadMenuConfig(const std::string& path);

	void sRender() override;
	void sDoAction(const Command& action) override;
	

};

