#ifndef MAP_H
#define MAP_H

#include "game.h"

// Inicializa as salas do mapa (recebe o ponteiro partilhado)
void InitializeMap(struct GameState *gs);

// Imprime a descrição da célula e o que ela contém
void PrintCell(struct GameState *gs, int cell_id);

#endif
