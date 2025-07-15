#include "StudentWorld.h"
#include "Actor.h"
#include "Level.h"
#include <string>
#include <string>
#include <cmath>
#include <algorithm>

StudentWorld::StudentWorld(std::string assetPath)
    : GameWorld(assetPath), m_player(nullptr), m_levelFinished(false), m_currentLevel(0) {}


void StudentWorld::setDisplayText()
{
    int score = getScore();
    int level = getLevel();
    int livesLeft = getLives();
    unsigned int burps = m_player->numBurps();


    //converting string format 
    std::string s = "Score: ";
    s += std::string(7 - std::to_string(score).length(), '0') + std::to_string(score);
    s += "  Level: ";
    s += std::string(2 - std::to_string(level).length(), '0') + std::to_string(level);
    s += "  Lives: ";
    s += std::string(2 - std::to_string(livesLeft).length(), '0') + std::to_string(livesLeft);
    s += "  Burps: ";
    s += std::string(2 - std::to_string(burps).length(), '0') + std::to_string(burps);

    //setting game text :)
    setGameStatText(s);
}


GameWorld* createStudentWorld(std::string assetPath)
{
    return new StudentWorld(assetPath);
}

StudentWorld::~StudentWorld() {
    cleanUp();
}
int StudentWorld::init() {

    // Construct the level file name
    std::string levelFile = "level";
    if (m_currentLevel < 10) {
        levelFile += "0";
    }
    levelFile += std::to_string(m_currentLevel) + ".txt";

    Level lev(assetPath());
    Level::LoadResult result = lev.loadLevel(levelFile);

    if (result == Level::load_fail_file_not_found) {
        // No more levels, player has won the game
        return GWSTATUS_PLAYER_WON;
    }
    else if (result == Level::load_fail_bad_format) {
        return GWSTATUS_LEVEL_ERROR;
    }

    //level loading as spec advised
    for (int x = 0; x < VIEW_WIDTH; x++) {
        for (int y = 0; y < VIEW_HEIGHT; y++) {
            Level::MazeEntry item = lev.getContentsOf(x, y);
            switch (item) {
            case Level::player:
                m_player = new Player(this, x, y);
                break;
            case Level::floor:
                m_actors.push_back(new Floor(this, x, y));
                break;
            case Level::fireball:
                m_actors.push_back(new Fireball(this, x, y));
                break;
            case Level::koopa:
                m_actors.push_back(new Koopa(this, x, y));
                break;
            case Level::left_kong:
                m_actors.push_back(new Left_Kong(this, x, y));
                break;
            case Level::right_kong:
                m_actors.push_back(new Right_Kong(this, x, y));
                break;
            case Level::ladder:
                m_actors.push_back(new Ladder(this, x, y));
                break;
            case Level::extra_life :
                m_actors.push_back(new ExtraLifeGoodie(this, x, y));
                break;
            case Level::garlic:
                m_actors.push_back(new Garlic(this, x, y));
                break;
            case Level::bonfire:
                m_actors.push_back(new Bonfire(this, x, y));
            }
        }
    }

    return GWSTATUS_CONTINUE_GAME;
}


void StudentWorld::addActor(Actor* newActor) {
    m_actorsToAdd.push_back(newActor);
}

int StudentWorld::move() {
    if (!m_player->isAlive())
        return GWSTATUS_PLAYER_DIED;

    m_player->doSomething(); //make the player move


    setDisplayText();
    std::vector<Actor*> actorsToDelete;
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->isAlive()) {
            (*it)->doSomething();
        }
        else {
            actorsToDelete.push_back(*it);
        }
    }

    for (std::vector<Actor*>::iterator it = actorsToDelete.begin(); it != actorsToDelete.end(); ++it) {
        std::vector<Actor*>::iterator actorIt = std::find(m_actors.begin(), m_actors.end(), *it);
        if (actorIt != m_actors.end()) {
            m_actors.erase(actorIt);
        }
        delete* it;
    }

    // Add new actors
    m_actors.insert(m_actors.end(), m_actorsToAdd.begin(), m_actorsToAdd.end());
    m_actorsToAdd.clear();

    if (m_levelFinished) {
        m_levelFinished = false;
        m_currentLevel++;
        return GWSTATUS_FINISHED_LEVEL;
    }

    return GWSTATUS_CONTINUE_GAME;
}


void StudentWorld::cleanUp() {
    //delete the player
    delete m_player;
    m_player = nullptr;
    //delete the actors
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        delete* it;
    }
    //clear the vector
    m_actors.clear();
}

bool StudentWorld::isBlockedAt(int x, int y) const {
    for (std::vector<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->getX() == x && (*it)->getY() == y && !(*it)->isWalkThroughAble() && !(*it)->isAttackable()) {
            return true;
        }
    }
    return false;
}


const std::vector<Actor*>& StudentWorld::getActors() const {
    return m_actors;
}


bool StudentWorld::isPlayerAt(int x, int y) const {
    return m_player->isAt(x, y);
}

void StudentWorld::playerHitByKoopa(){
    m_player->freeze();
}
bool StudentWorld::isLadderAt(int x, int y) const {
    for (std::vector<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->isClimbable() && (*it)->getX() == x && (*it)->getY() == y) {
            return true;
        }
    }
    return false;
}


bool StudentWorld::isBarrelAt(int x, int y) const {
    for (std::vector<Actor*>::const_iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->AttackableByCampfire() && !isPlayerAt(x, y)) {
            return true;
        }
    }
    return false;
}


void StudentWorld::BarrelHitByBonfire() {
    //set Barrel to dead
}

void StudentWorld::attackWithBurp(int x, int y) {
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->isAt(x, y) && (*it)->isAttackable()) {
            (*it)->onBurpHit();
        }
    }
}




void StudentWorld::getGarlic() {
    playSound(SOUND_GOT_GOODIE);
    increaseScore(25);
    m_player->incBurp(5);
}

void StudentWorld::getLife() { 
    playSound(SOUND_GOT_GOODIE);
    increaseScore(50);
    incLives();
}

void StudentWorld::playerHit() { //used whenever player is hit by an enemy
    playSound(SOUND_PLAYER_DIE);
    m_player->setDead();
    decLives();
}

void StudentWorld::createGarlic(int x, int y) {
    m_actors.push_back(new Garlic(this, x, y)); //have to create a new actor
}

void StudentWorld::createGoodie(int x, int y) {
    m_actors.push_back(new ExtraLifeGoodie(this, x, y)); //have to create a new actor
}

double StudentWorld::distanceFromPlayer(Actor* actor) {
    int ActorX = actor->getX();
    int ActorY = actor->getY();

    int PlayerX = m_player->getX();
    int PlayerY = m_player->getY();

    int dx = PlayerX - ActorX;
    int dy = PlayerY - ActorY;
    return sqrt(dx*dx + dy*dy); //classic distance formula (cmath sqrt function)
}

int StudentWorld::getN() { //getting the rate at which the barrels are produced
    int level = getLevel();
    int maxValue = max(200 - 50 * level, 50);
    return maxValue;
}

int StudentWorld::max(int n1, int n2) { //simply a max function of two arguements
    if (n1 > n2) {
        return n1;
    }
    else {
        return n2;
    }
}

bool StudentWorld::isDestructiveEntityAt(int x, int y) {
    for (std::vector<Actor*>::iterator it = m_actors.begin(); it != m_actors.end(); ++it) {
        if ((*it)->getX() == x && (*it)->getY() == y && (*it)->canAttackBarrel()) {
            return true;
        }
    }
    return false;
}



void StudentWorld::finishLevel() {
    // Increase the player's score by 1000 points
    increaseScore(1000);

    // Play the level completion sound
    playSound(SOUND_FINISHED_LEVEL);

    // Set the game status to indicate the level is finished
    // This will cause the framework to call cleanUp() and then init() for the next level
    m_levelFinished = true;
}