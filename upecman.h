/***************************************************************************
 *   upecman.h                                Version 20160529.013231      *
 *                                                                         *
 *   Pacman Ncurses                                                        *
 *   Copyright (C) 2016         by Ruben Carlo Benante                     *
 ***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *   To contact the author, please write to:                               *
 *   Ruben Carlo Benante                                                   *
 *   Email: rcb@beco.cc                                                    *
 *   Webpage: http://www.beco.cc                                           *
 *   Phone: +55 (81) 3184-7555                                             *
 ***************************************************************************/

/* ---------------------------------------------------------------------- */
/* definitions */

#ifndef VERSION
#define VERSION "20160529.013231" /**< Version Number */
#endif

/* Debug */
#ifndef DEBUG /* gcc -DDEBUG=1 */
#define DEBUG 0 /**< Activate/deactivate debug mode */
#endif

/* @brief Debug message if DEBUG on */
#define IFDEBUG(M) if(DEBUG) fprintf(stderr, "[DEBUG file:%s line:%d]: " M "\n", __FILE__, __LINE__); else {;}

/* limits */
#define SBUFF 256 /**< String buffer */
#define LABL 23 /**< Number of lines in labyrinth */
#define LABC 21 /**< Number of columns in labyrinth */


// fantasmas //
#define direita 1
#define esquerda 2
#define baixo 3
#define cima 4

/* ---------------------------------------------------------------------- */
/* globals */

static int verb = 0; /* verbose level, global within the file */

const char buh[10][17] =
{
    {"    ooooooo     "},
    {" ooooooooooooo  "},
    {"ooooooooooooooo "},
    {"oo     oo     oo"},
    {"oo   ##oo   ##oo"},
    {"oo   ##oo   ##oo"},
    {"oooooooooooooooo"},
    {"oooooooooooooooo"},
    {"ooo ooooooo oooo"},
    {"oo   ooooo   ooo"}
};

const char logo[15][106] =
{
    {"oooooooooo   oooooooooo   oooooooo                        "},
    {"oo  oooo  o  oo        o  oo                              "},
    {"oo  o  o  o  oo  ooooooo  oo                                                 ................o....."},
    {"oo  oooo oo  oo        o  oo                                                          ."},
    {"oo  o        oo  ooooooo  oo                     ooooooo                              ."},
    {"oo  o        oo        o  oo                  oooooooooooo         .                  ."},
    {"ooooo        ooooooooooo  oooooooo          ooooooooooo            .                  ."},
    {"oo      ooo    ooo    ooo       oo         oooooooooo              .                  ."},
    {"oo o   o oo   oo  oo  oo o      oo        oooooooo                 .                  ."},
    {"oo  o o  oo  oo    oo oo  o     oo        oooooooo        ...............o............................."},
    {"oo   o   oo  oo    oo oo   o    oo         oooooooooo                                        ."},
    {"oo       oo  oooooooo oo    o   oo          ooooooooooo                                      ."},
    {"oo       oo  oo    oo oo     o  oo            oooooooooooo                                   ."},
    {"oo       oo  oo    oo oo      o oo               oooooooo                                    ."},
    {"oo       oo  oo    oo oo       ooo                        "}
};

const char labmodel[LABL][LABC] =
{
    /*01234567890123456789*/
    {"####################"},    /* 0*/
    {"#........##........#"},    /* 1*/
    {"#o##.###.##.###.##o#"},    /* 2*/
    {"#..................#"},    /* 3*/
    {"#.##.#.######.#.##.#"},    /* 4*/
    {"#....#...##...#....#"},    /* 5*/
    {"####.###.##.###.####"},    /* 6*/
    {"   #.#........#.#   "},    /* 7*/
    {"   #.#.##--##.#.#   "},    /* 8*/
    {"####.#.#    #.#.####"},    /* 9*/
    {"    ...#    #...    "},    /*10*/
    {"####.#.#    #.#.####"},    /*11*/
    {"   #.#.######.#.#   "},    /*12*/
    {"   #.#........#.#   "},    /*13*/
    {"####.#.######.#.####"},    /*14*/
    {"#........##........#"},    /*15*/
    {"#o##.###.##.###.##o#"},    /*16*/
    {"#..#..... ......#..#"},    /*17*/
    {"##.#.#.######.#.#.##"},    /*18*/
    {"#....#...##...#....#"},    /*19*/
    {"#.######.##.######.#"},    /*20*/
    {"#..................#"},    /*21*/
    {"####################"}     /*22*/
};


typedef enum e_ghostmode {chase, scatter, afraid, dead} t_ghostmode;
typedef enum e_direction {up, left, down, right} t_direction;
typedef enum e_ghosts {blinky, pinky, inky, clyde} t_ghosts;

typedef struct st_position
{
    int y; /* line */
    int x; /* column */
} t_pos;

typedef struct st_pacman
{
    t_pos pos; /* current pacman position */
    t_direction dir; /* direction */
    int life; /* how many lifes pacman has */
    int pellet;
    int cereja1, cereja2, cereja3;
    int next;
    int score; /* dot = 10 point; pill = 50 points; ghost = 750 points; cherry = 500 */
} t_pacman;

typedef struct st_ghost
{
    t_pos pos; /* ghost position */
    t_direction dir; /* ghost current direction */
    t_pos starget; /* ghost scatter preferred corner */
    t_ghostmode mode; /* chase, scatter, afraid or dead */
} t_ghost;

typedef struct st_pacdata
{
    char lab[LABL][LABC]; /* the labyrinth map */
    t_pacman pacman; /* pacman data */
    t_ghost ghost[4]; /* ghost[blinky], ghost[pinky], ghost[inky], ghost[clyde] */
} t_game;

clock_t afraid_t;
int TempoAfraid = 0;

/* ---------------------------------------------------------------------- */
/* prototypes */

void help(void); /* print some help */
void copyr(void); /* print version and copyright information */
t_game upecman_init(void); /* initialize game variables */
void printlab(int scrx, int scry, t_game g); /* print the labyrinth */
void coresdolab(int scrx, int scry,t_game g); // mostra as cores do labirinto, pellets e pac dots
t_pacman mvpacman(t_game g); // funcao do movimento do pacman
t_game pastilhas(t_game g); // interacao mapa, pastilas e pacman
t_game modosfantasmas(t_game g); // 4 modos dos fantasmas
t_game IAfantasmas(t_game g, int i); // inteligencia artificial da movimentacao dos fantasmas
t_game temporizador(t_game g, int TempoInicio, int TempoAtual); // temporizador para os modos dos fantasas e da cereja
t_game colisaofantasma(t_game g); // define a morte do pacman ou do fantasma quando se encontram
t_game cereja(t_game g, int TempoInicio, int TempoAtual); // funcao da cereja
void pacHelp();//funÃ§Ã£o de printar a tela help
/* ---------------------------------------------------------------------- */
/* vi: set ai et ts=4 sw=4 tw=0 wm=0 fo=croql : C config for Vim modeline */
/* Template by Dr. Beco <rcb at beco dot cc> Version 20160612.142044      */