#include "Boss.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>

extern void gotoxy(int x, int y);
extern void setColor(int color);
// Constructor
Boss::Boss(int startX, int startY, int hp, BossType bossType) 
    : xpos(startX), ypos(startY), health(hp), maxHealth(hp), 
      isActive(true), attackReady(false), attackType(0), type(bossType) {
    pthread_mutex_init(&bossMutex, NULL);
    pthread_cond_init(&attackSignal, NULL);
    pthread_mutex_init(&shotMutex, NULL);
}
// Destructor
Boss::~Boss() {
    for (auto escort : escorts) {
        delete escort;
    }
    escorts.clear();
    pthread_mutex_destroy(&bossMutex);
    pthread_cond_destroy(&attackSignal);
    pthread_mutex_destroy(&shotMutex);
}
// controles de movimiento
void Boss::moveDown(int pasos) {
    ypos += pasos;
    if (ypos > 18) ypos = 18;
    updateEscortPositions();
}

void Boss::moveUp(int pasos) {
    ypos -= pasos;
    if (ypos < 5) ypos = 5;
    updateEscortPositions();
}

void Boss::moveLeft(int pasos) {
    xpos -= pasos;
    if (xpos < 8) xpos = 8;
    updateEscortPositions();
}

void Boss::moveRight(int pasos) {
    xpos += pasos;
    if (xpos > 71) xpos = 71;
    updateEscortPositions();
}
// dibujar cada tipo de jefe
void Boss::draw() {
    if (!isActive) return;
    
    setColor(14);
    
    switch (type) {
        case BOSS_TYPE_1:
            gotoxy(xpos - 1, ypos - 1);
            std::cout << " ▄▄▄ ";
            gotoxy(xpos - 2, ypos);
            std::cout << "|• •|";
            gotoxy(xpos - 2, ypos + 1);
            std::cout << " \\_/ ";
            break;
            
        case BOSS_TYPE_2:
            gotoxy(xpos - 1, ypos - 1);
            std::cout << " /▀\\";
            gotoxy(xpos - 1, ypos);
            std::cout << "| ⊚ |";
            gotoxy(xpos - 2, ypos + 1);
            std::cout << " \\   /";
            break;
            
        case BOSS_TYPE_3:
            gotoxy(xpos - 1, ypos - 1);
            std::cout << " /⍟\\";
            gotoxy(xpos - 2, ypos);
            std::cout << "| ▣ |";
            gotoxy(xpos - 2, ypos + 1);
            std::cout << "/ ▼ \\";
            break;
    }
}
// barra de vida
void Boss::drawHealthBar() {
    if (!isActive) return;
    
    
    setColor(14);
    gotoxy(2, 3);
    std::cout << "HP: ";
    
    int healthPercent = (health * 100) / maxHealth;
    int bars = (health * 20) / maxHealth;
    
    if (healthPercent > 60) {
        setColor(10);
    } else if (healthPercent > 30) {
        setColor(14);
    } else {
        setColor(12);
    }
    
    std::cout << "[";
    for (int i = 0; i < bars; i++) {
        std::cout << "█";
    }
    for (int i = bars; i < 20; i++) {
        std::cout << " ";
    }
    std::cout << "] " << healthPercent << "%";
}

void Boss::drawEscorts() {
    for (auto escort : escorts) {
        if (escort->isAlive) {
            setColor(13);
            gotoxy(escort->enemigo.getX(), escort->enemigo.getY());
            std::cout << escort->symbol;
        }
    }
}
// dibujar disparos del jefe y escoltas
void Boss::drawAllShots() {
    setColor(12);
    
    pthread_mutex_lock(&shotMutex);
    for (auto& shot : bossShots) {
        if (shot.active) {
            gotoxy(shot.x, shot.y);
            std::cout << "▼";
        }
    }
    pthread_mutex_unlock(&shotMutex);
    
    for (auto escort : escorts) {
        if (escort->isAlive) {
            pthread_mutex_lock(&escort->shotMutex);
            for (auto& shot : escort->shots) {
                if (shot.active) {
                    gotoxy(shot.x, shot.y);
                    std::cout << "v";
                }
            }
            pthread_mutex_unlock(&escort->shotMutex);
        }
    }
}
// controlar daño y destrucción
void Boss::takeDamage(int damage) {
    pthread_mutex_lock(&bossMutex);
    health -= damage;
    if (health <= 0) {
        health = 0;
        isActive = false;
        pthread_cond_broadcast(&attackSignal);
    }
    pthread_mutex_unlock(&bossMutex);
}
// destruir jefe
void Boss::destroy() {
    pthread_mutex_lock(&bossMutex);
    isActive = false;
    pthread_cond_broadcast(&attackSignal);
    pthread_mutex_unlock(&bossMutex);
}
// sistema de escoltas
void Boss::createEscorts(int count) {
    switch (type) {
        case BOSS_TYPE_1:
            for (int i = 0; i < count && i < 4; i++) {
                int positions[][2] = {{-6, 0}, {6, 0}, {-6, 2}, {6, 2}};
                int escortX = xpos + positions[i][0];
                int escortY = ypos + positions[i][1];
                Escort* escort = new Escort(escortX, escortY, positions[i][0], positions[i][1], 'X');
                escorts.push_back(escort);
            }
            break;
            
        case BOSS_TYPE_2:
            for (int i = 0; i < count && i < 4; i++) {
                int positions[][2] = {{-7, -1}, {7, -1}, {-7, 2}, {7, 2}};
                int escortX = xpos + positions[i][0];
                int escortY = ypos + positions[i][1];
                Escort* escort = new Escort(escortX, escortY, positions[i][0], positions[i][1], 'Y');
                escorts.push_back(escort);
            }
            break;
            
        case BOSS_TYPE_3:
            for (int i = 0; i < count && i < 4; i++) {
                int positions[][2] = {{-8, 0}, {8, 0}, {-8, 1}, {8, 1}};
                int escortX = xpos + positions[i][0];
                int escortY = ypos + positions[i][1];
                Escort* escort = new Escort(escortX, escortY, positions[i][0], positions[i][1], 'H');
                escorts.push_back(escort);
            }
            break;
    }
}
//  actualizar posiciones de escoltas
void Boss::updateEscortPositions() {
    for (auto escort : escorts) {
        if (escort->isAlive) {
            int newX = xpos + escort->offsetX;
            int newY = ypos + escort->offsetY;
            
            if (newX < 1) newX = 1;
            if (newX > 78) newX = 78;
            if (newY < 2) newY = 2;
            if (newY > 23) newY = 23;
            
            int currentX = escort->enemigo.getX();
            int currentY = escort->enemigo.getY();
            
            if (newX > currentX) {
                escort->enemigo.moveRight(newX - currentX);
            } else if (newX < currentX) {
                escort->enemigo.moveLeft(currentX - newX);
            }
            
            if (newY > currentY) {
                escort->enemigo.moveDown(newY - currentY);
            } else if (newY < currentY) {
                escort->enemigo.moveUp(currentY - newY);
            }
        }
    }
}
// contar escoltas vivas
int Boss::getAliveEscortsCount() const {
    int count = 0;
    for (auto escort : escorts) {
        if (escort->isAlive) count++;
    }
    return count;
}

//Corroborar la colisión con escoltas
bool Boss::checkEscortCollision(int x, int y) {
    for (auto escort : escorts) {
        if (escort->isAlive && 
            escort->enemigo.getX() == x && 
            escort->enemigo.getY() == y) {
            return true;
        }
    }
    return false;
}
// eliminar escolta en posición específica
void Boss::removeEscortAt(int x, int y) {
    for (auto escort : escorts) {
        if (escort->isAlive && 
            escort->enemigo.getX() == x && 
            escort->enemigo.getY() == y) {
            escort->isAlive = false;
            break;
        }
    }
}
// colisiones mejoradas según tipo de jefe
bool Boss::checkBossCollision(int x, int y) {
    if (!isActive) return false;
    
    switch (type) {
        case BOSS_TYPE_1:
            if (y >= ypos - 1 && y <= ypos + 1) {
                if (x >= xpos - 2 && x <= xpos + 2) {
                    return true;
                }
            }
            break;
            
        case BOSS_TYPE_2:
            if (y >= ypos - 1 && y <= ypos + 1) {
                if (x >= xpos - 1 && x <= xpos + 2) {
                    return true;
                }
            }
            break;
            
        case BOSS_TYPE_3:
            if (y >= ypos - 1 && y <= ypos + 1) {
                if (x >= xpos - 2 && x <= xpos + 2) {
                    return true;
                }
            }
            break;
    }
    
    return false;
}

//Enviar señal a los escoltas para atacasr
void Boss::signalAttack(int type) {
    pthread_mutex_lock(&bossMutex);
    attackType = type;
    attackReady = true;
    pthread_cond_broadcast(&attackSignal);
    pthread_mutex_unlock(&bossMutex);
}
// esperar señal de ataque
void Boss::waitForAttackSignal() {
    pthread_mutex_lock(&bossMutex);
    while (!attackReady && isActive) { //No tocar
        pthread_cond_wait(&attackSignal, &bossMutex);
    }
    pthread_mutex_unlock(&bossMutex);
}
// limpiar señal de ataque
void Boss::clearAttackSignal() {
    pthread_mutex_lock(&bossMutex);
    attackReady = false;
    pthread_mutex_unlock(&bossMutex);
}

// sistema de disparo del jefe
void Boss::shootBoss() {
    if (!isActive) return;
    
    pthread_mutex_lock(&shotMutex);
    if (bossShots.size() < 4) {
        bossShots.emplace_back(xpos, ypos + 2);
    }
    pthread_mutex_unlock(&shotMutex);
}

// disparar escolta específica
void Boss::shootEscort(int escortIndex) {
    if (escortIndex < 0 || escortIndex >= (int)escorts.size()) return;
    if (!escorts[escortIndex]->isAlive) return;
    
    Escort* escort = escorts[escortIndex];
    pthread_mutex_lock(&escort->shotMutex);
    if (escort->shots.size() < 2) {
        escort->shots.emplace_back(
            escort->enemigo.getX(), 
            escort->enemigo.getY() + 1
        );
    }
    pthread_mutex_unlock(&escort->shotMutex);
}
// actualizar disparos del jefe y escoltas
void Boss::updateShots() {
    pthread_mutex_lock(&shotMutex);
    for (int i = bossShots.size() - 1; i >= 0; i--) {
        if (bossShots[i].active) {
            bossShots[i].y++;
            if (bossShots[i].y > 23) {
                bossShots[i].active = false;
            }
        }
    }
    bossShots.erase(
        std::remove_if(bossShots.begin(), bossShots.end(),
            [](const BossShot& s) { return !s.active; }),
        bossShots.end()
    );
    pthread_mutex_unlock(&shotMutex);
    // actualizar disparos de escoltas
    for (auto escort : escorts) {
        if (escort->isAlive) {
            pthread_mutex_lock(&escort->shotMutex);
            for (int i = escort->shots.size() - 1; i >= 0; i--) {
                if (escort->shots[i].active) {
                    escort->shots[i].y++;
                    if (escort->shots[i].y > 23) {
                        escort->shots[i].active = false;
                    }
                }
            }
            escort->shots.erase(
                std::remove_if(escort->shots.begin(), escort->shots.end(),
                    [](const BossShot& s) { return !s.active; }),
                escort->shots.end()
            );
            pthread_mutex_unlock(&escort->shotMutex);
        }
    }
}
// verificar colisiones de disparos
bool Boss::checkShotCollision(int x, int y) {
    pthread_mutex_lock(&shotMutex);
    for (int i = bossShots.size() - 1; i >= 0; i--) {
        if (bossShots[i].active && 
            bossShots[i].x == x && 
            bossShots[i].y == y) {
            bossShots.erase(bossShots.begin() + i);
            pthread_mutex_unlock(&shotMutex);
            return true;
        }
    }
    pthread_mutex_unlock(&shotMutex);
    
    for (auto escort : escorts) {
        if (escort->isAlive) {
            pthread_mutex_lock(&escort->shotMutex);
            for (int i = escort->shots.size() - 1; i >= 0; i--) {
                if (escort->shots[i].active && 
                    escort->shots[i].x == x && 
                    escort->shots[i].y == y) {
                    escort->shots.erase(escort->shots.begin() + i);
                    pthread_mutex_unlock(&escort->shotMutex);
                    return true;
                }
            }
            pthread_mutex_unlock(&escort->shotMutex);
        }
    }
    
    return false;
}
// Hilos para jefe y escoltas
void* bossThread(void* arg) {
    BossThreadData* data = static_cast<BossThreadData*>(arg);
    Boss* boss = data->boss;
    
    unsigned int seed = time(NULL);
    int movementCounter = 0;
    int attackCounter = 0;
    int shotUpdateCounter = 0;
    
    const int MOVEMENT_INTERVAL = 15;
    const int ATTACK_INTERVAL = 40;
    const int SHOT_UPDATE_INTERVAL = 3;
    
    while (*(data->gameRunning) && boss->getIsActive()) {
        usleep(33000);
        
        movementCounter++;
        attackCounter++;
        shotUpdateCounter++;
        // Movimiento aleatorio del jefe
        if (movementCounter >= MOVEMENT_INTERVAL) {
            movementCounter = 0;
            
            pthread_mutex_lock(boss->getMutex());
            int movement = rand_r(&seed) % 10;
            
            if (movement < 3) {
                boss->moveLeft(1);
            } else if (movement < 6) {
                boss->moveRight(1);
            } else if (movement < 8) {
                boss->moveDown(1);
            }
            pthread_mutex_unlock(boss->getMutex());
        }
        
        if (shotUpdateCounter >= SHOT_UPDATE_INTERVAL) {
            shotUpdateCounter = 0;
            boss->updateShots();
        }
        // Ataque coordinado
        if (attackCounter >= ATTACK_INTERVAL) {
            attackCounter = 0;
            
            int attackType = rand_r(&seed) % 3;
            boss->signalAttack(attackType);
            boss->shootBoss();
            
            usleep(100000);
            boss->clearAttackSignal();
        }
    }
    
    return NULL;
}
// Hilo para escoltas
void* escortThread(void* arg) {
    EscortThreadData* data = static_cast<EscortThreadData*>(arg);
    Boss* boss = data->boss;
    int escortIndex = data->escortIndex;
    
    unsigned int seed = time(NULL) + escortIndex;
    
    while (*(data->gameRunning) && boss->getIsActive()) {
        usleep(33000);
        
        boss->waitForAttackSignal();
        
        if (boss->isAttackReady()) {
            int attackType = boss->getAttackType();
            
            switch (attackType) {
                case 0:
                    boss->shootEscort(escortIndex);
                    break;
                    
                case 1:
                    usleep(escortIndex * 100000);
                    boss->shootEscort(escortIndex);
                    boss->shootEscort(escortIndex);
                    break;
                    
                case 2:
                    boss->shootEscort(escortIndex);
                    usleep(150000);
                    boss->shootEscort(escortIndex);
                    break;
            }
        }
    }
    
    return NULL;
}