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
bool isMainMenu;
bool isInit;
std::string state;

void Scene_Menu:: init()
{
	isMainMenu = true;
	state = "mmenu";
	isInit = false;
    registerAction(sf::Keyboard::W,			"UP");
    registerAction(sf::Keyboard::Up,		"UP");
    registerAction(sf::Keyboard::S,			"DOWN");
    registerAction(sf::Keyboard::Down,	 	"DOWN");
	registerAction(sf::Keyboard::Enter,		"CONFIRM");
	registerAction(sf::Keyboard::Escape,	"QUIT");
	registerAction(sf::Keyboard::C,		"Controls");
	registerAction(sf::Keyboard::BackSpace, "BACK");
	

	m_title = "Galaxy Impact";
	//m_menuStrings.push_back("Start");
	//m_menuStrings.push_back("Quit");

	m_levelPaths.push_back("../assets/level1.txt");
	m_levelPaths.push_back("../assets/level2.txt");
	//m_levelPaths.push_back("../assets/level1.txt");*/
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

	// draw Play Text wheb isInit is false;
	if (!isInit and isMainMenu) {
		static sf::Text playT("Play Text ", Assets::getInstance().getFont("main"), 54);
		std::string playStr = "Press Enter to Play";
		playT.setString(playStr);
		playT.setPosition(view.getCenter().x / 2, view.getCenter().y);
		m_game->window().draw(playT);
	}
	// draw controls section
	else if (!isMainMenu and state == "controls") {
		sf::RectangleShape backgroundRect;

		// Draw the background first
		static sf::Text controlT("Controll Text ", Assets::getInstance().getFont("main"), 36);
		controlT.setFillColor(selectedColor);
		std::string controlStr = "W/UP: Move Up\nA/Left: Move Left\nS/Down: Move Down\nD/Right: Move Right\nEnter: Fire\nSpace: Launch Missile\nEsc: Enter Main Menu\n";
		controlT.setString(controlStr);
		controlT.setPosition(view.getCenter().x, view.getCenter().y);

		// Align text to center horizontally
		controlT.setOrigin(controlT.getLocalBounds().width / 2, controlT.getLocalBounds().height / 2);
		
		// Set up background for text
		backgroundRect.setSize(sf::Vector2f(controlT.getLocalBounds().width*1.5, controlT.getLocalBounds().height + 20)); // Adjust size as needed
		backgroundRect.setFillColor(sf::Color(50, 50, 50, 100)); // Set background color
		backgroundRect.setPosition(controlT.getLocalBounds().width / 2, controlT.getLocalBounds().height / 2); // Position it behind the text

		sf::Text footer("Back: <--    QUIT: ESC",
			Assets::getInstance().getFont("main"), 20);
		footer.setFillColor(selectedColor);
		footer.setPosition(32, 580);
		m_game->window().draw(backgroundRect);
		m_game->window().draw(controlT);
		m_game->window().draw(footer);

	}
	// draw menu after game was launched
	else if(isInit) {
		
		for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
		{
			m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
			m_menuText.setPosition(32, 32 + (i + 1) * 96);
			m_menuText.setString(m_menuStrings.at(i));
			m_game->window().draw(m_menuText);
		}
		
	}
   

   /*UP: W    DOWN: S*/
	sf::Text footer("Control: C    QUIT: ESC", 
		Assets::getInstance().getFont("main"), 20);
	footer.setFillColor(selectedColor);
	footer.setPosition(32, 580);

	//m_game->window().clear(backgroundColor);

	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(view.getCenter().x/2+50.f, 10);
	m_game->window().draw(m_menuText);
	if(isMainMenu)m_game->window().draw(footer);

	

	
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
		
		else if (action.name() == "CONFIRM" and m_menuIndex == 0 and isInit) {

			
			m_game->restartScene("PLAY", std::make_shared<Scene_GalaxyImpact>(m_game, m_levelPaths[m_game->levelIndex]));
			MusicPlayer::getInstance().play("levelTheme");
		}
		else if (action.name() == "CONFIRM" and m_menuIndex == 1 and isInit) {


			m_game->changeScene("PLAY", std::make_shared<Scene_GalaxyImpact>(m_game, m_levelPaths[m_game->levelIndex]));
			MusicPlayer::getInstance().play("levelTheme");
		}
		else if (action.name() == "Controls") {
			isMainMenu = false;
			state = "controls";
		}
		else if (action.name() == "BACK" and !isMainMenu) {
			
			isMainMenu = true;
			state = "mmenu";
		}
		else if (action.name() == "CONFIRM")
		{
			m_menuStrings.push_back("Restart");
			m_menuStrings.push_back("Resume");

			isInit = true;
			m_game->changeScene("PLAY", std::make_shared<Scene_GalaxyImpact>(m_game, m_levelPaths[m_game->levelIndex]));
			MusicPlayer::getInstance().play("levelTheme");


		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
