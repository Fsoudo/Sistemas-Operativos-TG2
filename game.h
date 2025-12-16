#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <string.h>

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

// --- Globais Partilhadas (extern) ---
// Definidas no main.c ou map.c, mas visíveis a todos

extern struct Player player;
extern struct Monster monster;
extern struct Object objects[MAX_OBJECTS];
extern struct Cell map[MAX_CELLS];
extern int nCells;
extern int nObjects;

#endif
