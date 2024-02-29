#pragma once


#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"

struct LevelConfig {
    float       scrollSpeed{ 100.f };
    float       playerSpeed{ 200.f };
    float       enemySpeed{ 200.f };
    float       bulletSpeed{ 400.f };
    float       missileSpeed{ 150.f };
    sf::Time    fireInterfal{ sf::seconds(5) };

    std::map<std::string,
    std::vector<std::pair<float, sf::Time>>> directions;
};



class Scene_GalaxyImpact : public Scene {
private:
    sPtrEntt        m_player{ nullptr };
    sf::View        m_worldView;
    sf::FloatRect   m_worldBounds;
    LevelConfig     m_config;
    enum Enemies
    {
        Rusher,
        Assault,
        Predator

    };

    std::map<Enemies, std::string> enemyNames = {
        {Enemies::Rusher, "rusher"},
        {Enemies::Assault, "assault"},
        {Enemies::Predator, "predator"}
    };



    bool			m_drawTextures{ true };
    bool			m_drawAABB{ false };
    bool			m_drawGrid{ false };
    int             m_score{ 0 };
    int             m_lives{ 3 };
    //int             lilyCount{ 0 };


    //systems
    void            sMovement(sf::Time dt);
    void            sCollisions();
    void            sUpdate(sf::Time dt);
    void            sAnimation(sf::Time dt);

    void	        onEnd() override;


    // helper functions
    void            playerMovement();
    void            adjustPlayerPosition();
    void            checkPlayerState();
    void	        registerActions();
    void            spawnPlayer();
    void            spawnEnemy();
    


    void            init();
    void            loadLevel(const std::string& path);
    sf::FloatRect   getViewBounds();

public:

    Scene_GalaxyImpact(GameEngine* gameEngine, const std::string& levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

};



#endif //BREAKOUT_SCENE_BREAKOUT_H
