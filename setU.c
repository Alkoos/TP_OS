/*=================================================*/
/* programme d'utilisation des "alarmes cycliques" */
/*-------------------------------------------------*/
/* Jacques BOONAERT / cours SEMBA et AMSE          */
/*_________________________________________________*/
/* simulation d'un systeme de type "premier ordre" */
/* (le signal d'entree est genere en "interne")    */
/*=================================================*/
/*Dans cette version l'entrée est stockée dans une zone
de mémoire partagée*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>          /* ->INDISPENSABLE pour les types tempo. */
#include <sys/mman.h>     /* For mode constants */
#include <fcntl.h>  

/*...................*/
/* prototypes locaux */
/*...................*/
void usage( char *);           /* ->aide de ce programme                */

/*&&&&&&&&&&&&&&&&&&&&&&*/
/* aide de ce programme */
/*&&&&&&&&&&&&&&&&&&&&&&*/
void usage( char *pgm_name )
{
  if( pgm_name == NULL )
  {
    exit( -1 );
  };
  printf("%s <valeur> \n", pgm_name );
  printf("modifie la valeur du signal d'entrée\n");
  printf("<valeur> : valeur du signal d'entrée\n");

  printf("\n");
  printf("exemple : \n");
  printf("%s 3\n", pgm_name );
  printf("impose 3 en valeur d'entrée\n");
}
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
/* gestionnaire de l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/

/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  double valeur;
  double                *u;       /* ->pointeur zone partagée*/
  int fd; //file descriptor zone partagée
  /* verification des arguments */
  if( argc != 2 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( (sscanf(argv[1],"%lf", &valeur     ) == 0))
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };
  /* initialisation */
//Création de zone partagée
 //file descriptor
fd = shm_open("ENTREE",O_RDWR ,0600);
if (fd<0){
    fprintf(stderr,"Erreur : MAIN --> appel shm_open\n");
    fprintf(stderr,"code d'erreur %d (%s)\n",errno,(char *)(strerror(errno)));
    return -errno;
}
ftruncate(fd,sizeof(double));
u = (double *)mmap(NULL,sizeof(double),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);

*u = valeur; //ecriture dans la zone partagée
close(fd);  
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
