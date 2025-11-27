/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "upecman.h"

/* Variável para controle de vitória dinâmica */
int TOTAL_PASTILHAS = 0;

/* ==========================================================================
   ETAPA 1: CONFIGURAÇÃO INICIAL E MENU
   ========================================================================== */

int main(int argc, char *argv[])
{
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);

    // Habilita leitura de teclas especiais (setas)
    keypad(stdscr, TRUE);

    int i, destaque = 0, escolha;
    int scrx, scry;
    int TempoInicio, TempoAtual;

    TempoAfraid = 0;

    TempoInicio = time(NULL);
    TempoAtual = TempoInicio;

    char opcoes[3][30] = {"# JOGAR  #", "INSTRUCOES", "#  SAIR  #"};
    getmaxyx(stdscr, scry, scrx);

    if(!has_colors()) {
        endwin();
        printf("Erro: O terminal nao suporta cores.\n");
        return -1;
    }

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLUE);

    WINDOW *janela = newwin(scry - 5, scrx - 10, 2, 5);
    box(janela, 0, 0);
    keypad(janela, true);

    while(1)
    {
        for(int k = 0; k < 15; k++) {
            wattron(janela, COLOR_PAIR(1));
            if(scrx > 80) mvwprintw(janela, k + 1, (scrx - 10)/2 - 53 + 20, "%s", logo[k]);
            else mvwprintw(janela, k + 1, 1, "%s", logo[k]);
            wattroff(janela, COLOR_PAIR(1));
        }

        for(i = 0; i < 3; i++) {
            mvwprintw(janela, 18, ((scrx-10) / 2) - 9, "ESCOLHA UMA OPCAO:");
            if(i == destaque) wattron(janela, A_REVERSE);
            mvwprintw(janela, i + 20, ((scrx-10) / 2) - 6, "%s", opcoes[i]);
            wattroff(janela, A_REVERSE);
        }

        escolha = wgetch(janela);
        switch(escolha) {
            case KEY_UP: destaque = (destaque - 1 < 0) ? 0 : destaque - 1; break;
            case KEY_DOWN: destaque = (destaque + 1 > 2) ? 2 : destaque + 1; break;
            case 10:
                if(destaque == 0) goto iniciar_jogo;
                if(destaque == 1) pacHelp();
                if(destaque == 2) { clear(); endwin(); return EXIT_SUCCESS; }
                break;
        }
    }

iniciar_jogo:

/* ==========================================================================
   ETAPA 2: INICIALIZAÇÃO DO JOGO
   ========================================================================== */

    t_game g;
    int kin = 0;
    int opt;

    opterr = 0;
    while((opt = getopt(argc, argv, "hvc")) != EOF)
        switch(opt) {
            case 'h': help(); break;
            case 'v': verb++; break;
            case 'c': copyr(); break;
        }

    g = upecman_init();

    // Contagem automatica para vitoria
    TOTAL_PASTILHAS = 0;
    for(int y=0; y<LABL; y++) {
        for(int x=0; x<LABC; x++) {
            if(g.lab[y][x] == '.' || g.lab[y][x] == 'o') TOTAL_PASTILHAS++;
        }
    }

    g.lab[17][9] = ' '; // Limpa start
    printlab(scrx, scry, g);

    attron(A_BLINK | A_BOLD);
    mvprintw(scry/2, scrx/2 - 10, "ARE YOU READY? (y/n)");
    attroff(A_BLINK | A_BOLD);
    refresh();

    timeout(-1);
    while(1) {
        kin = getch();
        if(kin == 'y' || kin == 'Y') break;
        if(kin == 'n' || kin == 'N') { endwin(); return EXIT_SUCCESS; }
    }

    timeout(150);
    TempoInicio = time(NULL);
    g.pacman.next = left;
    clear();

/* ==========================================================================
   ETAPA 3: LOOP PRINCIPAL (ENGINE)
   ========================================================================== */

    while(kin != 'e')
    {
        kin = getch();

        if(kin == KEY_LEFT) g.pacman.next = left;
        else if(kin == KEY_RIGHT) g.pacman.next = right;
        else if(kin == KEY_DOWN) g.pacman.next = down;
        else if(kin == KEY_UP) g.pacman.next = up;

        // --- ATUALIZACAO LOGICA ---
        g.pacman = mvpacman(g);
        g = pastilhas(g);

       if((TempoAfraid == 0) || (TempoAfraid == 1 && (time(NULL) - afraid_t) > 10)) {
            // Se o tempo acabou agora, precisamos "limpar" o medo manualmente
            if(TempoAfraid == 1) {
                TempoAfraid = 0;
                for(int k = 0; k < 4; k++) {
                    // Se o fantasma ainda estiver com medo, forçamos ele a voltar ao normal
                    // para que o temporizador possa assumir o controle
                    if(g.ghost[k].mode == afraid) g.ghost[k].mode = chase;
                }
            }

            TempoAtual = time(NULL);
            g = temporizador(g, TempoInicio, TempoAtual);
        }
        // A funcao cereja agora controla spawn E comer a cereja
        g = cereja(g, TempoInicio, TempoAtual);

        g = modosfantasmas(g);
        g = colisaofantasma(g);

        getmaxyx(stdscr, scry, scrx);
        printlab(scrx, scry, g);

/* ==========================================================================
   ETAPA 4: VERIFICAÇÃO DE ESTADO (WIN/LOSE)
   ========================================================================== */

        if(g.pacman.life == 0) // GAME OVER
        {
            timeout(-1);
            clear();
            char *texto_go[] = {
                "  ___   _   __  __ ___ ", " / __| /_\\ |  \\/  | __|",
                "| (_ |/ _ \\| |\\/| | _| ", " \\___/_/ \\_\\_|  |_|___|",
                "  _____   _____ ___    ", " / _ \\ \\ / / __| _ \\   ",
                "| (_) \\ V /| _||   /   ", " \\___/ \\_/ |___|_|_\\   "
            };

            int cy = scry / 2; int cx = scrx / 2;
            while(1) {
                attron(COLOR_PAIR(2));
                mvhline(cy - 9, cx - 20, ACS_HLINE, 40); mvhline(cy + 9, cx - 20, ACS_HLINE, 40);
                mvvline(cy - 9, cx - 20, ACS_VLINE, 18); mvvline(cy - 9, cx + 20, ACS_VLINE, 18);
                attroff(COLOR_PAIR(2));

                attron(COLOR_PAIR(3) | A_BOLD);
                for(int l=0; l<8; l++) mvprintw((cy-9)+2+l, cx-11, "%s", texto_go[l]);
                attroff(COLOR_PAIR(3) | A_BOLD);

                mvprintw(cy+2, cx-10, "O PACMAN FOI PEGO!");
                mvprintw(cy+4, cx-8, "SCORE FINAL: %d", g.pacman.score);
                mvprintw(cy+6, cx-12, "[Y] TENTAR NOVAMENTE");
                mvprintw(cy+7, cx-12, "[N] SAIR DO JOGO");
                refresh();

                int k = getch();
                if(k == 'y' || k == 'Y') {
                    g = upecman_init();
                    TOTAL_PASTILHAS = 0;
                    for(int y=0; y<LABL; y++) for(int x=0; x<LABC; x++)
                        if(g.lab[y][x] == '.' || g.lab[y][x] == 'o') TOTAL_PASTILHAS++;
                    g.lab[17][9] = ' ';
                    TempoInicio = time(NULL); timeout(150); break;
                }
                if(k == 'n' || k == 'N') { endwin(); return EXIT_SUCCESS; }
            }
        }

        // VITORIA
        if(g.pacman.pellet >= TOTAL_PASTILHAS)
        {
            timeout(-1);
            clear();
            while(1) {
                int cy = scry / 2; int cx = scrx / 2;
                attron(COLOR_PAIR(1) | A_BOLD | A_BLINK);
                mvprintw(cy-2, cx-15, "******************************");
                mvprintw(cy-1, cx-15, "* VOCE VENCEU!         *");
                mvprintw(cy+0, cx-15, "******************************");
                attroff(COLOR_PAIR(1) | A_BOLD | A_BLINK);

                mvprintw(cy+2, cx-12, "Score Total: %d", g.pacman.score);
                mvprintw(cy+4, cx-12, "[Y] Jogar Novamente   [N] Sair");
                refresh();

                int k = getch();
                if(k == 'y' || k == 'Y') {
                    g = upecman_init();
                    TOTAL_PASTILHAS = 0;
                    for(int y=0; y<LABL; y++) for(int x=0; x<LABC; x++)
                        if(g.lab[y][x] == '.' || g.lab[y][x] == 'o') TOTAL_PASTILHAS++;
                    g.lab[17][9] = ' ';
                    TempoInicio = time(NULL); timeout(150); break;
                }
                if(k == 'n' || k == 'N') { endwin(); return EXIT_SUCCESS; }
            }
        }
    }
    clear(); endwin(); return EXIT_SUCCESS;
}

/* ==========================================================================
   ETAPA 5: RENDERIZAÇÃO E UI
   ========================================================================== */

void printlab(int scrx, int scry, t_game g)
{
    erase();
    coresdolab(scrx, scry, g);

    int offX = 2; int offY = 1;

    // Desenha PACMAN
    mvprintw(g.pacman.pos.y + offY, g.pacman.pos.x + offX, "@");
    mvchgat(g.pacman.pos.y + offY, g.pacman.pos.x + offX, 1, A_BOLD, 4, NULL);

    // Desenha FANTASMAS
    for(int i = blinky; i <= clyde; i++) {
        char avatar = 'M'; int cor = 3;
        if(g.ghost[i].mode == afraid) {
            avatar = 'w'; cor = 7; attron(A_BLINK);
        } else {
            switch(i) {
                case blinky: avatar = 'B'; cor = 3; break;
                case pinky:  avatar = 'P'; cor = 5; break;
                case inky:   avatar = 'I'; cor = 6; break;
                case clyde:  avatar = 'C'; cor = 1; break;
            }
        }
        mvprintw(g.ghost[i].pos.y + offY, g.ghost[i].pos.x + offX, "%c", avatar);
        mvchgat(g.ghost[i].pos.y + offY, g.ghost[i].pos.x + offX, 1, A_BOLD, cor, NULL);
        attroff(A_BLINK);
    }
    refresh();
}

void coresdolab(int scrx, int scry, t_game g)
{
   (void)scrx;
    int y, k, v;
    int offX = 2; int offY = 1;

    for(y = 0; y < LABL; y++) {
        mvprintw(offY + y, offX, "%s", g.lab[y]);
        for(k = 0; k < LABC; k++) {
            char cell = g.lab[y][k];
            if(cell == ' ') continue;
            if(cell == '#') mvchgat(offY + y, offX + k, 1, A_NORMAL, 2, NULL);
            else if(cell == '.') mvchgat(offY + y, offX + k, 1, A_BOLD, 4, NULL);
            else if(cell == 'o') mvchgat(offY + y, offX + k, 1, A_BOLD | A_BLINK, 4, NULL);
            else if(cell == '%') mvchgat(offY + y, offX + k, 1, A_BOLD, 3, NULL);
        }
    }

    int hudX = 30;
    attron(A_BOLD | COLOR_PAIR(4));
    mvprintw(2, hudX, "PACMAN V2.5");
    attroff(A_BOLD | COLOR_PAIR(4));
    mvprintw(4, hudX, "VIDAS: %d", g.pacman.life);
    mvprintw(5, hudX, "SCORE: %d", g.pacman.score);

    for(v = 0; v < 10 && (v + 10) < scry; v++) {
        attron(COLOR_PAIR(1));
        mvprintw(v + 10, hudX, "%s", buh[v]);
        attroff(COLOR_PAIR(1));
    }
}

/* ==========================================================================
   ETAPA 6: LÓGICA E IA
   ========================================================================== */

t_pacman mvpacman(t_game g)
{
    int xnext = 0, ynext = 0;
    int py = g.pacman.pos.y;
    int px = g.pacman.pos.x;
    int next_dir = g.pacman.next;
    char c_next = '#';

    if(next_dir == down && py+1 < LABL)  c_next = g.lab[py + 1][px];
    if(next_dir == up   && py-1 >= 0)    c_next = g.lab[py - 1][px];
    if(next_dir == right && px+1 < LABC) c_next = g.lab[py][px + 1];
    if(next_dir == left  && px-1 >= 0)   c_next = g.lab[py][px - 1];

    if(c_next != '#' && c_next != '-') g.pacman.dir = next_dir;

    if(g.pacman.dir == down)  ynext = 1;
    if(g.pacman.dir == up)    ynext = -1;
    if(g.pacman.dir == right) xnext = 1;
    if(g.pacman.dir == left)  xnext = -1;

    // Warp Tunnel
    if(px + xnext >= LABC-1 && py == 10) { g.pacman.pos.x = 1; return g.pacman; }
    if(px + xnext <= 0      && py == 10) { g.pacman.pos.x = LABC-2; return g.pacman; }

    char c_curr = g.lab[py + ynext][px + xnext];
    if(c_curr != '#' && c_curr != '-') {
        g.pacman.pos.y += ynext;
        g.pacman.pos.x += xnext;
    }
    return g.pacman;
}

t_game pastilhas(t_game g)
{
    int py = g.pacman.pos.y;
    int px = g.pacman.pos.x;
    char celula = g.lab[py][px];

    // So processa pontos normais aqui. Cereja eh na funcao propria.
    if(celula == '.' || celula == 'o') {
        g.lab[py][px] = ' ';
        g.pacman.pellet++;
        if(celula == '.') g.pacman.score += 10;
        else {
            g.pacman.score += 50;
            TempoAfraid = 1;
            afraid_t = time(NULL);
            for(int j = blinky; j <= clyde; j++)
                if(g.ghost[j].mode != dead) g.ghost[j].mode = afraid;
        }
    }
    return g;
}

// FUNCAO CEREJA CORRIGIDA - Agora com logica de comer!
t_game cereja(t_game g, int TI, int TA) {
    int t = TA - TI;
    int window = 0;

    // Identifica qual janela de tempo da cereja estamos
    if(t>=10 && t<=20) window = 1;
    else if(t>=40 && t<=50) window = 2;
    else if(t>=70 && t<=80) window = 3;

    int cx = 9; // Posicao X da cereja
    int cy = 17; // Posicao Y da cereja

    if (window == 0) {
        // Fora do tempo: remove se existir
        if (g.lab[cy][cx] == '%') g.lab[cy][cx] = ' ';
    }
    else {
        // Dentro do tempo: verifica se ja foi comida nesta janela
        if (g.pacman.cereja1 == window) {
            // Ja comeu, garante que nao desenha
            if (g.lab[cy][cx] == '%') g.lab[cy][cx] = ' ';
        } else {
            // Ainda nao comeu: verifica se Pacman esta em cima
            if (g.pacman.pos.y == cy && g.pacman.pos.x == cx) {
                g.pacman.score += 100; // Pontos da cereja
                g.pacman.cereja1 = window; // Marca como comida nesta janela
                g.lab[cy][cx] = ' '; // Remove
            } else {
                // Se nao esta em cima, desenha a cereja
                if (g.lab[cy][cx] == ' ') g.lab[cy][cx] = '%';
            }
        }
    }
    return g;
}

t_game colisaofantasma(t_game g)
{
    int a;
    for(a = blinky; a <= clyde; a++) {
        if(g.pacman.pos.y == g.ghost[a].pos.y && g.pacman.pos.x == g.ghost[a].pos.x) {
            if(g.ghost[a].mode == afraid) {
                g.pacman.score += 750;
                g.ghost[a].pos.y = 10; g.ghost[a].pos.x = 10; g.ghost[a].mode = scatter;
            } else if(g.ghost[a].mode != dead) {
                timeout(-1); usleep(500000); g.pacman.life--;
                g.pacman.pos.y = 17; g.pacman.pos.x = 9;
                g.pacman.next = left; g.pacman.dir = left;
                g.ghost[blinky].pos.y = 7;  g.ghost[blinky].pos.x = 9;
                g.ghost[pinky].pos.y = 9;   g.ghost[pinky].pos.x = 10;
                g.ghost[inky].pos.y = 10;   g.ghost[inky].pos.x = 10;
                g.ghost[clyde].pos.y = 11;  g.ghost[clyde].pos.x = 10;
                flushinp(); sleep(1); timeout(150);
            }
        }
    }
    return g;
}

// Funcoes IA/Auxiliares
t_game modosfantasmas(t_game g) {
    int i;
    for(i = blinky; i <= clyde; i++) {
        if(g.ghost[i].pos.y > 7 && g.ghost[i].pos.y <= 11 && g.ghost[i].pos.x >= 9 && g.ghost[i].pos.x <= 11) {
             g.ghost[i].pos.y--; continue;
        }
        if(g.ghost[i].mode == afraid) {
            g.ghost[i].starget.y = rand() % LABL; g.ghost[i].starget.x = rand() % LABC;
        } else if(g.ghost[i].mode == dead) {
            g.ghost[i].pos.y = 10; g.ghost[i].pos.x = 10; g.ghost[i].mode = chase;
        } else if(g.ghost[i].mode == scatter) {
            if(i==blinky) { g.ghost[i].starget.y=0; g.ghost[i].starget.x=LABC-1; }
            if(i==pinky)  { g.ghost[i].starget.y=0; g.ghost[i].starget.x=0; }
            if(i==inky)   { g.ghost[i].starget.y=LABL-1; g.ghost[i].starget.x=LABC-1; }
            if(i==clyde)  { g.ghost[i].starget.y=LABL-1; g.ghost[i].starget.x=0; }
        } else {
            g.ghost[i].starget.y = g.pacman.pos.y; g.ghost[i].starget.x = g.pacman.pos.x;
        }
        g = IAfantasmas(g, i);
    }
    return g;
}

int dist_sq(int y1, int x1, int y2, int x2) { return (y1-y2)*(y1-y2) + (x1-x2)*(x1-x2); }

t_game IAfantasmas(t_game g, int i) {
    int py = g.ghost[i].pos.y; int px = g.ghost[i].pos.x;
    int sty = g.ghost[i].starget.y; int stx = g.ghost[i].starget.x;
    int dist[4]={9999,9999,9999,9999}; int viavel[4]={0,0,0,0};

    if(px+1 < LABC && g.lab[py][px+1] != '#' && g.lab[py][px+1] != '-' && g.ghost[i].dir != left) {
        viavel[0]=1; dist[0]=dist_sq(py, px+1, sty, stx);
    }
    if(px-1 >= 0 && g.lab[py][px-1] != '#' && g.lab[py][px-1] != '-' && g.ghost[i].dir != right) {
        viavel[1]=1; dist[1]=dist_sq(py, px-1, sty, stx);
    }
    if(py+1 < LABL && g.lab[py+1][px] != '#' && g.lab[py+1][px] != '-' && g.ghost[i].dir != up) {
        viavel[2]=1; dist[2]=dist_sq(py+1, px, sty, stx);
    }
    if(py-1 >= 0 && g.lab[py-1][px] != '#' && g.lab[py-1][px] != '-' && g.ghost[i].dir != down) {
        viavel[3]=1; dist[3]=dist_sq(py-1, px, sty, stx);
    }

    int min=10000; int best=-1; int order[]={3,1,2,0};
    for(int k=0; k<4; k++) {
        int d=order[k];
        if(viavel[d] && dist[d]<=min) { min=dist[d]; best=d; }
    }

    if(best==0) { g.ghost[i].pos.x++; g.ghost[i].dir=right; }
    else if(best==1) { g.ghost[i].pos.x--; g.ghost[i].dir=left; }
    else if(best==2) { g.ghost[i].pos.y++; g.ghost[i].dir=down; }
    else if(best==3) { g.ghost[i].pos.y--; g.ghost[i].dir=up; }
    else {
        if(g.ghost[i].dir==left) g.ghost[i].dir=right; else if(g.ghost[i].dir==right) g.ghost[i].dir=left;
        else if(g.ghost[i].dir==up) g.ghost[i].dir=down; else if(g.ghost[i].dir==down) g.ghost[i].dir=up;
    }
    if(g.ghost[i].pos.x <= 0) g.ghost[i].pos.x = LABC-2;
    if(g.ghost[i].pos.x >= LABC-1) g.ghost[i].pos.x = 1;
    return g;
}

t_game temporizador(t_game g, int TI, int TA) {
    int t = TA - TI; int nm = chase;
    if(t<=7) nm=scatter; else if(t<=27) nm=chase; else if(t<=34) nm=scatter;
    else if(t<=54) nm=chase; else if(t<=59) nm=scatter; else if(t<=79) nm=chase; else if(t<=84) nm=scatter;
    for(int i=blinky; i<=clyde; i++) if(g.ghost[i].mode!=afraid && g.ghost[i].mode!=dead) g.ghost[i].mode=nm;
    return g;
}

t_game upecman_init(void) {
    t_game g; int f,y;
    for(y=0; y<LABL; y++) strcpy(g.lab[y], labmodel[y]);
    g.pacman.pos.y=17; g.pacman.pos.x=9; g.pacman.dir=left; g.pacman.life=3;
    g.pacman.score=0; g.pacman.pellet=0; g.pacman.cereja1=0;
    for(f=blinky; f<=clyde; f++) {
        g.ghost[f].mode=chase; g.ghost[f].dir=left; g.ghost[f].pos.x=10;
        if(f==blinky) { g.ghost[f].pos.y=7; g.ghost[f].pos.x=9; } else g.ghost[f].pos.y=9+(f-pinky);
    }
    return g;
}

void pacHelp(void) {
    int scrx, scry;
    getmaxyx(stdscr, scry, scrx); // Pega o tamanho da tela para centralizar

    timeout(-1); // Garante que o jogo espere o usuário pressionar algo
    clear();     // Limpa a tela padrão (respeitando o PDF)

    // Cores definidas no main:
    // 1-Verde, 2-Azul, 3-Vermelho, 4-Amarelo, 5-Magenta, 6-Ciano

    attron(A_BOLD | COLOR_PAIR(4)); // Amarelo para título
    mvprintw(2, (scrx-22)/2, "=== REGRAS DO PACMAN ===");
    attroff(A_BOLD | COLOR_PAIR(4));

    int l = 4; // Linha inicial
    int c = 5; // Coluna esquerda

    // --- CONTROLES ---
    attron(A_BOLD); mvprintw(l++, c, "CONTROLES:"); attroff(A_BOLD);
    mvprintw(l++, c, "  [SETAS]  Movimentam o Pacman");
    mvprintw(l++, c, "  [ E ]    Sair do jogo");

    l++; // Pula linha

    // --- PONTUAÇÃO (Dados da Página 1 do PDF) ---
    attron(A_BOLD); mvprintw(l++, c, "PONTUACAO:"); attroff(A_BOLD);
    mvprintw(l++, c, "  Pelota (Dot):        10 pts");
    mvprintw(l++, c, "  Pastilha (Pill):     50 pts");
    mvprintw(l++, c, "  Cereja:             500 pts (Aparece por 10s)");
    mvprintw(l++, c, "  Fantasmas:          200, 400, 800, 1600 pts");
    // Bônus mencionado no PDF
    attron(COLOR_PAIR(1)); // Verde
    mvprintw(l++, c, "  BONUS (4 fantasmas): 12000 pts!");
    attroff(COLOR_PAIR(1));

    l++;

    // --- FANTASMAS (Dados das Páginas 3-8 do PDF) ---
    attron(A_BOLD); mvprintw(l++, c, "CONHECA OS INIMIGOS:"); attroff(A_BOLD);

    // Blinky (Vermelho)
    attron(COLOR_PAIR(3));
    mvprintw(l++, c, "  BLINKY (Vermelho): O 'Sombra'. Perseguidor implacavel.");
    attroff(COLOR_PAIR(3));

    // Pinky (Magenta/Rosa)
    attron(COLOR_PAIR(5));
    mvprintw(l++, c, "  PINKY  (Rosa):     O 'Traira'. Tenta te emboscar pela frente.");
    attroff(COLOR_PAIR(5));

    // Inky (Ciano/Azul Claro)
    attron(COLOR_PAIR(6));
    mvprintw(l++, c, "  INKY   (Azul):     O 'Doido'. Imprevisivel e temperamental.");
    attroff(COLOR_PAIR(6));

    // Clyde (Amarelo/Laranja)
    attron(COLOR_PAIR(4));
    mvprintw(l++, c, "  CLYDE  (Laranja):  O 'Pateta'. Foge se chegar muito perto.");
    attroff(COLOR_PAIR(4));

    l += 2;

    // Rodapé
    attron(A_BLINK);
    mvprintw(l, (scrx-35)/2, "Pressione qualquer tecla para voltar...");
    attroff(A_BLINK);

    refresh(); // Atualiza a tela padrão
    getch();   // Espera o usuário ler
    clear();   // Limpa antes de voltar ao menu
}
void help(void) { printf("./upecman\n"); exit(0); }
void copyr(void) { printf("v2.5 Cherry Fix\n"); exit(0); }
