// matrix.c - rode vallende tekens over je hele terminal
//
// De truc: speciale "escape-codes" die de terminal besturen.
//   \033[rij;kolomH   zet de cursor op een plek
//   \033[0;31m        rode tekst
//   \033[1;33m        felgele tekst
// Let op: \033 is altijd het "escape-teken" en mag je niet veranderen.
// De kleur zit in het getal vlak voor de m (31 = rood, 33 = geel).
//   \033[?25l / h     cursor verbergen / tonen
//
// Compileren:  gcc matrix.c -o matrix
// Uitvoeren:   ./matrix        (stoppen met Ctrl+C)

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

// Wordt 1 zodra je op Ctrl+C drukt, zodat we netjes kunnen opruimen.
static volatile sig_atomic_t stoppen = 0;

static void bij_ctrl_c(int signaal)
{
    (void)signaal;
    stoppen = 1;
}

static char willekeurig_teken(void)
{
    return (char)(33 + rand() % 94);  // zichtbare tekens: ! t/m ~
}

int main(void)
{
    // Vraag aan de terminal hoe groot het venster is.
    int rijen = 48, kolommen = 160;
    struct winsize venster;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &venster) == 0 &&
        venster.ws_row > 0 && venster.ws_col > 0) {
        rijen = venster.ws_row;
        kolommen = venster.ws_col;
    }

    // Per kolom: op welke rij de kop van de "druppel" is, en hoe lang de staart.
    int *kop = malloc(kolommen * sizeof(*kop));
    int *lengte = malloc(kolommen * sizeof(*lengte));
    if (kop == NULL || lengte == NULL) {
        perror("malloc");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (int k = 0; k < kolommen; k++) {
        kop[k] = -(rand() % (rijen * 2));  // start boven het scherm
        lengte[k] = 4 + rand() % rijen;
    }

    signal(SIGINT, bij_ctrl_c);
    printf("\033[2J\033[?25l");  // scherm wissen, cursor verbergen

    while (!stoppen) {
        for (int k = 0; k < kolommen; k++) {
            int rij = kop[k];
            int staart = rij - lengte[k];

            // De kop van de druppel: felgeel.
            if (rij >= 1 && rij <= rijen) {
                printf("\033[%d;%dH\033[1;33m%c", rij, k + 1, willekeurig_teken());
            }
            // Het teken erboven wordt rood.
            if (rij - 1 >= 1 && rij - 1 <= rijen) {
                printf("\033[%d;%dH\033[0;31m%c", rij - 1, k + 1, willekeurig_teken());
            }
            // Het einde van de staart wordt gewist.
            if (staart >= 1 && staart <= rijen) {
                printf("\033[%d;%dH ", staart, k + 1);
            }

            kop[k]++;

            // Druppel helemaal van het scherm af? Begin opnieuw bovenaan.
            if (staart > rijen) {
                kop[k] = -(rand() % rijen);
                lengte[k] = 4 + rand() % rijen;
            }
        }

        fflush(stdout);
        usleep(100000);  // 100 milliseconden wachten per beeldje
    }

    // Opruimen: kleur terugzetten, scherm wissen, cursor weer tonen.
    printf("\033[0m\033[2J\033[H\033[?25h");
    fflush(stdout);

    free(kop);
    free(lengte);
    return 0;
}
