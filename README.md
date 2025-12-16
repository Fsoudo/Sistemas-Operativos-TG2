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
2.  **Objetos**: `Gladio de Prata` (Arma) e `Agua Benta` (Cura).
3.  **Local da aventura**: Mapa com navegação (Norte, Sul...) e descrições ricas.
4.  **Monstro**: Duque das Sombras (Boss com energia e localização).
5.  **Tesouro**: Coroa Sagrada (Item de vitória).

### 3. Funcionalidades
*   Exploração de mapa (Navegação entre células).
*   Comandos de interação: `ver`, `apanhar`, `atacar`.
*   Ciclo de jogo (Game Loop) contínuo até vitória ou derrota.
