#include <pthread.h>   // Biblioteca para threads
#include <signal.h>    // Biblioteca para sinais (Ctrl+C)
#include <stdio.h>     // Biblioteca para input/output
#include <stdlib.h>    // Biblioteca para funcoes gerais (malloc, rand)
#include <string.h>    // Biblioteca para strings
#include <time.h>      // Biblioteca para tempo (seed, time)
#include <unistd.h>    // Biblioteca para sleep
#define MAX_NAME 50    // Tamanho maximo de nomes
#define MAX_DESC 200   // Tamanho maximo de descricoes
#define MAX_CELLS 20   // Maximo de celulas no mapa
#define MAX_OBJECTS 5  // Maximo de objetos
#define NUM_MONSTERS 2 // Numero de monstros
struct Object {
  char name[MAX_NAME];
  int damage;
}; // Estrutura de Objeto
struct Monster {
  char name[MAX_NAME];
  int energy;
  int cell;
}; // Estrutura de Monstro
struct Player {
  char name[MAX_NAME];
  int energy;
  int cell;
  int object_id;
  int treasure_status;
}; // Estrutura de Jogador
struct Cell {
  int dirs[4];
  int object_id;
  int treasure;
  char description[MAX_DESC];
}; // Estrutura de Celula (Sala)
struct GameState {
  struct Player player;
  struct Monster monsters[NUM_MONSTERS];
  struct Cell map[MAX_CELLS];
  struct Object objects[MAX_OBJECTS];
  int nCells;
  int nObjects;
  int game_running;
}; // Estado global do jogo
struct GameState *gamestate; // Ponteiro global para o estado
pthread_mutex_t mutex_gamestate =
    PTHREAD_MUTEX_INITIALIZER;                        // Mutex para o estado
pthread_mutex_t mutex_io = PTHREAD_MUTEX_INITIALIZER; // Mutex para IO
pthread_t monster_thread;                             // Thread dos monstros
void LockState() { pthread_mutex_lock(&mutex_gamestate); } // Trancar estado
void UnlockState() {
  pthread_mutex_unlock(&mutex_gamestate);
} // Destrancar estado
void LockIO() { pthread_mutex_lock(&mutex_io); }     // Trancar IO
void UnlockIO() { pthread_mutex_unlock(&mutex_io); } // Destrancar IO
void Cleanup(int sig) { // Funcao de limpeza ao sair
  (void)sig;            // Suprimir warning de parametro nao usado
  LockState();          // Trancar para escrever
  if (gamestate)
    gamestate->game_running = 0;           // Parar loop do jogo
  UnlockState();                           // Destrancar
  pthread_mutex_destroy(&mutex_gamestate); // Destruir mutex estado
  pthread_mutex_destroy(&mutex_io);        // Destruir mutex IO
  if (gamestate)
    free(gamestate);                       // Libertar memoria
  printf("\n[SISTEMA] Jogo terminado.\n"); // Mensagem de saida
  exit(0);                                 // Sair do programa
}
void LoadData() {                          // Carregar dados dos ficheiros
  FILE *f = fopen("objetos.txt", "r");     // Abrir ficheiro objetos
  if (f) {                                 // Se abriu com sucesso
    char line[100];                        // Buffer linha
    while (fgets(line, sizeof(line), f)) { // Ler linhas
      int id, dmg;
      char name[50]; // Variaveis temporarias
      // Parse: ID, depois nome (tudo ate numero), depois dano
      if (sscanf(line, "%d %49[^-0-9] %d", &id, name, &dmg) ==
          3) { // Nomes com espacos
        // Remover espacos no final do nome
        int len = strlen(name);
        while (len > 0 && name[len - 1] == ' ')
          name[--len] = '\0';
      } else if (sscanf(line, "%d %49s %d", &id, name, &dmg) !=
                 3) { // Fallback simples
        continue;     // Linha invalida, saltar
      }
      if (1) {                             // Parse e validar
        if (id >= 0 && id < MAX_OBJECTS) { // Se ID valido
          strncpy(gamestate->objects[id].name, name,
                  MAX_NAME - 1);                            // Copiar seguro
          gamestate->objects[id].name[MAX_NAME - 1] = '\0'; // Null terminator
          gamestate->objects[id].damage = dmg;              // Copiar dano
          if (id >= gamestate->nObjects)  // Atualizar contador
            gamestate->nObjects = id + 1; // Se ID maior ou igual
        }
      }
    }
    fclose(f); // Fechar ficheiro
  } else {
    printf("[AVISO] Ficheiro objetos.txt nao encontrado.\n");
  }
  f = fopen("mapa.txt", "r"); // Abrir ficheiro mapa
  if (f) {                    // Se abriu
    int id = 0;
    char buf[200], nums[100];                              // Variaveis leitura
    while (id < MAX_CELLS && fgets(buf, sizeof(buf), f)) { // Ler descricao
      buf[strcspn(buf, "\r\n")] = 0;                       // Remover newline
      strncpy(gamestate->map[id].description, buf,
              MAX_DESC - 1);                               // Copiar seguro
      gamestate->map[id].description[MAX_DESC - 1] = '\0'; // Terminator
      if (fgets(nums, sizeof(nums), f)) {                  // Ler numeros
        int v[8];                                          // Array auxiliar
        if (sscanf(nums, "%d %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3],
                   &v[4], &v[5], &v[6], &v[7]) >= 6) { // Parse e validar
          for (int i = 0; i < 4; i++)
            gamestate->map[id].dirs[i] = v[i]; // Copiar saidas
          gamestate->map[id].object_id = v[6]; // Copiar objeto ID
          gamestate->map[id].treasure = v[7];  // Copiar tesouro
          id++;                                // Proxima celula
        }
      }
    }
    gamestate->nCells = id; // Total celulas
    fclose(f);              // Fechar ficheiro
  } else {
    printf("[AVISO] Ficheiro mapa.txt nao encontrado.\n");
  }
}
void InitializeWorld() {                       // Inicializar mundo
  LoadData();                                  // Carregar do disco
  gamestate->player.energy = 100;              // HP inicial
  gamestate->player.cell = 0;                  // Sala inicial
  gamestate->player.object_id = -1;            // Sem item
  gamestate->player.treasure_status = -1;      // Sem tesouro
  strcpy(gamestate->monsters[0].name, "Boss"); // Nome Boss
  gamestate->monsters[0].energy = 100;         // HP Boss
  gamestate->monsters[0].cell = (gamestate->nCells > 6) ? 6 : 0; // Validar
  strcpy(gamestate->monsters[1].name, "Guarda");                 // Nome Guarda
  gamestate->monsters[1].energy = 50;                            // HP Guarda
  gamestate->monsters[1].cell = (gamestate->nCells > 4) ? 4 : 0; // Validar
  gamestate->game_running = 1;                                   // Jogo ON
}
void PrintCell(int cell) { // Imprimir sala
  if (cell < 0 || cell >= gamestate->nCells)
    return; // Validar celula
  printf("\n*** Status: %s | HP %d ***\n", gamestate->player.name,
         gamestate->player.energy);                        // Mostrar HP
  printf("Local: %s\n", gamestate->map[cell].description); // Mostrar Descricao
  int obj_id = gamestate->map[cell].object_id;             // ID do objeto
  if (obj_id != -1 && obj_id < MAX_OBJECTS)                // Se tem item valido
    printf("Item: %s\n", gamestate->objects[obj_id].name); // Mostrar Item
  if (gamestate->map[cell].treasure == 1)                  // Se tem tesouro
    printf("OBJETIVO: TESOURO!\n");                        // Avisar tesouro
  for (int i = 0; i < NUM_MONSTERS; i++) {                 // Loop monstros
    if (gamestate->monsters[i].cell == cell &&
        gamestate->monsters[i].energy > 0) // Se monstro aqui e vivo
      printf("INIMIGO: %s (HP: %d)\n", gamestate->monsters[i].name,
             gamestate->monsters[i].energy); // Mostrar Monstro
  }
  printf("Saidas:");                           // Label saidas
  const char *labels[] = {"N", "S", "E", "O"}; // Nomes direcoes
  for (int i = 0; i < 4; i++)                  // Loop direcoes
    if (gamestate->map[cell].dirs[i] != -1)    // Se saida existe
      printf(" %s", labels[i]);                // Imprimir direcao
  printf("\n");                                // Nova linha
}
void *MonsterLogic(void *arg) { // Thread Monstro
  (void)arg;                    // Suprimir warning
  unsigned int seed =
      (unsigned int)time(NULL) ^ (unsigned int)pthread_self(); // Seed random
  while (1) {                                                  // Loop infinito
    sleep(10);                      // Esperar 10s (Balanceamento)
    LockState();                    // Trancar estado
    if (!gamestate->game_running) { // Se jogo acabou
      UnlockState();                // Destrancar e sair
      break;                        // Break loop
    }
    if (gamestate->monsters[1].energy > 0) {  // Se Guarda vivo
      int curr = gamestate->monsters[1].cell; // Celula atual
      int dir_idx = rand_r(&seed) % 2;        // Escolher 0 ou 1 (N ou S)
      if (curr < gamestate->nCells) {         // Validar celula
        int next = gamestate->map[curr].dirs[dir_idx]; // Proxima celula
        if (next != -1)
          gamestate->monsters[1].cell = next; // Mover se possivel
      }
    }
    for (int i = 0; i < NUM_MONSTERS; i++) { // Loop ataque
      if (gamestate->monsters[i].energy > 0 &&
          gamestate->monsters[i].cell ==
              gamestate->player.cell) { // Se junto ao player
        gamestate->player.energy -= 10; // Dano no player
        LockIO();                       // Trancar IO para output
        printf("\nMonster attack! HP: %d\n> ",
               gamestate->player.energy); // Aviso ataque
        fflush(stdout);                   // Flush output
        UnlockIO();                       // Destrancar IO
      }
    }
    UnlockState(); // Destrancar
  }
  return NULL; // Fim thread
}
void ProcessCmd(char *cmd) {                      // Processar comando
  cmd[strcspn(cmd, "\n")] = 0;                    // Remover newline
  LockState();                                    // Trancar estado
  int p_cell = gamestate->player.cell;            // Celula jogador
  const char *move_cmds[] = {"n", "s", "e", "o"}; // Comandos movimento
  int moved = 0;                                  // Flag movimento
  for (int i = 0; i < 4; i++) {                   // Loop direcoes
    if (strcmp(cmd, move_cmds[i]) == 0) {         // Se comando match
      int next = gamestate->map[p_cell].dirs[i];  // Proxima celula
      if (next != -1)
        gamestate->player.cell = next; // Mover
      moved = 1;                       // Marcar movido
      break;                           // Sair loop
    }
  }
  if (moved) {
    UnlockState();
    return;
  } // Se moveu, retornar
  if (strcmp(cmd, "ver") == 0) { // Comando ver
    PrintCell(p_cell);
    UnlockState();
    return;                              // Mostrar sala
  } else if (strcmp(cmd, "sair") == 0) { // Comando sair
    UnlockState();
    Cleanup(0);                                          // Sair jogo
  } else if (strcmp(cmd, "apanhar") == 0) {              // Comando apanhar
    if (gamestate->map[p_cell].treasure == 1) {          // Se tesouro
      gamestate->player.treasure_status = 1;             // Marcar apanhado
      gamestate->map[p_cell].treasure = -1;              // Remover da sala
      printf("OURO!\n");                                 // Aviso
    } else if (gamestate->map[p_cell].object_id != -1) { // Se objeto
      int obj_id = gamestate->map[p_cell].object_id;     // ID do objeto
      if (obj_id >= 0 && obj_id < MAX_OBJECTS) {         // Validar ID
        int effect = gamestate->objects[obj_id].damage;  // Efeito do item
        // Se item e consumivel (dano negativo = cura), usar automaticamente
        if (effect < 0) {
          gamestate->player.energy -= effect; // Aplicar cura
          if (gamestate->player.energy > 100) // Limitar maximo
            gamestate->player.energy = 100;
          gamestate->map[p_cell].object_id = -1; // Remover da sala
          printf("Usaste %s! Energia: %d\n", gamestate->objects[obj_id].name,
                 gamestate->player.energy); // Aviso uso
        } else {
          // Comportamento normal: trocar ou apanhar
          int old_obj = gamestate->player.object_id;
          gamestate->player.object_id = obj_id;
          gamestate->map[p_cell].object_id = old_obj;
          if (old_obj != -1) {
            printf("Trocaste %s por %s.\n", gamestate->objects[old_obj].name,
                   gamestate->objects[obj_id].name);
          } else {
            printf("Item apanhado: %s\n", gamestate->objects[obj_id].name);
          }
        }
      }
    }
  } else if (strcmp(cmd, "atacar") == 0) {   // Comando atacar
    for (int i = 0; i < NUM_MONSTERS; i++) { // Loop monstros
      if (gamestate->monsters[i].cell == p_cell &&
          gamestate->monsters[i].energy > 0) {    // Se alvo valido
        int dmg = 5;                              // Dano base (soco)
        int p_obj = gamestate->player.object_id;  // ID objeto player
        if (p_obj != -1 && p_obj < MAX_OBJECTS)   // Se tem arma valida
          dmg = gamestate->objects[p_obj].damage; // Usar dano da arma
        gamestate->monsters[i].energy -= dmg;
        printf("Hit %s! Dano: %d\n", gamestate->monsters[i].name,
               dmg); // Aviso hit
        break;       // So ataca um monstro
      }
    }
  } else if (strcmp(cmd, "usar") == 0) {
    int p_obj = gamestate->player.object_id;         // ID objeto player
    if (p_obj != -1 && p_obj < MAX_OBJECTS) {        // Se tem item valido
      int effect = gamestate->objects[p_obj].damage; // Efeito do item
      gamestate->player.energy -= effect;            // Dano negativo cura
      if (gamestate->player.energy > 100)            // Limitar maximo
        gamestate->player.energy = 100;              // Cap em 100
      printf("Item usado! Energia: %d\n", gamestate->player.energy);
      gamestate->player.object_id = -1; // Consumir item
    } else {
      printf("Nenhum item para usar.\n");
    }
  }
  UnlockState(); // Destrancar fim
}
int main() {               // Funcao principal
  signal(SIGINT, Cleanup); // Handler Ctrl+C
  gamestate =
      (struct GameState *)malloc(sizeof(struct GameState)); // Alocar estado
  if (!gamestate) {
    perror("Malloc falhou");
    exit(1);
  } // Erro alocacao
  memset(gamestate, 0, sizeof(struct GameState)); // Limpar memoria
  InitializeWorld();                              // Iniciar mundo
  printf("Nome: ");                               // Pedir nome
  fflush(stdout);                                 // Garantir output
  char buf[50];
  if (fgets(buf, 50, stdin)) {
    buf[strcspn(buf, "\n")] = 0;                          // Remover enter
    if (strlen(buf) > 0) {                                // Se nome nao vazio
      strncpy(gamestate->player.name, buf, MAX_NAME - 1); // Copiar seguro
      gamestate->player.name[MAX_NAME - 1] = '\0';        // Terminator
    } else {
      strcpy(gamestate->player.name, "Jogador"); // Nome padrao
    }
  } // Ler nome
  if (pthread_create(&monster_thread, NULL, MonsterLogic, NULL) != 0) {
    perror("Falha");
    Cleanup(1);
  } // Criar thread
  while (1) {    // Loop jogo
    LockState(); // Trancar
    if (gamestate->player.treasure_status == 1 &&
        gamestate->player.cell == 0) { // Condicao Vitoria
      UnlockState();
      printf("\n\n*** VITORIA %s! ***\n",
             gamestate->player.name); // Print Vitoria
      LockState();
      gamestate->game_running = 0;
      UnlockState();
      break; // Sair loop
    }
    if (!gamestate->game_running ||
        gamestate->player.energy <= 0) { // Condicao Derrota
      UnlockState();
      printf("\n\n*** GAME OVER %s! ***\n", gamestate->player.name);
      break; // Sair loop
    }
    int c = gamestate->player.cell; // Sala atual
    LockIO();
    PrintCell(c); // Imprimir sala
    printf("[Comandos: n, s, e, o, ver, apanhar, atacar, usar, "
           "sair]\n> "); // Prompt
    fflush(stdout);
    UnlockIO();
    UnlockState();
    if (!fgets(buf, 50, stdin)) {
      LockState();
      gamestate->game_running = 0;
      UnlockState();
      break;
    } // Ler comando ou Sair
    ProcessCmd(buf); // Processar
  }
  LockState();                        // Trancar antes cleanup
  gamestate->game_running = 0;        // Parar thread
  UnlockState();                      // Destrancar
  pthread_join(monster_thread, NULL); // Esperar thread
  Cleanup(0);                         // Limpeza final
  return 0;                           // Return 0
}