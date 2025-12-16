# Análise do Documento – Trabalho de Grupo n.º 2

## Unidade Curricular
**Sistemas Operativos**  
Curso de Engenharia Informática  
Escola Superior de Tecnologia e Gestão de Beja

---

## 1. Objetivo do Trabalho

O trabalho tem como objetivo o desenvolvimento de um **jogo de aventuras em linguagem C**, a executar em ambiente **Linux**, aplicando conceitos fundamentais de **programação de sistemas**. O projeto inspira-se em jogos clássicos como *Colossal Cave Adventure* e *Zork*.

Para além da vertente lúdica, o foco principal está na utilização de mecanismos do sistema operativo, como:
- processos ou threads;
- memória partilhada;
- sincronização entre execuções concorrentes;
- interação através da linha de comandos.

---

## 2. Elementos Fundamentais do Jogo

O jogo deve integrar obrigatoriamente os seguintes cinco elementos:

1. **Jogador**
2. **Objetos**
3. **Local da aventura (salas/células)**
4. **Monstro**
5. **Tesouro**

### 2.1 Jogador
Representado por uma estrutura (`struct`) com os seguintes campos mínimos:
- nome;
- energia;
- localização atual;
- objeto transportado (`-1` se nenhum);
- posse do tesouro (`-1` ou `1`).

### 2.2 Local da Aventura
O mapa do jogo é representado por um **vetor de salas**. Cada sala contém:
- ligações possíveis (norte, sul, este, oeste, cima, baixo);
- descrição textual;
- objeto presente (`-1` se não existir);
- indicação da existência de tesouro.

### 2.3 Objetos
Os objetos são armazenados num vetor de estruturas, contendo:
- nome do objeto;
- nível de eficácia em combate.

### 2.4 Monstro
O monstro é representado por uma estrutura simples com:
- energia;
- localização atual.

---

## 3. Lógica Geral do Programa

O funcionamento base do jogo segue um ciclo de inicialização seguido de um ciclo principal onde o monstro e o jogador se movimentam, são processados comandos e ocorre combate quando ambos se encontram na mesma sala.

---

## 4. Requisitos Técnicos

### Requisitos Mínimos
- Implementação funcional do jogo base;
- Execução através da linha de comandos (`ja`);
- Relatório final.

### Requisitos Avançados
- Processos ou threads;
- Sincronização;
- Modo super user;
- Ficheiros de configuração;
- Gravação do jogo;
- Interface NCURSES.

---

## 5. Avaliação

- **−0,5 valores** – não cumpre requisitos mínimos  
- **0 valores** – cumpre requisitos mínimos  
- **+0,5 valores** – cumpre todos os requisitos

---

## 6. Conclusão

O trabalho avalia competências práticas em **Sistemas Operativos**, nomeadamente concorrência, sincronização e programação em C.
