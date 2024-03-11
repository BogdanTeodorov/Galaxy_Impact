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

std::uniform_real_distribution<float> spawnIntervalDistribution(2.0f, 4.0f);
sf::Time spawnInterval;
sf::Time spawnTimer;
sf::Vector2f enemyPrevPos;
int bugedEnemiesCount{ 0 };



Scene_GalaxyImpact::Scene_GalaxyImpact(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_worldView(gameEngine->window().getDefaultView()) 
{
    loadLevel(levelPath);
    registerActions();
    


    //MusicPlayer::getInstance().play("gameTheme");
    //MusicPlayer::getInstance().setVolume(50);

    init();

}




void Scene_GalaxyImpact::init() {
   
    enemyPrevPos = sf::Vector2f(0.f, 0.f);
    spawnPlayer();
    // Randomly determine the time interval between spawns
    spawnInterval = sf::seconds(spawnIntervalDistribution(rng));
    spawnTimer = sf::seconds(0.f);
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
    registerAction(sf::Keyboard::Enter, "FIRE");
    registerAction(sf::Keyboard::Space, "LAUNCH");
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
        //anim.getSprite().setRotation(tfm.angle);
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
void Scene_GalaxyImpact::sGunUpdate(sf::Time dt)
{
    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CGun>()) {
            bool isEnemy = (e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Predator]);
            auto& gun = e->getComponent<CGun>();
            gun.countdown -= dt;
            auto& viewBounds = m_worldView.getCenter();
            
            

            if (isEnemy && viewBounds.x*2 -50.f >= e->getComponent<CTransform>().pos.x) // enemy is  firing when in the view
                gun.isFiring = true;

            //
            // when firing
            //
            if (gun.isFiring && gun.countdown < sf::Time::Zero) {
                gun.isFiring = false;
                gun.countdown = m_config.fireInterfal / (1.f + gun.fireRate);

                auto pos = e->getComponent<CTransform>().pos;
            
                spawnBullet(pos /*+ sf::Vector2f(0.f, isEnemy ? 35.f : -35.f)*/, isEnemy);
               
                    

                
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
        //fire weapons
        else if (action.name() == "FIRE") { fireBullets(); }
        else if (action.name() == "LAUNCH") { fireMissile(); }
    }
    
    // on Key Release
    // the frog can only go in one direction at a time, no angles
    // use a bitset and exclusive setting.
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = false; }
       /* else if (action.name() == "FIRE") { m_player->getComponent<CGun>().isFiring = false; }*/
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
    m_player->addComponent<CGun>();
    m_player->addComponent<CMissiles>();
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
}

void Scene_GalaxyImpact::fireBullets()
{
    if (m_player->hasComponent<CGun>()) {
        m_player->getComponent<CGun>().isFiring = true;
    }
}

void Scene_GalaxyImpact::spawnBullet(sf::Vector2f pos, bool isEnemy)
{
    float speed;

    if (isEnemy) {
        speed = -m_config.bulletSpeed;
       /* SoundPlayer::getInstance().play("EnemyGunfire", pos);*/
    }
    else {
        speed = m_config.bulletSpeed;
        /*SoundPlayer::getInstance().play("AlliedGunfire", pos);*/
    }
    auto bullet = m_entityManager.addEntity(isEnemy ? "EnemyBullet" : "PlayerBullet");
    auto bb = isEnemy ? bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation("ebullet-v1")).animation.getBB():
    bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation("bullet")).animation.getBB();
    if (!isEnemy) {
        bullet->getComponent<CAnimation>().animation.getSprite().setRotation(-90.f);
    }
    bullet->addComponent<CBoundingBox>(bb);
    bullet->addComponent<CTransform>(pos, sf::Vector2f(speed, 0.f));


}

sf::Vector2f Scene_GalaxyImpact::findClosestEnemy(sf::Vector2f mPos)
{
    float closest = std::numeric_limits<float>::max();
    bool isEnemy;
    sf::Vector2f posClosest{ 0.f, 0.f };
    for (auto e : m_entityManager.getEntities()) {
        isEnemy = (e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Predator] || e->getTag() == enemyNames[Rusher]);
        if (isEnemy && e->getComponent<CTransform>().has) {
            auto ePos = e->getComponent<CTransform>().pos;
            float distToEnemy = dist(mPos, ePos);
            if (distToEnemy < closest) {
                closest = distToEnemy;
                posClosest = ePos;
            }
        }
    }
    return posClosest;
}

void Scene_GalaxyImpact::sGuideMissiles(sf::Time dt)
{
    const float approachRate = 500.f;
    for (auto e : m_entityManager.getEntities("missile")) {
        if (e->getComponent<CTransform>().has) {
            auto& tfm = e->getComponent<CTransform>();
            auto ePos = findClosestEnemy(tfm.pos);

            auto targetDir = normalize(ePos - tfm.pos);
            tfm.vel = m_config.missileSpeed * normalize(approachRate * dt.asSeconds() * targetDir + tfm.vel);
            tfm.angle = bearing(tfm.vel) + 90;
        }
    }

}

void Scene_GalaxyImpact::fireMissile()
{
    if (m_player->hasComponent<CMissiles>()) {
        size_t& ammo = m_player->getComponent<CMissiles>().missileCount;
        if (ammo > 0) {
            ammo -= 1;
            auto pos = m_player->getComponent<CTransform>().pos;

            auto missile = m_entityManager.addEntity("missile");
            missile->addComponent<CTransform>(
                pos + sf::Vector2f(0.f, -60.f),
                sf::Vector2f(m_config.missileSpeed, 0.f));
            auto bb = missile->addComponent<CAnimation>(Assets::getInstance()
                .getAnimation("missile")).animation.getBB();
            missile->addComponent<CBoundingBox>(bb);
            auto& mRotation = missile->getComponent<CAnimation>().animation.getSprite();
            mRotation.setRotation(-90.f);
           // SoundPlayer::getInstance().play("LaunchMissile", pos);
        }
    }
}

void Scene_GalaxyImpact::spawnEnemy()
{
   float enemySpeed = 0.f;
   auto& viewH = m_worldView.getCenter();
   auto& viewV = m_worldView.getSize();

   // Random number generator for enemy type, quantity, and spawn intervals
   std::uniform_int_distribution<int> enemyTypeDistribution(0, enemyNames.size() - 1);
   std::uniform_int_distribution<int> quantityDistribution(1, 4);
   std::uniform_real_distribution<float> enemyVerticalSpawnRange(1.0f, 4.0f);


   // basic characterstics
   sf::Vector2f eVel(1.f, 0.f);
   int numEnemies = quantityDistribution(rng);

  
   for (int i = 0; i < numEnemies; i++) {
      Enemies enemyType = static_cast<Enemies>(enemyTypeDistribution(rng));
      auto pos = sf::Vector2f((viewH.x*2) + 100.f, viewV.y / enemyVerticalSpawnRange(rng));
      const std::string& enemyName = enemyNames[enemyType];

      auto enemy = m_entityManager.addEntity(enemyName); // Add entity 
      if (enemyPrevPos.y == 0 || (enemyPrevPos.y - pos.y) > std::abs(40.f)) {
          if (enemyType == Enemies::Rusher) {
              // Customize Rusher enemy
              pos.x = (viewH.x * 2);
              pos.y = m_player->getComponent<CTransform>().pos.y;
              enemySpeed = -800;
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Rusher]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, eVel);


          }
          else if (enemyType == Enemies::Assault) {
              // Customize Assault enemy
              enemySpeed = -200;
              // Add additional customization if needed
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              
              enemy->addComponent<CGun>();
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Predator]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CGun>();
              enemy->addComponent<CTransform>(pos, eVel);
              auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
              eRotation.setRotation(90.f);
          }
          else if (enemyType == Enemies::Predator) {
              // Customize Predator enemy
              // Add additional customization if needed
              enemySpeed = -200;
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Assault]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CGun>();
              enemy->addComponent<CTransform>(pos, eVel);
          }
      }
      else {

          if (enemyType == Enemies::Rusher) {
              // Customize Rusher enemy
              pos.x = (viewH.x * 2) + 50.f;
              pos.y = m_player->getComponent<CTransform>().pos.y;
              enemySpeed = -800;
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Rusher]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, eVel);


          }
          else if (enemyType == Enemies::Assault) {
              // Customize Assault enemy
              enemySpeed = -200;
              pos.y += 200.f;
              // Add additional customization if needed
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CGun>();
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Predator]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, eVel);

              auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
              eRotation.setRotation(90.f);
          }
          else if (enemyType == Enemies::Predator) {
              // Customize Predator enemy
              // Add additional customization if needed
              enemySpeed = -200;
              pos.y += 200.f;
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CGun>();
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Assault]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, eVel);
          }

      }
      enemyPrevPos.y = pos.y;
      
     

   }
   // add all tags of enemy ships;
   //for (const auto& enemyPair : enemyNames) {
   //    const Enemies enemyType = enemyPair.first;   // Get the enum value
   //    const std::string& enemyName = enemyPair.second; // Get the enemy name

   //     auto enemy = m_entityManager.addEntity(enemyName); // Add entity using the enemy name

   //   //////use enemyType if needed to differentiate between enemy types
   //   // if (enemyType == Enemies::Rusher) {
   //   //     // Do something specific for the Rusher enemy type
   //   //     enemySpeed = -500;
   //   //     eVel = normalize(eVel);
   //   //     eVel = eVel * enemySpeed;
   //   //     enemy->addComponent<CBoundingBox>(sf::Vector2f(50.f, 45.f));
   //   //     enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Rusher]));
   //   //     enemy->addComponent<CTransform>(pos, eVel);

   //   // }
   //   // if (enemyType == Enemies::Predator) {
   //   //     // Do something specific for the Rusher enemy type
   //   //     enemySpeed = -200;
   //   //     eVel = normalize(eVel);
   //   //     eVel = eVel * enemySpeed;
   //   //     pos = sf::Vector2f(view.x + 100.f, view.y / 3);
   //   //     enemy->addComponent<CBoundingBox>(sf::Vector2f(50.f, 45.f));
   //   //     enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Predator]));
   //   //     enemy->addComponent<CTransform>(pos, eVel);
   //   //     auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
   //   //     eRotation.setRotation(90.f);
   //   //    

   //   // }

   //    //if (enemyType == Enemies::Assault) {
   //    //    // Do something specific for the Assault enemy type
   //    //    enemySpeed = -200;
   //    //    eVel = normalize(eVel);
   //    //    eVel = eVel * enemySpeed;
   //    //    pos = sf::Vector2f(view.x + 100.f, view.y / 3);
   //    //    enemy->addComponent<CBoundingBox>(sf::Vector2f(37.f, 38.f));
   //    //    enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Assault]));
   //    //    enemy->addComponent<CTransform>(pos, eVel);
   //    //    auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
   //    //    


   //    //}

   //}
}


sf::FloatRect Scene_GalaxyImpact::getViewBounds() {
    return sf::FloatRect();
}

void Scene_GalaxyImpact::sCollisions() {
    adjustPlayerPosition();
    checkShipCollisions();
    checkBulletCollison();
    checkMissileCollision();

}

void Scene_GalaxyImpact::checkShipCollisions()
{
    //player vs rusher
    for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {

        auto overlap = Physics::getOverlap(m_player, e);

        if (overlap.x > 0 and overlap.y > 0) {
            m_player->destroy();
            e->destroy();
            spawnPlayer();
        }

    }
    //player vs assault
    for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {

        auto overlap = Physics::getOverlap(m_player, e);

        if (overlap.x > 0 and overlap.y > 0) {
            m_player->destroy();
            e->destroy();
            spawnPlayer();
        }

    }
    // player vs Predator
    for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {

        auto overlap = Physics::getOverlap(m_player, e);

        if (overlap.x > 0 and overlap.y > 0) {
            m_player->destroy();
            e->destroy();
            spawnPlayer();
        }

    }
}

void Scene_GalaxyImpact::checkBulletCollison()
{
    //Player bullets vs enemies
    for (auto bullet : m_entityManager.getEntities("PlayerBullet")) {
        for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {

            auto overlap = Physics::getOverlap(bullet, e);

            if (overlap.x > 0 and overlap.y > 0) {
               
               bullet->destroy();
               e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
               e->destroy();
            }

        }

        for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {

            auto overlap = Physics::getOverlap(bullet, e);

            if (overlap.x > 0 and overlap.y > 0) {

                bullet->destroy();
                e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                e->destroy();
            }

        }

        for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {

            auto overlap = Physics::getOverlap(bullet, e);

            if (overlap.x > 0 and overlap.y > 0) {

                bullet->destroy();
                e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                e->destroy();
            }

        }


    }

    // Enemy bullets vs player
    for (auto eBullet : m_entityManager.getEntities("EnemyBullet")) {
        auto overlap = Physics::getOverlap(m_player, eBullet);
        
        if (overlap.x > 0 and overlap.y > 0) {

            m_player->destroy();
            eBullet->destroy();
            spawnPlayer();
        }
    }




}

void Scene_GalaxyImpact::checkMissileCollision()
{
    for (auto missile : m_entityManager.getEntities("missile")) {

        // missile vs rusher
        for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {
            auto overlap = Physics::getOverlap(missile, e);
            if (overlap.x > 0 and overlap.y > 0) {
                missile->destroy();
                e->destroy();
            }
        }

        // missile vs predator
        for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {
            auto overlap = Physics::getOverlap(missile, e);
            if (overlap.x > 0 and overlap.y > 0) {
                missile->destroy();
                e->destroy();
            }
        }

        // missile vs assault
        for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {
            auto overlap = Physics::getOverlap(missile, e);
            if (overlap.x > 0 and overlap.y > 0) {
                missile->destroy();
                e->destroy();
            }
        }
    }
}

void Scene_GalaxyImpact::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    m_entityManager.update();
    m_worldView.move( m_config.scrollSpeed * dt.asSeconds() * 1, 0.f);
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval) {
        spawnTimer = sf::seconds(0.f);
        spawnEnemy();
        spawnInterval = sf::seconds(spawnIntervalDistribution(rng));
    }
    // fixing bug where some enemy entities have positive velocity
    for (auto e : m_entityManager.getEntities()) {
        bool isEnemy = (e->getTag() == enemyNames[Rusher] || e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Assault]);
        if (isEnemy && e->getComponent<CTransform>().vel.x > 0) {

            e->destroy();
            bugedEnemiesCount++;
            std::cout << "Bugged Enemies destroyed " << bugedEnemiesCount << "\n";
        }
    }


    if (m_isPaused)
        return;

    sAnimation(dt);
    sMovement(dt);
    sCollisions();
    adjustPlayerPosition();
    sRender();
    sGuideMissiles(dt);
    sGunUpdate(dt);
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

