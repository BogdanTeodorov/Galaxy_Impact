

#ifndef BREAKOUT_COMPONENTS_H
#define BREAKOUT_COMPONENTS_H


#include <memory>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include "Animation.h"
#include <bitset>


struct Component
{
    bool		has{ false };
    Component() = default;
};


struct CAnimation : public Component {
    Animation   animation;

    CAnimation() = default;
    CAnimation(const Animation& a) : animation(a) {}

};

struct CScore : public Component {

    int score{ 0 };
    CScore() = default;
    CScore(int& _score) : score(_score){}
};

struct CSprite : public Component {
    sf::Sprite sprite;

    CSprite() = default;

    CSprite(const sf::Texture &t)
            : sprite(t) {
        centerOrigin(sprite);
    }

    CSprite(const sf::Texture &t, sf::IntRect r)
            : sprite(t, r) {
        centerOrigin(sprite);
    }
};


struct CTransform : public Component
{

    sf::Transformable  tfm;
    sf::Vector2f	pos			{ 0.f, 0.f };
    sf::Vector2f	prevPos		{ 0.f, 0.f };
    sf::Vector2f	vel			{ 0.f, 0.f };
    sf::Vector2f	scale		{ 1.f, 1.f };

    float           angVel{ 0 };
    float	        angle{ 0.f };

    CTransform() = default;
    CTransform(const sf::Vector2f& p) : pos(p)  {}
    CTransform(const sf::Vector2f& p, const sf::Vector2f& v)
            : pos(p), prevPos(p),  vel(v){}

};

struct CLives : public Component 
{
    int lives{ 3 };
    CLives() = default;
    CLives(int& l): lives(l){}
};

struct CBoundingBox : public Component
{
    sf::Vector2f size{0.f, 0.f};
    sf::Vector2f halfSize{ 0.f, 0.f };

    CBoundingBox() = default;
    CBoundingBox(const sf::Vector2f& s) : size(s), halfSize(0.5f * s)
    {}
};

struct CState : public Component {
    std::string state{"none"};

    CState() = default;
    CState(const std::string& s) : state(s){}

};


struct COccupied : public Component {

    bool isOccupied = false;
    COccupied() = default;
    COccupied(bool& occupied):isOccupied(occupied){}
};

struct CGun : public Component {
    int gunDamage{ 0 };
    bool isFiring{ false };
    sf::Time countdown{ sf::Time::Zero };
    int fireRate{ 1 };
    //int spreadLevel{ 1 };

    CGun() = default;
    CGun(int gunDamage_):gunDamage(gunDamage_){}
};

struct CLaser : public Component {

    bool isShooting{ false };
    long laserCharge{ 0 };
    int laserDamage{ 0 };
    int chargeCost{ 1 };
    CLaser() = default;
    CLaser(int laserDamage_) : laserDamage(laserDamage_) {}
    CLaser(long laserCharge_) : laserCharge(laserCharge_) {}
    CLaser(long laserCharge_, int laserDamage_) : laserCharge(laserCharge_), laserDamage(laserDamage_) {}
};

struct CHealth : public Component {
    int         hp{ 1 };

    CHealth() = default;
    CHealth(int hp) : hp(hp) {}
};

struct CMissiles : public Component {
    size_t    missileCount{ 3 };
    int missileDamage{ 0 };
    CMissiles() = default;
    CMissiles(size_t missleCount_) :missileCount(missleCount_) {}
    CMissiles(int missleDmage_) :missileDamage(missleDmage_) {}
    CMissiles(size_t missleCount_, int missileDamage_) :missileCount(missleCount_), missileDamage(missileDamage_) {}

};

struct CInput : public Component
{
    /*enum dirs {
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3
    };

    unsigned char dir{0};*/

    bool up{ false };
    bool left{ false };
    bool right{ false };
    bool down{ false };
    bool fire{ false };

    CInput() = default;
};


#endif //BREAKOUT_COMPONENTS_H
