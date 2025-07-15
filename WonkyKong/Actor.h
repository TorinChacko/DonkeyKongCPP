#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <string>

class StudentWorld;

class Actor : public GraphObject {
public:
    Actor(StudentWorld* world, int imageID, int startX, int startY, int dir = 0, double size = 1.0)
        : GraphObject(imageID, startX, startY, dir, size),
        m_world(world),
        m_alive(true)
    {}
    virtual bool canAttackBarrel() {
        return false;
    }
    virtual void doSomething() = 0;

    bool isAlive() const { return m_alive; }
    void setDead() { m_alive = false; }
    virtual bool isAttackable() const {
        return false; //by default they are not attackable
    }
    virtual bool isClimbable() {
        return false;
    }

    virtual void onBurpHit();

    virtual bool isWalkThroughAble() {
        return false;
    }

    virtual bool AttackableByCampfire() {
        return false;
    }
    bool isAt(int x, int y) const;

    StudentWorld* getWorld() const { return m_world; }

private:
    bool m_alive;
    StudentWorld* m_world;
};

//==============================================
//Player Class
//==============================================


class Player : public Actor {
public:
    Player(StudentWorld* world, int startX, int startY)
        : Actor(world, IID_PLAYER, startX, startY, right),
        m_burps(0),
        m_frozen(false),
        m_freezeTimer(0),
        m_jumping(false),
        m_jumpStep(0)
    {}
    virtual bool AttackableByCampfire() {
        return true;
    }
    virtual void doSomething() override;
    void freeze();
    void incBurp(int burps);
    void createABurp();

    int numBurps() {
        return m_burps;
    }

private:
    int m_burps;
    bool m_frozen;
    int m_freezeTimer;
    bool m_jumping;
    int m_jumpStep;

    void executeJumpStep();
    bool isOnSolidGround() const;
    bool isBlockedByFloor(int dir) const;
    bool canClimb() const;
    bool canJump() const;
    void initiateJump();
    int viewLives();
};

//================================
//Floor Class
//================================


class Floor : public Actor{
public:
    Floor(StudentWorld* world, int startX, int startY) : Actor(world, IID_FLOOR, startX, startY, 0) {}

    virtual void doSomething() override {
        // Do nothing
    }

    virtual void getAttacked() {
        // Do nothing
    }
};

//=================================
//Fireball
//=================================



class Fireball : public Actor {
public:
    Fireball(StudentWorld* world, int startX, int startY) 
        : Actor(world, IID_FIREBALL, startX, startY, randInt(0, 1) == 0 ? left : right), m_climbingUp(false), m_climbingDown(false)
    {
        m_tickCounter = 0;
    }

    virtual void doSomething() override;
    virtual bool isAttackable() const override {
        return true; // Fireballs are attackable
    }
    void getAttacked();
    bool isClimbable(int x, int y) const;
    bool hasSolidObjectAbove(int x, int y) const;

    void onBurpHit();


private:
    bool m_climbingUp;
    bool m_climbingDown;
    int m_tickCounter;
};


//====================================
// Ladder
//====================================

class Ladder : public Actor {
public:
    Ladder(StudentWorld* world, int startX, int startY)
        : Actor(world, IID_LADDER, startX, startY, 0)
    {}
    virtual bool isWalkThroughAble() {
        return true;
    }
    virtual void doSomething() override { return; }
    bool isClimbable() {
        return true;
    }
};



//===================================
// Koopa 
//===================================

class Koopa : public Actor {
public:
    Koopa(StudentWorld* world, int startX, int startY)
        : Actor(world, IID_KOOPA, startX, startY, randInt(0, 1) == 0 ? left : right), m_freezeTimer(0), m_ticks(0)
    {}

    virtual void doSomething() override;
    bool isAttackable() const {
        return true; //by default they are not attackable
    }
    void onBurpHit();

private:
    int m_freezeTimer;
    int m_ticks;
};

//=================================
// Burp
//=================================

class Burp : public Actor {
public:
    Burp(StudentWorld* world, int startX, int startY, int dir);

    virtual void doSomething() override;
    virtual bool isWalkThroughAble() {
        return true;
    }
    bool canAttackBarrel() {
        return true;
    }
private:
    int m_lifetime;
};

//===================================
// Barrel
//===================================
class Barrel : public Actor {
public:
    Barrel(StudentWorld* world, int dir, int startX, int startY) : Actor(world, IID_BARREL, startX, startY, dir), m_ticks(0) {}
    virtual void doSomething() override;
    virtual bool isAttackable() const override {
        return true; // Fireballs are attackable
    }
    virtual bool AttackableByCampfire() {
        return true;
    }

    void onBurpHit();
private:
    int m_ticks;
};

//=====================================
// Extra Life Goodie
//=====================================

class ExtraLifeGoodie : public Actor {
public:
    ExtraLifeGoodie(StudentWorld* world, int startX, int startY) : Actor(world, IID_EXTRA_LIFE_GOODIE, startX, startY, 0) {}
    virtual bool isWalkThroughAble() {
        return true;
    }
    virtual void doSomething() override;
};

//======================================
// Garlic
//======================================

class Garlic : public Actor {
public:
    Garlic(StudentWorld* world, int startX, int startY) : Actor(world, IID_GARLIC_GOODIE, startX, startY, 0) 
    {}
    virtual bool isWalkThroughAble() {
        return true;
    }
    virtual void doSomething() override;
};


//======================================
// Kong - Left and Right
//======================================
class Kong : public Actor {
public:
    Kong(StudentWorld* world, int startX, int startY, int dir)
        : Actor(world, IID_KONG, startX, startY, dir)
    {
        m_fleeing = false;
        m_ticks = 0;
    }
    virtual void doSomething();
    void createABarrel();
private:
    bool m_fleeing;
    int m_ticks;
};

class Left_Kong : public Kong {
public:
    Left_Kong(StudentWorld* world, int startX, int startY)
        : Kong(world, startX, startY, left)
    {}
};

class Right_Kong : public Kong {
public:
    Right_Kong(StudentWorld* world, int startX, int startY)
        : Kong(world, startX, startY, right)
    {}
};

//===========================================
// Bonfire
//===========================================
class Bonfire : public Actor {
public:
    Bonfire(StudentWorld* world, int startX, int startY) : Actor(world, IID_BONFIRE, startX, startY, 0)
    {}
    virtual void doSomething() override;
    virtual bool isWalkThroughAble() {
        return true;
    }
    bool canAttackBarrel() {
        return true;
    }
};




#endif // ACTOR_H_
