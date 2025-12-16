#include "game.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Variáveis Globais (Estado do Mundo - Definição Real)
struct Player player;
struct Monster monster;
struct Object objects[MAX_OBJECTS];
int nObjects = 0;

// O 'map' e 'nCells' estão definidos em map.c e são acessíveis via extern
// (game.h)

// Protótipos Locais
void InitializePlayer();
void InitializeWorld();
void PrintPlayer();
// PrintCell está em map.h
void MovePlayer(int direction_cell);
void ProcessCommand(char *command);
void Battle();

// Funções de Inicialização

void InitializePlayer() {
  printf("==========================================\n");
  printf("AS CRONICAS DE LISBOA: O REINO DAS SOMBRAS\n");
  printf("==========================================\n\n");
  printf("A escuridao abateu-se sobre Lisboa. O Terramoto de 1755 nao foi "
         "apenas natural...\n");
  printf("Fendas abriram-se para o Inferno, e demónios caminham agora sobre a "
         "calcada portuguesa.\n");
  printf("Tu es o ultimo Inquisidor, enviado para purgar o mal que reside no "
         "coração da cidade.\n\n");
  printf("Qual e o teu nome, Inquisidor?\n> ");
  if (scanf("%49s", player.name) != 1) {
    strcpy(player.name, "Inquisidor");
  }
  // Limpar o buffer de entrada
  while (getchar() != '\n')
    ;

  player.energy = 100;
  player.max_energy = 100;
  player.cell = 0; // Começa no Pátio
  player.object_id = -1;
  player.treasure_status = -1;
}

void InitializeWorld() {
  // Objetos
  strcpy(objects[0].name, "Gladio de Prata Abencoado");
  objects[0].damage_efficacy = 35; // Dano sagrado
  nObjects++;

  strcpy(objects[1].name, "Frasco de Agua Benta da Se");
  objects[1].damage_efficacy = -25; // Cura potente
  nObjects++;

  // Inicializar Mapa (Agora chama a função do módulo map.c)
  InitializeMap();

  // Monstro: O Duque das Sombras
  monster.energy = 80;
  monster.max_energy = 80;
  monster.cell = 4; // Na Cripta
}

// Funções de Output

void PrintPlayer() {
  printf("\n--- Status do Inquisidor ---\n");
  printf("Nome: %s | Vida: %d/%d\n", player.name, player.energy,
         player.max_energy);
  if (player.object_id != -1) {
    printf("Equipado: %s (Poder: %d)\n", objects[player.object_id].name,
           objects[player.object_id].damage_efficacy);
  } else {
    printf("Equipado: Mãos nuas (Dano baixo)\n");
  }
  if (player.treasure_status == 1) {
    printf("Item de Missao: A Coroa Sagrada (Recuperada)\n");
  }
  printf("----------------------------\n");
}

// Lógica de Jogo (Game Loop Logic)

void MovePlayer(int direction_cell) {
  if (direction_cell != -1) {
    player.cell = direction_cell;
    printf("Caminhas pela escuridao...\n");
    // sleep(1);
  } else {
    printf("O caminho esta bloqueado por escombros ou magia negra.\n");
  }
}

void Battle() {
  if (monster.cell != player.cell || monster.energy <= 0) {
    printf("Nao ha hereges ou demonios para purgar aqui.\n");
    return;
  }

  int damage = 5;
  if (player.object_id != -1 && objects[player.object_id].damage_efficacy > 0) {
    damage = objects[player.object_id].damage_efficacy;
  }

  printf("Invocas a luz sagrada e atacas! Causas %d de dano ao Espectro.\n",
         damage);
  monster.energy -= damage;

  if (monster.energy <= 0) {
    printf("O Espectro solta um grito final e desfaz-se em cinzas! A luz "
           "retorna a cripta.\n");
    monster.energy = 0;
  } else {
    printf("O Espectro ri-se e ataca a tua alma!\n");
    player.energy -= 15;
    printf("Sentes a vida a esvair-se (-15 Vida).\n");
  }
}

void ProcessCommand(char *command) {
  if (strcmp(command, "norte") == 0)
    MovePlayer(map[player.cell].north);
  else if (strcmp(command, "sul") == 0)
    MovePlayer(map[player.cell].south);
  else if (strcmp(command, "este") == 0)
    MovePlayer(map[player.cell].east);
  else if (strcmp(command, "oeste") == 0)
    MovePlayer(map[player.cell].west);
  else if (strcmp(command, "cima") == 0)
    MovePlayer(map[player.cell].up);
  else if (strcmp(command, "baixo") == 0)
    MovePlayer(map[player.cell].down);
  else if (strcmp(command, "ver") == 0) {
    PrintCell(player.cell);
    PrintPlayer();
  } else if (strcmp(command, "atacar") == 0) {
    Battle();
  } else if (strcmp(command, "apanhar") == 0) {
    if (map[player.cell].object_id != -1) {
      if (player.object_id == -1) {
        player.object_id = map[player.cell].object_id;
        printf("Equipaste: %s\n", objects[player.object_id].name);
        map[player.cell].object_id = -1;

        if (objects[player.object_id].damage_efficacy < 0) {
          printf("Bebeste a Agua Benta e sentes o calor divino!\n");
          player.energy += 25;
          if (player.energy > player.max_energy)
            player.energy = player.max_energy;
          player.object_id = -1;
        }
      } else {
        printf(
            "Ja carregas um objeto sagrado. (Sistema de inventario cheio)\n");
      }
    } else if (map[player.cell].treasure == 1) {
      if (monster.cell == player.cell && monster.energy > 0) {
        printf("O Espectro protege a Coroa com uma barreira de sombras! Tens "
               "de o derrotar primeiro.\n");
      } else {
        player.treasure_status = 1;
        map[player.cell].treasure = -1;
        printf(
            "RECUPERASTE A COROA SAGRADA! A tua missao esta quase cumprida.\n");
      }
    } else {
      printf("Nao ha reliquias aqui.\n");
    }
  } else if (strcmp(command, "ajuda") == 0) {
    printf("Liturgia (Comandos): norte, sul, este, oeste, cima, baixo, ver, "
           "apanhar, atacar, sair\n");
  } else {
    printf("As vozes na tua cabeca confundem-te. (Comando desconhecido. "
           "Escreva 'ajuda')\n");
  }
}

int main() {
  char command[50];

  InitializeWorld();
  InitializePlayer();
  PrintPlayer();

  // Ciclo Principal do Jogo
  while (player.energy > 0) {
    if (player.treasure_status == 1 && player.cell == 0 &&
        monster.energy <= 0) {
      printf("\n\nGLORIA! Escapaste das ruinas com a Coroa.\nLisboa tem uma "
             "nova esperanca gracas a ti, Inquisidor.\n");
      break;
    }

    PrintCell(player.cell); // Mostra onde está

    printf("\nO que desejas fazer? > ");
    if (scanf("%49s", command) != 1)
      break;

    if (strcmp(command, "sair") == 0) {
      printf("Fugiste para as sombras... Game Over.\n");
      break;
    }

    ProcessCommand(command);

    if (monster.cell == player.cell && monster.energy > 0) {
      printf("Sentes o odio do Espectro a queimar a tua pele!\n");
    }
  }

  if (player.energy <= 0) {
    printf("\n\nCAISTE EM COMBATE... A tua alma pertence agora ao "
           "Inferno.\nGAME OVER\n");
  }

  return 0;
}
