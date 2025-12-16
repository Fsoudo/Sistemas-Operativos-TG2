#include "map.h"
#include <stdio.h>
#include <string.h>

// Definição da variável global 'map' e 'nCells'
// (As outras globais como objects/monster estão definidas no main e acedidas
// via extern no game.h)
struct Cell map[MAX_CELLS];
int nCells = 0;

void InitializeMap() {
  nCells = 0;

  // Mapa das Ruínas de Lisboa

  // Cell 0: Pátio do Carrasco (Limoeiro)
  map[0].north = 1;
  map[0].south = -1;
  map[0].west = -1;
  map[0].east = -1;
  map[0].up = -1;
  map[0].down = -1;
  map[0].object_id = -1;
  map[0].treasure = -1;
  strcpy(map[0].description,
         "Patio do Limoeiro (Cadeia). A chuva cai sobre a pedra escurecida "
         "pelo sangue de antigas execucoes.\nO unico caminho seguro parece ser "
         "a norte, em direcao as ruinas da Catedal.");
  nCells++;

  // Cell 1: Ruínas da Sé de Lisboa (Nave Principal)
  map[1].north = 2;
  map[1].south = 0;
  map[1].west = 3;
  map[1].east = -1; // Oeste: Arsenal
  map[1].up = -1;
  map[1].down = 4; // Baixo: Cripta (Tesouro/Boss?)
  map[1].object_id = -1;
  map[1].treasure = -1;
  strcpy(map[1].description,
         "A Nave Principal da Se. O teto desabou, revelando um ceu vermelho "
         "tempestuoso.\nEsqueletos de monges jazem nos bancos. Sente uma "
         "presenca maligna vinda das profundezas (baixo).");
  nCells++;

  // Cell 2: Altar-Mor Profanado
  map[2].north = -1;
  map[2].south = 1;
  map[2].west = -1;
  map[2].east = -1;
  map[2].up = -1;
  map[2].down = -1;
  map[2].object_id = 1; // Agua Benta
  map[2].treasure = -1;
  strcpy(map[2].description,
         "O Altar-Mor. Onde antes havia ouro, agora ha runas demoniacas.\nUm "
         "brilho azulado emana de um frasco intacto no chao.");
  nCells++;

  // Cell 3: Arsenal da Guarda Real (Destroços)
  map[3].north = -1;
  map[3].south = -1;
  map[3].west = -1;
  map[3].east = 1;
  map[3].up = -1;
  map[3].down = -1;
  map[3].object_id = 0; // Gladio de Prata
  map[3].treasure = -1;
  strcpy(map[3].description,
         "Antigo posto da Guarda Real. Armaduras retorcidas decoram as "
         "paredes.\nNum canto, uma espada brilha com uma luz divina, intocada "
         "pela ferrugem.");
  nCells++;

  // Cell 4: Cripta dos Reis (Boss Room)
  map[4].north = -1;
  map[4].south = -1;
  map[4].west = -1;
  map[4].east = -1;
  map[4].up = 1;
  map[4].down = -1;
  map[4].object_id = -1;
  map[4].treasure = 1; // A Coroa Perdida de D. Sebastião (ou Relíquia Sagrada)
  strcpy(map[4].description,
         "A Cripta Real. O ar e gelido e cheira a enxofre.\nNo centro, sobre o "
         "tumulo profanado, repousa o Artefato Sagrado que procuras.");
  nCells++;
}

void PrintCell(int cell_id) {
  printf("\nLOCALIZACAO: %s\n", map[cell_id].description);

  // Acede a objects e monster via extern definidos em game.h
  if (map[cell_id].object_id != -1) {
    printf("ENCONTRADO: %s\n", objects[map[cell_id].object_id].name);
  }
  if (map[cell_id].treasure == 1) {
    printf("OBJETIVO: Ve a Coroa Sagrada a flutuar sobre o altar negro!\n");
  }
  if (monster.cell == cell_id && monster.energy > 0) {
    printf("PERIGO: O ESPECTRO DO REI LOUCO bloqueia o caminho! (Energia "
           "Maligna: %d)\n",
           monster.energy);
  }
  printf("Caminhos:");
  if (map[cell_id].north != -1)
    printf(" [norte]");
  if (map[cell_id].south != -1)
    printf(" [sul]");
  if (map[cell_id].east != -1)
    printf(" [este]");
  if (map[cell_id].west != -1)
    printf(" [oeste]");
  if (map[cell_id].up != -1)
    printf(" [cima]");
  if (map[cell_id].down != -1)
    printf(" [baixo - CUIDADO]");
  printf("\n");
}
