# Requisitos do Jogo de Aventuras – Trabalho de Grupo n.º 2

## 1. Requisitos Gerais
- O jogo deve ser desenvolvido em **linguagem C**.
- Deve ser executado no **sistema operativo Linux**.
- A execução do jogo deve ser feita através da **linha de comandos**.
- O programa deve chamar-se **`ja`**.

---

## 2. Elementos Obrigatórios do Jogo
O jogo deve conter, no mínimo, os seguintes elementos:

1. **Jogador**
2. **Objetos**
3. **Local da aventura (salas/células)**
4. **Monstro**
5. **Tesouro**

---

## 3. Requisitos Estruturais

### 3.1 Jogador
O jogador deve ser representado por uma estrutura contendo:
- Nome;
- Energia;
- Localização atual;
- Indicação do objeto transportado (`-1` se nenhum);
- Indicação se possui o tesouro (`-1` ou `1`).

### 3.2 Local da Aventura
- O local da aventura deve ser representado por um **vetor de salas**.
- Cada sala deve conter:
  - Ligações possíveis: norte, sul, este, oeste, cima e baixo;
  - Descrição textual da sala;
  - Objeto presente (`-1` se não existir);
  - Indicação da existência de tesouro (`-1` ou `1`).
- Quando não existir passagem numa direção, o valor deve ser `-1`.

### 3.3 Objetos
- Os objetos devem ser armazenados num vetor de estruturas.
- Cada objeto deve conter:
  - Nome;
  - Nível de eficácia em combate.

### 3.4 Monstro
- O monstro deve ser representado por uma estrutura contendo:
  - Energia;
  - Localização atual.
- O monstro deve movimentar-se entre salas tal como o jogador.

---

## 4. Requisitos Funcionais

### 4.1 Ciclo Principal do Jogo
O jogo deve seguir a seguinte lógica base:
1. Inicialização do jogador;
2. Inicialização dos objetos;
3. Inicialização do local da aventura;
4. Inicialização do monstro;
5. Ciclo principal:
   - Movimentação do monstro;
   - Descrição da sala atual do jogador;
   - Leitura do comando do jogador;
   - Movimentação do jogador;
   - Combate quando jogador e monstro se encontram na mesma sala;
6. Apresentação do resultado final.

---

## 5. Requisitos Mínimos
- Implementação de uma versão simples e funcional do jogo;
- Cumprimento da estrutura base do pseudo-código fornecido;
- Relatório escrito descrevendo o trabalho realizado.

---

## 6. Requisitos Avançados

### 6.1 Concorrência
- O jogador e o monstro devem ser executados em **processos distintos ou threads**.
- As variáveis comuns devem ser **partilhadas** entre os processos/threads.

### 6.2 Sincronização
- Devem ser identificadas situações críticas que requeiram sincronização.
- Durante o combate, o jogador e o monstro não devem mover-se.
- Devem ser utilizados mecanismos de sincronização abordados nas aulas.

### 6.3 Modo Super User
- O jogo deve permitir execução em modo especial através da linha de comandos.
- Devem ser passados como argumentos:
  - Código especial;
  - Energia inicial do jogador;
  - Sala inicial;
  - Objeto transportado.
- Neste modo deve ser visível a localização do monstro.

### 6.4 Ficheiros
- As salas devem ser inicializadas a partir de um ficheiro de texto.
- Os objetos também devem ser carregados a partir de ficheiro.
- Cada sala deve ser separada por uma linha vazia no ficheiro.

### 6.5 Funcionalidades Extra
- Gravação e carregamento do estado do jogo;
- Interface melhorada usando a biblioteca **NCURSES**.

---

## 7. Requisitos de Avaliação
- Cumprimento apenas dos requisitos mínimos: classificação base;
- Cumprimento de todos os requisitos: classificação máxima;
- Apresentação oral obrigatória com domínio total do código.
