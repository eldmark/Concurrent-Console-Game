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
#include <fstream>
#include <sys/wait.h>
#include <signal.h>
#include "Pantalla.h"
#include "Nave.h"
#include "Enemigo.h"
#include <pthread.h>
#include "Boss.h"
using namespace std;

/*
Universidad del Valle de Guatemala
Facultad de Ingeniera
Curso: Programación de Microprocesadores
Proyecto Final: Galaga 
Autores:
    - Marcelo Detlefsen 24553
    - Alejandro Jerez 24678
    - Julián Divas 24687
    - Marco Díaz 24229

*/


// Estructura para los puntajes
struct Score
{
    string name;
    int points;
};

// Vector global para almacenar puntajes
vector<Score> highScores;

// Estructura para disparos individuales de cada enemigo
struct EnemyShot
{
    int x;
    int y;
    bool active;

    EnemyShot(int sx, int sy) : x(sx), y(sy), active(true) {}
};

struct PlayerThreadData
{
    int *naveX;
    int *naveY;
    vector<pair<int, int>> *disparos;
    pthread_mutex_t *mutex;
    bool *gameRunning;
    bool *inGame;
    int *score;
    char lastKey;
    bool hasNewKey;
    pthread_mutex_t keyMutex;

    PlayerThreadData()
    {
        pthread_mutex_init(&keyMutex, NULL);
        lastKey = 0;
        hasNewKey = false;
    }

    ~PlayerThreadData()
    {
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
    newt.c_lflag &= ~(ICANON | ECHO);
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

void gotoxy(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

void setColor(int color)
{
    switch (color)
    {
    case 7:
        printf("\033[0m");
        break;
    case 10:
        printf("\033[92m");
        break;
    case 11:
        printf("\033[96m");
        break;
    case 12:
        printf("\033[91m");
        break;
    case 13:
        printf("\033[95m");
        break;
    case 14:
        printf("\033[93m");
        break;
    case 15:
        printf("\033[97m");
        break;
    default:
        printf("\033[0m");
        break;
    }
}

void drawControlIcons()
{
    setColor(13);
    gotoxy(5, 8);
    cout << "┌─────────────┐";
    gotoxy(5, 9);
    cout << "│  ← Mover ←  │";
    gotoxy(5, 10);
    cout << "│      A      │";
    gotoxy(5, 11);
    cout << "└─────────────┘";
    gotoxy(5, 13);
    cout << "┌─────────────┐";
    gotoxy(5, 14);
    cout << "│  → Mover →  │";
    gotoxy(5, 15);
    cout << "│     D       │";
    gotoxy(5, 16);
    cout << "└─────────────┘";
    gotoxy(59, 8);
    cout << "┌──────────────┐";
    gotoxy(59, 9);
    cout << "│ ↑ DISPARAR ↑ │";
    gotoxy(59, 10);
    cout << "│   [______]   │";
    gotoxy(59, 11);
    cout << "│   [ESPACIO]  │";
    gotoxy(59, 12);
    cout << "└──────────────┘";
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

void hideCursor()
{
    printf("\033[?25l");
}

void showCursor()
{
    printf("\033[?25h");
}

void clearScreen()
{
    printf("\033[2J\033[H");
}

void drawFrame()
{
    setColor(11);
    gotoxy(0, 0);
    for (int i = 0; i < 80; i++)
        cout << "═";
    gotoxy(0, 24);
    for (int i = 0; i < 80; i++)
        cout << "═";
    for (int i = 1; i < 24; i++)
    {
        gotoxy(0, i);
        cout << "║";
        gotoxy(79, i);
        cout << "║";
    }
    gotoxy(0, 0);
    cout << "╔";
    gotoxy(79, 0);
    cout << "╗";
    gotoxy(0, 24);
    cout << "╚";
    gotoxy(79, 24);
    cout << "╝";
}

void showSplashScreen()
{
    clearScreen();
    drawFrame();
    setColor(15);
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

string getPlayerName(bool isVictory = false)
{
    clearScreen();
    drawFrame();
    if (isVictory)
        setColor(10);
    else
        setColor(14);
    gotoxy(30, 10);
    if (isVictory)
        cout << "¡VICTORIA!";
    else
        cout << "¡GAME OVER!";
    setColor(15);
    gotoxy(25, 12);
    cout << "Ingresa tu nombre (max 15 chars): ";
    showCursor();
    setColor(10);
    string name;
    char c;
    gotoxy(25, 14);
    while (true)
    {
        c = getchLinux();
        if (c == '\n' || c == '\r')
        {
            break;
        }
        else if (c == 127 || c == 8)
        {
            if (name.length() > 0)
            {
                name.pop_back();
                gotoxy(25, 14);
                cout << string(20, ' ');
                gotoxy(25, 14);
                cout << name;
            }
        }
        else if (c >= 32 && c <= 126 && name.length() < 15)
        {
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
// Nombre del archivo de puntajes
const string SCORES_FILE = "galaga_scores.dat";

// Función para guardar puntajes en archivo
void saveScores() {
    ofstream file(SCORES_FILE);
    if (!file.is_open()) {
        cerr << "Error: No se pudo guardar los puntajes" << endl;
        return;
    }
    
    // Ordenar puntajes antes de guardar
    sort(highScores.begin(), highScores.end(),
        [](const Score &a, const Score &b) { 
            return a.points > b.points; 
        });
    
    // Guardar solo los mejores 10 puntajes
    int count = min(10, (int)highScores.size());
    for (int i = 0; i < count; i++) {
        file << highScores[i].points << " " << highScores[i].name << endl;
    }
    
    file.close();
}

// Función para cargar puntajes desde archivo
void loadScores() {
    ifstream file(SCORES_FILE);
    if (!file.is_open()) {
        // Si el archivo no existe, no hay problema
        return;
    }
    
    highScores.clear();
    
    string name;
    int points;
    
    while (file >> points) {
        file.ignore(); // Ignorar el espacio o salto de línea
        getline(file, name);
        
        if (!name.empty()) {
            highScores.push_back({name, points});
        }
    }
    
    file.close();
}

void showScoresScreen() {
    loadScores();
    
    clearScreen();
    drawFrame();
    setColor(11);
    gotoxy(32, 3);
    cout << "MEJORES PUNTAJES";
    setColor(14);
    gotoxy(30, 4);
    cout << "────────────────────";

    setColor(15);

    if (highScores.empty()) {
        gotoxy(30, 12);
        cout << "No hay puntajes aún";
    } else {
        sort(highScores.begin(), highScores.end(),
            [](const Score &a, const Score &b) { 
                return a.points > b.points; 
            });

        int y = 6;  // Empezar más arriba
        int maxDisplay = min(10, (int)highScores.size());
        
        for (int i = 0; i < maxDisplay; i++) {
            gotoxy(25, y);  // Posición X ajustada
            cout << (i + 1) << ". ";
            
            // Limitar nombre a 12 caracteres
            string displayName = highScores[i].name;
            if (displayName.length() > 12) {
                displayName = displayName.substr(0, 12);
            }
            cout << displayName;
            
            // Alinear puntajes a la derecha
            string pointsStr = to_string(highScores[i].points);
            gotoxy(52, y);  // Posición fija para puntajes
            cout << pointsStr;
            
            y++;  // Incrementar de 1 en 1 en lugar de 2
        }
    }
    setColor(10);
    gotoxy(24, 21);  // Ajustar posición del mensaje
    cout << "Presiona cualquier tecla para regresar...";
    setColor(7);
    getchLinux();
}

// ---------------- SISTEMA DE ENEMIGOS ----------------

struct EnemySystem;

struct EnemyThreadData
{
    Enemigo *enemigo;
    int *direccionX;
    EnemySystem *system;
    pthread_mutex_t *mutex;
    int enemyIndex;
    vector<EnemyShot> shots;
    pthread_mutex_t shotMutex;

    bool isActive();

    EnemyThreadData()
    {
        pthread_mutex_init(&shotMutex, NULL);
    }

    EnemyThreadData(Enemigo *e, int *dir, EnemySystem *sys, pthread_mutex_t *mtx, int idx)
        : enemigo(e), direccionX(dir), system(sys), mutex(mtx), enemyIndex(idx)
    {
        pthread_mutex_init(&shotMutex, NULL);
    }

    ~EnemyThreadData()
    {
        pthread_mutex_destroy(&shotMutex);
    }
};

struct EnemySystem
{
    vector<Enemigo> enemigos;
    vector<int> direccionX;
    vector<pthread_t> threads;
    vector<bool> threadActive;
    vector<EnemyThreadData *> threadData;
    pthread_mutex_t systemMutex;

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

bool EnemyThreadData::isActive()
{
    return system->isThreadActive(enemyIndex);
}

void *enemyThread(void *arg)
{
    EnemyThreadData *data = static_cast<EnemyThreadData *>(arg);
    unsigned int seed = time(NULL) + (unsigned int)data->enemyIndex;
    int movementCounter = 0;
    int shootCounter = 0;
    const int MOVEMENT_INTERVAL = 10;
    const int SHOOT_INTERVAL = 30;
    const int SHOT_MOVE_INTERVAL = 2;
    int shotMoveCounter = 0;

    while (data->isActive())
    {
        usleep(33000);
        movementCounter++;
        shootCounter++;
        shotMoveCounter++;
        
        if (movementCounter >= MOVEMENT_INTERVAL)
        {
            movementCounter = 0;
            if (pthread_mutex_trylock(data->mutex) == 0)
            {
                if (!data->system->isThreadActive(data->enemyIndex))
                {
                    pthread_mutex_unlock(data->mutex);
                    break;
                }
                Enemigo &enemigo = data->system->enemigos[data->enemyIndex];
                int currentX = enemigo.getX();
                int currentY = enemigo.getY();
                int movimiento = rand_r(&seed) % 7;
                switch (movimiento)
                {
                case 0:
                case 1:
                    if (currentX > 3)
                        enemigo.moveLeft(1);
                    break;
                case 2:
                case 3:
                    if (currentX < 75)
                        enemigo.moveRight(1);
                    break;
                case 4:
                case 5:
                    if (currentY < 22)
                        enemigo.moveDown(1);
                    break;
                case 6:
                    if (currentY > 2)
                        enemigo.moveUp(1);
                    break;
                }
                pthread_mutex_unlock(data->mutex);
            }
        }

        if (shotMoveCounter >= SHOT_MOVE_INTERVAL)
        {
            shotMoveCounter = 0;
            pthread_mutex_lock(&data->shotMutex);
            for (int i = data->shots.size() - 1; i >= 0; i--)
            {
                if (data->shots[i].active)
                {
                    data->shots[i].y++;
                    if (data->shots[i].y > 23)
                    {
                        data->shots[i].active = false;
                    }
                }
            }
            data->shots.erase(remove_if(data->shots.begin(), data->shots.end(),
                                        [](const EnemyShot &s)
                                        { return !s.active; }),
                            data->shots.end());
            pthread_mutex_unlock(&data->shotMutex);
        }

        if (shootCounter >= SHOOT_INTERVAL)
        {
            shootCounter = 0;
            if (rand_r(&seed) % 100 < 70)
            {
                if (pthread_mutex_trylock(data->mutex) == 0)
                {
                    if (data->system->isThreadActive(data->enemyIndex))
                    {
                        int currentX = data->system->enemigos[data->enemyIndex].getX();
                        int currentY = data->system->enemigos[data->enemyIndex].getY();
                        pthread_mutex_unlock(data->mutex);
                        if (currentY < 23 && currentY > 2)
                        {
                            pthread_mutex_lock(&data->shotMutex);
                            if (data->shots.size() < 3)
                            {
                                data->shots.emplace_back(currentX, currentY + 1);
                            }
                            pthread_mutex_unlock(&data->shotMutex);
                        }
                    }
                    else
                    {
                        pthread_mutex_unlock(data->mutex);
                    }
                }
            }
        }
    }
    return NULL;
}

void *playerThread(void *arg)
{
    PlayerThreadData *data = static_cast<PlayerThreadData *>(arg);
    while (*(data->gameRunning) && *(data->inGame))
    {
        usleep(16000);
        pthread_mutex_lock(&data->keyMutex);
        if (data->hasNewKey)
        {
            char tecla = data->lastKey;
            data->hasNewKey = false;
            pthread_mutex_unlock(&data->keyMutex);
            pthread_mutex_lock(data->mutex);
            switch (tecla)
            {
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
        }
        else
        {
            pthread_mutex_unlock(&data->keyMutex);
        }
    }
    return NULL;
}

EnemySystem::EnemySystem()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&systemMutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

EnemySystem::~EnemySystem()
{
    cleanup();
    pthread_mutex_destroy(&systemMutex);
}

void EnemySystem::cleanup()
{
    for (size_t i = 0; i < threadActive.size(); i++)
    {
        setThreadActive(i, false);
    }
    for (size_t i = 0; i < threads.size(); i++)
    {
        if (threads[i] != 0)
        {
            pthread_join(threads[i], NULL);
        }
    }
    pthread_mutex_lock(&systemMutex);
    for (auto data : threadData)
    {
        if (data)
        {
            delete data;
        }
    }
    threads.clear();
    threadActive.clear();
    threadData.clear();
    enemigos.clear();
    direccionX.clear();
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::isThreadActive(int index)
{
    pthread_mutex_lock(&systemMutex);
    bool ret = (index >= 0 && index < (int)threadActive.size()) && threadActive[index];
    pthread_mutex_unlock(&systemMutex);
    return ret;
}

void EnemySystem::setThreadActive(int index, bool value)
{
    pthread_mutex_lock(&systemMutex);
    if (index >= 0 && index < (int)threadActive.size())
    {
        threadActive[index] = value;
    }
    pthread_mutex_unlock(&systemMutex);
}

void EnemySystem::createEnemies(int num)
{
    cleanup();
    pthread_mutex_lock(&systemMutex);
    if (num > 15)
        num = 15;
    for (int i = 0; i < num; i++)
    {
        enemigos.emplace_back(i * 8 + 10, 3 + (i % 3));
        threadActive.push_back(true);
        threads.push_back(0);
        direccionX.push_back(1);
        EnemyThreadData *data = new EnemyThreadData(
            &enemigos.back(),
            &direccionX.back(),
            this,
            &systemMutex,
            i);
        threadData.push_back(data);
    }
    pthread_mutex_unlock(&systemMutex);
    for (int i = 0; i < num; i++)
    {
        if (pthread_create(&threads[i], NULL, &enemyThread, threadData[i]) != 0)
        {
            cerr << "Error creando hilo para enemigo " << i << endl;
            setThreadActive(i, false);
            pthread_mutex_lock(&systemMutex);
            if (i < (int)enemigos.size())
            {
                enemigos.erase(enemigos.begin() + i);
                delete threadData[i];
                threadData.erase(threadData.begin() + i);
                threadActive.erase(threadActive.begin() + i);
                threads.erase(threads.begin() + i);
            }
            pthread_mutex_unlock(&systemMutex);
            for (int j = i; j < (int)threadData.size(); j++)
            {
                if (threadData[j])
                {
                    threadData[j]->enemyIndex = j;
                }
            }
        }
    }
}

void EnemySystem::drawAllEnemyShots()
{
    pthread_mutex_lock(&systemMutex);
    setColor(12);
    for (auto data : threadData)
    {
        if (data)
        {
            pthread_mutex_lock(&data->shotMutex);
            for (auto &shot : data->shots)
            {
                if (shot.active)
                {
                    gotoxy(shot.x, shot.y);
                    cout << "v";
                }
            }
            pthread_mutex_unlock(&data->shotMutex);
        }
    }
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::checkCollisionWithPlayer(int naveX, int naveY)
{
    pthread_mutex_lock(&systemMutex);
    bool collision = false;
    for (auto &enemigo : enemigos)
    {
        if (enemigo.getX() == naveX && enemigo.getY() == naveY)
        {
            collision = true;
            break;
        }
    }
    pthread_mutex_unlock(&systemMutex);
    return collision;
}

bool EnemySystem::checkInvasion()
{
    pthread_mutex_lock(&systemMutex);
    bool invasion = false;
    for (auto &enemigo : enemigos)
    {
        if (enemigo.getY() >= 22)
        {
            invasion = true;
            break;
        }
    }
    pthread_mutex_unlock(&systemMutex);
    return invasion;
}

void EnemySystem::removeEnemy(int index)
{
    if (index < 0 || index >= (int)enemigos.size())
        return;
    setThreadActive(index, false);
    if (index < (int)threads.size() && threads[index] != 0)
    {
        pthread_join(threads[index], NULL);
    }
    pthread_mutex_lock(&systemMutex);
    if (index < (int)enemigos.size())
    {
        enemigos.erase(enemigos.begin() + index);
    }
    for (int i = index + 1; i < (int)threadData.size(); i++)
    {
        if (threadData[i])
        {
            threadData[i]->enemyIndex = i - 1;
        }
    }
    if (index < (int)threadData.size())
    {
        delete threadData[index];
        threadData.erase(threadData.begin() + index);
    }
    if (index < (int)threadActive.size())
    {
        threadActive.erase(threadActive.begin() + index);
    }
    if (index < (int)threads.size())
    {
        threads.erase(threads.begin() + index);
    }
    pthread_mutex_unlock(&systemMutex);
}

void EnemySystem::removeEnemyByPosition(int x, int y)
{
    pthread_mutex_lock(&systemMutex);
    for (int i = enemigos.size() - 1; i >= 0; i--)
    {
        if (enemigos[i].getX() == x && enemigos[i].getY() == y)
        {
            pthread_mutex_unlock(&systemMutex);
            removeEnemy(i);
            return;
        }
    }
    pthread_mutex_unlock(&systemMutex);
}

size_t EnemySystem::size() const
{
    return enemigos.size();
}

void EnemySystem::drawEnemies()
{
    pthread_mutex_lock(&systemMutex);
    setColor(12);
    for (int i = 0; i < (int)enemigos.size(); i++)
    {
        gotoxy(enemigos[i].getX(), enemigos[i].getY());
        cout << "X";
    }
    pthread_mutex_unlock(&systemMutex);
}

bool EnemySystem::checkAnyShotCollisionWithPlayer(int naveX, int naveY)
{
    pthread_mutex_lock(&systemMutex);
    bool hit = false;
    for (auto data : threadData)
    {
        if (data)
        {
            pthread_mutex_lock(&data->shotMutex);
            for (int i = data->shots.size() - 1; i >= 0; i--)
            {
                if (data->shots[i].x == naveX && data->shots[i].y == naveY)
                {
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

vector<pair<int, int>> EnemySystem::getEnemyPositions()
{
    pthread_mutex_lock(&systemMutex);
    vector<pair<int, int>> positions;
    for (auto &enemigo : enemigos)
    {
        positions.push_back({enemigo.getX(), enemigo.getY()});
    }
    pthread_mutex_unlock(&systemMutex);
    return positions;
}

bool checkEnemyShotCollision(EnemySystem &enemySystem, int naveX, int naveY, int *vidas)
{
    pthread_mutex_lock(&enemySystem.systemMutex);
    for (auto data : enemySystem.threadData)
    {
        if (data)
        {
            pthread_mutex_lock(&data->shotMutex);
            for (int i = data->shots.size() - 1; i >= 0; i--)
            {
                if (data->shots[i].active &&
                    data->shots[i].x == naveX &&
                    data->shots[i].y == naveY)
                {
                    data->shots.erase(data->shots.begin() + i);
                    (*vidas)--;
                    pthread_mutex_unlock(&data->shotMutex);
                    pthread_mutex_unlock(&enemySystem.systemMutex);
                    return true;
                }
            }
            pthread_mutex_unlock(&data->shotMutex);
        }
    }
    pthread_mutex_unlock(&enemySystem.systemMutex);
    return false;
}

void showLifeLostEffect(int naveX, int naveY, int vidasRestantes)
{
    setColor(12);
    gotoxy(naveX, naveY);
    cout << "*";
    gotoxy(30, 12);
    cout << "¡IMPACTO!";
    gotoxy(28, 13);
    cout << "Vidas restantes: " << vidasRestantes;
    setColor(7);
    usleep(800000);
    gotoxy(30, 12);
    cout << "          ";
    gotoxy(28, 13);
    cout << "                    ";
}

// ---------------- GAME LOOP ----------------
void runGame(int enemyCount, int wavesToWin, int gameMode)
{
    bool gameRunning = true;
    bool inGame = true;
    int score = 0;
    int naveX = 40;
    int naveY = 20;
    int match = 0;
    int vidas = 3;
    bool bossMode = (gameMode == 4); // En modo jefe misterioso, empieza directamente en bossMode

    vector<pair<int, int>> disparos;
    EnemySystem enemySystem;
    Boss *boss = nullptr;
    vector<pthread_t> escortThreads;
    vector<EscortThreadData *> escortData;
    pthread_t bossThreadHandle = 0;
    BossThreadData *bossData = nullptr;

    pthread_mutex_t playerMutex;
    pthread_mutex_init(&playerMutex, NULL);

    PlayerThreadData playerData;
    playerData.naveX = &naveX;
    playerData.naveY = &naveY;
    playerData.disparos = &disparos;
    playerData.mutex = &playerMutex;
    playerData.gameRunning = &gameRunning;
    playerData.inGame = &inGame;
    playerData.score = &score;

    pthread_t playerThreadHandle;
    if (pthread_create(&playerThreadHandle, NULL, &playerThread, &playerData) != 0) {
        cerr << "Error creando hilo del jugador" << endl;
        pthread_mutex_destroy(&playerMutex);
        return;
    }

    // Si es modo jefe misterioso, inicializar un jefe aleatorio
    if (gameMode == 4) {
        // Seleccionar jefe aleatoriamente
        srand(time(NULL));
        int selectedBoss = (rand() % 3) + 1; // 1, 2 o 3
        BossType bossType;
        switch (selectedBoss)
        {
        case 1:
            bossType = BOSS_TYPE_1;
            break;
        case 2:
            bossType = BOSS_TYPE_2;
            break;
        case 3:
            bossType = BOSS_TYPE_3;
            break;
        default:
            bossType = BOSS_TYPE_1;
            break;
        }

        boss = new Boss(40, 8, 15, bossType);
        boss->createEscorts(4);

        bossData = new BossThreadData{boss, &gameRunning, &playerMutex};
        if (pthread_create(&bossThreadHandle, NULL, &bossThread, bossData) != 0) {
            cerr << "Error creando hilo del jefe" << endl;
            delete boss;
            delete bossData;
            pthread_join(playerThreadHandle, NULL);
            pthread_mutex_destroy(&playerMutex);
            return;
        }

        for (int i = 0; i < 4; i++)
        {
            EscortThreadData *data = new EscortThreadData{boss, i, &gameRunning};
            escortData.push_back(data);

            pthread_t thread;
            if (pthread_create(&thread, NULL, &escortThread, data) != 0) {
                cerr << "Error creando hilo de escolta " << i << endl;
                delete data;
                continue;
            }
            escortThreads.push_back(thread);
        }
    } else if (enemyCount > 0) {
        enemySystem.createEnemies(enemyCount);
    }

    while (gameRunning)
    {
        if (inGame)
        {
            // Mover disparos del jugador
            pthread_mutex_lock(&playerMutex);
            for (int i = disparos.size() - 1; i >= 0; i--)
            {
                disparos[i].second--;
                if (disparos[i].second < 2)
                    disparos.erase(disparos.begin() + i);
            }
            pthread_mutex_unlock(&playerMutex);

            // Modo jefe misterioso
            if (bossMode && boss && boss->getIsActive())
            {
                pthread_mutex_lock(&playerMutex);
                for (int i = disparos.size() - 1; i >= 0; i--)
                {
                    int dx = disparos[i].first;
                    int dy = disparos[i].second;

                    if (boss->checkBossCollision(dx, dy))
                    {
                        boss->takeDamage(1);
                        score += 500;
                        disparos.erase(disparos.begin() + i);
                        continue;
                    }

                    if (boss->checkEscortCollision(dx, dy))
                    {
                        boss->removeEscortAt(dx, dy);
                        score += 300;
                        disparos.erase(disparos.begin() + i);
                    }
                }
                pthread_mutex_unlock(&playerMutex);

                if (!boss->getIsActive())
                {
                    boss->destroy();
                    for (auto thread : escortThreads)
                    {
                        if (thread != 0)
                            pthread_join(thread, NULL);
                    }
                    if (bossThreadHandle != 0) {
                        pthread_join(bossThreadHandle, NULL);
                    }

                    for (auto data : escortData)
                        if (data) delete data;
                    escortData.clear();
                    escortThreads.clear();
                    if (bossData) {
                        delete bossData;
                        bossData = nullptr;
                    }
                    if (boss) {
                        delete boss;
                        boss = nullptr;
                    }

                    setColor(10);
                    gotoxy(28, 12);
                    cout << "¡JEFE MISTERIOSO DERROTADO!";
                    gotoxy(30, 13);
                    cout << "BONUS: +2000 puntos";
                    setColor(7);
                    score += 2000;
                    usleep(3000000);

                    gameRunning = false;
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);

                    clearScreen();
                    drawFrame();
                    setColor(10);
                    gotoxy(28, 10);
                    cout << "╔════════════════════════════╗";
                    gotoxy(28, 11);
                    cout << "║ ¡JEFE MISTERIOSO DERROTADO! ║";
                    gotoxy(28, 12);
                    cout << "║   ¡VICTORIA COMPLETA!      ║";
                    gotoxy(28, 13);
                    cout << "╚════════════════════════════╝";
                    setColor(14);
                    gotoxy(32, 15);
                    cout << "BONUS: +2000 puntos";
                    setColor(7);
                    score += 2000;
                    usleep(4000000);

                    string playerName = getPlayerName(true);
                    highScores.push_back({playerName, score});
                    saveScores();
                    showScoresScreen();

                    pthread_mutex_destroy(&playerMutex);
                    return;
                }

                if (boss && boss->checkShotCollision(naveX, naveY))
                {
                    vidas--;
                    
                    if (vidas <= 0)
                    {
                        inGame = false;
                        pthread_join(playerThreadHandle, NULL);

                        if (boss) {
                            boss->destroy();
                            for (auto thread : escortThreads)
                            {
                                if (thread != 0)
                                    pthread_join(thread, NULL);
                            }
                            if (bossThreadHandle != 0) {
                                pthread_join(bossThreadHandle, NULL);
                            }

                            for (auto data : escortData)
                                if (data) delete data;
                            escortData.clear();
                            escortThreads.clear();
                            if (bossData) {
                                delete bossData;
                                bossData = nullptr;
                            }
                            delete boss;
                            boss = nullptr;
                        }

                        setColor(12);
                        gotoxy(30, 12);
                        cout << "¡SIN VIDAS!";
                        gotoxy(35, 14);
                        cout << "GAME OVER";
                        setColor(7);
                        usleep(3000000);

                        if (score > 0)
                        {
                            string playerName = getPlayerName();
                            highScores.push_back({playerName, score});
                            saveScores();
                            showScoresScreen();
                        }

                        pthread_mutex_destroy(&playerMutex);
                        gameRunning = false;
                        return;
                    }
                    else
                    {
                        showLifeLostEffect(naveX, naveY, vidas);
                    }
                }

                if (boss && boss->checkBossCollision(naveX, naveY))
                {
                    vidas = 0;
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);

                    if (boss) {
                        boss->destroy();
                        for (auto thread : escortThreads)
                        {
                            if (thread != 0)
                                pthread_join(thread, NULL);
                        }
                        if (bossThreadHandle != 0) {
                            pthread_join(bossThreadHandle, NULL);
                        }

                        for (auto data : escortData)
                            if (data) delete data;
                        escortData.clear();
                        escortThreads.clear();
                        if (bossData) {
                            delete bossData;
                            bossData = nullptr;
                        }
                        delete boss;
                        boss = nullptr;
                    }

                    setColor(12);
                    gotoxy(28, 12);
                    cout << "¡COLISIÓN CON EL JEFE!";
                    gotoxy(35, 14);
                    cout << "GAME OVER";
                    setColor(7);
                    usleep(3000000);

                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        saveScores();
                        showScoresScreen();
                    }

                    pthread_mutex_destroy(&playerMutex);
                    gameRunning = false;
                    return;
                }
            }
            // Modos normales (sin jefes)
            else
            {
                if (enemySystem.size() == 0 && match < wavesToWin)
                {
                    match++;
                    if (match < wavesToWin)
                    {
                        enemySystem.createEnemies(enemyCount);
                        setColor(10);
                        gotoxy(28, 12);
                        cout << "¡OLEADA " << match + 1 << " COMIENZA!";
                        setColor(7);
                        usleep(2000000);
                    }
                }

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

                if (enemySystem.size() == 0 && match >= wavesToWin)
                {
                    gameRunning = false;
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);

                    clearScreen();
                    drawFrame();
                    setColor(10);
                    gotoxy(28, 10);
                    cout << "╔════════════════════════╗";
                    gotoxy(28, 11);
                    cout << "║ ¡OLEADAS COMPLETADAS!  ║";
                    gotoxy(28, 12);
                    cout << "║  ¡VICTORIA COMPLETA!   ║";
                    gotoxy(28, 13);
                    cout << "╚════════════════════════╝";
                    setColor(14);
                    gotoxy(32, 15);
                    cout << "BONUS: +2000 puntos";
                    setColor(7);
                    score += 2000;
                    usleep(4000000);

                    string playerName = getPlayerName(true);
                    highScores.push_back({playerName, score});
                    saveScores();
                    showScoresScreen();

                    pthread_mutex_destroy(&playerMutex);
                    return;
                }

                // Colisión directa con la nave
                if (enemySystem.checkCollisionWithPlayer(naveX, naveY))
                {
                    vidas--;
                    if (vidas <= 0)
                    {
                        inGame = false;
                        pthread_join(playerThreadHandle, NULL);
                        setColor(12);
                        gotoxy(30, 12);
                        cout << "¡COLISIÓN DIRECTA!";
                        gotoxy(35, 14);
                        cout << "GAME OVER";
                        setColor(7);
                        usleep(3000000);
                        if (score > 0)
                        {
                            string playerName = getPlayerName();
                            highScores.push_back({playerName, score});
                            saveScores();
                            showScoresScreen();
                        }
                        gameRunning = false;
                        pthread_mutex_destroy(&playerMutex);
                        return;
                    }
                    else
                    {
                        showLifeLostEffect(naveX, naveY, vidas);
                    }
                }

                // Verificar colisión con disparos enemigos
                if (checkEnemyShotCollision(enemySystem, naveX, naveY, &vidas))
                {
                    if (vidas <= 0)
                    {
                        inGame = false;
                        pthread_join(playerThreadHandle, NULL);
                        setColor(12);
                        gotoxy(30, 12);
                        cout << "¡SIN VIDAS!";
                        gotoxy(35, 14);
                        cout << "GAME OVER";
                        setColor(7);
                        usleep(3000000);
                        if (score > 0)
                        {
                            string playerName = getPlayerName();
                            highScores.push_back({playerName, score});
                            saveScores();
                            showScoresScreen();
                        }
                        gameRunning = false;
                        pthread_mutex_destroy(&playerMutex);
                        return;
                    }
                    else
                    {
                        showLifeLostEffect(naveX, naveY, vidas);
                    }
                }

                // Invasión completada
                if (enemySystem.checkInvasion())
                {
                    vidas = 0; // Invasión = pérdida automática
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);
                    setColor(12);
                    gotoxy(28, 12);
                    cout << "¡INVASIÓN COMPLETADA!";
                    gotoxy(35, 14);
                    cout << "GAME OVER";
                    setColor(7);
                    usleep(3000000);
                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        saveScores();
                        showScoresScreen();
                    }
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                    return;
                }
            }

            clearScreen();
            drawFrame();

            if (bossMode && boss && boss->getIsActive())
            {
                boss->drawHealthBar();
            }

            setColor(14);
            gotoxy(5, 1);
            if (bossMode && boss)
            {
                if (boss->getAliveEscortsCount() > 0)
                {
                    gotoxy(15, 1);
                    cout << "ESCOLTAS: " << boss->getAliveEscortsCount();
                }
            }
            else
            {
                cout << "ENEMIGOS: " << enemySystem.size();
            }

            setColor(14);
            gotoxy(28, 1);
            cout << "PUNTAJE: " << score;

            setColor(11);
            gotoxy(60, 1);
            cout << "VIDAS: ";
            for (int i = 0; i < vidas; i++)
            {
                cout << "♥";
            }
            for (int i = vidas; i < 3; i++)
            {
                cout << "♡";
            }

            setColor(7);
            gotoxy(43, 1);
            if (bossMode)
            {
                cout << "JEFE MISTERIOSO";
            }
            else
            {
                cout << "OLEADA: " << (match + 1) << "/" << wavesToWin;
            }

            pthread_mutex_lock(&playerMutex);
            setColor(11);
            for (auto &d : disparos)
            {
                gotoxy(d.first, d.second);
                cout << "|";
            }
            pthread_mutex_unlock(&playerMutex);

            if (bossMode && boss && boss->getIsActive())
            {
                boss->draw();
                boss->drawEscorts();
                boss->drawAllShots();
            }
            else
            {
                enemySystem.drawEnemies();
                enemySystem.drawAllEnemyShots();
            }

            pthread_mutex_lock(&playerMutex);
            setColor(10);
            gotoxy(naveX, naveY);
            cout << "A";
            pthread_mutex_unlock(&playerMutex);

            setColor(11);
            gotoxy(7, 22);
            cout << "A/D: Mover - ESPACIO: Disparar - M: Terminar - Q: Salir - P: Pausa";

            if (kbhit())
            {
                int tecla = getchLinux();

                if (tecla == 'm' || tecla == 'M')
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);

                    if (bossMode && boss)
                    {
                        boss->destroy();
                        for (auto thread : escortThreads)
                        {
                            if (thread != 0)
                                pthread_join(thread, NULL);
                        }
                        if (bossThreadHandle != 0) {
                            pthread_join(bossThreadHandle, NULL);
                        }

                        for (auto data : escortData)
                            if (data) delete data;
                        escortData.clear();
                        escortThreads.clear();
                        if (bossData) {
                            delete bossData;
                            bossData = nullptr;
                        }
                        if (boss) {
                            delete boss;
                            boss = nullptr;
                        }
                    }

                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        saveScores();
                        showScoresScreen();
                    }
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
                else if (tecla == 'q' || tecla == 'Q')
                {
                    inGame = false;
                    pthread_join(playerThreadHandle, NULL);

                    if (bossMode && boss)
                    {
                        boss->destroy();
                        for (auto thread : escortThreads)
                        {
                            if (thread != 0)
                                pthread_join(thread, NULL);
                        }
                        if (bossThreadHandle != 0) {
                            pthread_join(bossThreadHandle, NULL);
                        }

                        for (auto data : escortData)
                            if (data) delete data;
                        escortData.clear();
                        escortThreads.clear();
                        if (bossData) {
                            delete bossData;
                            bossData = nullptr;
                        }
                        if (boss) {
                            delete boss;
                            boss = nullptr;
                        }
                    }

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
            cout << "PUNTUACIÓN: Enemigo = +200pts, Escolta = +300pts";
            gotoxy(8, 22);
            cout << "Jefe = +500pts por golpe, Victoria = +2000pts";

            if (kbhit())
            {
                int tecla = getchLinux();
                if (tecla == ' ')
                {
                    inGame = true;
                    if (pthread_create(&playerThreadHandle, NULL, &playerThread, &playerData) != 0) {
                        cerr << "Error recreando hilo del jugador tras pausa" << endl;
                        gameRunning = false;
                        if (bossMode && boss)
                        {
                            boss->destroy();
                            for (auto thread : escortThreads)
                            {
                                if (thread != 0)
                                    pthread_join(thread, NULL);
                            }
                            if (bossThreadHandle != 0) {
                                pthread_join(bossThreadHandle, NULL);
                            }

                            for (auto data : escortData)
                                if (data) delete data;
                            escortData.clear();
                            escortThreads.clear();
                            if (bossData) {
                                delete bossData;
                                bossData = nullptr;
                            }
                            if (boss) {
                                delete boss;
                                boss = nullptr;
                            }
                        }
                        pthread_mutex_destroy(&playerMutex);
                    }
                }
                else if (tecla == 'm' || tecla == 'M')
                {
                    if (bossMode && boss)
                    {
                        boss->destroy();
                        for (auto thread : escortThreads)
                        {
                            if (thread != 0)
                                pthread_join(thread, NULL);
                        }
                        if (bossThreadHandle != 0) {
                            pthread_join(bossThreadHandle, NULL);
                        }

                        for (auto data : escortData)
                            if (data) delete data;
                        escortData.clear();
                        escortThreads.clear();
                        if (bossData) {
                            delete bossData;
                            bossData = nullptr;
                        }
                        if (boss) {
                            delete boss;
                            boss = nullptr;
                        }
                    }

                    if (score > 0)
                    {
                        string playerName = getPlayerName();
                        highScores.push_back({playerName, score});
                        saveScores();
                        showScoresScreen();
                    }
                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
                else if (tecla == 'q' || tecla == 'Q')
                {
                    if (bossMode && boss)
                    {
                        boss->destroy();
                        for (auto thread : escortThreads)
                        {
                            if (thread != 0)
                                pthread_join(thread, NULL);
                        }
                        if (bossThreadHandle != 0) {
                            pthread_join(bossThreadHandle, NULL);
                        }

                        for (auto data : escortData)
                            if (data) delete data;
                        escortData.clear();
                        escortThreads.clear();
                        if (bossData) {
                            delete bossData;
                            bossData = nullptr;
                        }
                        if (boss) {
                            delete boss;
                            boss = nullptr;
                        }
                    }

                    gameRunning = false;
                    pthread_mutex_destroy(&playerMutex);
                }
            }
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

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
    cout << "Selecciona tu modo de juego";
    gotoxy(14, 10);
    cout << "1 - Modo Principiante (5 enemigos, 5 oleadas)";
    gotoxy(14, 11);
    cout << "2 - Modo Intermedio (8 enemigos, 5 oleadas)";
    gotoxy(14, 12);
    cout << "3 - Modo Experto (10 enemigos, 5 oleadas)";
    gotoxy(14, 13);
    cout << "4 - Jefe Misterioso (Batalla contra un jefe aleatorio)";

    setColor(14);
    gotoxy(14, 15);
    cout << "ESPACIO - Disparar";
    gotoxy(14, 16);
    cout << "A/D - Mover nave";

    setColor(10);
    gotoxy(20, 18);
    cout << "Presiona 1, 2, 3 o 4 para comenzar...";

    char gameMode;
    while (true)
    {
        gameMode = getchLinux();
        if (gameMode >= '1' && gameMode <= '4')
            break;
    }

    int enemyCount = 5;
    int wavesToWin = 5;
    int mode = 1;

    switch (gameMode)
    {
    case '1':
        enemyCount = 5;
        wavesToWin = 5;
        mode = 1;
        break;
    case '2':
        enemyCount = 8;
        wavesToWin = 5;
        mode = 2;
        break;
    case '3':
        enemyCount = 10;
        wavesToWin = 5;
        mode = 3;
        break;
    case '4':
        enemyCount = 0;
        wavesToWin = 1; // Solo un jefe
        mode = 4;
        break;
    default:
        enemyCount = 5;
        wavesToWin = 5;
        mode = 1;
        break;
    }

    // Pantalla de preparación
    clearScreen();
    drawFrame();
    setColor(15);
    gotoxy(30, 10);
    if (mode == 4) {
        cout << "JEFE MISTERIOSO SELECCIONADO";
    } else {
        cout << "MODO " << (mode == 1 ? "PRINCIPIANTE" : mode == 2 ? "INTERMEDIO" : "EXPERTO") << " SELECCIONADO";
    }
    setColor(14);
    gotoxy(25, 12);
    cout << "Preparándote para la batalla...";
    setColor(11);
    gotoxy(22, 14);
    if (mode == 4) {
        cout << "¡Prepárate para un jefe sorpresa!";
    } else {
        cout << "Enemigos por oleada: " << enemyCount;
        gotoxy(22, 15);
        cout << "Oleadas hasta victoria: " << wavesToWin;
    }
    setColor(10);
    gotoxy(25, 18);
    cout << "¡Buena suerte, comandante!";
    setColor(7);
    usleep(2000000);

    runGame(enemyCount, wavesToWin, mode);
}

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
            gotoxy(22, 10);
            cout << "Desarrollado por: ";
            gotoxy(40, 11);
            cout << "Marcelo Detlefsen";
            gotoxy(40, 12);
            cout << "Alejandro Jerez";
            gotoxy(40, 13);
            cout << "Julián Divas";
            gotoxy(40, 14);
            cout << "Marco Díaz";
            gotoxy(22, 15);
            cout << "Curso: Programacion de microprocesadores";
            gotoxy(22, 16);
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
int main() {
    // Cargar puntajes al inicio
    loadScores();
    
    // Manejar señales para limpiar al salir
    signal(SIGINT, [](int) {
        showCursor();
        exit(0);
    });
    
    hideCursor();
    showSplashScreen();
    showMainMenu();
    showCursor();
    
    return 0;
}