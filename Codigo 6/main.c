#include "game.h"
#include "map.h"
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Ponteiro para a Memória Partilhada
struct GameState *gamestate;
int shmid;
int semid;

#if defined(__linux__) || defined(__unix__)
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
#endif

void Cleanup(int sig);
void InitializeWorld();
void MonsterLogic();
void ProcessCommand(char *command);
void Battle();
void MovePlayer(int direction_cell);
void PrintPlayer();
void PrintCell(struct GameState *gs, int cell_id); // Do map.c

// --- FUNCOES DE SEMAFORO (P e V) ---
void P(int sem_index) {
  struct sembuf operacao;
  operacao.sem_num = sem_index;
  operacao.sem_op = -1;
  operacao.sem_flg = 0;
  if (semop(semid, &operacao, 1) == -1) {
    perror("Erro no P()");
  }
}

void V(int sem_index) {
  struct sembuf operacao;
  operacao.sem_num = sem_index;
  operacao.sem_op = 1;
  operacao.sem_flg = 0;
  if (semop(semid, &operacao, 1) == -1) {
    perror("Erro no V()");
  }
}

void InitializeWorld() {
  LoadObjectsFromFile(gamestate);
  InitializeMap(gamestate);

  gamestate->player.energy = 100;
  gamestate->player.max_energy = 100;
  gamestate->player.cell = 0;
  gamestate->player.object_id = -1;
  gamestate->player.treasure_status = -1;

  // Monstros Simplificados
  // Monster 0: Boss (Rei)
  strcpy(gamestate->monsters[0].name, "Rei Esqueleto");
  gamestate->monsters[0].energy = 100;
  gamestate->monsters[0].max_energy = 100;
  gamestate->monsters[0].cell = 6; // Sala do Trono

  // Monster 1: Guard (Roaming simples)
  strcpy(gamestate->monsters[1].name, "Guarda");
  gamestate->monsters[1].energy = 50;
  gamestate->monsters[1].max_energy = 50;
  gamestate->monsters[1].cell = 4; // Corredor

  // Monster 2: Inativo
  gamestate->monsters[2].energy = 0; // Desativado

  gamestate->game_running = 1;
}

void Cleanup(int sig) {
  if (gamestate) {
    gamestate->game_running = 0;
    shmdt(gamestate);
  }
  semctl(semid, 0, IPC_RMID);
  shmctl(shmid, IPC_RMID, NULL);
  printf("\n[SISTEMA] Jogo Terminado.\n");
  exit(0);
}

void MonsterLogic() {
  srand(time(NULL) ^ getpid());
  while (1) {
    sleep(15); // Move a cada 15s

    P(SEM_GAMESTATE);
    if (!gamestate->game_running) {
      V(SEM_GAMESTATE);
      break;
    }

    // Simplificacao: Apenas o Monstro 1 se move aleatoriamente
    if (gamestate->monsters[1].energy > 0) {
      int current = gamestate->monsters[1].cell;
      // Tenta mover-se para uma sala adjacente
      int next = -1;
      // Logica muito simples: escolhe uma direcao valida
      // Array de vizinhos
      int opts[4] = {
          gamestate->map[current].north, gamestate->map[current].south,
          gamestate->map[current].east, gamestate->map[current].west};
      int valid[4];
      int count = 0;
      for (int i = 0; i < 4; i++)
        if (opts[i] != -1)
          valid[count++] = opts[i];

      if (count > 0)
        next = valid[rand() % count];

      if (next != -1) {
        gamestate->monsters[1].cell = next;
        if (next == gamestate->player.cell) {
          printf("\n[ALERTA] Um monstro entrou na sala!\n> ");
          fflush(stdout);
        }
      }
    }
    V(SEM_GAMESTATE);
  }
  exit(0);
}

void PrintPlayer() {
  P(SEM_IO);
  P(SEM_GAMESTATE);
  printf("\nStatus: %s | Vida: %d/%d\n", gamestate->player.name,
         gamestate->player.energy, gamestate->player.max_energy);

  if (gamestate->player.object_id != -1) {
    printf("Item: %s\n", gamestate->objects[gamestate->player.object_id].name);
  } else {
    printf("Item: Nenhum\n");
  }
  if (gamestate->player.treasure_status == 1) {
    printf("COROA RECUPERADA!\n");
  }
  V(SEM_GAMESTATE);
  V(SEM_IO);
}

void MovePlayer(int direction_cell) {
  P(SEM_GAMESTATE);

  // Bloqueio simples por monstro
  for (int i = 0; i < NUM_MONSTERS; i++) {
    if (gamestate->monsters[i].cell == gamestate->player.cell &&
        gamestate->monsters[i].energy > 0) {
      V(SEM_GAMESTATE);
      P(SEM_IO);
      printf("Um monstro (%s) bloqueia o caminho!\n",
             gamestate->monsters[i].name);
      V(SEM_IO);
      return;
    }
  }

  if (direction_cell != -1) {
    gamestate->player.cell = direction_cell;
    P(SEM_IO);
    printf("Moveste-te para uma nova sala.\n");
    V(SEM_IO);
  } else {
    P(SEM_IO);
    printf("Nao podes ir por ai.\n");
    V(SEM_IO);
  }
  V(SEM_GAMESTATE);
}

void Battle() {
  P(SEM_GAMESTATE);

  int target_idx = -1;
  for (int i = 0; i < NUM_MONSTERS; i++) {
    if (gamestate->monsters[i].cell == gamestate->player.cell &&
        gamestate->monsters[i].energy > 0) {
      target_idx = i;
      break;
    }
  }

  if (target_idx == -1) {
    V(SEM_GAMESTATE);
    P(SEM_IO);
    printf("Nao ha ninguem para atacar.\n");
    V(SEM_IO);
    return;
  }

  // Combate Simples: Eu bato, Ele bate
  int damage = 10; // Dano base mao nua
  if (gamestate->player.object_id != -1) {
    damage = gamestate->objects[gamestate->player.object_id].damage_efficacy;
    if (damage <= 0)
      damage = 10; // Item de cura nao da dano
  }

  gamestate->monsters[target_idx].energy -= damage;
  printf("\nAtacas %s causando %d de dano.\n",
         gamestate->monsters[target_idx].name, damage);

  if (gamestate->monsters[target_idx].energy <= 0) {
    gamestate->monsters[target_idx].energy = 0;
    printf("O monstro morreu!\n");
  } else {
    // Contra-ataque
    gamestate->player.energy -= 10;
    printf("O monstro contra-ataca! Perdeste 10 de vida.\n");
  }

  V(SEM_GAMESTATE);
}

void ProcessCommand(char *full_command) {
  char *cmd = strtok(full_command, " \n\r");
  if (!cmd)
    return;

  if (strcmp(cmd, "norte") == 0 || strcmp(cmd, "n") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].north;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "sul") == 0 || strcmp(cmd, "s") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].south;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "este") == 0 || strcmp(cmd, "e") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].east;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "oeste") == 0 || strcmp(cmd, "o") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].west;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "ver") == 0) {
    P(SEM_GAMESTATE);
    int p_cell = gamestate->player.cell;
    V(SEM_GAMESTATE);
    P(SEM_IO);
    PrintCell(gamestate, p_cell);
    V(SEM_IO);
    PrintPlayer();
  } else if (strcmp(cmd, "atacar") == 0) {
    Battle();
  } else if (strcmp(cmd, "apanhar") == 0) {
    P(SEM_GAMESTATE);
    int cell = gamestate->player.cell;

    // Tesouro
    if (gamestate->map[cell].treasure == 1) {
      if (gamestate->monsters[0].cell == cell &&
          gamestate->monsters[0].energy > 0) {
        printf("O Boss protege o tesouro!\n");
      } else {
        gamestate->player.treasure_status = 1;
        gamestate->map[cell].treasure = -1;
        printf("Apanhaste o TESOURO!\n");
      }
    }
    // Items
    else if (gamestate->map[cell].object_id != -1) {
      if (gamestate->player.object_id == -1) {
        gamestate->player.object_id = gamestate->map[cell].object_id;
        gamestate->map[cell].object_id = -1;
        printf("Apanhaste um item.\n");
      } else {
        printf("Mochila cheia.\n");
      }
    } else {
      printf("Nada aqui.\n");
    }
    V(SEM_GAMESTATE);

  } else if (strcmp(cmd, "sair") == 0) {
    Cleanup(0);
  } else if (strcmp(cmd, "ajuda") == 0) {
    printf("Comandos: n, s, e, o, ver, atacar, apanhar, sair\n");
  } else {
    printf("Comando desconhecido.\n");
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, Cleanup);
  srand(time(NULL));

  shmid = shmget(IPC_PRIVATE, sizeof(struct GameState), IPC_CREAT | 0666);
  gamestate = (struct GameState *)shmat(shmid, NULL, 0);
  semid = semget(IPC_PRIVATE, NUM_SEMS, IPC_CREAT | 0666);

  union semun arg;
  arg.val = 1;
  semctl(semid, SEM_GAMESTATE, SETVAL, arg);
  semctl(semid, SEM_IO, SETVAL, arg);

  printf("Nome do Jogador: ");
  char buffer[100];
  if (fgets(buffer, sizeof(buffer), stdin)) {
    buffer[strcspn(buffer, "\n")] = 0;
    strcpy(gamestate->player.name, buffer);
  }

  InitializeWorld();

  if (fork() == 0) {
    MonsterLogic();
  } else {
    int running = 1;
    while (running) {
      P(SEM_GAMESTATE);
      if (!gamestate->game_running) {
        running = 0;
        V(SEM_GAMESTATE);
        break;
      }

      // Win condition
      if (gamestate->player.treasure_status == 1 &&
          gamestate->player.cell == 0) {
        printf("\nVICTORIA! Escapaste com o tesouro!\n");
        gamestate->game_running = 0;
        V(SEM_GAMESTATE);
        break;
      }
      if (gamestate->player.energy <= 0) {
        printf("\nDERROTA! Morreste.\n");
        gamestate->game_running = 0;
        V(SEM_GAMESTATE);
        break;
      }

      int p_cell = gamestate->player.cell;
      V(SEM_GAMESTATE);

      PrintCell(gamestate, p_cell);
      printf("> ");

      if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        break;
      ProcessCommand(buffer);
    }
    wait(NULL);
    Cleanup(0);
  }
  return 0;
}
