#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// Banco de Descrições (Pool)
const char *descricoes_pool[] = {
    "Patio do Limoeiro. A chuva cai fria sobre as pedras manchadas de sangue.",
    "Nave Principal da Se. O teto desabou e ves o ceu tempestuoso.",
    "Cripta dos Reis. O ar cheira a enxofre e os tumulos estao abertos.",
    "Arsenal da Guarda Real. Armaduras vazias olham para ti.",
    "Ruinas do Convento do Carmo. Apenas os arcos goticos restam de pe.",
    "Terreiro do Paco Inundado. A agua do Tejo esta negra e viscosa.",
    "Bairro de Alfama Labirintico. Becos estreitos cheios de nevoeiro.",
    "Muralhas do Castelo de S. Jorge. Ves a cidade a arder la em baixo.",
    "Hospital de Todos os Santos. Ouvem-se lamentos fantasmagoricos.",
    "Aqueduto das Aguas Livres. A silhueta e imponente contra a lua."};

#define POOL_SIZE 10

// map.c atualizado para Geração PROCEDURAL 2D (Sem Cima/Baixo)

void InitializeMap(struct GameState *gs) {
  gs->nCells = 0;

  // 1. Determinar tamanho do mapa
  int num_salas = 6 + (rand() % 4); // Entre 6 e 9 salas
  if (num_salas > POOL_SIZE)
    num_salas = POOL_SIZE;

  gs->nCells = num_salas;
  printf("[SISTEMA] A gerar mapa 2D aleatorio com %d salas...\n", num_salas);

  // 2. Inicializar salas vazias
  for (int i = 0; i < num_salas; i++) {
    strcpy(gs->map[i].description, descricoes_pool[i]);

    gs->map[i].north = -1;
    gs->map[i].south = -1;
    gs->map[i].east = -1;
    gs->map[i].west = -1;
    // Cima/Baixo desativados
    gs->map[i].up = -1;
    gs->map[i].down = -1;

    gs->map[i].object_id = -1;
    gs->map[i].treasure = -1;
  }

  // 3. Criar Conectividade (Espinha Dorsal Simples)
  // Sala i liga à i+1 usando apenas N/S/E/W
  for (int i = 0; i < num_salas - 1; i++) {
    // Escolher direção aleatória: 0=N, 1=S, 2=E, 3=W (Sem 4/5)
    int dir = rand() % 4;

    // Verifica se a porta já está ocupada, se sim, tenta outra
    // Simplificação: Sobrescreve para garantir caminho
    if (dir == 0) {
      gs->map[i].north = i + 1;
      gs->map[i + 1].south = i;
    } else if (dir == 1) {
      gs->map[i].south = i + 1;
      gs->map[i + 1].north = i;
    } else if (dir == 2) {
      gs->map[i].east = i + 1;
      gs->map[i + 1].west = i;
    } else if (dir == 3) {
      gs->map[i].west = i + 1;
      gs->map[i + 1].east = i;
    }
  }

  // 4. Adicionar conexões extra (atalhos)
  for (int i = 0; i < num_salas; i++) {
    if (rand() % 100 < 25) { // 25% chance
      int alvo = rand() % num_salas;
      if (alvo != i) {
        // Tenta ligar se slots livres
        if (gs->map[i].north == -1 && gs->map[alvo].south == -1) {
          gs->map[i].north = alvo;
          gs->map[alvo].south = i;
        } else if (gs->map[i].east == -1 && gs->map[alvo].west == -1) {
          gs->map[i].east = alvo;
          gs->map[alvo].west = i;
        }
      }
    }
  }

  // 5. Distribuir Objetos
  int items_placed = 0;
  while (items_placed < 3) {
    int r = 1 + (rand() % (num_salas - 1));
    if (gs->map[r].object_id == -1) {
      int obj = rand() % 4;
      gs->map[r].object_id = obj;
      items_placed++;
    }
  }

  // 6. Colocar Tesouro
  gs->map[num_salas - 1].treasure = 1;

  // 7. Colocar Monstro
  gs->monster.cell = num_salas - 1;

  printf("[SISTEMA] Ruinas geradas. Tesouro na sala %d.\n", num_salas - 1);
}

void PrintCell(struct GameState *gs, int cell_id) {
  if (cell_id < 0 || cell_id >= gs->nCells)
    return;

  printf("\nLOCALIZACAO: %s\n", gs->map[cell_id].description);

  if (gs->map[cell_id].object_id != -1) {
    printf("ENCONTRADO: %s\n", gs->objects[gs->map[cell_id].object_id].name);
  }
  if (gs->map[cell_id].treasure == 1) {
    printf("OBJETIVO: Ve a Coroa Sagrada a flutuar!\n");
  }
  if (gs->monster.cell == cell_id && gs->monster.energy > 0) {
    printf(
        "PERIGO: O ESPECTRO DO REI LOUCO bloqueia o caminho! (Energia: %d)\n",
        gs->monster.energy);
  }
  printf("Caminhos:");
  if (gs->map[cell_id].north != -1)
    printf(" [norte]");
  if (gs->map[cell_id].south != -1)
    printf(" [sul]");
  if (gs->map[cell_id].east != -1)
    printf(" [este]");
  if (gs->map[cell_id].west != -1)
    printf(" [oeste]");
  // Cima/Baixo removidos do print
  printf("\n");
}
