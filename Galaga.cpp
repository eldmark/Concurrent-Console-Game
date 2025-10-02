#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <string>
#include "Pantalla.h"
#include "Nave.h"
#include "Enemigo.h"
#include <pthread.h>

using namespace std;

// Estructura para los puntajes
struct Score
{
    string name;
    int points;
};

// Vector global para almacenar puntajes
vector<Score> highScores;

// Estructura para disparos individuales de cada enemigo
struct EnemyShot {
    int x;
    int y;
    bool active;
    
    EnemyShot(int sx, int sy) : x(sx), y(sy), active(true) {}
};

struct PlayerThreadData {
    int* naveX;
    int* naveY;
    vector<pair<int, int>>* disparos;
    pthread_mutex_t* mutex;
    bool* gameRunning;
    bool* inGame;
    int* score;
    char lastKey;
    bool hasNewKey;
    pthread_mutex_t keyMutex;
    
    PlayerThreadData() {
        pthread_mutex_init(&keyMutex, NULL);
        lastKey = 0;
        hasNewKey = false;
    }
    
    ~PlayerThreadData() {
        pthread_mutex_destroy(&keyMutex);
    }
};

// ---------------- FUNCIONES AUXILIARES PARA LINUX ----------------

int getchLinux()
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // sin buffer, sin eco
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int kbhit()
{
    termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// Mover cursor
void gotoxy(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

// Colores ANSI
void setColor(int color)
{
    switch (color)
    {
    case 7:
        printf("\033[0m");
        break; // Normal
    case 10:
        printf("\033[92m");
        break; // Verde brillante
    case 11:
        printf("\033[96m");
        break; // Cyan brillante
    case 12:
        printf("\033[91m");
        break; // Rojo brillante
    case 13:
        printf("\033[95m");
        break; // Magenta
    case 14:
        printf("\033[93m");
        break; // Amarillo brillante
    case 15:
        printf("\033[97m");
        break; // Blanco brillante
    default:
        printf("\033[0m");
        break; // Normal
    }
}

// Función para mostrar iconos de control
void drawControlIcons()
{
    setColor(13);
    // Magenta // Icono mover izquierda
    gotoxy(5, 8);
    cout << "┌─────────────┐";
    gotoxy(5, 9);
    cout << "│  ← Mover ←  │";
    gotoxy(5, 10);
    cout << "│      A      │";
    gotoxy(5, 11);
    cout << "└─────────────┘"; // Icono mover derecha
    gotoxy(5, 13);
    cout << "┌─────────────┐";
    gotoxy(5, 14);
    cout << "│  → Mover →  │";
    gotoxy(5, 15);
    cout << "│     D       │";
    gotoxy(5, 16);
    cout << "└─────────────┘"; // Icono disparar
    gotoxy(59, 8);
    cout << "┌──────────────┐";
    gotoxy(59, 9);
    cout << "│ ↑ DISPARAR ↑ │";
    gotoxy(59, 10);
    cout << "│   [______]   │";
    gotoxy(59, 11);
    cout << "│   [ESPACIO]  │";
    gotoxy(59, 12);

    cout << "└──────────────┘"; // Nave del jugador
    setColor(11);

    gotoxy(60, 13);
    cout << "┌─────────────┐";
    gotoxy(60, 14);
    cout << "│     NAVE    │";
    gotoxy(60, 15);
    cout << "│      A      │";
    gotoxy(60, 16);
    cout << "└─────────────┘";
}

// Ocultar cursor
void hideCursor()
{
    printf("\033[?25l");
}

// Mostrar cursor
void showCursor()
{
    printf("\033[?25h");
}

// Limpiar pantalla
void clearScreen()
{
    printf("\033[2J\033[H");
}

// ---------------- INTERFAZ VISUAL ----------------

// Marco decorativo
void drawFrame()
{
    setColor(11); // Cyan brillante
    // Marco superior
    gotoxy(0, 0);
    for (int i = 0; i < 80; i++)
        cout << "═";

    // Marco inferior
    gotoxy(0, 24);
    for (int i = 0; i < 80; i++)
        cout << "═";

    // Marco lateral
    for (int i = 1; i < 24; i++)
    {
        gotoxy(0, i);
        cout << "║";
        gotoxy(79, i);
        cout << "║";
    }

    // Esquinas
    gotoxy(0, 0);
    cout << "╔";
    gotoxy(79, 0);
    cout << "╗";
    gotoxy(0, 24);
    cout << "╚";
    gotoxy(79, 24);
    cout << "╝";
}

// ------------PANTALLA DE INICIO Y NOMBRE DE JUGADOR ------------
void showSplashScreen()
{
    clearScreen();
    drawFrame();

    setColor(15); // Blanco brillante
    gotoxy(15, 8);
    cout << "  ██████   █████  ██       █████   ██████   █████ ";
    gotoxy(15, 9);
    cout << " ██       ██   ██ ██      ██   ██ ██       ██   ██";
    gotoxy(15, 10);
    cout << " ██   ███ ███████ ██      ███████ ██   ███ ███████";
    gotoxy(15, 11);
    cout << " ██    ██ ██   ██ ██      ██   ██ ██    ██ ██   ██";
    gotoxy(15, 12);
    cout << "  ██████  ██   ██ ███████ ██   ██  ██████  ██   ██";

    setColor(10);
    gotoxy(20, 18);
    cout << "Presiona cualquier tecla para continuar...";

    setColor(7);
    getchLinux();
}

/// Solicitar nombre de jugador
/// @param isVictory
/// @return String con el nombre del jugador
string getPlayerName(bool isVictory = false)
{
    clearScreen();
    drawFrame();

    if (isVictory)
        setColor(10); // Verde brillante para darle brillo a la victoria
    else
        setColor(14); // Amarillo para game over dramático

    gotoxy(30, 10);
    if (isVictory)
        cout << "¡VICTORIA!";
    else
        cout << "¡GAME OVER!";

    setColor(15);
    gotoxy(25, 12);
    cout << "Ingresa tu nombre (max 15 chars): ";

    // Mostrar cursor para escribir
    showCursor();
    setColor(10);

    string name;
    char c;
    gotoxy(25, 14);

    // Leer caracteres uno por uno
    while (true)
    {
        c = getchLinux();

        if (c == '\n' || c == '\r')
        { // Enter
            break;
        }
        else if (c == 127 || c == 8)
        { // Backspace
            if (name.length() > 0)
            {
                name.pop_back();
                gotoxy(25, 14);
                cout << string(20, ' '); // Limpiar línea
                gotoxy(25, 14);
                cout << name;
            }
        }
        else if (c >= 32 && c <= 126 && name.length() < 15)
        { // Caracteres imprimibles
            name += c;
            cout << c;
        }
    }

    hideCursor();

    if (name.empty())
    {
        name = "ANONIMO";
    }

    return name;
}

// --------------------- PANTALLA DE PUNTAJES ----------------
void showScoresScreen()
{
    clearScreen();
    drawFrame();

    setColor(11);
    gotoxy(32, 3);
    cout << "MEJORES PUNTAJES";

    setColor(14);
    gotoxy(30, 4);
    cout << "━━━━━━━━━━━━━━━━━━━━";

    setColor(15);

    if (highScores.empty())
    {
        gotoxy(30, 12);
        cout << "No hay puntajes aún";
    }
    else
    {
        // Ordenar puntajes de mayor a menor
        sort(highScores.begin(), highScores.end(),
             [](const Score &a, const Score &b)
             { return a.points > b.points; });

        int y = 7;
        for (int i = 0; i < min(10, (int)highScores.size()); i++)
        {
            gotoxy(20, y);
            cout << (i + 1) << ". " << highScores[i].name;

            // Puntos alineados a la derecha
            string pointsStr = to_string(highScores[i].points);
            gotoxy(60 - pointsStr.length(), y);
            cout << pointsStr;

            y += 2;
        }
    }

    setColor(10);
    gotoxy(24, 22);
    cout << "Presiona cualquier tecla para regresar...";

    setColor(7);
    getchLinux();
}

// ---------------- SISTEMA DE ENEMIGOS CON HILOS MEJORADO ----------------

// Declaración forward
struct EnemySystem;

struct EnemyThreadData {
    Enemigo* enemigo;
    int* direccionX;
    EnemySystem* system;
    pthread_mutex_t* mutex;
    int enemyIndex;
    
    // Disparos del enemigo
    vector<EnemyShot> shots;
    pthread_mutex_t shotMutex;
    
    bool isActive();

    //Constructores
    EnemyThreadData() {
        pthread_mutex_init(&shotMutex, NULL);
    }
    
    EnemyThreadData(Enemigo* e, int* dir, EnemySystem* sys, pthread_mutex_t* mtx, int idx) 
        : enemigo(e), direccionX(dir), system(sys), mutex(mtx), enemyIndex(idx) {
        pthread_mutex_init(&shotMutex, NULL);
    }
    

    ~EnemyThreadData() {
        pthread_mutex_destroy(&shotMutex);
    }
};

struct EnemySystem {
    vector<Enemigo> enemigos;
    vector<int> direccionX;
    vector<pthread_t> threads;
    vector<bool> threadActive;
    vector<EnemyThreadData*> threadData;
    pthread_mutex_t systemMutex;
    
    // Constructor y destructor
    EnemySystem();
    ~EnemySystem();
    
    void cleanup();
    bool isThreadActive(int index);
    void setThreadActive(int index, bool value);
    void createEnemies(int num);
    bool checkCollisionWithPlayer(int naveX, int naveY);
    bool checkInvasion();
    void removeEnemy(int index);
    void removeEnemyByPosition(int x, int y);
    size_t size() const;
    void drawEnemies();
    vector<pair<int, int>> getEnemyPositions();
    void drawAllEnemyShots();
    bool checkAnyShotCollisionWithPlayer(int naveX, int naveY);
};


bool EnemyThreadData::isActive() {
    return system->isThreadActive(enemyIndex);
}

// Función del hilo del enemigo MEJORADA
void* enemyThread(void* arg) {
    EnemyThreadData* data = static_cast<EnemyThreadData*>(arg);
    
    // Semilla única para cada hilo
    unsigned int seed = time(NULL) + (unsigned int)data->enemyIndex;
    
    int movementCounter = 0;
    int shootCounter = 0;
    const int MOVEMENT_INTERVAL = 10; // Moverse cada 5 iteraciones
    const int SHOOT_INTERVAL = 30; // Disparar cada 30 iteraciones
    const int SHOT_MOVE_INTERVAL = 2; // Mover disparo cada 2 iteraciones
    int shotMoveCounter = 0;

    while (data->isActive()) {
        usleep(33000); // ~30 FPS (más lento para dar chance al jugador)
        
        movementCounter++;
        shootCounter++;
        shotMoveCounter++;
        if (movementCounter >= MOVEMENT_INTERVAL) {
            movementCounter = 0;
            
            // Intentar obtener el mutex sin bloquear
            if(pthread_mutex_trylock(data->mutex) == 0) {
                // Verificar de nuevo si todavía activo después de obtener el lock
                if (!data->system->isThreadActive(data->enemyIndex)) {
                    pthread_mutex_unlock(data->mutex);
                    break;
                }

                Enemigo& enemigo = data->system->enemigos[data->enemyIndex];
                int currentX = enemigo.getX();
                int currentY = enemigo.getY();
                
                // Movimiento más simple y predecible
                int movimiento = rand_r(&seed) % 7; // 0-7
                
                switch (movimiento) {
                    case 0:
                        
                    case 1: // Izquierda (25% probabilidad)
                        if (currentX > 3) enemigo.moveLeft(1); 
                        break;
                    case 2: 
                        
                    case 3: // Derecha (25% probabilidad)
                        if (currentX < 75) enemigo.moveRight(1); 
                        break;
                    case 4: 
                        
                    case 5: // Abajo (25% probabilidad)
                        if (currentY < 22) enemigo.moveDown(1); 
                        break;
                    case 6: // Arriba (12.5% probabilidad)
                        if (currentY > 2) enemigo.moveUp(1); 
                        break;

                }
                pthread_mutex_unlock(data->mutex);
            }
            // Si no podemos obtener el mutex, omitimos este movimiento
        }

        if (shotMoveCounter >= SHOT_MOVE_INTERVAL) {
            shotMoveCounter = 0;
            pthread_mutex_lock(&data->shotMutex);
            for (int i = data->shots.size() - 1; i >= 0; i--) {
                if (data->shots[i].active) {
                    data->shots[i].y++;
                    if (data->shots[i].y > 23) {
                        data->shots[i].active = false;
                    }
                }
            }
            // Eliminar disparos inactivos
            data->shots.erase(remove_if(data->shots.begin(), data->shots.end(),
                                        [](const EnemyShot& s) { return !s.active; }),
                              data->shots.end());
            pthread_mutex_unlock(&data->shotMutex);
        }

        if (shootCounter>= SHOOT_INTERVAL) {
            shootCounter = 0;
            // Disparar
            if(rand_r(&seed) % 100 < 70) { // 70% de probabilidad de disparar
                if(pthread_mutex_trylock(data->mutex) == 0) {
                    if (data->system->isThreadActive(data->enemyIndex)) {
                        int currentX = data->system->enemigos[data->enemyIndex].getX();
                        int currentY = data->system->enemigos[data->enemyIndex].getY();
                        pthread_mutex_unlock(data->mutex);

                        if (currentY<23 && currentY>2){
                            pthread_mutex_unlock(data->mutex);

                            if(data->shots.size() < 5) { // Limitar número de disparos activos
                                pthread_mutex_lock(&data->shotMutex);

                                if(data->shots.size() < 3){
                                    data->shots.emplace_back(currentX, currentY + 1);
                                }
                                pthread_mutex_unlock(&data->shotMutex);
                            }


                        }
                    } else {
                        pthread_mutex_unlock(data->mutex);
                    }
                }
            }
        }
    }
    
    return NULL;
}

// ============= HILO DEL JUGADOR =============
void* playerThread(void* arg) {
    PlayerThreadData* data = static_cast<PlayerThreadData*>(arg);
    
    while (*(data->gameRunning) && *(data->inGame)) {
        usleep(16000); // ~60 FPS
        
        // Verificar si hay tecla presionada
        pthread_mutex_lock(&data->keyMutex);
        if (data->hasNewKey) {
            char tecla = data->lastKey;
            data->hasNewKey = false;
            pthread_mutex_unlock(&data->keyMutex);
            
            // Procesar la tecla
            pthread_mutex_lock(data->mutex);
            
            switch (tecla) {
                case 'a':
                case 'A':
                    if (*(data->naveX) > 2)
                        (*(data->naveX))--;
                    break;
                case 'd':
                case 'D':
                    if (*(data->naveX) < 77)
                        (*(data->naveX))++;
                    break;
                case ' ':
                    if (data->disparos->size() < 5)
                        data->disparos->push_back({*(data->naveX), *(data->naveY) - 1});
                    break;
            }
            
            pthread_mutex_unlock(data->mutex);
        } else {
            pthread_mutex_unlock(&data->keyMutex);
        }
    }
    
    return NULL;
}

// Implementaciones de EnemySystem MEJORADAS
EnemySystem::EnemySystem() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&systemMutex, &attr);
    pthread_mutexattr_destroy(&attr);
    drawAllEnemyShots();
    checkAnyShotCollisionWithPlayer(0, 0);
}

EnemySystem::~EnemySystem() {
    cleanup();
    pthread_mutex_destroy(&systemMutex);
}

void EnemySystem::cleanup() {
    // Detener todos los hilos
    for (size_t i = 0; i < threadActive.size(); i++) {
        setThreadActive(i, false);
    }
    
    // Esperar a que los hilos terminen
    for (size_t i = 0; i < threads.size(); i++) {
        if (threads[i] != 0) {
            pthread_join(threads[i], NULL);
        }
    }
    
    // Limpiar datos
    pthread_mutex_lock(&systemMutex);
    for (auto data : threadData) {
        if (data) {
            delete data;
        }
    }
    
    threads.clear();
    threadActive.clear();
    threadData.clear();
    enemigos.clear();
    
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::isThreadActive(int index) {
    pthread_mutex_lock(&systemMutex);
    bool ret = (index >= 0 && index < (int)threadActive.size()) && threadActive[index];
    pthread_mutex_unlock(&systemMutex);
    return ret;
}

void EnemySystem::setThreadActive(int index, bool value) {
    pthread_mutex_lock(&systemMutex);
    if (index >= 0 && index < (int)threadActive.size()) {
        threadActive[index] = value;
    }
    pthread_mutex_unlock(&systemMutex);
}

void EnemySystem::createEnemies(int num) {
    cleanup();
    
    pthread_mutex_lock(&systemMutex);
    
    // Limitar número máximo para evitar sobrecarga
    if (num > 15) num = 15;
    
    for (int i = 0; i < num; i++) {
        enemigos.emplace_back(i * 8 + 10, 3 + (i % 3));
        threadActive.push_back(true);
        threads.push_back(0);
        
        EnemyThreadData* data = new EnemyThreadData{
            &enemigos.back(),
            &direccionX.emplace_back(1),
            this,
            &systemMutex,
            i
        };
        threadData.push_back(data);
    }
    
    pthread_mutex_unlock(&systemMutex);
    
    // Crear hilos con verificación de error mejorada
    for (int i = 0; i < num; i++) {
        if (pthread_create(&threads[i], NULL, &enemyThread, threadData[i]) != 0) {
            cerr << "Error creando hilo para enemigo " << i << endl;
            setThreadActive(i, false);
            
            // Si falla la creación del hilo, eliminar el enemigo correspondiente
            pthread_mutex_lock(&systemMutex);
            if (i < (int)enemigos.size()) {
                enemigos.erase(enemigos.begin() + i);
                delete threadData[i];
                threadData.erase(threadData.begin() + i);
                threadActive.erase(threadActive.begin() + i);
                threads.erase(threads.begin() + i);
            }
            pthread_mutex_unlock(&systemMutex);
            
            // Reindexar los elementos restantes
            for (int j = i; j < (int)threadData.size(); j++) {
                if (threadData[j]) {
                    threadData[j]->enemyIndex = j;
                }
            }
        }
    }
}

// Dibujar todos los disparos enemigos
void EnemySystem::drawAllEnemyShots() {
    pthread_mutex_lock(&systemMutex);
    
    setColor(12); // Rojo brillante
    for (auto data : threadData) {
        if (data) {
            pthread_mutex_lock(&data->shotMutex);
            
            for (auto &shot : data->shots) {
                if (shot.active) {
                    gotoxy(shot.x, shot.y);
                    cout << "v";
                }
            }
            
            pthread_mutex_unlock(&data->shotMutex);
        }
    }
    
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::checkCollisionWithPlayer(int naveX, int naveY) {
    pthread_mutex_lock(&systemMutex);
    
    bool collision = false;
    for (auto &enemigo : enemigos) {
        if (enemigo.getX() == naveX && enemigo.getY() == naveY) {
            collision = true;
            break;
        }
    }
    
    pthread_mutex_unlock(&systemMutex);
    return collision;
}

bool EnemySystem::checkInvasion() {
    pthread_mutex_lock(&systemMutex);
    
    bool invasion = false;
    for (auto &enemigo : enemigos) {
        if (enemigo.getY() >= 22) {
            invasion = true;
            break;
        }
    }
    
    pthread_mutex_unlock(&systemMutex);
    return invasion;
}

void EnemySystem::removeEnemy(int index) {
    if (index < 0 || index >= (int)enemigos.size()) return;
    
    // Detener el hilo PRIMERO
    setThreadActive(index, false);
    
    // Unir al hilo para esperar a que termine completamente
    if (index < (int)threads.size() && threads[index] != 0) {
        pthread_join(threads[index], NULL);
    }
    
    pthread_mutex_lock(&systemMutex);
    
    // Eliminar elementos
    if (index < (int)enemigos.size()) {
        enemigos.erase(enemigos.begin() + index);
    }
    
    // Reindexar los threadData restantes
    for (int i = index + 1; i < (int)threadData.size(); i++) {
        if (threadData[i]) {
            threadData[i]->enemyIndex = i - 1;
        }
    }
    
    if (index < (int)threadData.size()) {
        delete threadData[index];
        threadData.erase(threadData.begin() + index);
    }
    
    if (index < (int)threadActive.size()) {
        threadActive.erase(threadActive.begin() + index);
    }
    
    if (index < (int)threads.size()) {
        threads.erase(threads.begin() + index);
    }
    
    pthread_mutex_unlock(&systemMutex);
}

void EnemySystem::removeEnemyByPosition(int x, int y) {
    pthread_mutex_lock(&systemMutex);
    
    for (int i = enemigos.size() - 1; i >= 0; i--) {
        if (enemigos[i].getX() == x && enemigos[i].getY() == y) {
            pthread_mutex_unlock(&systemMutex); // IMPORTANTE: liberar antes de removeEnemy
            removeEnemy(i);
            return;
        }
    }
    
    pthread_mutex_unlock(&systemMutex);
}

size_t EnemySystem::size() const {
    return enemigos.size();
}

void EnemySystem::drawEnemies() {
    pthread_mutex_lock(&systemMutex);
    
    setColor(12);
    for (int i = 0; i < (int)enemigos.size(); i++) {
        gotoxy(enemigos[i].getX(), enemigos[i].getY());
        // Carácter más simple para mejor rendimiento
        cout << "X";
    }
    
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::checkAnyShotCollisionWithPlayer(int naveX, int naveY) {
    pthread_mutex_lock(&systemMutex);
    
    bool hit = false;
    for (auto data : threadData) {
        if (data) {
            pthread_mutex_lock(&data->shotMutex);
            
            for (int i = data->shots.size() - 1; i >= 0; i--) {
                if (data->shots[i].x == naveX && data->shots[i].y == naveY) {
                    data->shots.erase(data->shots.begin() + i);
                    hit = true;
                    pthread_mutex_unlock(&data->shotMutex);
                    pthread_mutex_unlock(&systemMutex);
                    return true;
                }
            }
            
            pthread_mutex_unlock(&data->shotMutex);
        }
    }
    
    pthread_mutex_unlock(&systemMutex);
    return hit;
}

vector<pair<int, int>> EnemySystem::getEnemyPositions() {
    pthread_mutex_lock(&systemMutex);
    
    vector<pair<int, int>> positions;
    for (auto &enemigo : enemigos) {
        positions.push_back({enemigo.getX(), enemigo.getY()});
    }
    
    pthread_mutex_unlock(&systemMutex);
    return positions;
}

// ---------------- GAME LOOP ----------------

void runGame(int enemyCount, int wavesToWin)
{
    bool gameRunning = true;
    bool inGame = true;
    int score = 0;
    int naveX = 40;
    int naveY = 20;
    int match = 0;

    vector<pair<int, int>> disparos;
    EnemySystem enemySystem;

    // Crear primera oleada de enemigos
    
    pthread_mutex_t playerMutex;
    pthread_mutex_init(&playerMutex, NULL);
    
    PlayerThreadData playerData;
    playerData.naveX = &naveX;
    playerData.naveY = &naveY;
    playerData.disparos = &disparos;
    playerData.mutex = &playerMutex;
    playerData.gameRunning = &gameRunning;
    playerData.inGame = &inGame;
    pthread_t playerThreadHandle;
    pthread_create(&playerThreadHandle, NULL, &playerThread, &playerData);
    enemySystem.createEnemies(enemyCount);

    while (gameRunning)
    {
        if (inGame)
        {
            // Mover disparos
            pthread_mutex_lock(&playerMutex);
            for (int i = disparos.size() - 1; i >= 0; i--)
            {
                disparos[i].second--;
                if (disparos[i].second < 2)
                    disparos.erase(disparos.begin() + i);
            }
            pthread_mutex_unlock(&playerMutex);

            // Colisiones disparos - enemigos
            vector<pair<int, int>> enemyPositions = enemySystem.getEnemyPositions();
            pthread_mutex_lock(&playerMutex);
            for (int i = disparos.size() - 1; i >= 0; i--)
            {
                for (int j = enemyPositions.size() - 1; j >= 0; j--)
                {
                    if (disparos[i].first == enemyPositions[j].first &&
                        disparos[i].second == enemyPositions[j].second)
                    {
                        score += 200;
                        disparos.erase(disparos.begin() + i);
                        enemySystem.removeEnemyByPosition(enemyPositions[j].first, enemyPositions[j].second);
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&playerMutex);
            // Generar nueva oleada si no quedan enemigos
            if (enemySystem.size() == 0)
            {
                match++;
                if (match >= wavesToWin)
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);   
                    // Victoria
                    setColor(10);
                    gotoxy(30, 12);
                    cout << "¡VICTORIA TOTAL!";
                    gotoxy(28, 14);
                    cout << "¡Todos los enemigos eliminados!";
                    setColor(14);
                    gotoxy(32, 16);
                    cout << "BONUS: +1000 puntos";
                    setColor(7);
                    score += 1000;
                    sleep(4);
                    string playerName = getPlayerName(true);
                    highScores.push_back({playerName, score});
                    showScoresScreen();
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                    continue;
                }
                else
                {
                    // Nueva oleada
                    enemySystem.createEnemies(enemyCount);
                }
            }

            // Colisión directa con la nave
            if (enemySystem.checkCollisionWithPlayer(naveX, naveY))
            {
                inGame = false;
                pthread_join(playerThreadHandle, NULL);
                setColor(12);
                gotoxy(30, 12);
                cout << "¡COLISIÓN DIRECTA!";
                gotoxy(35, 14);
                cout << "GAME OVER";
                setColor(7);
                sleep(3);
                if (score > 0)
                {
                    string playerName = getPlayerName();
                    highScores.push_back({playerName, score});
                    showScoresScreen();
                }
                gameRunning = false;
                pthread_mutex_destroy(&playerMutex);
                continue;
            }

            // Invasión completada
            if (enemySystem.checkInvasion())
            {
                inGame = false;
                pthread_join(playerThreadHandle, NULL);
                setColor(12);
                gotoxy(28, 12);
                cout << "¡INVASIÓN COMPLETADA!";
                gotoxy(35, 14);
                cout << "GAME OVER";
                setColor(7);
                sleep(3);
                if (score > 0)
                {
                    string playerName = getPlayerName();
                    highScores.push_back({playerName, score});
                    showScoresScreen();
                }
                gameRunning = false;
                pthread_mutex_destroy(&playerMutex);
                continue;
            }

            // Dibujar todo
            clearScreen();
            drawFrame();

            setColor(14);
            gotoxy(5, 1);
            cout << "ENEMIGOS: " << enemySystem.size();

            setColor(14);
            gotoxy(20, 1);
            cout << "GALAGA - PUNTAJE: " << score;
            
            setColor(11); 
            gotoxy(60, 1);
            cout << "VIDAS: ♥♥♥";

            setColor(7);
            gotoxy(46, 1);
            cout << "OLEADA: " << (match + 1) << "/" << wavesToWin;

            // Disparos
            pthread_mutex_lock(&playerMutex);
            setColor(11);
            for (auto &d : disparos)
            {
                gotoxy(d.first, d.second);
                cout << "|";
            }
            pthread_mutex_unlock(&playerMutex);

            
            // Enemigos
            enemySystem.drawEnemies();
            enemySystem.drawAllEnemyShots();

            // Nave
            pthread_mutex_lock(&playerMutex);
            setColor(10);
            gotoxy(naveX, naveY);
            cout << "A";
            pthread_mutex_unlock(&playerMutex);

            // Instrucciones
            setColor(11);
            gotoxy(7, 22);
            cout << "A/D: Mover - ESPACIO: Disparar - M: Terminar - Q: Salir - P: Pausa";

            // Controles
            if (kbhit())
            {
                int tecla = getchLinux();
                
                // Teclas de control del juego (manejadas por el hilo principal)
                if (tecla == 'm' || tecla == 'M')
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);
                    
                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        showScoresScreen();
                    }
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
                else if (tecla == 'q' || tecla == 'Q')
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
                else if (tecla == 'p' || tecla == 'P')
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);
                }
                else
                {
                    pthread_mutex_lock(&playerData.keyMutex);
                    playerData.lastKey = tecla;
                    playerData.hasNewKey = true;
                    pthread_mutex_unlock(&playerData.keyMutex);
                }
            }
        }
        else
        {
            // Pausa
            clearScreen();
            drawFrame();
            setColor(15);
            gotoxy(30, 8);
            cout << "JUEGO PAUSADO";
            gotoxy(28, 10);
            cout << "PUNTAJE: " << score;
            setColor(14);
            gotoxy(15, 14);
            cout << "ESPACIO - Continuar jugando";
            gotoxy(15, 16);
            cout << "M - Terminar y guardar puntaje";
            gotoxy(15, 18);
            cout << "Q - Salir sin guardar";
            setColor(13);
            gotoxy(8, 21);
            cout << "PUNTUACIÓN: Enemigo destruido = +200pts, Victoria = +1000pts";

            if (kbhit())
            {
                int tecla = getchLinux();
                if (tecla == ' ')
                {
                    inGame = true;

                    pthread_create(&playerThreadHandle, NULL, &playerThread, &playerData);
                }
                else if (tecla == 'm' || tecla == 'M')
                {
                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        showScoresScreen();
                    }
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
                else if (tecla == 'q' || tecla == 'Q')
                {
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

// ---------------- MENÚ PRINCIPAL ----------------
void gameScreen()
{
    // Pantalla inicial
    clearScreen();
    drawFrame();
    setColor(15);
    gotoxy(32, 6);
    cout << "GALAGA";
    setColor(14);
    gotoxy(14, 9);
    cout << "Selecciona tu modo de juego  ";
    gotoxy(14, 10);
    cout << "1 - Modo 1 (5 enemigos por oleada, 5 oleadas)";
    gotoxy(14, 11);
    cout << "2 - Modo 2 (8 enemigos por oleada, 5 oleadas)";
    gotoxy(14, 12);
    cout << "3 - Modo 3 (10 enemigos por oleada, 5 oleadas)";

    gotoxy(14, 14);
    cout << "ESPACIO - Disparar ";
    gotoxy(14, 15);
    cout << "A/D - Mover nave";
    gotoxy(14, 16);
    cout << "M - Terminar partida y guardar puntaje";
    gotoxy(14, 17);
    cout << "Q - Salir sin guardar";
    setColor(13);
    gotoxy(20, 21);
    cout << "¡Destruye enemigos antes de que te disparen!";
    setColor(10);
    gotoxy(20, 22);
    cout << "Presiona cualquier tecla para comenzar...";

    char gameMode = getchLinux();

    int enemyCount = 5;
    int wavesToWin = 5;

    switch (gameMode)
    {
    case '1':
        enemyCount = 5;
        wavesToWin = 1; // proof
        break;
    case '2':
        enemyCount = 8;
        wavesToWin = 5;
        break;
    case '3':
        enemyCount = 10;
        wavesToWin = 5;
        break;
    default:
        enemyCount = 5;
        wavesToWin = 5;
        break;
    }

    runGame(enemyCount, wavesToWin);
}

// Menú principal
void showMainMenu()
{
    bool menuRunning = true;
    while (menuRunning)
    {
        clearScreen();
        drawFrame();

        setColor(15);
        gotoxy(30, 8);
        cout << "MENU PRINCIPAL GALAGA";

        setColor(10);
        gotoxy(32, 12);
        cout << "1. Iniciar Juego";

        gotoxy(32, 13);
        cout << "2. Instrucciones";

        gotoxy(32, 14);
        cout << "3. Puntajes";

        gotoxy(32, 15);
        cout << "4. Creditos";

        gotoxy(32, 16);
        cout << "5. Salir";

        setColor(14);
        gotoxy(25, 20);
        cout << "Selecciona una opcion (1-5): ";

        setColor(7);
        char opcion = getchLinux();

        switch (opcion)
        {
        case '1':
            gameScreen();
            break;
        case '2':
            clearScreen();
            drawFrame();
            setColor(14);
            gotoxy(34, 5);
            cout << "INSTRUCCIONES";
            drawControlIcons();
            setColor(10);
            gotoxy(22, 22);
            cout << "Presiona cualquier tecla para regresar...";
            getchLinux();
            break;
        case '3':
            showScoresScreen();
            break;
        case '4':
            clearScreen();
            drawFrame();
            setColor(15);
            gotoxy(34, 5);
            cout << "CREDITOS";
            setColor(11);
            gotoxy(28, 10);
            cout << "Desarrollado por: ";
            gotoxy(28, 11);
            cout << "Marco Díaz";
            gotoxy(28, 12);
            cout << "Marcelo Detlefsen";
            gotoxy(28, 13);
            cout << "Alejandro Jerez";
            gotoxy(28, 14);
            cout << "Julián Divas";
            gotoxy(28, 15);
            cout << "Curso: Programacion de microprocesadores";
            gotoxy(28, 16);
            cout << "Universidad del Valle de Guatemala";
            setColor(10);
            gotoxy(22, 22);
            cout << "Presiona cualquier tecla para regresar...";
            getchLinux();
            break;
        case '5':
            menuRunning = false;
            break;
        }
    }
}

// ---------------- MAIN ----------------
int main()
{
    hideCursor();
    showSplashScreen();
    showMainMenu();
    showCursor();
    return 0;
}