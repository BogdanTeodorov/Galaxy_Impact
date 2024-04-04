#include "Scene_Menu.h"
#include "Scene_GalaxyImpact.h"
#include <memory>
#include <fstream>
#include "MusicPlayer.h"

void Scene_Menu::onEnd()
{
	m_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}
bool isTransperent{ false };


void Scene_Menu:: init()
{
    registerAction(sf::Keyboard::W,			"UP");
    registerAction(sf::Keyboard::Up,		"UP");
    registerAction(sf::Keyboard::S,			"DOWN");
    registerAction(sf::Keyboard::Down,	 	"DOWN");
	registerAction(sf::Keyboard::Enter,		"PLAY");
	registerAction(sf::Keyboard::Escape,	"QUIT");
	

	m_title = "Galaxy Impact";
	//m_menuStrings.push_back("Start");
	//m_menuStrings.push_back("Quit");

	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level2.txt");
	m_levelPaths.push_back("../assets/level1.txt");
	loadMenuConfig("../assets/level1.txt");
	m_menuText.setFont(Assets::getInstance().getFont("main"));

	const size_t CHAR_SIZE{ 54 };
	m_menuText.setCharacterSize(CHAR_SIZE);
	
	MusicPlayer::getInstance().play("menuTheme");
	MusicPlayer::getInstance().setVolume(50);

}

void Scene_Menu::update(sf::Time dt)
{
	m_entityManager.update();
}

void Scene_Menu::loadMenuConfig(const std::string& path)
{
	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Mbkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = m_entityManager.addEntity("mbkg");
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token[0] == '#') {
			std::cout << token;
		}

		config >> token;
	}

	config.close();
}


void Scene_Menu::sRender()
{
	 
	sf::View view = m_game->window().getView();
	view.setCenter(m_game->window().getSize().x / 2.f, m_game->window().getSize().y / 2.f);
	m_game->window().setView(view);

	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);


	// draw bkg first
	for (auto e : m_entityManager.getEntities("mbkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			m_game->window().draw(sprite);
		}
	}

	// draw Play Text
   static sf::Text playT("Play Text ", Assets::getInstance().getFont("main"), 54);
   std::string playStr = "Press Enter to Play";
   playT.setString(playStr);
   if (isTransperent) {
	   playT.setFillColor(sf::Color::White);
	   
   }
   
   playT.setPosition(view.getCenter().x/2, view.getCenter().y);
   m_game->window().draw(playT);
   /*UP: W    DOWN: S*/
	sf::Text footer("PLAY: Enter    QUIT: ESC", 
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 580);

	//m_game->window().clear(backgroundColor);

	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(view.getCenter().x/2+50.f, 10);
	m_game->window().draw(m_menuText);
	m_game->window().draw(footer);

	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(32, 32 + (i+1) * 96);
		m_menuText.setString(m_menuStrings.at(i));
		m_game->window().draw(m_menuText);
	} 

	
	//m_game->window().display();

}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		} 
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			m_game->changeScene("PLAY", std::make_shared<Scene_GalaxyImpact>(m_game, m_levelPaths[m_menuIndex]));
			MusicPlayer::getInstance().play("levelTheme");

			//Scene_GalaxyImpact gi(GameEngine * gameEngine, const std::string & levelPath);

		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
