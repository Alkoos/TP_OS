#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

int K,tau,Te,T,temps;

int x0;
double z0 ;
double a0 ;

int entree;

double yk,uk;
double yk1;

void simuler();

void simuler(){
    z0 = exp(-Te/tau);
    a0 = K*(1-z0);
    yk1 = yk*z0 + a0*uk;

}

int main(int argc, char *argv[]) {
    printf("test\n");
    if (argc != 6) {
        printf("Nombre incorrect d'arguments.\n");
        return 1;
    }
    
    if (sscanf(argv[1], "%d", &K) != 1) {
        printf("Erreur lors de la conversion de l'argument en entier.\n");
        return 1;
    }

    printf("K = %d\n", K);
    return 0;
}
