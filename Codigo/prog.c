#include <stdio.h>
#include <string.h>

struct Player{
	char name[ 20 ];
	int energy;
	int cell;
	int object;
};

struct Cell{
	int north;
	int south;
	int west;
	int east;
	int up;
	int down;
	int object;
	int treasure;
	char description[ 100 ];
};

void InitializePlayer( struct Player* pPlayer ){
	printf( "Olá aventureiro! Como te chamas?\n" );
	scanf( "%s", pPlayer->name );
	pPlayer->energy = 100;
	pPlayer->cell = 0;
	pPlayer->object = -1;
}

void PrintPlayer( struct Player player ){
	printf( "Olá %s tens mesmo a certeza de quereres avançar!!!\n", player.name ); 
	printf( "Energia: %d\n", player.energy );
	printf( "Célula: %d\n", player.cell );
	printf( "Objeto: %d\n", player.object );
}


int main(){
	struct Player player;
	struct Cell map[ 100 ];
	int nCells;
	
	//Initialize Player
	//player campos vazios
	InitializePlayer( &player );
	//player campos preenchidos
	
	//Print Player
	PrintPlayer( player );
	
	//Initialize Monster
	//...
	
	//Print Monster
	//...
	
	//Initialize Map
	//Cell 0
	map[ 0 ].north = -1;
	map[ 0 ].south = -1;
	map[ 0 ].west = -1;
	map[ 0 ].east = 1;
	map[ 0 ].up = -1;
	map[ 0 ].down = -1;
	map[ 0 ].object = -1;
	map[ 0 ].treasure = -1;
	strcpy( map[ 0 ].description, "Está no quintal de um castelo abandonado. Ouvem-se ruídos estranhos lá dentro!" );
	
	//Cell 1
	map[ 1 ].north = -1;
	map[ 1 ].south = 2;
	map[ 1 ].west = 0;
	map[ 1 ].east = 3;
	map[ 1 ].up = -1;
	map[ 1 ].down = -1;
	map[ 1 ].object = -1;
	map[ 1 ].treasure = -1;
	strcpy( map[ 1 ].description, "Está no hall de entrada! Vê uma armadura enferrujada junto à parede." );
	
	nCells = 2;
	
	//Print Map
	//for
	
	return 0;
}
