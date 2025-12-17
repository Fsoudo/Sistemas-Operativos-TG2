#ifndef GAME_H
#define GAME_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// --- Configurações de IPC ---
// Chaves para Shared Memory e Semaphores
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Indices dos Semáforos no array
#define SEM_GAMESTATE 0 // Protege a memoria partilhada
#define SEM_IO 1        // Protege o ecrã (printf)
#define NUM_SEMS 2

#define MAX_NAME 50
#define MAX_DESC 200
#define MAX_CELLS 100
#define MAX_OBJECTS 10

// --- Estruturas ---

struct Object {
  char name[MAX_NAME];
  int damage_efficacy;
};

struct Monster {
  int energy;
  int cell;
  int max_energy;
};

struct Player {
  char name[MAX_NAME];
  int energy;
  int max_energy;
  int cell;
  int object_id;
  int treasure_status;
};

struct Cell {
  int north;
  int south;
  int west;
  int east;
  int up;
  int down;
  int object_id;
  int treasure;
  char description[MAX_DESC];
};

// --- Estrutura Partilhada (Estado Completo do Jogo) ---
struct GameState {
  struct Player player;
  struct Monster monster;
  struct Cell map[MAX_CELLS];
  struct Object objects[MAX_OBJECTS];
  int nCells;
  int nObjects;
  int game_running; // 1 = Running, 0 = Stop
};

// Funcoes Globais
void InitializeMap(struct GameState *gs);
void LoadMapFromFile(struct GameState *gs);
void LoadObjectsFromFile(struct GameState *gs);
void PrintCell(struct GameState *gs, int cell_id);

// Extern para acesso global ao ponteiro da memória partilhada (definido no
// main)
extern struct GameState *gamestate;

#endif
