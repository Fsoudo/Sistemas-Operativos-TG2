// As Crónicas de Lisboa - Minimized & Fixed
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  char n[50];
  int d;
} Obj;
typedef struct {
  char n[50];
  int hp, c, o, t;
} Char; // Added o, t
typedef struct {
  int d[4], o, t;
  char desc[200];
} Cell;
typedef struct {
  Char p, m[2];
  Cell map[20];
  Obj objs[5];
  int run;
} Game;

Game *g;
pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;
void L() { pthread_mutex_lock(&M); }
void U() { pthread_mutex_unlock(&M); }

void Mov(Char *c, int t) {
  if (t != -1)
    c->c = t;
}
int Dir(char *s) {
  return *s == 'n' ? 0 : *s == 's' ? 1 : *s == 'e' ? 2 : *s == 'o' ? 3 : -1;
}

void *Mon(void *a) {
  unsigned int s = time(0) ^ pthread_self();
  while (1) {
    sleep(10);
    L();
    if (!g->run) {
      U();
      break;
    }
    if (g->m[1].hp > 0)
      Mov(&g->m[1], g->map[g->m[1].c].d[rand_r(&s) % 2]);
    U();
  }
  return 0;
}

void Load() {
  FILE *f = fopen("objetos.txt", "r");
  char b[200];
  int i = 0, v[8];
  // Simplified object loading, assumes correct file
  if (f) {
    while (fgets(b, 200, f))
      sscanf(b, "%d %s %d", &i, g->objs[i].n, &g->objs[i].d);
    fclose(f);
  }
  f = fopen("mapa.txt", "r");
  i = 0;
  if (f) {
    while (i < 20 && fgets(b, 200, f)) {
      b[strcspn(b, "\r\n")] = 0;
      strcpy(g->map[i].desc, b);
      if (fgets(b, 200, f)) {
        sscanf(b, "%d %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4],
               &v[5], &v[6], &v[7]);
        memcpy(g->map[i].d, v, 16);
        g->map[i].o = v[6];
        g->map[i].t = v[7];
        i++;
      }
    }
    g->run = 1;
    fclose(f);
  } // Set run=1 here just in case, but Init does it too
}

int main() {
  g = calloc(1, sizeof(Game));
  Load();
  g->run = 1;
  strcpy(g->p.n, "Hero");
  g->p.hp = 100;
  g->p.o = -1;
  g->p.t = -1;
  strcpy(g->m[0].n, "Boss");
  g->m[0].hp = 100;
  g->m[0].c = 6;
  strcpy(g->m[1].n, "Guard");
  g->m[1].hp = 50;
  g->m[1].c = 4;

  pthread_t t;
  pthread_create(&t, 0, Mon, 0);
  L();
  printf("Nome: ");
  U();
  char b[50];
  if (fgets(g->p.n, 50, stdin))
    g->p.n[strcspn(g->p.n, "\n")] = 0;

  while (g->run && g->p.hp > 0) {
    L();
    int c = g->p.c;
    if (g->p.t == 1 && c == 0) {
      printf("WIN!\n");
      U();
      break;
    }
    printf("\n%s\n", g->map[c].desc);
    if (g->map[c].o != -1)
      printf("Item: %s\n", g->objs[g->map[c].o].n);
    if (g->map[c].t == 1)
      printf("TREASURE!\n");
    // Enemy print logic condensed
    Char *e = 0;
    if (g->m[0].c == c && g->m[0].hp > 0)
      e = &g->m[0];
    else if (g->m[1].c == c && g->m[1].hp > 0)
      e = &g->m[1];
    if (e)
      printf("Enemy: %s (%d)\n", e->n, e->hp);

    printf("Exits:");
    char *dirs[] = {"N", "S", "E", "O"};
    for (int i = 0; i < 4; i++)
      if (g->map[c].d[i] != -1)
        printf(" %s", dirs[i]);
    printf("\n> ");
    fflush(stdout);
    U();

    if (!fgets(b, 50, stdin))
      break;
    b[strcspn(b, "\n")] = 0;
    L();
    int d = Dir(b);
    if (d != -1)
      Mov(&g->p, g->map[c].d[d]);
    else if (!strcmp(b, "ver"))
      ;
    else if (!strcmp(b, "sair")) {
      g->run = 0;
    } else if (!strcmp(b, "apanhar")) {
      if (g->map[c].t == 1) {
        g->p.t = 1;
        g->map[c].t = -1;
        printf("GOLD!\n");
      }
      if (g->map[c].o != -1) {
        g->p.o = g->map[c].o;
        g->map[c].o = -1;
        printf("Got %s\n", g->objs[g->p.o].n);
      }
    } else if (!strcmp(b, "usar")) {
      if (g->p.o != -1) {
        int effect = g->objs[g->p.o].d;
        if (effect < 0) {  // Healing item
          g->p.hp -= effect;  // Subtract negative = add
          printf("Usou %s e recuperou %d HP! (HP atual: %d)\n", 
                 g->objs[g->p.o].n, -effect, g->p.hp);
          g->p.o = -1;  // Remove item
        } else {
          printf("Nao pode usar %s dessa forma.\n", g->objs[g->p.o].n);
        }
      } else {
        printf("Nao tem nenhum item para usar.\n");
      }
    } else if (!strcmp(b, "atacar")) {
      Char *m = g->m[0].c == c ? &g->m[0] : g->m[1].c == c ? &g->m[1] : 0;
      if (m && m->hp > 0) {
        m->hp -= g->p.o != -1 ? g->objs[g->p.o].d : 10;
        printf("Hit %s!\n", m->n);
      } else {
        printf("Nao ha inimigo aqui para atacar.\n");
      }
    } else {
      printf("Comando invalido. Comandos: n/s/e/o, ver, apanhar, usar, atacar, sair\n");
    }
    U();
  }
  g->run = 0;
  pthread_cancel(t);
  pthread_join(t, 0); // Cancel logic for faster exit
  free(g);
  printf("End.\n");
  return 0;
}
