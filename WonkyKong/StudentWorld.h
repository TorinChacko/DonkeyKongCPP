#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <vector>

class Actor;
class Player;

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetPath);
    virtual ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();

    bool isBlockedAt(int x, int y) const;
    bool isBarrelAt(int x, int y) const;
    bool isPlayerAt(int x, int y) const;
    bool isLadderAt(int x, int y) const;
    void playerHitByKoopa();
    void attackWithBurp(int x, int y);
    void getGarlic();
    void addActor(Actor* newActor);
    void getLife();
    void BarrelHitByBonfire();
    void createGarlic(int x, int y);
    void createGoodie(int x, int y);
    void setDisplayText();
    double distanceFromPlayer(Actor* actor);
    //add here
    void playerHit();
    const std::vector<Actor*>& getActors() const;
    int getN();
    int max(int n1, int n2);

    bool isDestructiveEntityAt(int x, int y);
    void finishLevel();
private:
    std::vector<Actor*> m_actors;
    Player* m_player;
    std::vector<Actor*> m_actorsToAdd;
    bool m_levelFinished;
    int m_currentLevel;
};

#endif // STUDENTWORLD_H_
