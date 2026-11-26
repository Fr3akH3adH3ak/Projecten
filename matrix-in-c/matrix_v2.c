// matrix_v2.c - vallende tekens over je hele terminal, met een kleur- en snelheidskeuze
//
// Gebruik:  ./matrix_v2 [kleur] [wachttijd_in_ms]
//   kleur:  rood (standaard), groen of blauw
//   wacht:  hoeveel milliseconden tussen twee beeldjes (standaard 100, kleiner is sneller)
// Voorbeeld: ./matrix_v2 groen 50
//
// Compileren:  gcc -Wall -Wextra matrix_v2.c -o matrix_v2
// Stoppen:     Ctrl+C

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t stoppen = 0;

static void bij_signaal(int signaal)
{
    (void)signaal;
    stoppen = 1;
}

static char willekeurig_teken(void)
{
    return (char)(33 + rand() % 94);
}

int main(int argc, char *argv[])
{
    // Kleur van de staart: 31 = rood, 32 = groen, 34 = blauw.
    int kleur = 31;
    if (argc > 1) {
        if (strcmp(argv[1], "groen") == 0) kleur = 32;
        else if (strcmp(argv[1], "blauw") == 0) kleur = 34;
        else if (strcmp(argv[1], "rood") != 0) {
            fprintf(stderr, "Gebruik: %s [rood|groen|blauw] [wachttijd_in_ms]\n", argv[0]);
            return 1;
        }
    }

    // Wachttijd per beeldje, tussen 10 en 1000 milliseconden.
    int wacht_ms = 100;
    if (argc > 2) {
        wacht_ms = atoi(argv[2]);
        if (wacht_ms < 10 || wacht_ms > 1000) {
            fprintf(stderr, "De wachttijd moet tussen 10 en 1000 liggen.\n");
            return 1;
        }
    }

    int rijen = 48, kolommen = 160;
    struct winsize venster;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &venster) == 0 &&
        venster.ws_row > 0 && venster.ws_col > 0) {
        rijen = venster.ws_row;
        kolommen = venster.ws_col;
    }

    int *kop = malloc(kolommen * sizeof(*kop));
    int *lengte = malloc(kolommen * sizeof(*lengte));
    if (kop == NULL || lengte == NULL) {
        perror("malloc");
        free(kop);
        free(lengte);
        return 1;
    }

    srand((unsigned)time(NULL));
    for (int k = 0; k < kolommen; k++) {
        kop[k] = -(rand() % (rijen * 2));
        lengte[k] = 4 + rand() % rijen;
    }

    // Ook "kill" (SIGTERM) ruimt netjes op, niet alleen Ctrl+C.
    signal(SIGINT, bij_signaal);
    signal(SIGTERM, bij_signaal);
    printf("\033[2J\033[?25l");

    while (!stoppen) {
        for (int k = 0; k < kolommen; k++) {
            int rij = kop[k];
            int staart = rij - lengte[k];

            if (rij >= 1 && rij <= rijen) {
                printf("\033[%d;%dH\033[1;33m%c", rij, k + 1, willekeurig_teken());
            }
            if (rij - 1 >= 1 && rij - 1 <= rijen) {
                printf("\033[%d;%dH\033[0;%dm%c", rij - 1, k + 1, kleur, willekeurig_teken());
            }
            if (staart >= 1 && staart <= rijen) {
                printf("\033[%d;%dH ", staart, k + 1);
            }

            kop[k]++;

            if (staart > rijen) {
                kop[k] = -(rand() % rijen);
                lengte[k] = 4 + rand() % rijen;
            }
        }

        fflush(stdout);
        usleep((useconds_t)wacht_ms * 1000);
    }

    printf("\033[0m\033[2J\033[H\033[?25h");
    fflush(stdout);

    free(kop);
    free(lengte);
    return 0;
}
