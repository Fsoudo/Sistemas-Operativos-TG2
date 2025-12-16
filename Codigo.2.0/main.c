#include "game.h"
#include "map.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Ponteiro para a Memória Partilhada
struct GameState *gamestate;
int shmid;

// Protótipos
void Cleanup(int sig);
void InitializeWorld();
void MonsterLogic();
void ProcessCommand(char *command);
void Battle();
void MovePlayer(int direction_cell);
void PrintPlayer();

void InitializeWorld() {
  gamestate->nObjects = 0;

  strcpy(gamestate->objects[0].name, "Gladio de Prata Abencoado");
  gamestate->objects[0].damage_efficacy = 35;
  gamestate->nObjects++;

  strcpy(gamestate->objects[1].name, "Frasco de Agua Benta da Se");
  gamestate->objects[1].damage_efficacy = -25;
  gamestate->nObjects++;

  strcpy(gamestate->objects[2].name, "Machado do Carrasco");
  gamestate->objects[2].damage_efficacy = 45;
  gamestate->nObjects++;

  strcpy(gamestate->objects[3].name, "Rosario de Madeira");
  gamestate->objects[3].damage_efficacy = -15;
  gamestate->nObjects++;

  InitializeMap(gamestate);

  gamestate->player.energy = 100;
  gamestate->player.max_energy = 100;
  gamestate->player.cell = 0;
  gamestate->player.object_id = -1;
  gamestate->player.treasure_status = -1;

  gamestate->monster.energy = 80;
  gamestate->monster.max_energy = 80;

  gamestate->game_running = 1;
}

void Cleanup(int sig) {
  if (gamestate) {
    gamestate->game_running = 0;
    shmdt(gamestate);
  }
  shmctl(shmid, IPC_RMID, NULL);
  printf("\n[SISTEMA] Memoria Partilhada limpa. Adeus.\n");
  exit(0);
}

// --- Lógica do Monstro ---
void MonsterLogic() {
  srand(time(NULL) ^ getpid());

  while (gamestate->game_running) {
    sleep(25);

    if (gamestate->monster.energy <= 0)
      break;

    int current = gamestate->monster.cell;
    int directions[4] = {
        gamestate->map[current].north, gamestate->map[current].south,
        gamestate->map[current].east, gamestate->map[current].west
        // Sem up/down
    };

    int valid_moves[4];
    int count = 0;
    for (int i = 0; i < 4; i++) {
      if (directions[i] != -1)
        valid_moves[count++] = directions[i];
    }

    if (count > 0) {
      int next_cell = valid_moves[rand() % count];
      gamestate->monster.cell = next_cell;

      if (gamestate->player.cell == next_cell) {
        printf("\n\n!!! DE REPENTE, O ESPECTRO ENTRA NA SALA !!!\n> ");
        fflush(stdout);
      }
    }
  }
  exit(0);
}

void PrintPlayer() {
  printf("\n--- Status do Inquisidor ---\n");
  printf("Nome: %s | Vida: %d/%d\n", gamestate->player.name,
         gamestate->player.energy, gamestate->player.max_energy);
  if (gamestate->player.object_id != -1) {
    printf("Equipado: %s (Poder: %d)\n",
           gamestate->objects[gamestate->player.object_id].name,
           gamestate->objects[gamestate->player.object_id].damage_efficacy);
  } else {
    printf("Equipado: Mãos nuas (Dano baixo)\n");
  }
  if (gamestate->player.treasure_status == 1) {
    printf("Item de Missao: A Coroa Sagrada (Recuperada)\n");
  }
  printf("----------------------------\n");
}

void MovePlayer(int direction_cell) {
  if (gamestate->monster.cell == gamestate->player.cell &&
      gamestate->monster.energy > 0) {
    printf("O ESPECTRO BARRA O CAMINHO! Tens de o enfrentar ou esperar que ele "
           "se mova...\n");
    return;
  }

  if (direction_cell != -1) {
    gamestate->player.cell = direction_cell;
    printf("Caminhas pela escuridao...\n");
  } else {
    printf("O caminho esta bloqueado.\n");
  }
}

void Battle() {
  if (gamestate->monster.cell != gamestate->player.cell ||
      gamestate->monster.energy <= 0) {
    printf("Nao ha hereges aqui para atacar.\n");
    return;
  }

  int damage = 5;
  if (gamestate->player.object_id != -1 &&
      gamestate->objects[gamestate->player.object_id].damage_efficacy > 0) {
    damage = gamestate->objects[gamestate->player.object_id].damage_efficacy;
  }

  printf("Invocas a luz sagrada! Causas %d de dano.\n", damage);
  gamestate->monster.energy -= damage;

  if (gamestate->monster.energy <= 0) {
    printf("O Espectro solta um grito e desfaz-se em cinzas! Vitoria "
           "momentanea.\n");
    gamestate->monster.energy = 0;
  } else {
    printf("O Espectro contra-ataca!\n");
    gamestate->player.energy -= 15;
    printf("Perdeste 15 de Vida.\n");
  }
}

void ProcessCommand(char *full_command) {
  char *cmd = strtok(full_command, " \n\r");
  if (!cmd)
    return;

  if (strcmp(cmd, "norte") == 0)
    MovePlayer(gamestate->map[gamestate->player.cell].north);
  else if (strcmp(cmd, "sul") == 0)
    MovePlayer(gamestate->map[gamestate->player.cell].south);
  else if (strcmp(cmd, "este") == 0)
    MovePlayer(gamestate->map[gamestate->player.cell].east);
  else if (strcmp(cmd, "oeste") == 0)
    MovePlayer(gamestate->map[gamestate->player.cell].west);

  // Cima e Baixo removidos

  else if (strcmp(cmd, "ver") == 0) {
    PrintCell(gamestate, gamestate->player.cell);
    PrintPlayer();
  } else if (strcmp(cmd, "atacar") == 0)
    Battle();
  else if (strcmp(cmd, "apanhar") == 0) {
    int cell = gamestate->player.cell;
    if (gamestate->map[cell].object_id != -1) {
      if (gamestate->player.object_id == -1) {
        gamestate->player.object_id = gamestate->map[cell].object_id;
        printf("Equipaste: %s\n",
               gamestate->objects[gamestate->player.object_id].name);
        gamestate->map[cell].object_id = -1;

        if (gamestate->objects[gamestate->player.object_id].damage_efficacy <
            0) {
          printf("Usaste o item sagrado!\n");
          gamestate->player.energy += 25;
          if (gamestate->player.energy > gamestate->player.max_energy)
            gamestate->player.energy = gamestate->player.max_energy;
          gamestate->player.object_id = -1;
        }
      } else {
        printf("Mochila cheia.\n");
      }
    } else if (gamestate->map[cell].treasure == 1) {
      if (gamestate->monster.cell == gamestate->player.cell &&
          gamestate->monster.energy > 0) {
        printf("O Espectro protege a Coroa!\n");
      } else {
        gamestate->player.treasure_status = 1;
        gamestate->map[cell].treasure = -1;
        printf("RECUPERASTE A COROA SAGRADA!\n");
      }
    } else {
      printf("Nada para apanhar.\n");
    }
  } else if (strcmp(cmd, "ajuda") == 0) {
    printf("Comandos: norte, sul, este, oeste, ver, apanhar, atacar, sair\n");
  } else if (strcmp(cmd, "sair") == 0) {
    Cleanup(0);
  } else {
    printf("Comando invalido.\n");
  }
}

int main() {
  signal(SIGINT, Cleanup);
  srand(time(NULL));

  shmid = shmget(IPC_PRIVATE, sizeof(struct GameState), IPC_CREAT | 0666);
  if (shmid < 0) {
    perror("Erro shmget");
    exit(1);
  }

  gamestate = (struct GameState *)shmat(shmid, NULL, 0);
  if (gamestate == (void *)-1) {
    perror("Erro shmat");
    exit(1);
  }

  printf("Qual e o teu nome, Inquisidor?\n> ");
  char buffer[100];
  char pl_name[50] = "Inquisidor";
  if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0)
      strcpy(pl_name, buffer);
  }
  strcpy(gamestate->player.name, pl_name);

  char replay_opt;

  // --- LOOP DE JOGO (REPLAY) ---
  do {
    printf("\n\n--- NOVO JOGO INICIADO ---\n");
    InitializeWorld(); // Gera novo mapa aleatorio
    strcpy(gamestate->player.name, pl_name);

    pid_t pid = fork();

    if (pid == 0) {
      MonsterLogic();
    } else {
      PrintPlayer();

      while (gamestate->game_running && gamestate->player.energy > 0) {
        if (gamestate->player.treasure_status == 1 &&
            gamestate->player.cell == 0 && gamestate->monster.energy <= 0) {
          printf("\n\nGLORIA! Escapaste das ruinas com a Coroa.\n");
          gamestate->game_running = 0;
          break;
        }

        PrintCell(gamestate, gamestate->player.cell);

        // MOSTRAR COMANDOS DISPONIVEIS (Modo 2D)
        printf("[OPCOES: norte, sul, este, oeste, ver, apanhar, atacar, sair, "
               "ajuda]\n> ");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
          break;
        ProcessCommand(buffer);

        if (gamestate->monster.cell == gamestate->player.cell &&
            gamestate->monster.energy > 0) {
          printf("O Espectro esta na sala contigo!\n");
        }
      }

      if (gamestate->player.energy <= 0)
        printf("\nGAME OVER... A tua alma foi colhida.\n");

      gamestate->game_running = 0;
      wait(NULL);

      printf("\nJogar Novamente? (s/n): ");
      if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        replay_opt = buffer[0];
      } else
        replay_opt = 'n';
    }

  } while (replay_opt == 's' || replay_opt == 'S');

  Cleanup(0);
  return 0;
}
