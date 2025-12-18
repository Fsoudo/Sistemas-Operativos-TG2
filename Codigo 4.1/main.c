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
int semid; // ID do array de semaforos

// Union para semctl (caso nao esteja definida no sys/sem.h)
// Em alguns sistemas modernos ja existe, mas e boa pratica definir se
// necessario
#if defined(__linux__) || defined(__unix__)
union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
#endif

// Protótipos
void Cleanup(int sig);
void InitializeWorld();
void MonsterLogic();
void ProcessCommand(char *command);
void Battle();
void MovePlayer(int direction_cell);
void PrintPlayer(int superuser);

// --- FUNCOES DE SEMAFORO (P e V) ---
void P(int sem_index) {
  struct sembuf operacao;
  operacao.sem_num = sem_index;
  operacao.sem_op = -1; // Decrementa (Wait)
  operacao.sem_flg = 0;
  if (semop(semid, &operacao, 1) == -1) {
    perror("Erro no P()");
    // Nao sair fatalmente para nao quebrar o jogo todo, mas avisar
  }
}

void V(int sem_index) {
  struct sembuf operacao;
  operacao.sem_num = sem_index;
  operacao.sem_op = 1; // Incrementa (Signal)
  operacao.sem_flg = 0;
  if (semop(semid, &operacao, 1) == -1) {
    perror("Erro no V()");
  }
}

// --- NOVAS FUNCOES (ATMOSFERA) ---
void AmbientEvents() {
    int r = rand() % 100;
    if (r < 20) { // 20% de chance de evento de flavor
        const char *events[] = {
            "Ouve-se um ruido de correntes a arrastar...",
            "Sentes um arrepio na espinha.",
            "Uma gota de agua cai no teu ombro.",
            "As tochas parecem tremeluzir por um instante.",
            "Um vento gelado passa por ti.",
            "Parece que viste algo a mexer nas sombras..."
        };
        int num_events = 6;
        P(SEM_IO);
        printf("\n(Atmosfera) %s\n", events[rand() % num_events]);
        V(SEM_IO);
    }
}

// Wrapper para printf protegido
void SafePrint(const char *format, ...) {
  P(SEM_IO);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  fflush(stdout);
  V(SEM_IO);
}

void InitializeWorld() {
  // Nota: InitializeWorld é chamado no main antes do fork ou em loop protegido,
  // mas como mexe na shared memory, idealmente devia estar protegido se fosse
  // chamado concorrentemente. No inicio do jogo (antes do fork) nao precisa de
  // semaforo, mas no restart sim.

  // Carregar Objetos do Ficheiro (Req 6.4)
  LoadObjectsFromFile(gamestate);

  InitializeMap(gamestate);

  gamestate->player.energy = 100;
  gamestate->player.max_energy = 100;
  gamestate->player.cell = 0;
  gamestate->player.object_id = -1;
  gamestate->player.treasure_status = -1;

  gamestate->player.treasure_status = -1;

  // --- CONFIGURACAO DOS MONSTROS ---
  
  // 1. Rei Esqueleto (BOSS) - Estatico
  strcpy(gamestate->monsters[0].name, "Rei Esqueleto");
  gamestate->monsters[0].energy = 150;
  gamestate->monsters[0].max_energy = 150;
  gamestate->monsters[0].cell = 9; // Sala do Tesouro (ou guarda a entrada)

  // 2. Fantasma (ROAM) - Vagueia
  strcpy(gamestate->monsters[1].name, "Fantasma Errante");
  gamestate->monsters[1].energy = 55;
  gamestate->monsters[1].max_energy = 55;
  gamestate->monsters[1].cell = gamestate->nCells - 1; // Comeca longe

  // 3. O Chato (BLOCKER) - Estatico
  strcpy(gamestate->monsters[2].name, "O Chato");
  gamestate->monsters[2].energy = 45;
  gamestate->monsters[2].max_energy = 45;
  gamestate->monsters[2].cell = 11; // Num chokepoint novo

  gamestate->game_running = 1;
}

void Cleanup(int sig) {
  // Ignorar sinal no filho para evitar cleanup duplo imediato?
  // Ou melhor: Apenas o pai faz cleanup dos recursos IPC.
  // Mas ambos devem terminar.

  if (gamestate) {
    gamestate->game_running = 0;
    shmdt(gamestate);
  }

  // Identificar se somos o pai ou filho?
  // Nao temos pid aqui facil (variavel global?).
  // Mas podemos tentar limpar sempre. Se falhar falhou.

  // truque: se semctl falhar é porque ja foi limpo.
  // Vamos reduzir o ruido.

  semctl(semid, 0, IPC_RMID);
  shmctl(shmid, IPC_RMID, NULL);

  printf("\n[SISTEMA] O Inquisidor terminou a sua jornada.\n");
  exit(0);
}

// --- Lógica do Monstro ---
void MonsterLogic() {
  srand(time(NULL) ^ getpid());

  while (1) {
    sleep(20); // Abrandar para 20s para dar tempo ao jogador

    // Verificar se o jogo ainda corre
    // Verificar se o jogo ainda corre
    P(SEM_GAMESTATE);
    if (!gamestate->game_running) {
      V(SEM_GAMESTATE);
      break;
    }
    // Apenas o Fantasma (ID 1) se move
    if (gamestate->monsters[1].energy <= 0) {
        V(SEM_GAMESTATE);
        continue; // Se morto, nao faz nada
    }

    int current = gamestate->monsters[1].cell;
    int directions[4] = {
        gamestate->map[current].north, gamestate->map[current].south,
        gamestate->map[current].east, gamestate->map[current].west};
    V(SEM_GAMESTATE); // Liberta lock enquanto calcula (embora aqui seja rapido)

    int valid_moves[4];
    int count = 0;
    for (int i = 0; i < 4; i++) {
      if (directions[i] != -1)
        valid_moves[count++] = directions[i];
    }

    if (count > 0) {
      int next_cell = valid_moves[rand() % count];

      // Bloqueia para escrever novo estado
      // Bloqueia para escrever novo estado
      P(SEM_GAMESTATE);
      gamestate->monsters[1].cell = next_cell;
      int player_cell = gamestate->player.cell;
      V(SEM_GAMESTATE);

      if (player_cell == next_cell) {
        P(SEM_IO);
        printf("\n\n!!! DE REPENTE, O ESPECTRO ENTRA NA SALA !!!\n> ");
        fflush(stdout);
        V(SEM_IO);
      }
    }
  }
  exit(0);
}

void PrintPlayer(int superuser) {
  P(SEM_IO);
  printf("\n--- Status do Inquisidor ---\n");
  // Leitura protegida (copia rapida seria melhor, mas aqui bloqueamos print
  // todo)
  P(SEM_GAMESTATE);
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

  if (superuser) {
    printf("\n[SUPER USER HUD]\n");
    for(int i=0; i<NUM_MONSTERS; i++) {
        printf("Monstro %d (%s): Sala %d (Vida: %d)\n", i, gamestate->monsters[i].name, 
               gamestate->monsters[i].cell, gamestate->monsters[i].energy);
    }
  }

  printf("----------------------------\n");
  V(SEM_GAMESTATE);
  V(SEM_IO);
}

void MovePlayer(int direction_cell) {
  P(SEM_GAMESTATE); // Lock inicio

  P(SEM_GAMESTATE); // Lock inicio

  // Verificar Bloqueio por QUALQUER monstro vivo na sala ATUAL ou DESTINO?
  // Normalmente bloqueiam a SALA ONDE ESTAO.
  // Se o jogador esta na Cell X, e quer ir para Y.
  // Se houver monstro em X, bloqueia saida.
  
  for(int i=0; i<NUM_MONSTERS; i++) {
      if (gamestate->monsters[i].cell == gamestate->player.cell && 
          gamestate->monsters[i].energy > 0) {
              V(SEM_GAMESTATE);
              P(SEM_IO);
              printf("%s BARRA O CAMINHO! Tens de o enfrentar!\n", gamestate->monsters[i].name);
              V(SEM_IO);
              return;
          }
  }

  if (direction_cell != -1) {
    gamestate->player.cell = direction_cell;
    V(SEM_GAMESTATE);
    
    // --- NOVO: ARMADILHAS E AMBIENTE ---
    int trap_roll = rand() % 100;
    if (trap_roll < 10) { // 10% chance de armadilha
        P(SEM_GAMESTATE);
        gamestate->player.energy -= 10;
        V(SEM_GAMESTATE);
        P(SEM_IO);
        printf("ARMADILHA! Pisaste uma laje solta e um dardo atingiu-te (-10 Vida)!\n");
        V(SEM_IO);
    } else {
        P(SEM_IO);
        printf("Caminhas pela escuridao...\n");
        V(SEM_IO);
        AmbientEvents();
    }

  } else {
    V(SEM_GAMESTATE);
    P(SEM_IO);
    printf("O caminho esta bloqueado.\n");
    V(SEM_IO);
  }
}

void Battle() {
  P(SEM_GAMESTATE);
  P(SEM_GAMESTATE);
  
  // Encontrar alvo
  int target_idx = -1;
  for(int i=0; i<NUM_MONSTERS; i++) {
      if (gamestate->monsters[i].cell == gamestate->player.cell && 
          gamestate->monsters[i].energy > 0) {
              target_idx = i;
              break; // Ataca o primeiro que encontrar
          }
  }

  if (target_idx == -1) {
    V(SEM_GAMESTATE);
    P(SEM_IO);
    printf("Nao ha hereges aqui para atacar.\n");
    V(SEM_IO);
    return;
  }

  int damage = 5;
  if (gamestate->player.object_id != -1 &&
      gamestate->objects[gamestate->player.object_id].damage_efficacy > 0) {
    damage = gamestate->objects[gamestate->player.object_id].damage_efficacy;
  }

  gamestate->monsters[target_idx].energy -= damage;
  
  // --- NOVO: COMBATE CRITICO ---
  int crit = rand() % 100;
  if (crit < 15) { // 15% chance de critico
      damage *= 2;
      gamestate->monsters[target_idx].energy -= damage; 
      P(SEM_IO);
      printf("CRITICO! Um golpe devastador!\n");
      V(SEM_IO);
  }

  int m_energy = gamestate->monsters[target_idx].energy;
  char m_name[50];
  strcpy(m_name, gamestate->monsters[target_idx].name);
  
  V(SEM_GAMESTATE); 

  P(SEM_IO);
  printf("Invocas a luz sagrada contra %s! Causas %d de dano total.\n", m_name, damage);
  V(SEM_IO);

  if (m_energy <= 0) {
    P(SEM_GAMESTATE);
    gamestate->monsters[target_idx].energy = 0; // Garante 0
    V(SEM_GAMESTATE);
    P(SEM_IO);
    printf("%s solta um grito e desfaz-se em cinzas! Vitoria momentanea.\n", m_name);
    V(SEM_IO);
  } else {
    P(SEM_GAMESTATE);
    gamestate->player.energy -= 15; // Dano fixo por agora
    V(SEM_GAMESTATE);
    P(SEM_IO);
    printf("%s contra-ataca!\nPerdeste 15 de Vida.\n", m_name);
    V(SEM_IO);
  }
}

void PrintFullMap() {
  P(SEM_IO);
  P(SEM_GAMESTATE);

  printf("\n--- DEBUG: MAPA DO MUNDO (Array Dump) ---\n");
  for (int i = 0; i < gamestate->nCells; i++) {
    printf("CELL %d: [N:%d S:%d E:%d W:%d] | Item: %d | Treasure: %d\n", i,
           gamestate->map[i].north, gamestate->map[i].south,
           gamestate->map[i].east, gamestate->map[i].west,
           gamestate->map[i].object_id, gamestate->map[i].treasure);

    if (gamestate->player.cell == i)
      printf("   >> JOGADOR AQUI <<\n");
    
    for(int m=0; m<NUM_MONSTERS; m++) {
        if (gamestate->monsters[m].cell == i)
            printf("   >> MONSTRO %d (%s) AQUI <<\n", m, gamestate->monsters[m].name);
    }
  }
  printf("-----------------------------------------\n");

  V(SEM_GAMESTATE);
  V(SEM_IO);
}

void ProcessCommand(char *full_command) {
  char *cmd = strtok(full_command, " \n\r");
  if (!cmd)
    return;

  // Nota: map[] e read-only na maior parte, mas player.cell muda.
  // Para "ver" (norte, sul etc), precisamos ler o MAPA em relacao a CELL do
  // player. MovePlayer trata do lock.

  if (strcmp(cmd, "norte") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].north;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "sul") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].south;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "este") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].east;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  } else if (strcmp(cmd, "oeste") == 0) {
    P(SEM_GAMESTATE);
    int next = gamestate->map[gamestate->player.cell].west;
    V(SEM_GAMESTATE);
    MovePlayer(next);
  }

  else if (strcmp(cmd, "ver") == 0) {
    P(SEM_GAMESTATE);
    int p_cell = gamestate->player.cell;
    V(SEM_GAMESTATE);

    P(SEM_IO);
    PrintCell(gamestate, p_cell); // PrintCell so le, nao precisa de lock de
                                  // escrita, mas precisa consistente
    V(SEM_IO);
    PrintPlayer(0); // PrintPlayer ja tem locks, 0 = no debug hud
  } else if (strcmp(cmd, "mapa") == 0) {
    PrintFullMap();
  } else if (strcmp(cmd, "atacar") == 0)
    Battle();
  else if (strcmp(cmd, "apanhar") == 0) {
    P(SEM_GAMESTATE);
    int cell = gamestate->player.cell;

    // Logica de apanhar
    if (gamestate->map[cell].object_id != -1) {
      if (gamestate->player.object_id == -1) {
        gamestate->player.object_id = gamestate->map[cell].object_id;
        int obj_id = gamestate->player.object_id;
        gamestate->map[cell].object_id = -1; // Remove do map

        // Efeito imediato
        if (gamestate->objects[obj_id].damage_efficacy < 0) {
          gamestate->player.energy += 25;
          if (gamestate->player.energy > gamestate->player.max_energy)
            gamestate->player.energy = gamestate->player.max_energy;
          gamestate->player.object_id = -1; // Consumido

          V(SEM_GAMESTATE);
          P(SEM_IO);
          printf("Usaste o item sagrado! Recuperaste vida.\n");
          V(SEM_IO);
        } else {
          V(SEM_GAMESTATE);
          P(SEM_IO);
          printf("Equipaste: %s\n", gamestate->objects[obj_id].name);
          V(SEM_IO);
        }

      } else {
        V(SEM_GAMESTATE);
        P(SEM_IO);
        printf("Mochila cheia.\n");
        V(SEM_IO);
      }
    } else if (gamestate->map[cell].treasure == 1) {
      if (gamestate->monsters[0].cell == gamestate->player.cell &&
          gamestate->monsters[0].energy > 0) {
        V(SEM_GAMESTATE);
        P(SEM_IO);
        printf("%s protege a Coroa!\n", gamestate->monsters[0].name);
        V(SEM_IO);
      } else {
        gamestate->player.treasure_status = 1;
        gamestate->map[cell].treasure = -1;
        V(SEM_GAMESTATE);
        P(SEM_IO);
        printf("RECUPERASTE A COROA SAGRADA!\n");
        V(SEM_IO);
      }
    } else {
      V(SEM_GAMESTATE);
      P(SEM_IO);
      printf("Nada para apanhar.\n");
      V(SEM_IO);
    }

  } else if (strcmp(cmd, "largar") == 0) {
    P(SEM_GAMESTATE);
    // Verificar se temos item
    if (gamestate->player.object_id == -1) {
      V(SEM_GAMESTATE);
      P(SEM_IO);
      printf("Nao tens nenhum item para largar.\n");
      V(SEM_IO);
    } else {
      // Verificar se sala tem espaco
      int cell = gamestate->player.cell;
      if (gamestate->map[cell].object_id != -1) {
        V(SEM_GAMESTATE);
        P(SEM_IO);
        printf("A sala ja tem um item (%s). Nao podes largar aqui.\n",
               gamestate->objects[gamestate->map[cell].object_id].name);
        V(SEM_IO);
      } else {
        // Largar
        gamestate->map[cell].object_id = gamestate->player.object_id;
        int dropped_id = gamestate->player.object_id;
        gamestate->player.object_id = -1;
        V(SEM_GAMESTATE);

        P(SEM_IO);
        printf("Largaste: %s\n", gamestate->objects[dropped_id].name);
        V(SEM_IO);
      }
    }

  } else if (strcmp(cmd, "ajuda") == 0) {
    P(SEM_IO);
    printf("Comandos: norte, sul, este, oeste, ver, apanhar, largar, atacar, esperar, "
           "mapa, "
           "sair\n");
    V(SEM_IO);
  } else if (strcmp(cmd, "esperar") == 0) {
      P(SEM_GAMESTATE);
      // Regeneracao
      if (gamestate->player.energy < gamestate->player.max_energy) {
          gamestate->player.energy += 5;
          if (gamestate->player.energy > gamestate->player.max_energy)
              gamestate->player.energy = gamestate->player.max_energy;
          V(SEM_GAMESTATE); 
          P(SEM_IO);
          printf("Descansas um pouco e recuperas forcas (+5 Vida).\n");
          V(SEM_IO);
      } else {
          V(SEM_GAMESTATE);
          P(SEM_IO);
          printf("Ja estas com a energia maxima.\n");
          V(SEM_IO);
      }
      AmbientEvents();
  } else if (strcmp(cmd, "sair") == 0) {
    Cleanup(0);
  } else {
    P(SEM_IO);
    printf("Comando invalido.\n");
    V(SEM_IO);
  }
}

int main(int argc, char *argv[]) {
  signal(SIGINT, Cleanup);
  srand(time(NULL));

  // 1. Criar Memoria Partilhada
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

  // 2. Criar Semaforos
  semid = semget(IPC_PRIVATE, NUM_SEMS, IPC_CREAT | 0666);
  if (semid < 0) {
    perror("Erro semget");
    exit(1);
  }

  // 3. Inicializar Semaforos
  union semun arg;
  arg.val = 1;
  if (semctl(semid, SEM_GAMESTATE, SETVAL, arg) == -1) {
    perror("Erro semctl SEM_GAMESTATE");
    exit(1);
  }
  if (semctl(semid, SEM_IO, SETVAL, arg) == -1) {
    perror("Erro semctl SEM_IO");
    exit(1);
  }

  // VERIFICAR SUPER USER MODE (Req 6.3)
  // ./ja SADMIN Energia Sala Objeto
  int super_mode = 0;
  int start_energy = 100;
  int start_cell = 0;
  int start_object = -1;

  if (argc >= 5 && strcmp(argv[1], "SADMIN") == 0) {
    super_mode = 1;
    start_energy = atoi(argv[2]);
    start_cell = atoi(argv[3]);
    start_object = atoi(argv[4]);
    printf(">>> SUPER USER MODE ATIVADO <<<\n");
    printf("Energia: %d | Sala Inicial: %d | Objeto: %d\n", start_energy,
           start_cell, start_object);
  }

  P(SEM_IO);
  printf("Qual e o teu nome, Inquisidor?\n> ");
  V(SEM_IO);

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
    P(SEM_IO);
    printf("\n\n--- NOVO JOGO INICIADO ---\n");
    V(SEM_IO);

    // Carregar do ficheiro map.txt
    InitializeWorld();

    // Aplicar Overrides do Super User
    if (super_mode) {
      gamestate->player.energy = start_energy;
      gamestate->player.max_energy = start_energy;
      gamestate->player.cell = start_cell;
      gamestate->player.object_id = start_object;
      // Se comecamos la a frente, ter cuidado com o tesouro/monstro
    }

    strcpy(gamestate->player.name, pl_name);

    pid_t pid = fork();

    if (pid == 0) {
      // PROCESSO FILHO (MONSTRO)
      MonsterLogic();
    } else {
      // PROCESSO PAI (JOGADOR)
      PrintPlayer(super_mode);

      int running = 1;
      while (running) {
        // Verifica vitoria/derrota
        P(SEM_GAMESTATE);
        if (!gamestate->game_running) {
          running = 0;
          V(SEM_GAMESTATE);
          break;
        }

        if (gamestate->player.treasure_status == 1 &&
            gamestate->player.cell == 0 && gamestate->monsters[0].energy <= 0) { // So precisa matar o REI para ganhar?
            // Ou matar todos? Vamos assumir que matar o Rei e fugir basta.
          V(SEM_GAMESTATE);
          P(SEM_IO);
          printf("\n\nGLORIA! Escapaste das ruinas com a Coroa.\n");
          V(SEM_IO);

          P(SEM_GAMESTATE);
          gamestate->game_running = 0;
          V(SEM_GAMESTATE);
          break;
        }
        if (gamestate->player.energy <= 0) {
          V(SEM_GAMESTATE);
          P(SEM_IO);
          printf("\nGAME OVER... A tua alma foi colhida.\n");
          V(SEM_IO);

          P(SEM_GAMESTATE);
          gamestate->game_running = 0;
          V(SEM_GAMESTATE);
          break;
        }

        int p_cell = gamestate->player.cell;
        V(SEM_GAMESTATE);

        P(SEM_IO);
        // PrintCell le e imprime
        // Como PrintCell nao altera nada, e seguro chamar dentro do lock de IO
        PrintCell(gamestate, p_cell);

        printf("[OPCOES: norte, sul, este, oeste, ver, apanhar, atacar, esperar, sair, "
               "ajuda]\n> ");
        fflush(stdout); // Importante para ver o prompt antes do fgets
        V(SEM_IO);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
          break;

        ProcessCommand(buffer);

        V(SEM_GAMESTATE);
      }

      // Fim do jogo
      P(SEM_GAMESTATE);
      gamestate->game_running = 0;
      V(SEM_GAMESTATE);

      wait(NULL); // Espera que o filho morra

      P(SEM_IO);
      printf("\nJogar Novamente? (s/n): ");
      fflush(stdout);
      V(SEM_IO);

      if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        replay_opt = buffer[0];
      } else
        replay_opt = 'n';
    }

  } while (replay_opt == 's' || replay_opt == 'S');

  Cleanup(0);
  return 0;
}
