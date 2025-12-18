# As Crónicas de Lisboa: O Reino das Sombras

**Trabalho Prático de Sistemas Operativos - Engenharia Informática (IPBeja)**

Este projeto é um jogo de aventura em texto ("Text-Based Adventure") desenvolvido em linguagem C, ambientado numa Lisboa pós-terramoto sobrenatural.

## 📖 História
A escuridão abateu-se sobre Lisboa. O Terramoto de 1755 não foi apenas natural: fendas abriram-se para o Inferno. Como o último Inquisidor, a tua missão é entrar nas ruínas da Sé e do Limoeiro, recuperar a Coroa Sagrada e purgar o mal.

## 🚀 Como Compilar e Jogar

### Requisitos
*   GCC (Compilador C)
*   Make (Opcional, mas recomendado)
*   Git (para gestão de versões)

### Compilação
No terminal, dentro da pasta do projeto:

**Opção A: Usando Makefile (Recomendado)**
```bash
make
./ja
```

**Opção B: Compilação Manual**
```bash
gcc main.c map.c -o ja
./ja
```

## 📂 Estrutura do Projeto
*   `main.c`: Ciclo principal do jogo e lógica do jogador.
*   `map.c`: Definição do mundo, salas e descrições.
*   `game.h`: Estruturas de dados (Player, Monster, Cell).
*   `Makefile`: Automação da compilação.

---

## 📋 Contexto Académico: Análise de Requisitos (TG2)

Este projeto foi desenvolvido com base nos seguintes requisitos da unidade curricular de Sistemas Operativos.

### 1. Objetivo do Trabalho
Desenvolvimento de um jogo de aventuras em linguagem C,executável em Linux, aplicando conceitos de programação de sistemas (processos, memória partilhada, etc.).

### 2. Elementos Fundamentais Implementados
1.  **Jogador**: Possui nome, energia, inventário e estado de missão.
2.  **Objetos**: Carregados de objeto.txt (ex: `Gladio de Prata`, `Agua Benta`).
3.  **Local da aventura**: Mapa carregado de `mapa.txt` com navegação e armadilhas.
4.  **Monstros** (3 Tipos):
    *   **Rei Esqueleto** (Boss): Protege a Coroa.
    *   **Fantasma** (Errante): Vagueia pelo mapa.
    *   **O Chato** (Bloqueador): Impede a passagem numa sala.
5.  **Tesouro**: Coroa Sagrada (Item de vitória).
6.  **Atmosfera**: Eventos aleatórios e descrições dinâmicas.

### 3. Funcionalidades
*   Exploração de mapa (Navegação entre células).
*   Carregamento dinâmico de Mapa e Objetos (`mapa.txt`, `objetos.txt`).
*   **Concorrência**: Processos distintos para Jogador e Monstros.
*   **Sincronização**: Uso de Semáforos para Memória Partilhada e I/O.
*   Ciclo de jogo (Game Loop) contínuo até vitória ou derrota.
