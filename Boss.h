#ifndef BOSS_H
#define BOSS_H

#include <pthread.h>
#include <vector>
#include "Enemigo.h"
#include "Pantalla.h"

// Tipos de jefes
enum BossType {
    BOSS_TYPE_1 = 1,  // Jefe básico
    BOSS_TYPE_2 = 2,  // Jefe intermedio
    BOSS_TYPE_3 = 3   // Jefe avanzado
};

// Estructura para los disparos del jefe
struct BossShot {
    int x;
    int y;
    bool active;
    BossShot(int sx, int sy) : x(sx), y(sy), active(true) {}
};

// Estructura para cada escolta
struct Escort {
    Enemigo enemigo;
    int offsetX;
    int offsetY;
    bool isAlive;
    std::vector<BossShot> shots;
    pthread_mutex_t shotMutex;
    char symbol;  // Símbolo visual de la escolta
    
    Escort(int x, int y, int oX, int oY, char sym) 
        : enemigo(x, y), offsetX(oX), offsetY(oY), isAlive(true), symbol(sym) {
        pthread_mutex_init(&shotMutex, NULL);
    }
    
    ~Escort() {
        pthread_mutex_destroy(&shotMutex);
    }
};

// Estructura del Jefe
class Boss {
private:
    int xpos;
    int ypos;
    int health;
    int maxHealth;
    bool isActive;
    BossType type;
    
    // Escoltas
    std::vector<Escort*> escorts;
    
    // Sincronización
    pthread_mutex_t bossMutex;
    pthread_cond_t attackSignal;
    
    // Estado de ataque coordinado
    bool attackReady;
    int attackType;
    
    // Disparos del jefe
    std::vector<BossShot> bossShots;
    pthread_mutex_t shotMutex;

public:
    Boss(int startX, int startY, int hp, BossType bossType);
    ~Boss();
    
    // Movimiento
    void moveDown(int pasos);
    void moveUp(int pasos);
    void moveLeft(int pasos);
    void moveRight(int pasos);
    
    // Dibujo
    void draw();
    void drawEscorts();
    void drawAllShots();
    void drawHealthBar();
    
    // Getters
    int getX() const { return xpos; }
    int getY() const { return ypos; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    bool getIsActive() const { return isActive; }
    BossType getType() const { return type; }
    
    // Daño
    void takeDamage(int damage);
    void destroy();
    
    // Sistema de escoltas
    void createEscorts(int count);
    void updateEscortPositions();
    int getAliveEscortsCount() const;
    bool checkEscortCollision(int x, int y);
    void removeEscortAt(int x, int y);
    
    // Colisiones mejoradas según tipo de jefe
    bool checkBossCollision(int x, int y);
    
    // Sistema de ataque coordinado
    void signalAttack(int type);
    void waitForAttackSignal();
    bool isAttackReady() const { return attackReady; }
    int getAttackType() const { return attackType; }
    void clearAttackSignal();
    
    // Disparos
    void shootBoss();
    void shootEscort(int escortIndex);
    void updateShots();
    bool checkShotCollision(int x, int y);
    
    // Acceso a mutex
    pthread_mutex_t* getMutex() { return &bossMutex; }
    pthread_cond_t* getCondition() { return &attackSignal; }
};

// Estructura de datos para hilos
//Hilo de jefe
struct BossThreadData {
    Boss* boss;
    bool* gameRunning;
    pthread_mutex_t* gameMutex;
};

//Hilo de escolta
struct EscortThreadData {
    Boss* boss;
    int escortIndex;
    bool* gameRunning;
};

// Funciones de hilos
void* bossThread(void* arg);
void* escortThread(void* arg);

#endif