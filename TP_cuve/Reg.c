/*=============================*/
/* modele dynamique d'une cuve */
/*_____________________________*/
/* Jacques BOONAERT-LEPERS     */
/* evaluation AMSE 2023-2024   */
/*=============================*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
/*.............................*/
/* constantes de l'application */
/*.............................*/
#define     CONSIGNE    "CONSIGNE"
#define     DEBIT       "DEBIT"
#define     NIVEAU      "NIVEAU"
/*....................*/
/* variables globales */
/*....................*/
double  K,                        /* ->Gain                */
        *y,                       /* ->hauteur de fluide dans la cuve     */
        Te;                       /* ->periode d'echantillonnage          */
double  *qe;                      /* ->qe : pointeur sur la zone partagee */
double  *yc;                       //pointeur sur la consigne
int     GoOn = 1;                 /* ->controle d'execution               */
/*...................*/
/* prototypes locaux */
/*...................*/
void usage( char *);           /* ->aide de ce programme                  */
void cycl_alm_handler( int );  /* ->gestionnaire pour l'alarme cyclique   */
/*&&&&&&&&&&&&&&&&&&&&&&*/
/* aide de ce programme */
/*&&&&&&&&&&&&&&&&&&&&&&*/
void usage( char *pgm_name )
{
  if( pgm_name == NULL )
  {
    exit( -1 );
  };
  printf("%s <Gain> <periode d'ech.>\n", pgm_name );
  printf("simule cuve de section connue.\n");
  printf("<section>        : section de la cuve en m².\n");
  printf("<periode d'ech.> : periode d'echantillonnage en s.\n");
  printf("\n");
  printf("exemple : \n");
  printf("%s 0.65 0.01\n", pgm_name );
  printf("simulation d'une cuve de section 0.65 m² avec une");
  printf("periode d'echantillonnage de 0.01 s\n");
}
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
/* gestionnaire de l'alarme cyclique */
/*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
void cycl_alm_handler( int signal )
{
    /*...............................*/
    /* mise a jour entree / sortie : */
    /*...............................*/
    if( signal == SIGALRM)
    {
        (*qe)=K*(*yc)-(*y);
    };
    /*................................*/
    /* arret du processus a reception */
    /* de SIGUSR1                     */
    /*................................*/
    if( signal == SIGUSR1)
    {
        GoOn = 0;
    };
}
/*#####################*/
/* programme principal */
/*#####################*/
int main( int argc, char *argv[])
{
  struct sigaction      sa,         /* ->configuration de la gestion de l'alarme */
                        sa_old;     /* ->ancienne config de gestion d'alarme     */
  sigset_t              blocked;    /* ->liste des signaux bloques               */
  struct itimerval      period;     /* ->periode de l'alarme cyclique            */
  int                   fd_qe;      /* ->zone partagee DEBIT  */
  int                   fd_consigne;  /* ->zone partagee VOLUME */
  int                   fd_niveau;  /* ->zone partagee NIVEAU */
  /* verification des arguments */
  if( argc != 3 )
  {
    usage( argv[0] );
    return( 0 );
  };
  /* recuperation des arguments */
  if( (sscanf(argv[1],"%lf", &K     ) == 0)||
      (sscanf(argv[2],"%lf", &Te    ) == 0)   )  
  {
    printf("ERREUR : probleme de format des arguments\n");
    printf("         passe en ligne de commande.\n");
    usage( argv[0] );
    return( 0 );
  };
  /*................*/
  /* initialisation */
  /*................*/
  /* creation des zones partagees */
  /*           --->DEBIT<-----    */
  printf("zone DEBIT\n");
  fd_qe = shm_open(DEBIT, O_RDWR , 0600);
  if( fd_qe < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open() NIVEAU\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd_qe, sizeof(double));
  qe =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd_qe, 
                      0                         );
  /*         --->VOLUME<---            */
  printf("zone CONSIGNE\n");
  fd_consigne = shm_open(CONSIGNE, O_RDWR | O_CREAT, 0600);
  if( fd_consigne < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open() VOLUME\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd_consigne, sizeof(double));
  yc =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd_consigne, 
                      0                         );
  /*         --->NIVEAU<---            */
  printf("zone NIVEAU\n");
  fd_niveau = shm_open(NIVEAU, O_RDWR , 0600);
  if( fd_niveau < 0)
  {
    fprintf(stderr,"ERREUR : main() ---> appel a shm_open() NIVEAU\n");
    fprintf(stderr,"        code d'erreur %d (%s)\n", 
                            errno, 
                            (char *)(strerror(errno)));
    return( -errno );
  };
  ftruncate( fd_niveau, sizeof(double));
  y =  (double *)mmap(NULL, 
                      sizeof(double), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED, 
                      fd_niveau, 
                      0                         );
  sigemptyset( &blocked );
  memset( &sa, 0, sizeof( sigaction )); /* ->precaution utile... */
  sa.sa_handler = cycl_alm_handler;
  sa.sa_flags   = 0;
  sa.sa_mask    = blocked;
  /* installation du gestionnaire de signal */
  sigaction(SIGALRM, &sa, NULL );
  /* initialisation de l'alarme  */
  period.it_interval.tv_sec  = (int)(Te);  
  period.it_interval.tv_usec = (int)((Te - (int)(Te))*1e6);
  period.it_value.tv_sec     = (int)(Te);
  period.it_value.tv_usec    = (int)((Te - (int)(Te))*1e6);
  /* demarrage de l'alarme */
  setitimer( ITIMER_REAL, &period, NULL );
  /* on ne fait desormais plus rien d'autre que */
  /* d'attendre les signaux                     */
  //printf("SIMULATION :\n");
  do
  {
    pause();
    //printf("qe = %lf\t v = %lf y = %lf\n", *qe, *yc, *y);
    printf("yc=%lf\n",*yc);
  }
  while( GoOn == 1 );
  /* menage */
  close(fd_qe);
  close(fd_niveau);
  close(fd_consigne);
  /* fini */
  printf("FIN.\n");
  return( 0 );  /* ->on n'arrive pas jusque la en pratique */
}

  
