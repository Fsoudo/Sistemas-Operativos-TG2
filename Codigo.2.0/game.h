#ifndef GAME_H
#define GAME_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


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

// Extern para acesso global ao ponteiro da memória partilhada (definido no
// main)
extern struct GameState *gamestate;

#endif
