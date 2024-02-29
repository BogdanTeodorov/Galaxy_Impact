#include "Scene_GalaxyImpact.h"

//
// Created by David Burchill on 2023-09-27.
//

#include <fstream>
#include <iostream>

#include "Scene_GalaxyImpact.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include <random>
#include <set>
#include<algorithm>

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
}



Scene_GalaxyImpact::Scene_GalaxyImpact(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_worldView(gameEngine->window().getDefaultView()) {
    loadLevel(levelPath);
    registerActions();


    //MusicPlayer::getInstance().play("gameTheme");
    //MusicPlayer::getInstance().setVolume(50);

    init();

}




void Scene_GalaxyImpact::init() {
   
    spawnPlayer();
    spawnEnemy();
}

void Scene_GalaxyImpact::sMovement(sf::Time dt) {
    playerMovement();

    // move all objects
    for (auto e : m_entityManager.getEntities()) {

        if (e->hasComponent<CTransform>()) {
            auto& tfm = e->getComponent<CTransform>();

            tfm.pos += tfm.vel * dt.asSeconds();
            tfm.angle += tfm.angVel * dt.asSeconds();
        }
    }
}


void Scene_GalaxyImpact::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
    registerAction(sf::Keyboard::Q, "QUIT");
    registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");

    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Down, "DOWN");
}


void Scene_GalaxyImpact::onEnd() {
    m_game->changeScene("MENU", nullptr, false);
}

void Scene_GalaxyImpact::playerMovement() {
    // no movement if player is dead
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    /*auto& dir = m_player->getComponent<CInput>().dir;*/
    auto& vel = m_player->getComponent<CTransform>().vel;
    auto& input = m_player->getComponent<CInput>();
    float pSpeed = 200.f;

    sf::Vector2f pv{ 0.f, 0.f };
    


    if (input.up) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.y -= 1.f;
    }
    if (input.down) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.y += 1.f;
    }

    if (input.left) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.x -=1.f;
    }

    if (input.right) {
        m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.x += 1.f;
    }

    pv = normalize(pv);
    vel = pv * pSpeed;

    //if (dir != 0) {
    //    //SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
    //    dir = 0;
    //}

    
}


void Scene_GalaxyImpact::sRender() {
    m_game->window().setView(m_worldView);

    // draw bkg first
    for (auto e : m_entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto& sprite = e->getComponent<CSprite>().sprite;
            m_game->window().draw(sprite);
        }
    }

    // draw Score
    /*static sf::Text scoreT("Score ", Assets::getInstance().getFont("main"), 20);
    std::string scoreStr = "Score " + std::to_string(m_score);
    scoreT.setString(scoreStr);
    scoreT.setFillColor(sf::Color::Cyan);
    scoreT.setPosition(0.f, 0.f);
    m_game->window().draw(scoreT);*/

    //// draw Lives
    //static sf::Text livesT("Lives ", Assets::getInstance().getFont("Arcade"), 20);
    //std::string livesStr = "Lives " + std::to_string(m_lives);
    //livesT.setString(livesStr);
    //livesT.setFillColor(sf::Color::Red);
    //livesT.setPosition(400.f, 0.f);
    //m_game->window().draw(livesT);

    // draw win Screen

   /* if (lilyCount == 5) {

        auto& pos = m_worldView.getSize();
        static sf::Text winnerT("Congratulation Level Completed", Assets::getInstance().getFont("Arcade"), 35);
        std::string  winnerStr = "Congratulation!\n Level Completed";
        winnerT.setString(winnerStr);
        winnerT.setFillColor(sf::Color::Green);
        winnerT.setPosition(pos.x / 4, pos.y / 2);
        m_game->window().draw(winnerT);
        m_isPaused = true;



    }

    if (m_lives == 0) {
        auto& pos = m_worldView.getSize();
        static sf::Text looserT("Loose", Assets::getInstance().getFont("Arcade"), 35);
        std::string  looserStr = "Sorry you loose\n Press Q to restart\n";
        looserT.setString(looserStr);
        looserT.setFillColor(sf::Color::Red);
        looserT.setPosition(pos.x / 4, pos.y / 2);
        m_game->window().draw(looserT);
        m_isPaused = true;
        MusicPlayer::getInstance().stop();


    }*/

    for (auto& e : m_entityManager.getEntities()) {
        if (!e->hasComponent<CAnimation>())
            continue;

        // Draw Sprite
        auto& anim = e->getComponent<CAnimation>().animation;
        auto& tfm = e->getComponent<CTransform>();
        anim.getSprite().setPosition(tfm.pos);
        anim.getSprite().setRotation(tfm.angle);
        m_game->window().draw(anim.getSprite());

        if (m_drawAABB) {
            if (e->hasComponent<CBoundingBox>()) {
                auto box = e->getComponent<CBoundingBox>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
                centerOrigin(rect);
                rect.setPosition(e->getComponent<CTransform>().pos);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color{ 0, 255, 0 });
                rect.setOutlineThickness(2.f);
                m_game->window().draw(rect);
            }
        }
    }
}


void Scene_GalaxyImpact::update(sf::Time dt) {
    sUpdate(dt);
}

void Scene_GalaxyImpact::sDoAction(const Command& action) {
    // On Key Press
    if (action.type() == "START") {
        if (action.name() == "PAUSE") { setPaused(!m_isPaused); }
        else if (action.name() == "QUIT") { m_game->quitLevel(); }
        else if (action.name() == "BACK") { m_game->backLevel(); }

        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawAABB = !m_drawAABB; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }

        // Player control
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = true; }
        else if (action.name() == "UP") {m_player->getComponent<CInput>().up = true;}
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = true; }
    }
    // on Key Release
    // the frog can only go in one direction at a time, no angles
    // use a bitset and exclusive setting.
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = false; }
    }
}


void Scene_GalaxyImpact::spawnPlayer() {
    //spawn position
    auto pos = m_worldView.getSize();
    pos.x = 0.f;
    pos.y = pos.y / 2;
    
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(pos);
    m_player->addComponent<CBoundingBox>(sf::Vector2f(60.f, 36.f));
    m_player->addComponent<CState>().state = "grounded";
    m_player->addComponent<CInput>();
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
}

void Scene_GalaxyImpact::spawnEnemy()
{
   float enemySpeed = 0.f;
   auto view = m_worldView.getSize();
   auto pos = sf::Vector2f(view.x + 100.f, view.y / 2);
   sf::Vector2f eVel(1.f, 0.f);
   // add all tags of enemy ships;
   for (const auto& enemyPair : enemyNames) {
       const Enemies enemyType = enemyPair.first;   // Get the enum value
       const std::string& enemyName = enemyPair.second; // Get the enemy name

        auto enemy = m_entityManager.addEntity(enemyName); // Add entity using the enemy name

      //use enemyType if needed to differentiate between enemy types
       if (enemyType == Enemies::Rusher) {
           // Do something specific for the Rusher enemy type
           enemySpeed = -200;
           eVel = normalize(eVel);
           eVel = eVel * enemySpeed;
           enemy->addComponent<CBoundingBox>(sf::Vector2f(50.f, 45.f));
           enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Rusher]));
           enemy->addComponent<CTransform>(pos, eVel);

       }
   }
}


sf::FloatRect Scene_GalaxyImpact::getViewBounds() {
    return sf::FloatRect();
}

void Scene_GalaxyImpact::sCollisions() {
    adjustPlayerPosition();


}


void Scene_GalaxyImpact::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    m_entityManager.update();
    m_worldView.move( m_config.scrollSpeed * dt.asSeconds() * 1, 0.f);


    if (m_isPaused)
        return;

    sAnimation(dt);
    sMovement(dt);
    sCollisions();
    adjustPlayerPosition();
    sRender();
}


void Scene_GalaxyImpact::sAnimation(sf::Time dt) {
    auto list = m_entityManager.getEntities();
    auto state = m_player->getComponent<CState>().state;
    auto playerAnim = m_player->getComponent<CAnimation>();


    for (auto e : m_entityManager.getEntities()) {


        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& anim = e->getComponent<CAnimation>();
            anim.animation.update(dt);
            if (playerAnim.animation.hasEnded() and state == "dead") {
                m_player->destroy();
                spawnPlayer();

            }

            for (auto& t : m_entityManager.getEntities("3turtles")) {

                auto tanim = t->getComponent<CAnimation>();
                if (tanim.animation.hasEnded()) {
                    t->removeComponent<CBoundingBox>();
                }
                else if (!tanim.animation.hasEnded()) {
                    t->addComponent<CBoundingBox>(sf::Vector2f(100.f, 15.f));
                }


            }

            for (auto& t : m_entityManager.getEntities("2turtles")) {

                auto tanim = t->getComponent<CAnimation>();
                if (tanim.animation.hasEnded()) {
                    t->removeComponent<CBoundingBox>();
                }
                else if (!tanim.animation.hasEnded()) {
                    t->addComponent<CBoundingBox>(sf::Vector2f(66.f, 15.f));
                }


            }


        }


    }





}


void Scene_GalaxyImpact::adjustPlayerPosition() {
    auto center = m_worldView.getCenter();
    sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


    auto left = center.x - viewHalfSize.x;
    auto right = center.x + viewHalfSize.x;
    auto top = center.y - viewHalfSize.y;
    auto bot = center.y + viewHalfSize.y;

    auto& player_pos = m_player->getComponent<CTransform>().pos;
    auto halfSize = sf::Vector2f{ m_player->getComponent<CBoundingBox>().halfSize.x, m_player->getComponent<CBoundingBox>().halfSize.y};
    // keep player in bounds
    player_pos.x = std::max(player_pos.x, left + halfSize.x);
    player_pos.x = std::min(player_pos.x, right - halfSize.x);
    player_pos.y = std::max(player_pos.y, top + halfSize.y);
    player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void Scene_GalaxyImpact::checkPlayerState() {

}

void Scene_GalaxyImpact::loadLevel(const std::string& path) {
    std::ifstream config(path);
    if (config.fail()) {
        std::cerr << "Open file " << path << " failed\n";
        config.close();
        exit(1);
    }

    std::string token{ "" };
    config >> token;
    while (!config.eof()) {
        if (token == "Bkg") {
            std::string name;
            sf::Vector2f pos;
            config >> name >> pos.x >> pos.y;
            auto e = m_entityManager.addEntity("bkg");

            // for background, no textureRect its just the whole texture
            // and no center origin, position by top left corner
            // stationary so no CTransfrom required.
            auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
            sprite.setOrigin(0.f, 0.f);
            sprite.setPosition(pos);
        }
        if (token == "ScrollSpeed") {
            config >> m_config.scrollSpeed;
        }
        else if (token[0] == '#') {
            std::cout << token;
        }

        config >> token;
    }

    config.close();
}

