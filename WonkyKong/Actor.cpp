#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

//===================================
// Actor Implementations
//===================================

bool Actor::isAt(int x, int y) const {
    return (getX() == x && getY() == y);
}
void Actor::onBurpHit() {
    return;
}


//Player Implementations

void Player::doSomething() {
    if (!isAlive()) {
        return;
    }

    if (m_jumping) {
        executeJumpStep();
        return;
    }

    if (m_frozen) {
        m_freezeTimer--;
        if (m_freezeTimer <= 0) {
            m_frozen = false;
        }
        return;
    }

    // Check for falling
    if (!isOnSolidGround() && !canClimb()) {
        moveTo(getX(), getY() - 1);
        return;
    }

    int ch;
    if (getWorld()->getKey(ch)) {
        switch (ch) {
        case KEY_PRESS_LEFT:
            if (getDirection() != left) {
                setDirection(left);
                break;
            }
            if (!isBlockedByFloor(left)) {
                moveTo(getX() - 1, getY());
            }
            break;
        case KEY_PRESS_RIGHT:
            if (getDirection() != right) {
                setDirection(right);
                break;
            }
            if (!isBlockedByFloor(right)) {
                moveTo(getX() + 1, getY());
            }
            break;
        case KEY_PRESS_UP:
            if (canClimb()) {
                moveTo(getX(), getY() + 1);
                break;
            }
            else if (canJump()) {
                getWorld()->playSound(SOUND_JUMP);
                if (!getWorld()->isBlockedAt(getX(), getY() + 1))
                    initiateJump();
            }
            break;
        case KEY_PRESS_DOWN:
            if (getWorld()->isLadderAt(getX(), getY() - 1) || (canClimb())) {
                if (!isBlockedByFloor(down)) {
                    moveTo(getX(), getY() - 1);
                }
            }
            break;
        case KEY_PRESS_SPACE:
            if (canJump()) {
                getWorld()->playSound(SOUND_JUMP);
                initiateJump();
                break;
            }
            if (canClimb()) {
                moveTo(getX(), getY() + 1);
                break;
            }
            break;
        case KEY_PRESS_TAB:
            if (m_burps > 0) {
                createABurp();
                getWorld()->playSound(SOUND_BURP);
                m_burps--;
            }
            break;
        }
    }
}



void Player::executeJumpStep() {
    switch (m_jumpStep) {
    case 0:
        if (!getWorld()->isBlockedAt(getX(), getY() + 1)) {
            m_jumpStep++;
            moveTo(getX(), getY() + 1);
            if (getWorld()->isLadderAt(getX(), getY())) {
                m_jumping = false;
                m_jumpStep = 0;
                return;
            }
        }
        else {
            m_jumping = false;
            m_jumpStep = 0;
        }
        break;
    case 1:
    case 2:
    case 3:
        if (isBlockedByFloor(getDirection()) || getWorld()->isLadderAt(getX(), getY())) {
            m_jumping = false;
            m_jumpStep = 0;
            return;
        }
        if (getDirection() == right)
            moveTo(getX() + 1, getY());
        if (getDirection() == left)
            moveTo(getX() - 1, getY());
        m_jumpStep++;
        break;
    case 4:
        if (isBlockedByFloor(getDirection()) || getWorld()->isLadderAt(getX(), getY())) {
            m_jumping = false;
            m_jumpStep = 0;
            return;
        }
        if(!isBlockedByFloor(down))
            moveTo(getX(), getY() - 1);
        m_jumpStep = 0;
        m_jumping = false;
        break;
    }
}

int Player::viewLives() { 
    return getWorld()->getLives(); 
}


bool Player::isOnSolidGround() const {
    int belowX, belowY;
    getPositionInThisDirection(down, 1, belowX, belowY);

    // Check for solid ground (floor)
    if (getWorld()->isBlockedAt(belowX, belowY)) {
        return true;
    }

    // Check for top of ladder
    const auto& actors = getWorld()->getActors();
    for (auto actor : actors) {
        if (getWorld()->isLadderAt(belowX, belowY) && actor->isAt(belowX, belowY)) {
            return true; // Player is standing on top of a ladder
        }
    }

    return false; // Not on solid ground or ladder top
}


bool Player::isBlockedByFloor(int dir) const {
    int checkX, checkY;
    getPositionInThisDirection(dir, 1, checkX, checkY);
    return getWorld()->isBlockedAt(checkX, checkY);
}

bool Player::canClimb() const {
    const auto& actors = getWorld()->getActors(); // Access actors via getter
    for (auto actor : actors) {
        if (actor->isClimbable() && actor->isAt(getX(), getY())) {
            return true; // Player is on a ladder
        }
    }
    return false;
}

bool Player::canJump() const {
    return !m_jumping && !m_frozen && isOnSolidGround();
}

void Player::initiateJump() {
    m_jumping = true;
}
void Player::incBurp(int burps) {
    m_burps += burps;
}

void Player::createABurp() {
    if (m_burps > 0) {
        int burpX = getX();
        int burpY = getY();

        switch (getDirection()) {
        case GraphObject::right:
            burpX++;
            break;
        case GraphObject::left:
            burpX--;
            break;
        }

        if (!getWorld()->isBlockedAt(burpX, burpY)) {
            Burp* newBurp = new Burp(getWorld(), burpX, burpY, getDirection());
            getWorld()->addActor(newBurp);
            getWorld()->playSound(SOUND_BURP);
        }
    }
}



void Player::freeze() {
    m_freezeTimer = 50;
    m_frozen = true;
}


//=============================================================
// Fireball Implementation
//=============================================================

void Fireball::doSomething() {
    // Step 1: Check if alive
    if (!isAlive()) {
        return;
    }

    // Step 2: Check if on same square as Player
    if (getWorld()->isPlayerAt(getX(), getY())) {
        getWorld()->playerHit();
        return;
    }

    // Step 3: Move every 10 ticks
    m_tickCounter++;
    if (m_tickCounter == 10) {
        m_tickCounter = 0;

        // Step 3a: Check if current square is climbable and can climb up
        if (isClimbable(getX(), getY()) && !hasSolidObjectAbove(getX(), getY()) && !m_climbingDown) {
            if (m_climbingUp || (randInt(1,3)==1)) {
                m_climbingUp = true;
                m_climbingDown = false;
                moveTo(getX(), getY() + 1);
                if (getWorld()->isPlayerAt(getX(), getY())) {
                    getWorld()->playerHit();
                }
                return;
            }
        }

        // Step 3b: Check if can climb down
        if (isClimbable(getX(), getY() - 1) && !m_climbingUp) {
            if (m_climbingDown || (randInt(1, 3) == 1)) {
                m_climbingUp = false;
                m_climbingDown = true;
                moveTo(getX(), getY() - 1);
                if (getWorld()->isPlayerAt(getX(), getY())) {
                    getWorld()->playerHit();
                }
                return;
            }
        }

        // Step 3c: Check if climbing state should end
        if ((m_climbingUp && !isClimbable(getX(), getY() + 1)) || (m_climbingDown && !isClimbable(getX(), getY() - 1))){
            m_climbingUp = false;
            m_climbingDown = false;
        }

        // Step 3d: Horizontal movement
        int nextX = (getDirection() == left) ? getX() - 1 : getX() + 1;
        if (!(getWorld()->isLadderAt(nextX, getY()-1)) && (getWorld()->isBlockedAt(nextX, getY()) || !getWorld()->isBlockedAt(nextX, getY() - 1))) {
            // Reverse direction
            setDirection(getDirection() == left ? right : left);
            nextX = (getDirection() == left) ? getX() - 1 : getX() + 1;
            return;
        }
        if (getWorld()->isBlockedAt(nextX, getY() - 1) || getWorld()->isLadderAt(nextX, getY() - 1) || getWorld()->isLadderAt(getX(), getY()))
            moveTo(nextX, getY());


        // Step 3e: Check if on same square as Player after moving
        if (getWorld()->isPlayerAt(getX(), getY())) {
            getWorld()->playerHit();
        }
    }
}


bool Fireball::isClimbable(int x, int y) const {
    // Check if the square at (x, y) is climbable (e.g., a ladder)
    return getWorld()->isLadderAt(x, y);
}

bool Fireball::hasSolidObjectAbove(int x, int y) const {
    // Check if there's a solid object above the given coordinates
    return getWorld()->isBlockedAt(x, y + 1);
}

void Fireball::onBurpHit() {
    setDead();
    getWorld()->playSound(SOUND_ENEMY_DIE);
    getWorld()->increaseScore(100);
    if (randInt(1, 3) == 1) {
        getWorld()->createGarlic(getX(), getY());
    }
}

//=============================================================
// Ladder Implementation
//=============================================================


//=============================================================
// Burp
//=============================================================
Burp::Burp(StudentWorld* world, int startX, int startY, int dir) : Actor(world, IID_BURP, startX, startY, dir), m_lifetime(5)
{}

void Burp::doSomething() {
    if (!isAlive()) {
        return;
    }
    getWorld()->attackWithBurp(getX(), getY());
    m_lifetime--;
    if (m_lifetime <= 0) {
        setDead();
    }
}

//==============================================================
// Barrel
//==============================================================

void Barrel::doSomething() {
    if (!isAlive()) {
        return;
    }

    if (getWorld()->isPlayerAt(getX(), getY())) {
        getWorld()->playerHit();
        return;
    }


    if (getWorld()->isDestructiveEntityAt(getX(), getY())) { //checks if there is a burp or camfire there
        setDead();
        getWorld()->playSound(SOUND_ENEMY_DIE);
        return;
    }

    if (!getWorld()->isBlockedAt(getX(), getY() - 1)) {
        moveTo(getX(), getY() - 1);
        if (getWorld()->isBlockedAt(getX(), getY() - 1)) {
            setDirection(getDirection() == left ? right : left);
        }
    }

    m_ticks++;
    if (m_ticks == 10) {
        m_ticks = 0;

        int nextX = (getDirection() == left) ? getX() - 1 : getX() + 1;
        if (getWorld()->isBlockedAt(nextX, getY())) {
            setDirection(getDirection() == left ? right : left);
        }
        else {
            moveTo(nextX, getY());

            // Check if on same square as Player after moving
            if (getWorld()->isPlayerAt(getX(), getY())) {
                getWorld()->playerHit();
            }
        }
    }
}

void Barrel::onBurpHit() {
    setDead();
    getWorld()->playSound(SOUND_ENEMY_DIE);
    getWorld()->increaseScore(100);
}

//==============================================================
// Koopa Implementation
//==============================================================

void Koopa::doSomething() {
    m_ticks++;
    // Step 1: Check if alive
    if (!isAlive()) {
        return;
    }

    // Step 2: Check if on same square as Player
    if (getWorld()->isPlayerAt(getX(), getY())) {
        if (m_freezeTimer <= 0) {
            getWorld()->playerHitByKoopa();
            m_freezeTimer = 50;
            return;
        }
        m_freezeTimer--;
    }

    if (m_ticks == 10) {
        m_ticks = 0;
        int nextX = (getDirection() == left) ? getX() - 1 : getX() + 1;

        // Check if next position is blocked or there is no floor
        if (!(getWorld()->isLadderAt(nextX, getY() - 1)) && (getWorld()->isBlockedAt(nextX, getY()) || !getWorld()->isBlockedAt(nextX, getY() - 1))) {
            // Reverse direction
            setDirection(getDirection() == left ? right : left);
            nextX = (getDirection() == left) ? getX() - 1 : getX() + 1;
            return;
        }

        // Move to the next position
        moveTo(nextX, getY());

        // Check for player collision after moving
        if (getWorld()->isPlayerAt(getX(), getY())) {
            getWorld()->playerHitByKoopa();
        }
    }
}



void Koopa::onBurpHit() {
    setDead();
    getWorld()->playSound(SOUND_ENEMY_DIE);
    getWorld()->increaseScore(100);
    if (randInt(1, 3) == 1) {
        getWorld()->createGoodie(getX(), getY());
    }
}

//============================================
// Garlic implementation
//============================================

void Garlic::doSomething() {
    if (!isAlive())
        return;
    if (getWorld()->isPlayerAt(getX(), getY())) {
        setDead();
        getWorld()->getGarlic();
    }
}

//===========================================
// Extra Life Goodie
//===========================================

void ExtraLifeGoodie::doSomething() {
    if (!isAlive()) {
        return;
    }
    if (getWorld()->isPlayerAt(getX(), getY())) {
        setDead();
        getWorld()->getLife();
    }
}

//============================================
// Bonfire
//============================================

void Bonfire::doSomething() {
    increaseAnimationNumber();
    if (getWorld()->isPlayerAt(getX(), getY())) {
        getWorld()->playerHit();
    }
    if (getWorld()->isBarrelAt(getX(), getY())) {
        getWorld()->BarrelHitByBonfire();
    }
}

//Can be used to identify barrels!!!!



//==============================================
// Kong 
//==============================================

void Kong::createABarrel() {
    int barrelX = getX();
    int barrelY = getY();

    // Adjust position based on Kong's direction
    if (getDirection() == right) {
        barrelX++;
    }
    else {
        barrelX--;
    }

    // Check if the position is valid for barrel placement
    if (!getWorld()->isBlockedAt(barrelX, barrelY)) {
        Barrel* newBarrel = new Barrel(getWorld(), getDirection(), barrelX, barrelY);
        getWorld()->addActor(newBarrel);
    }
}

void Kong::doSomething() {
    if (!isAlive()) {
        return;
    }
    increaseAnimationNumber();

    if (getWorld()->distanceFromPlayer(this) <= 2) {
        m_fleeing = true;
    }

    // Handle barrel throwing if not fleeing
    if (!m_fleeing) {
        m_ticks++;
        int N = getWorld()->getN();
        if (m_ticks >= N) {
            createABarrel();
            m_ticks = 0;
            getWorld()->playSound(SOUND_ENEMY_DIE); 
        }
    }
    // Step 5: Handle fleeing behavior every 5 ticks
    else {
        m_ticks++;
        if (m_ticks % 5 == 0) {
            m_ticks = 0;

            // Attempt to move up by one square
            if (getY() < VIEW_HEIGHT - 1) {
                moveTo(getX(), getY() + 1);
            }
            else {
                // Kong reached the top of the screen
                getWorld()->increaseScore(1000);
                getWorld()->playSound(SOUND_FINISHED_LEVEL);
                getWorld()->setGameStatText("Level Complete!");
                getWorld()->finishLevel(); 
            }
        }
    }
}


