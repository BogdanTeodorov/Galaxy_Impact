#pragma once


#ifndef BREAKOUT_SCENE_BREAKOUT_H
#define BREAKOUT_SCENE_BREAKOUT_H

#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include "EntityManager.h"
#include "Entity.h"
#include "Scene.h"
#include "GameEngine.h"
#include "Scene_Menu.h"

struct LevelConfig {
    float       scrollSpeed{ 100.f };
    float       playerSpeed{ 200.f };
    float       enemySpeed{ 200.f };
    float       bulletSpeed{ 400.f };
    float       missileSpeed{ 150.f };
    sf::Time    fireInterval{ sf::seconds(1) };
    std::vector<std::string> levelPaths;

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
    void            sGunUpdate(sf::Time dt);
    void            sGuideMissiles(sf::Time dt);
    void	        onEnd() override;
    void            sDestroyOutsideBattleField();


    // helper functions
    void            playerMovement();
    void            adjustPlayerPosition();
    void            adjustEnemyPosition();
    void            checkPlayerState();
    void            checkShipCollisions();
    void            checkBulletCollison();
    void            checkLaserCollision();
    void            checkMissileCollision();
    void	        registerActions();
    void            spawnPlayer();
    void            spawnEnemy();
    void            spawnLaser(sf::Vector2f pos);
    void            fireBullets();
    void            fireMissile();
    sf::Vector2f    findClosestEnemy(sf::Vector2f mPos);
    void            spawnBullet(sf::Vector2f pos, bool isEnemy);
    void            init();
    void            loadLevel(const std::string& path);
    void            assaultMovement();
    void            destroyBulletsOutsideBattlefield();
    void            destroyEnemiesOutsideBattleField();
    void            destroyMissilesOutsideBattleField();
public:

    Scene_GalaxyImpact(GameEngine* gameEngine, const std::string& levelPath);

    void		  update(sf::Time dt) override;
    void		  sDoAction(const Command& command) override;
    void		  sRender() override;

};



#endif //BREAKOUT_SCENE_BREAKOUT_H

