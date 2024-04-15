#include "Scene_GalaxyImpact.h"


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
#include <vector>
#include<algorithm>
#include "Scene_Menu.h"

namespace {
    std::random_device rd;
    std::mt19937 rng(rd());
}

std::uniform_real_distribution<float> spawnIntervalDistribution (1.0f, 4.0f);
sf::Time spawnInterval;
sf::Time spawnTimer;
sf::Vector2f enemyPrevPos;
int bugedEnemiesCount{ 0 };
int bossCount;
bool sortedUp;
sf::Time changeSceneTime;
sf::Time playerInvincibleTime;
int deathCount;
bool canSpawnEnemies;
sf::Time bossChargeAttackCD;
sf::Time bossMissileCD;
int totalLives;
bool canPlayMusic;
bool isBossDead;


Scene_GalaxyImpact::Scene_GalaxyImpact(GameEngine* gameEngine, const std::string& levelPath)
    : Scene(gameEngine)
    , m_worldView(gameEngine->window().getDefaultView()) 
{
    loadLevel(levelPath);
    registerActions();
    MusicPlayer::getInstance().play("levelTheme");
    MusicPlayer::getInstance().setVolume(35);

    init();
    
}




void Scene_GalaxyImpact::init() {

    isBossDead = false;
    canPlayMusic = false;
    lvlIndex = 0;
    bossMissileCD = sf::seconds(3.f);
    bossCount = 1;
    canPlayMusic = false;
    bossMissileCD = sf::seconds(3.f);
    totalLives = 3;
    bossChargeAttackCD = sf::seconds(5.f);
    canSpawnEnemies = true;
    deathCount = 0;
    playerInvincibleTime = sf::seconds(2.f);
    enemyPrevPos = sf::Vector2f(0.f, 0.f);
    changeSceneTime = sf::seconds(5.f);
    m_config.levelPaths.push_back("../assets/level1.txt");
    m_config.levelPaths.push_back("../assets/level2.txt");
    spawnInterval = sf::seconds(spawnIntervalDistribution(rng));
    spawnTimer = sf::seconds(0.f);
    sortedUp = false;
    spawnPlayer();
    renderLives();
}

void Scene_GalaxyImpact::sMovement(sf::Time dt) {
    playerMovement();
    assaultMovement();
    bossMovement(dt);

    // move all objects
    for (auto e : m_entityManager.getEntities()) {

        if (e->hasComponent<CTransform>()) {
            auto& tfm = e->getComponent<CTransform>();

            tfm.pos += tfm.vel * dt.asSeconds();
            tfm.angle += tfm.angVel * dt.asSeconds();
        }
    }

    for (auto l : m_entityManager.getEntities("laser")) {
       auto& lpos = l->getComponent<CTransform>().pos;
       auto ppos = m_player->getComponent<CTransform>().pos;
       lpos.x = ppos.x + 400.f;
       lpos.y = ppos.y;

    }
}


void Scene_GalaxyImpact::registerActions() {
    registerAction(sf::Keyboard::P, "PAUSE");
    registerAction(sf::Keyboard::Escape, "BACK");
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

void Scene_GalaxyImpact::renderLives()
{
    auto view = m_HudView.getCenter();

    for (int i{ 0 }; i < totalLives; i++) {

        auto life = m_entityManager.addEntity("life");
        auto pos = sf::Vector2f(view.x / 2 + (i * 15), view.y / view.y + 35.f);
        life->addComponent<CAnimation>(Assets::getInstance().getAnimation("life"));
        life->addComponent<CTransform>(pos, sf::Vector2f(0.f, 0.f));

        
    }
    


}


void Scene_GalaxyImpact::onEnd() {
    m_game->changeScene("MENU", nullptr, false);
}

void Scene_GalaxyImpact::sDestroyOutsideBattleField()
{
    destroyBulletsOutsideBattlefield();
    destroyEnemiesOutsideBattleField();
    destroyMissilesOutsideBattleField();
    
}

void Scene_GalaxyImpact::playerMovement() {
    // no movement if player is dead
    if (m_player->hasComponent<CState>() && m_player->getComponent<CState>().state == "dead")
        return;

    auto& vel = m_player->getComponent<CTransform>().vel;
    auto& input = m_player->getComponent<CInput>();
    float pSpeed = 200.f;

    sf::Vector2f pv{ 0.f, 0.f };
    


    if (input.up) {
        /*m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));*/
        pv.y -= 1.f;
    }
    if (input.down) {
        //m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.y += 1.f;
    }

    if (input.left) {
        //m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.x -=1.f;
    }

    if (input.right) {
        //m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
        pv.x += 1.f;
    }

    pv = normalize(pv);
    vel = pv * pSpeed;

    //if (dir != 0) {
    //    //SoundPlayer::getInstance().play("hop", m_player->getComponent<CTransform>().pos);
    //    dir = 0;
    //}

    
}

bool Scene_GalaxyImpact::bossTime(int bossCount)
{
    return m_player->getComponent<CTransform>().pos.x >= 1600.f and bossCount == 1;
}

void Scene_GalaxyImpact::bossLaunchMissile()
{
    for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
        if (b->hasComponent<CMissiles>()) {

            auto pos = b->getComponent<CTransform>().pos;
            auto missile = m_entityManager.addEntity("BossMissile");
            missile->addComponent<CState>("launched");
            missile->addComponent<CTransform>(pos + sf::Vector2f(0.f, -60.f), sf::Vector2f(m_config.missileSpeed, 0.f));
            auto bb = missile->addComponent<CAnimation>(Assets::getInstance().getAnimation("boss-missile")).animation.getBB();
            missile->addComponent<CBoundingBox>(bb);
           


        }
    }
}




void Scene_GalaxyImpact::sRender() {
    m_game->window().setView(m_worldView);
    sf::View view = m_game->window().getView();
    

    

    // draw bkg first
    for (auto e : m_entityManager.getEntities("bkg")) {
        if (e->getComponent<CSprite>().has) {
            auto& sprite = e->getComponent<CSprite>().sprite;
            m_game->window().draw(sprite);
        }
    }
    
    
    // draw Laser gun ammo
    auto laserAmmo = m_player->getComponent<CLaser>().laserCharge;
    sf::Vector2f topCenter(view.getCenter().x, 0.f );
    sf::Vector2f laserTextPos = topCenter + sf::Vector2f(10.f, 10.f);
    static sf::Text laserText("LaserAmmo ", Assets::getInstance().getFont("Arcade"), 20);
    std::string laserStr; 
    
    if (laserAmmo <= 0) {
        laserStr = "Laser 0 Energy";
    }
    else {

        laserStr = "Laser " + std::to_string(laserAmmo) + " Energy";
    }
    laserText.setString(laserStr);
    laserText.setPosition(laserTextPos);
    laserText.setFillColor(sf::Color::Green);
    m_game->window().draw(laserText);

    // draw missile ammo 
    auto missileAmmo = m_player->getComponent<CMissiles>().missileCount;
   sf::Vector2f topRight(view.getCenter().x + view.getSize().x / 2.f, 10.f);
   sf::Vector2f missileTextPos = topRight - sf::Vector2f(200.f, 0.f);
    static sf::Text missileText("MissileAmmo ", Assets::getInstance().getFont("Arcade"), 20);
    std::string missileStr;
    
    if (missileAmmo <= 0) {
        missileStr = "Missiles X 0";
    }
    else {

        missileStr = "Missiles X " + std::to_string(missileAmmo);
    }
    missileText.setString(missileStr);
    missileText.setPosition(missileTextPos);
    missileText.setFillColor(sf::Color::Blue);
    m_game->window().draw(missileText);


    //// draw Lives
    auto& pHealth = m_player->getComponent<CHealth>().hp;
    // Get the position of the top-left corner of the view
    sf::Vector2f viewTopLeft = view.getCenter() - view.getSize() / 2.f;

    // Calculate the position of the text relative to the view
    sf::Vector2f textPosition = viewTopLeft + sf::Vector2f(10.f, 10.f);
    static sf::Text livesT("Lives ", Assets::getInstance().getFont("Arcade"), 20);
    std::string livesStr = "Health " + std::to_string(pHealth) + " HP";
    livesT.setString(livesStr);
    livesT.setFillColor(sf::Color::Red);
    livesT.setPosition(textPosition);
    m_game->window().draw(livesT);

    // draw win Screen

    for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
        auto bstate = b->getComponent<CState>().state;
        // W scenario
        if (bstate == "defeated" and totalLives > 0) {
            static sf::Text winnerT("Congratulation\n Demo Level Completed\n", Assets::getInstance().getFont("Arcade"), 35);
            winnerT.setFillColor(sf::Color::Green);
            winnerT.setPosition(view.getCenter().x -200, view.getCenter().y);
            m_game->window().draw(winnerT);
        }
        
       
    }
    // draw loose Screen
    if (totalLives <= 0) {
        static sf::Text looseT("Game Over", Assets::getInstance().getFont("Arcade"), 35);
        std::string  loosestr = "Game Over!\n Your Impact\n Was Not\n Good Enough !!!";
        looseT.setString(loosestr);
        looseT.setFillColor(sf::Color::Red);
        looseT.setPosition(view.getCenter().x - 200, view.getCenter().y);
        m_game->window().draw(looseT);
    }

   
    for (auto& e : m_entityManager.getEntities()) {
        if (!e->hasComponent<CAnimation>())
            continue;
        if (e->getTag() != "life") {

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
    m_game->window().setView(m_HudView);
    auto hview = m_game->window().getView();
    // render here Hud elemnts in the hudview
     // Render other HUD elements
    for (auto& e : m_entityManager.getEntities("life")) {
        if (e->hasComponent<CAnimation>()) {
            auto& anim = e->getComponent<CAnimation>().animation;
            auto& tfm = e->getComponent<CTransform>();
            anim.getSprite().setPosition(tfm.pos);
            m_game->window().draw(anim.getSprite());
        }
    }

    // Switch back to the main view
    m_game->window().setView(m_worldView);

   
}
void Scene_GalaxyImpact::sGunUpdate(sf::Time dt)
{
    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CGun>()) {
            bool isEnemy = (e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Predator]);
            bool isBoss = e->getTag() == bossNames[Lv1Boss];
            auto& gun = e->getComponent<CGun>();
            gun.countdown -= dt;
            auto& viewBounds = m_worldView.getCenter();
            
            

            if (isEnemy or isBoss  && viewBounds.x*2 -50.f >= e->getComponent<CTransform>().pos.x) // enemy is  firing when in the view
                gun.isFiring = true;

            
            //
            // when firing
            //
            if (gun.isFiring && gun.countdown < sf::Time::Zero) {
                gun.isFiring = false;
                gun.countdown = m_config.fireInterval / (1.f + gun.fireRate);

                auto pos = e->getComponent<CTransform>().pos;
            
                spawnBullet(pos, isEnemy, isBoss);
               
                    

                
            }
            
        }
    }
}

void Scene_GalaxyImpact:: assaultMovement()
{
    
    auto& viewBounds = m_worldView.getCenter();
    
    for (auto& p : m_entityManager.getEntities(enemyNames[Assault]))
    {
        auto pos = p->getComponent<CTransform>().pos;
        auto halfSize = p->getComponent<CBoundingBox>().halfSize;
        auto& state = p->getComponent<CState>().state;
        auto& pVel = p->getComponent<CTransform>().vel;
           
      if (viewBounds.y * 2 <= pos.y + halfSize.y) {

                pVel.y *= -1;
               

      }
      else if(viewBounds.y /viewBounds.y >= pos.y - halfSize.y) {

                pVel.y *= -1;
                
      }

        
        
    }
}
void Scene_GalaxyImpact::destroyPickupOutsideBattleField()
{
    for (auto& e : m_entityManager.getEntities("Pickup")) {
        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;

        auto pickupPosX = e->getComponent<CTransform>().pos.x;

        if (pickupPosX <= outsideViewPos) {
            e->destroy();

        }
    }
}

void Scene_GalaxyImpact::destroyBulletsOutsideBattlefield()
{
    // destroy player bullets outside of the view 
    for (auto& e : m_entityManager.getEntities("PlayerBullet")) {
        auto outsideViewPos = m_worldView.getCenter().x * 2 + 50.f;
        auto bulletPosX = e->getComponent<CTransform>().pos.x;

        if (bulletPosX >= outsideViewPos) {
            e->destroy();
            
        }
    }

    // destroy enemy bullets outside of the view 
    for (auto& e : m_entityManager.getEntities("EnemyBullet")) {

        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;

        auto bulletPosX = e->getComponent<CTransform>().pos.x;

        if (bulletPosX <= outsideViewPos) {
            e->destroy();
            
        }

    }

    for (auto& e : m_entityManager.getEntities("BossBullet")) {

        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;

        auto bulletPosX = e->getComponent<CTransform>().pos.x;

        if (bulletPosX <= outsideViewPos) {
            e->destroy();

        }
    }




}

void Scene_GalaxyImpact::destroyEnemiesOutsideBattleField()
{
    for (auto& e : m_entityManager.getEntities(enemyNames[Assault])) {

        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;
        auto ePosX = e->getComponent<CTransform>().pos.x;

        if ( ePosX <= outsideViewPos) {
            e->destroy();

            
        }
    }

    for (auto& e : m_entityManager.getEntities(enemyNames[Predator])) {

        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;
        auto ePosX = e->getComponent<CTransform>().pos.x;

        if (ePosX <= outsideViewPos) {
            e->destroy();

            
        }
    }

    for (auto& e : m_entityManager.getEntities(enemyNames[Rusher])) {

        auto outsideViewPos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;
        auto ePosX = e->getComponent<CTransform>().pos.x;

        if (ePosX <= outsideViewPos) {
            e->destroy();

            
        }
    }
}

void Scene_GalaxyImpact::destroyMissilesOutsideBattleField()
{
    for (auto& e : m_entityManager.getEntities("missile")) {
        auto leftOutsidePos = m_worldView.getCenter().x / m_worldView.getCenter().x - 50.f;
        auto rightOutsidePos = m_worldView.getCenter().x * 2 + 50.f;
        auto mPosX = e->getComponent<CTransform>().pos.x;

        if (mPosX >= rightOutsidePos or mPosX <= leftOutsidePos) {
            e->destroy();
        }
    }
}

void Scene_GalaxyImpact::dropPickup(sf::Vector2f pos)
{
    static const std::string pickups[] = { "power-charge", "missile-pickup", "laser-pickup" };
    std::uniform_int_distribution<int> d1(1, 3);
    std::uniform_int_distribution<int> d2(0, 2);
    
    if (d1(rng) < 2)  //chance to drop a pickup
    {
        auto pickup = pickups[d2(rng)];
        auto p = m_entityManager.addEntity("Pickup");
        p->addComponent<CTransform>(pos);
        auto bb = p->addComponent<CAnimation>(Assets::getInstance()
            .getAnimation(pickup)).animation.getBB();
        p->addComponent<CBoundingBox>(bb);
    }

    
}

void Scene_GalaxyImpact::update(sf::Time dt) {
    sUpdate(dt);
}

void Scene_GalaxyImpact::sDoAction(const Command& action) {
    // On Key Press
    if (action.type() == "START") {
        if (action.name() == "PAUSE") { 
            setPaused(!m_isPaused); 
            MusicPlayer::getInstance().setPaused(m_isPaused);
        }
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
        else if (action.name() == "FIRE") {
            m_player->getComponent<CInput>().fire = true;
            fireBullets(); 
        }
        else if (action.name() == "LAUNCH") { fireMissile(); }
    }
    
    // on Key Release
    else if (action.type() == "END") {
        if (action.name() == "LEFT") { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "RIGHT") { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "UP") { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "DOWN") { m_player->getComponent<CInput>().down = false; }
        else if (action.name() == "FIRE") { 
            m_player->getComponent<CLaser>().isShooting = false;
            m_player->getComponent<CInput>().fire = false;
            for (auto& l : m_entityManager.getEntities("laser")) {
                l->destroy();
            }
        }
    }
}



void Scene_GalaxyImpact::spawnPlayer() {
    //spawn position
    auto pos = m_worldView.getSize();
    pos.x = 0.f;
    pos.y = pos.y / 2;
    int gunDamage = 25;
    int missileDamage = 50;
    int laserDamage = 50;

    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CTransform>(pos);
    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("mc"));
    auto bb = m_player->getComponent<CAnimation>().animation.getBB();
    m_player->addComponent<CBoundingBox>(sf::Vector2f(bb));
    m_player->addComponent<CState>().state = "invincible";
    m_player->addComponent<CInput>();
    m_player->addComponent<CGun>(gunDamage);
    m_player->addComponent<CMissiles>(missileDamage);
    m_player->addComponent<CHealth>(100);
    m_player->addComponent<CLaser>(laserDamage);
   
}

void Scene_GalaxyImpact::fireBullets()
{
    auto laserCharge = m_player->getComponent<CLaser>().laserCharge;
    auto chargeCost = m_player->getComponent<CLaser>().chargeCost;
    auto pos = m_player->getComponent<CTransform>().pos;
    sf::Vector2f laserPos(pos.x, pos.y);
    if (m_player->hasComponent<CGun>() && laserCharge <=0) {
        m_player->getComponent<CGun>().isFiring = true;
        m_player->getComponent<CLaser>().isShooting = false;
        SoundPlayer::getInstance().play("pgun", m_player->getComponent<CTransform>().pos);

        for (auto& l : m_entityManager.getEntities("laser")) {
            l->destroy();
        }

    }
    else 
    {
        m_player->getComponent<CGun>().isFiring = false;
        m_player->getComponent<CLaser>().isShooting = true;

        if (m_player->getComponent<CLaser>().isShooting = true) {
            spawnLaser(laserPos);
           

        }
        
    }
}
void Scene_GalaxyImpact::spawnLaser(sf::Vector2f pos)
{
    auto lvel = m_player->getComponent<CTransform>().vel;
    auto laser = m_entityManager.addEntity("laser");
    auto bb = laser->addComponent<CAnimation>(Assets::getInstance().getAnimation("laser-beam")).animation.getBB();
    laser->addComponent<CBoundingBox>(bb);
    laser->addComponent<CTransform>(pos, lvel);
    laser->addComponent<CInput>();
    SoundPlayer::getInstance().play("laser");

}
void Scene_GalaxyImpact::spawnBullet(sf::Vector2f pos, bool isEnemy, bool isBoss)
{
    float speed;

    if (isEnemy or isBoss) {
        speed = -m_config.bulletSpeed;
       /* SoundPlayer::getInstance().play("EnemyGunfire", pos);*/
    }
    else {
        speed = m_config.bulletSpeed;
        /*SoundPlayer::getInstance().play("AlliedGunfire", pos);*/
    }
    if (isEnemy) {
        auto bullet = m_entityManager.addEntity("EnemyBullet");
        if (isEnemy && canSpawnEnemies) {
            auto bb = bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation("ebullet-v1")).animation.getBB();
            bullet->addComponent<CBoundingBox>(bb);
            for (auto e : m_entityManager.getEntities()) {
                if (e->getTag() == enemyNames[Assault])
                    bullet->addComponent<CState>("assaultBullet");
                else if (e->getTag() == enemyNames[Predator]) {
                    bullet->addComponent<CState>("predatorBullet");
                }
            }
            bullet->addComponent<CTransform>(pos, sf::Vector2f(speed, 0.f));
        }
    }
    else if (isBoss) {
        auto bBullet = m_entityManager.addEntity("BossBullet");
        auto bossBb = bBullet->addComponent<CAnimation>(Assets::getInstance().getAnimation("ebullet-v2")).animation.getBB();
        auto& bossBA = bBullet->getComponent<CAnimation>().animation.getSprite();
        bossBA.setRotation(180.f);
        bBullet->addComponent<CBoundingBox>(bossBb);
        bBullet->addComponent<CTransform>(pos, sf::Vector2f(speed, 0.f));
        SoundPlayer::getInstance().play("bgun", pos);
        
    }
    else {
        auto bullet = m_entityManager.addEntity("PlayerBullet");
        auto bb = bullet->addComponent<CAnimation>(Assets::getInstance().getAnimation("bullet")).animation.getBB();
        bullet->getComponent<CAnimation>().animation.getSprite().setRotation(-90.f);
        bullet->addComponent<CBoundingBox>(bb);
        bullet->addComponent<CTransform>(pos, sf::Vector2f(speed, 0.f));
    }
    
    

}

void Scene_GalaxyImpact::bossMovement(sf:: Time dt)
{
    auto view = m_worldView.getCenter();
    auto triggerPoint = view.x + 300.f;
    float chargeSpeed = -500.f;
    auto chargeEndPoint = view.x - 300.f;

  
    for (auto boss : m_entityManager.getEntities(bossNames[Lv1Boss])) {
        auto halfSize = boss->getComponent<CBoundingBox>().halfSize;
        auto& state = boss->getComponent<CState>().state;
        auto& pos = boss->getComponent<CTransform>().pos;
        auto& vel = boss->getComponent<CTransform>().vel;
        if (triggerPoint >= pos.x and state == "alive") {

            
            vel.x = 0.f;
            vel.y = 1.f * m_config.enemySpeed;
            state = "moveV";
        }
        else if (triggerPoint >= pos.x and state == "moveV" and bossChargeAttackCD > sf::Time::Zero) {

            bossChargeAttackCD -= dt;

            if (view.y * 2 <= pos.y + halfSize.y) {

                vel.y *= -1;


            }
            else if (view.y / view.y >= pos.y - halfSize.y) {

                vel.y *= -1;

            }

        }
        else if (bossChargeAttackCD <= sf::Time::Zero and state == "moveV") {

            vel.y = 0;
            vel.x = 1.f * chargeSpeed;
            state = "moveH";

            

        }
        else if (chargeEndPoint >= pos.x and state == "moveH") {

            vel.x = 1.f * m_config.enemySpeed;
            state = "moveB";
            bossChargeAttackCD = sf::seconds(5.f);
        }
        else if (pos.x > triggerPoint and state == "moveB") {

            pos.x = triggerPoint;
            vel.x = 0.f;
            vel.y = 1.f * m_config.enemySpeed;
            state = "moveV";

        }

    }
   
   

}
sf::Vector2f Scene_GalaxyImpact::findPlayer(sf::Vector2f mPos)
{
    bool isPlayer;
    float closest = std::numeric_limits<float>::max();
    sf::Vector2f posClosest{ 0.f, 0.f };
   
        isPlayer = m_player->getTag()=="player";
        if (isPlayer  && m_player->getComponent<CTransform>().has) {
            auto pPos = m_player->getComponent<CTransform>().pos;
            float distToEnemy = dist(mPos, pPos);
            if (distToEnemy < closest) {
                closest = distToEnemy;
                posClosest = pPos;
            }
        }
    return posClosest;
}

sf::Vector2f Scene_GalaxyImpact::findClosestEnemy(sf::Vector2f mPos)
{
    float closest = std::numeric_limits<float>::max();
    bool isEnemy;
    bool isBoss;
    sf::Vector2f posClosest{ 0.f, 0.f };
    for (auto e : m_entityManager.getEntities()) {
        isEnemy = (e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Predator] || e->getTag() == enemyNames[Rusher]);
        isBoss = e->getTag() == bossNames[Lv1Boss];
        if (isEnemy or isBoss && e->getComponent<CTransform>().has) {
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

    for (auto e : m_entityManager.getEntities("BossMissile")) {
        if (e->getComponent<CTransform>().has) {
            auto& tfm = e->getComponent<CTransform>();
            auto ePos = findPlayer(tfm.pos);

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
           SoundPlayer::getInstance().play("missile", pos);
        }
    }
}

void Scene_GalaxyImpact::spawnBoss()
{
    auto view = m_worldView.getCenter();
    float bossSpeed = -200.f;
    auto pos = sf::Vector2f(view.x * 2 + 50.f, view.y);
    auto bossVel = sf::Vector2f(1.f, 0.f);
    int gunDamage = 45;
    int missileDamage = 80;
    auto boss = m_entityManager.addEntity(bossNames[Lv1Boss]);

    boss->addComponent<CTransform>(pos, bossVel * bossSpeed);
    boss->addComponent<CAnimation>(Assets::getInstance().getAnimation("Lvl1Boss"));
    auto bb = boss->getComponent<CAnimation>().animation.getBB();
    boss->addComponent<CBoundingBox>(bb);
    auto& bRotation = boss->getComponent<CAnimation>().animation.getSprite();
    bRotation.setRotation(-90.f);
    boss->addComponent<CHealth>(400);
    boss->addComponent<CGun>(gunDamage);
    boss->addComponent<CMissiles>(1, 100);
    boss->addComponent<CState>("alive");

   
}

void Scene_GalaxyImpact::spawnEnemy()
{
   float enemySpeed = 0.f;
   float assaultSpeed = 50.f;
   auto& viewH = m_worldView.getCenter();
   auto& viewV = m_worldView.getSize();

   // Random number generator for enemy type, quantity, and spawn intervals
   std::uniform_int_distribution<int> enemyTypeDistribution(0, enemyNames.size() - 1);
   std::uniform_int_distribution<int> quantityDistribution(1, 1);
   std::uniform_real_distribution<float> enemyVerticalSpawnRange(1.0f, 4.0f);


   // basic characterstics
   sf::Vector2f eVel(1.f, 0.f);
   int numEnemies = quantityDistribution(rng);

  
   for (int i = 1; i <= numEnemies; i++) {
      Enemies enemyType = static_cast<Enemies>(enemyTypeDistribution(rng));
      auto pos = sf::Vector2f((viewH.x*2) + 100.f, viewV.y / enemyVerticalSpawnRange(rng));
      const std::string& enemyName = enemyNames[enemyType];
      int gunDamage;

      auto enemy = m_entityManager.addEntity(enemyName); // Add entity 
      if (enemyPrevPos.y == 0 || std::abs(enemyPrevPos.y - pos.y) > 40.f) {
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
              enemy->addComponent<CHealth>(10);
              enemy->addComponent<CState>();
              SoundPlayer::getInstance().play("rusher", pos);



          }
          else if (enemyType == Enemies::Assault) {
              // Customize Assault enemy
              auto aVel = sf::Vector2f(1.f, 1.f);
              enemySpeed = -100;
              gunDamage = 35;
              // Add additional customization if needed
              aVel.x = aVel.x * enemySpeed;
              aVel.y = -aVel.y * assaultSpeed;
              enemy->addComponent<CGun>();
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Predator]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CGun>(gunDamage);
              enemy->addComponent<CState>();
              enemy->addComponent<CTransform>(pos, aVel);
              auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
              enemy->addComponent<CHealth>(35);
              eRotation.setRotation(90.f);
              enemy->addComponent<CState>();

          }
          else if (enemyType == Enemies::Predator) {

              // Customize Predator enemy
              gunDamage = 25;
              enemySpeed = -200;
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Assault]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CGun>(gunDamage);
              enemy->addComponent<CHealth>(25);
              enemy->addComponent<CTransform>(pos, eVel);
              enemy->addComponent<CState>();

          }
      }
      else {

          if (enemyType == Enemies::Assault) {
              // Customize Assault enemy
              enemySpeed = -100;
              if (!sortedUp) {
                  pos.y -= 200.f;
                  sortedUp = true;
              }
              else
              {
                  pos.y += 200.f;
                  sortedUp = false;
              }
              
              // Add additional customization if needed
              gunDamage = 35;
              auto aVel = sf::Vector2f(1.f, 1.f);
              aVel.x = aVel.x * enemySpeed;
              aVel.y = -aVel.y * assaultSpeed;
              enemy->addComponent<CGun>(gunDamage);
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Predator]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, aVel);
              enemy->addComponent<CHealth>(35);
              enemy->addComponent<CState>();

              auto& eRotation = enemy->getComponent<CAnimation>().animation.getSprite();
              eRotation.setRotation(90.f);
          }
          else if (enemyType == Enemies::Predator) {
              // Customize Predator enemy
              // Add additional customization if needed
              enemySpeed = -200;
              gunDamage = 25;

              if (!sortedUp) {
                  pos.y -= 200.f;
                  sortedUp = true;
              }
              else
              {
                 pos.y += 200.f;
                 sortedUp = false;
              }
              eVel = normalize(eVel);
              eVel = eVel * enemySpeed;
              enemy->addComponent<CGun>(gunDamage);
              enemy->addComponent<CAnimation>(Assets::getInstance().getAnimation(enemyNames[Assault]));
              auto bb = enemy->getComponent<CAnimation>().animation.getBB();
              enemy->addComponent<CBoundingBox>(sf::Vector2f(bb));
              enemy->addComponent<CTransform>(pos, eVel);
              enemy->addComponent<CHealth>(25);
              enemy->addComponent<CState>();


          }

      }
      enemyPrevPos.y = pos.y;
      
     

   }
   
}





void Scene_GalaxyImpact::sCollisions() {
    adjustPlayerPosition();
    adjustEnemyPosition();
    adjustBossPosition();
    checkShipCollisions();
    checkBulletCollison();
    checkMissileCollision();
    checkLaserCollision();
    checkPickupCollisions();

}

void Scene_GalaxyImpact::sPlayerInvincibleState(sf::Time dt)
{
    auto textOpacity = m_player->getComponent<CAnimation>().animation.getSprite().getColor().a;
    auto& pAnimationColor = m_player->getComponent<CAnimation>().animation.getSprite();
    auto& pState = m_player->getComponent<CState>().state;
    //auto bb = m_player->addComponent<CAnimation>().animation.getBB();
    sf::Color fillColor;
    sf::Color currentColor = pAnimationColor.getColor();

    
    if (playerInvincibleTime > sf::seconds(0.f) and pState == "invincible") {
        playerInvincibleTime -= dt;
        if (!isTransperent(pAnimationColor)) {
            
          textOpacity = 0;
          sf::Color newColor(currentColor.r, currentColor.g, currentColor.b, textOpacity);
          pAnimationColor.setColor(newColor);
          
          
          
        }
       
        else if(isTransperent(pAnimationColor)) {

           
            textOpacity = 255;
            sf::Color newColor(currentColor.r, currentColor.g, currentColor.b, textOpacity);
            pAnimationColor.setColor(newColor);
            

            
        }
        m_player->removeComponent<CBoundingBox>();

        

    }
    else if (playerInvincibleTime <= sf::Time::Zero and pState!="dead") {
        
        pState = "flying";
        m_player->addComponent<CBoundingBox>(sf::Vector2f(60.f, 34.f));
        textOpacity = 255;
        sf::Color newColor(currentColor.r, currentColor.g, currentColor.b, textOpacity);
        pAnimationColor.setColor(newColor);
        
       

    }

}

void Scene_GalaxyImpact::checkShipCollisions()
{
    auto& pHealth = m_player->getComponent<CHealth>().hp;
    auto& pState = m_player->getComponent<CState>().state;
  
    //player vs rusher
    for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {

        auto overlap = Physics::getOverlap(m_player, e);
        auto& eHealth = e->getComponent<CHealth>().hp;
        auto& eState = e->getComponent<CState>().state;

        if (overlap.x > 0 and overlap.y > 0) {
          
            pHealth -= pHealth;
            eHealth -= eHealth;
            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                totalLives--;
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));

            }
            if (eHealth <= 0) {
                eState = "dead";
                e->removeComponent<CBoundingBox>();
                SoundPlayer::getInstance().play("eexplosion");
                e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));

            }
        }

    }
    //player vs assault
    for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {

        auto overlap = Physics::getOverlap(m_player, e);
        auto& eHealth = e->getComponent<CHealth>().hp;
        auto& eState = e->getComponent<CState>().state;

        if (overlap.x > 0 and overlap.y > 0) {
            pHealth -= pHealth;
            eHealth -= eHealth;
            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                totalLives--;


            }
            if (eHealth <= 0) {
                eState = "dead";
                SoundPlayer::getInstance().play("eexplosion");
                e->removeComponent<CBoundingBox>();
                e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));

            }
        }

    }
    // player vs Predator
    for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {

        auto overlap = Physics::getOverlap(m_player, e);
        auto& eHealth = e->getComponent<CHealth>().hp;
        auto& eState = e->getComponent<CState>().state;

        if (overlap.x > 0 and overlap.y > 0) {
            pHealth -= pHealth;
            eHealth -= eHealth;
            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                totalLives--;

            }
            if (eHealth <= 0) {
                eState = "dead";
                SoundPlayer::getInstance().play("eexplosion");
                e->removeComponent<CBoundingBox>();
                e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));

            }
        }

    }

    // player vs boss
    for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
        auto overlap = Physics::getOverlap(m_player, b);

        if (overlap.x > 0 and overlap.y > 0) {
            pHealth -= pHealth;
            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                totalLives--;

            }
        }
    }
}

void Scene_GalaxyImpact::checkBulletCollison()
{
    auto& pHealth = m_player->getComponent<CHealth>().hp;
    auto pGunDmg = m_player->getComponent<CGun>().gunDamage;
    auto& pState = m_player->getComponent<CState>().state;

    //Player bullets vs enemies and Boss missiles
    for (auto bullet : m_entityManager.getEntities("PlayerBullet")) {
        for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {

            auto& eHealth = e->getComponent<CHealth>().hp;
            auto overlap = Physics::getOverlap(bullet, e);
            auto& eState = e->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {
               
               bullet->destroy();
               eHealth -= pGunDmg;

               if (eHealth <= 0) {
                   eState = "dead";
                   SoundPlayer::getInstance().play("eexplosion");
                   e->removeComponent<CBoundingBox>();
                   e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
               }
               
            }

        }

        for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {

            auto& eHealth = e->getComponent<CHealth>().hp;
            auto overlap = Physics::getOverlap(bullet, e);
            auto& eState = e->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {

                bullet->destroy();
                eHealth -= pGunDmg;

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion");
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }

            }
        }

        for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {

            auto& eHealth = e->getComponent<CHealth>().hp;
            auto overlap = Physics::getOverlap(bullet, e);
            auto& eState = e->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {

                bullet->destroy();
                eHealth -= pGunDmg;

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion");
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }

            }
        }
        for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {

            auto& bHealth = b->getComponent<CHealth>().hp;
            auto overlap = Physics::getOverlap(bullet, b);
            auto& bState = b->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {
                bullet->destroy();
                bHealth -= pGunDmg;
                if (bHealth <= 0) {
                    bState = "dead";
                    b->removeComponent<CBoundingBox>();
                    b->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }

        }
        for (auto bm : m_entityManager.getEntities("BossMissile")) {

            auto overlap = Physics::getOverlap(bullet, bm);
            auto& bmState = bm->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {

                bullet->destroy();
                bmState = "destroyed";
                SoundPlayer::getInstance().play("eexplosion");
                bm->removeComponent<CBoundingBox>();
                bm->addComponent<CAnimation>(Assets::getInstance().getAnimation("rusher-explosion"));
                
            }

        }
    }

    // Enemy bullets vs player
    for (auto eBullet : m_entityManager.getEntities("EnemyBullet")) {
        auto overlap = Physics::getOverlap(m_player, eBullet);
        auto bState = eBullet->getComponent<CState>().state;
        

        if (overlap.x > 0 and overlap.y > 0) {
            if (bState == "assaultBullet"){ pHealth -= 35; } 
            else if (bState == "predatorBullet") { pHealth -= 25; }

            eBullet->destroy();
            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                totalLives--;


            }


        }
    }

    //Boss bullets vs Player
    for (auto bBullet : m_entityManager.getEntities("BossBullet")) {
        auto overlap = Physics::getOverlap(m_player, bBullet);
        for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {

            auto bGunDmg = b->getComponent<CGun>().gunDamage;

            if (overlap.x > 0 and overlap.y > 0) {
               
                pHealth -= bGunDmg;
                bBullet->destroy();

                if (pHealth <= 0) {
                    pState = "dead";
                    SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                    m_player->removeComponent<CBoundingBox>();
                    m_player->removeComponent<CInput>();
                    m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                    totalLives--;

                }
            }


        }
        
    }
}
       

void Scene_GalaxyImpact::checkLaserCollision()
{
    auto laserDmg = m_player->getComponent<CLaser>().laserDamage;
    for (auto laser : m_entityManager.getEntities("laser")) {

        //laser vs rusher
        for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {

            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(laser, e);

            if (overlap.x > 0 and overlap.y > 0) {
                
                eHealth -= laserDmg;

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion");
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }

        // laser vs predator
        for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {
            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(laser, e);

            if (overlap.x > 0 and overlap.y > 0) {

                eHealth -= laserDmg;

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion");
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }
        //laser vs Assault
        for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {
            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(laser, e);

            if (overlap.x > 0 and overlap.y > 0) {

                eHealth -= laserDmg;

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion");
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }
         // laser vs Boss
        for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {

            auto& bHealth = b->getComponent<CHealth>().hp;
            auto& bState = b->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(laser, b);

            if (overlap.x > 0 and overlap.y > 0) {

                bHealth -= laserDmg/5;

                if (bHealth <= 0) {
                    bState = "dead";
                    b->removeComponent<CBoundingBox>();
                    b->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }

        }
        //laser vs Boss missiles
        for (auto bm : m_entityManager.getEntities("BossMissile")) {

            auto overlap = Physics::getOverlap(laser, bm);
            auto& bmState = bm->getComponent<CState>().state;

            if (overlap.x > 0 and overlap.y > 0) {

                bmState = "destroyed";
                SoundPlayer::getInstance().play("eexplosion");
                bm->removeComponent<CBoundingBox>();
                bm->addComponent<CAnimation>(Assets::getInstance().getAnimation("rusher-explosion"));

            }

        }
        
    }
}
void Scene_GalaxyImpact::checkMissileCollision()
{
    auto missileDmg = m_player->getComponent<CMissiles>().missileDamage;

    for (auto missile : m_entityManager.getEntities("missile")) {

        // missile vs rusher
        for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {
            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(missile, e);

            if (overlap.x > 0 and overlap.y > 0) {
             
                eHealth -= missileDmg;
                missile->destroy();

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion",e->getComponent<CTransform>().pos);
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }

        // missile vs predator
        for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {
            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(missile, e);

            if (overlap.x > 0 and overlap.y > 0) {

                eHealth -= missileDmg;
                missile->destroy();

                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion", e->getComponent<CTransform>().pos);
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }

        // missile vs assault
        for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {
            auto& eHealth = e->getComponent<CHealth>().hp;
            auto& eState = e->getComponent<CState>().state;
            auto overlap = Physics::getOverlap(missile, e);

            if (overlap.x > 0 and overlap.y > 0) {

                eHealth -= missileDmg;
                missile->destroy();


                if (eHealth <= 0) {
                    eState = "dead";
                    SoundPlayer::getInstance().play("eexplosion", e->getComponent<CTransform>().pos);
                    e->removeComponent<CBoundingBox>();
                    e->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }
        // missile vs lv1Boss
        for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
            auto& bState = b->getComponent<CState>().state;
            auto& bHealth = b->getComponent<CHealth>().hp;
            auto overlap = Physics::getOverlap(missile, b);

            if (overlap.x > 0 and overlap.y > 0) {

                bHealth -= missileDmg;
                missile->destroy();


                if (bHealth <= 0) {
                    bState = "dead";
                    b->removeComponent<CBoundingBox>();
                    b->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                }
            }
        }
    }

    //Boss missile vs Player
    for (auto bm : m_entityManager.getEntities("BossMissile")) {
        auto& pState = m_player->getComponent<CState>().state;
        auto& pHealth = m_player->getComponent<CHealth>().hp;
        auto mPos = bm->getComponent<CTransform>().pos;
        auto overlap = Physics::getOverlap(m_player, bm);
       

        if (overlap.x > 0 and overlap.y > 0) {

            pHealth -= pHealth
                ;
            bm->destroy();


            if (pHealth <= 0) {
                pState = "dead";
                SoundPlayer::getInstance().play("pexplosion", m_player->getComponent<CTransform>().pos);
                m_player->removeComponent<CBoundingBox>();
                m_player->removeComponent<CInput>();
                m_player->addComponent<CAnimation>(Assets::getInstance().getAnimation("explosion"));
                totalLives--;

            }
        }


    }
}

void Scene_GalaxyImpact::checkPickupCollisions()
{
    for (auto e : m_entityManager.getEntities("Pickup")) {

        // player collids with pickup;
        auto overlap = Physics::getOverlap(m_player, e);
        if (overlap.x > 0 and overlap.y > 0) {
            SoundPlayer::getInstance().play("pick-up", m_player->getComponent<CTransform>().pos);
            auto name = e->getComponent<CAnimation>().animation.m_name;
            if (name == "power-charge") {

                m_player->getComponent<CHealth>().hp += 40;
               
                if (m_player->getComponent<CHealth>().hp >= 100) {

                    m_player->getComponent<CHealth>().hp = 100;
                }
                
                
            }
            if (name == "missile-pickup") m_player->getComponent<CMissiles>().missileCount += 1;

            if (name == "laser-pickup") {

                m_player->getComponent<CLaser>().laserCharge += 40;
                if (m_player->getComponent<CLaser>().laserCharge >= 100) {

                    m_player->getComponent<CLaser>().laserCharge = 100;
                }
                
            }

            e->destroy();

        }

    }
}

void Scene_GalaxyImpact::sUpdate(sf::Time dt) {
    SoundPlayer::getInstance().removeStoppedSounds();
    m_entityManager.update();
    if (!m_isPaused) {
        if (!m_isPaused and canSpawnEnemies) { m_worldView.move(m_config.scrollSpeed * dt.asSeconds() * 1, 0.f); }
        spawnTimer += dt;


        // change laser charge amount
        auto& laserCharge = m_player->getComponent<CLaser>().laserCharge;
        auto chargeCost = m_player->getComponent<CLaser>().chargeCost;
        if (m_player->getComponent<CLaser>().isShooting == true && m_player->getComponent<CInput>().fire == true) {
            laserCharge -= chargeCost;
        }

        if (bossMissileCD > sf::Time::Zero && !canSpawnEnemies) {
            bossMissileCD -= dt;
        }
        else if (bossMissileCD <= sf::Time::Zero) {

            bossLaunchMissile();
            bossMissileCD = sf::seconds(3.f);
        }


        if (spawnTimer >= spawnInterval && canSpawnEnemies) {
            spawnTimer = sf::seconds(0.f);
            spawnEnemy();
            spawnInterval = sf::seconds(spawnIntervalDistribution(rng));
        }

        //check if player reached bossTime postion

        if (bossTime(bossCount)) {
            bossCount--;
            spawnBoss();
            canSpawnEnemies = false;
            MusicPlayer::getInstance().play("bossTheme");


        }

        for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
            auto& bstate = b->getComponent<CState>().state;
            // W scenario
            if (bstate == "dead" and totalLives > 0) {
                bool inLoop = true;
                m_isPaused = true;
                SoundPlayer::getInstance().play("bexplosion");
                MusicPlayer::getInstance().play("victory_song");
                MusicPlayer::getInstance().setLoop(!inLoop);
                bstate = "defeated";
                m_player->removeComponent<CInput>();
                isBossDead = true;
                //m_game->levelIndex++;


            }



        }

        // fixing bug where some enemy entities have positive velocity
        for (auto e : m_entityManager.getEntities()) {
            bool isEnemy = (e->getTag() == enemyNames[Rusher] || e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Assault]);
            if (isEnemy && e->getComponent<CTransform>().vel.x > 0) {

                e->destroy();
                bugedEnemiesCount++;
                /*std::cout << "Bugged Enemies destroyed " << bugedEnemiesCount << "\n";*/
            }
        }


        if (m_isPaused) {
            m_worldView.move(0.f, 0.f);
            return;
        }
        sAnimation(dt);
        sPlayerInvincibleState(dt);
        sMovement(dt);
        sCollisions();
        adjustPlayerPosition();
        adjustEnemyPosition();
        sRender();
        sGuideMissiles(dt);
        sGunUpdate(dt);
        sDestroyOutsideBattleField();
    }
    if (isBossDead) {


        changeSceneTime -= dt;
    }
    else if (!isBossDead and totalLives == 0) {

        changeSceneTime -= dt;
    }
    // L scenario
    if (totalLives <= 0 and m_player->getComponent<CState>().state == "dead") {
        m_isPaused = true;
        bool inLoop = true;
        MusicPlayer::getInstance().play("defeat_song");
        MusicPlayer::getInstance().setLoop(!inLoop);
        m_player->addComponent<CState>().state = "defeated";
        m_player->removeComponent<CInput>();

    }

    if (changeSceneTime <= sf::Time::Zero and totalLives == 0) {

        isBossDead = false;
        m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game), true);

    }

    
    if (changeSceneTime <= sf::Time::Zero and isBossDead) {

        isBossDead = false;
        m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game), true);

    }
   

}


void Scene_GalaxyImpact::sAnimation(sf::Time dt) {
    auto list = m_entityManager.getEntities();
    auto state = m_player->getComponent<CState>().state;
    auto playerAnim = m_player->getComponent<CAnimation>();


    for (auto e : m_entityManager.getEntities()) {
        //auto eAnimation = e->getComponent<CAnimation>().animation;
        bool isEnemy = (e->getTag() == enemyNames[Assault] || e->getTag() == enemyNames[Predator] || e->getTag() == enemyNames[Rusher]);
        bool isBoss = e->getTag() == bossNames[Lv1Boss];
        // update all animations
        if (e->hasComponent<CAnimation>()) {
            auto& anim = e->getComponent<CAnimation>();
            anim.animation.update(dt);
            if (playerAnim.animation.hasEnded() and state == "dead") {
               
                for (auto e : m_entityManager.getEntities("life")) {
                    e->destroy();
                }
                
                m_player->destroy();
                deathCount++;
                spawnPlayer();
                renderLives();
                playerInvincibleTime = sf::seconds(2.f);
                if (deathCount > 0) {
                    m_player->getComponent<CMissiles>().missileCount = 0;
                }

            }

            for (auto e : m_entityManager.getEntities(enemyNames[Rusher])) {
                auto eState = e->getComponent<CState>().state;
                auto eAnimation = e->getComponent<CAnimation>().animation;

                if (isEnemy && eAnimation.hasEnded() and eState == "dead") {
                    auto pos = e->getComponent<CTransform>().pos;
                    dropPickup(pos);
                    e->destroy();
                }
            }

            for (auto e : m_entityManager.getEntities(enemyNames[Assault])) {
                auto eState = e->getComponent<CState>().state;
                auto eAnimation = e->getComponent<CAnimation>().animation;

                if (isEnemy && eAnimation.hasEnded() and eState == "dead") {
                    auto pos = e->getComponent<CTransform>().pos;
                    dropPickup(pos);
                    e->destroy();
                }
            }

            for (auto e : m_entityManager.getEntities(enemyNames[Predator])) {
                auto eState = e->getComponent<CState>().state;
                auto eAnimation = e->getComponent<CAnimation>().animation;

                if (isEnemy && eAnimation.hasEnded() and eState == "dead") {
                    auto pos = e->getComponent<CTransform>().pos;
                    dropPickup(pos);
                    e->destroy();
                }
            }

            for (auto b : m_entityManager.getEntities(bossNames[Lv1Boss])) {
                auto bAnim = b->getComponent<CAnimation>();
                auto bState = b->getComponent<CState>().state;
                if (isBoss && bAnim.animation.hasEnded() and bState == "dead") {

                    b->destroy();
                }
            }

            for (auto bm : m_entityManager.getEntities("BossMissile")) {
                auto bmAnim = bm->getComponent<CAnimation>();
                auto bmState = bm->getComponent<CState>().state;
                if (bmAnim.animation.hasEnded() and bmState == "destroyed") {
                    bm->destroy();
                }
            }
        }
    }

}

void Scene_GalaxyImpact::adjustEnemyPosition()
{
    auto center = m_worldView.getCenter();
    sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;
  
    auto top = center.y - viewHalfSize.y;
    auto bot = center.y + viewHalfSize.y;

    for (auto& e : m_entityManager.getEntities(enemyNames[Rusher])) {
        auto& ePos = e->getComponent<CTransform>().pos;
        auto halfSize = sf::Vector2f{ e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CBoundingBox>().halfSize.y };

        ePos.y = std::max(ePos.y, top + halfSize.y);
        ePos.y = std::min(ePos.y, bot - halfSize.y);
    }

    for (auto& e : m_entityManager.getEntities(enemyNames[Assault])) {
        auto& ePos = e->getComponent<CTransform>().pos;
        auto halfSize = sf::Vector2f{ e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CBoundingBox>().halfSize.y };
       
        ePos.y = std::max(ePos.y, top + halfSize.y);
        ePos.y = std::min(ePos.y, bot - halfSize.y);
    }

    for (auto& e : m_entityManager.getEntities(enemyNames[Predator])) {
        auto& ePos = e->getComponent<CTransform>().pos;
        auto halfSize = sf::Vector2f{ e->getComponent<CBoundingBox>().halfSize.x, e->getComponent<CBoundingBox>().halfSize.y };
      
        ePos.y = std::max(ePos.y, top + halfSize.y);
        ePos.y = std::min(ePos.y, bot - halfSize.y);
    }


}

bool Scene_GalaxyImpact::isTransperent(sf::Sprite colorOpacity)
{
   
    return  colorOpacity.getColor().a == 0;
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

void Scene_GalaxyImpact::adjustBossPosition()
{
    auto center = m_worldView.getCenter();
    sf::Vector2f viewHalfSize = m_worldView.getSize() / 2.f;


    auto left = center.x - viewHalfSize.x;
    auto right = center.x + viewHalfSize.x;
    auto top = center.y - viewHalfSize.y;
    auto bot = center.y + viewHalfSize.y;

    for (auto boss : m_entityManager.getEntities(bossNames[Lv1Boss])) {
        auto& boss_pos = boss->getComponent<CTransform>().pos;
        auto halfSize = sf::Vector2f{ boss->getComponent<CBoundingBox>().halfSize.x, boss->getComponent<CBoundingBox>().halfSize.y };
        // keep boss in bounds
        boss_pos.y = std::max(boss_pos.y, top + halfSize.y);
        boss_pos.y = std::min(boss_pos.y, bot - halfSize.y);
    }
   
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
        if (token == "Mbkg") {
            std::string name;
            sf::Vector2f pos;
            config >> name >> pos.x >> pos.y;
            auto e = m_entityManager.addEntity("mbkg");
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

