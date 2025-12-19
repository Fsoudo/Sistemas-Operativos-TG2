#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Função para carregar objetos de ficheiro (Req 6.4)
void LoadObjectsFromFile(struct GameState *gs) {
  FILE *f = fopen("objetos.txt", "r");
  if (!f) {
    perror("Erro ao abrir objetos.txt");
    // Nao sair, pode ser opcional ou erro fatal. Vamos assumir fatal.
    exit(1);
  }

  char line[256];
  while (fgets(line, sizeof(line), f)) {
    if (gs->nObjects >= MAX_OBJECTS)
      break;

    // Formato esperado: "0 Nome do Item 35"
    // O ultimo token e o power. O primeiro e o ID. O meio e o nome.

    int temp_id, temp_power;
    char *last_space = strrchr(line, ' ');
    if (!last_space)
      continue;

    *last_space = '\0'; // Corta a string no ultimo espaco
    temp_power =
        atoi(last_space + 1); // O que vem depois do ultimo espaco e o power

    char *first_space = strchr(line, ' ');
    if (!first_space)
      continue;

    temp_id = atoi(line); // O inicio e o ID

    char *name_start = first_space + 1;

    int idx = temp_id;
    if (idx < 0 || idx >= MAX_OBJECTS)
      continue;

    strcpy(gs->objects[idx].name, name_start);
    gs->objects[idx].damage_efficacy = temp_power;
    gs->nObjects++;
  }

  printf("[SISTEMA] Objetos carregados de 'objetos.txt'.\n");
  fclose(f);
}

// Função para carregar mapa de ficheiro (Req 6.4)
void LoadMapFromFile(struct GameState *gs) {
  FILE *f = fopen("mapa.txt", "r");
  if (!f) {
    perror("Erro ao abrir mapa.txt");
    exit(1);
  }

  int id = 0;

  // Formato do ficheiro:
  // Linha 1: Descricao
  // Linha 2: N S E W U D Object Treasure

  char line_desc[256];
  char line_nums[256];

  while (id < MAX_CELLS && fgets(line_desc, sizeof(line_desc), f)) {
    // Remover newline (LF e CRLF)
    line_desc[strcspn(line_desc, "\r\n")] = 0;

    // Ignorar linhas vazias ou muito curtas
    if (strlen(line_desc) == 0)
      continue;

    strcpy(gs->map[id].description, line_desc);

    // Ler a proxima linha com os numeros
    if (!fgets(line_nums, sizeof(line_nums), f))
      break;

    int n, s, e, w, u, d, obj, treas;
    if (sscanf(line_nums, "%d %d %d %d %d %d %d %d", &n, &s, &e, &w, &u, &d,
               &obj, &treas) != 8) {
      printf("[AVISO] Erro de formatacao na sala %d. A parar carga.\n", id);
      break;
    }

    gs->map[id].north = n;
    gs->map[id].south = s;
    gs->map[id].east = e;
    gs->map[id].west = w;
    gs->map[id].up = u;
    gs->map[id].down = d;
    gs->map[id].object_id = obj;
    gs->map[id].treasure = treas;

    id++;
  }

  gs->nCells = id;

  // Configurar monstro na ultima sala por defeito (pode ser sobrescrito por
  // Args)
  // gs->monsters[0].cell = id - 1; // Agora init no Main

  printf("[SISTEMA] Mapa carregado de 'mapa.txt' com %d salas.\n", id);
  fclose(f);
}

void InitializeMap(struct GameState *gs) {
  // Wrapper para manter compatibilidade com chamada no main, mas usa arquivo
  // agora
  LoadMapFromFile(gs);
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

  // Verificar presença de monstros
  for (int i = 0; i < NUM_MONSTERS; i++) {
    if (gs->monsters[i].cell == cell_id && gs->monsters[i].energy > 0) {
      printf("PERIGO: %s bloqueia o caminho! (Energia: %d)\n",
             gs->monsters[i].name, gs->monsters[i].energy);
    }
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
  printf("\n");
}
