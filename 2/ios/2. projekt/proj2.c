//proj2.c
//Autor: Miroslav Harag (xharag02)
//Date 15.4.2021

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>

#include "santas_home.h"

//programy jednotlivich bytosti
#define ELF_PROGRAM "./elf"
#define REINDEER_PROGRAM "./reindeer"
#define SANTA_PROGRAM "./santa"

sem_t *workshop;      // dvere do santovej dielne (otvara ich santa/zatvara posledny elf s cakarne) 
sem_t *waitroom;      // dvere do cakarne pre dielnou (ak nie je plna su otvoren. Ked sa napli posledny skriatok zatvori dvere)
sem_t *empty_workshop;// znak pre santu ze sa mu ziadny skriatkovia nemotaju po dielni a ze moze ist v klude spat 
sem_t *stables;       // Dvere do stajni. Ked ich santa otvori soby sa mozu ist zapriahnut
sem_t *sleigh_ready;  // znak pre sa santu, ze vsetci soby su uz zapriahnuty 
sem_t *work_for_santa;// santov telefon. Ked ma niekto robotu pre santu, zavola mu.
// Semafory pre zachovanie konzistencie dat
sem_t *elf_mutex;
sem_t *reindeer_mutex;
sem_t *log_file;

int shmid;
struct santas_home *santas_home;

pid_t *elves = NULL;
int numOfElves;
pid_t *reindeers = NULL;
int numOfReindeers;
pid_t santa;

int ne,nr;
char *te_str, *tr_str;

void exitProperly(int exitCode);
void exitAbnormaly();

void elfGenerator(){
    elves = realloc(elves, (numOfElves+ne)*sizeof(pid_t));
    if(elves == NULL){
        printf("ERROR: Nepodarilo sa alokovat dostatocnu pamet pre elves");
        exitAbnormaly();
    }

    for (int i = numOfElves; i < numOfElves+ne; i++) {
        if ((elves[i] = fork()) < 0) {
            printf("ERROR: Nepodarila sa operacia fork()");
            exitAbnormaly();
        }
        if (elves[i] == 0) {
            char id[10];
            sprintf(id, "%d", i+1);
            if(execl(ELF_PROGRAM, ELF_PROGRAM, id, te_str, NULL) < 0) {
                printf("ERROR: Nepodarila sa operacia execl()");
                exitAbnormaly();
            }
        }
    }
    numOfElves += ne;
}

void reindeerGenerator(){
    char nr_string[10];
    sprintf(nr_string, "%d", nr);

    reindeers = malloc(nr*sizeof(pid_t));
    if(reindeers == NULL){
        printf("ERROR: Nepodarilo sa alokovat dostatocnu pamet pre elves");
        exitAbnormaly();
    }
    for (int i = 0; i < nr; i++) {
        if ((reindeers[i] = fork()) < 0) {
            printf("ERROR: Nepodarila sa operacia fork()");
            exitAbnormaly();
        }
        if (reindeers[i] == 0) {
            char id[10];
            sprintf(id, "%d", i+1);
            if (execl(REINDEER_PROGRAM, REINDEER_PROGRAM, id, tr_str, nr_string,NULL) < 0) {
                printf("ERROR: Nepodarila sa operacia execl()");
                exitAbnormaly();
            }
        }
    }
}

void santaGenerator(){
    char nr_string[10];
    sprintf(nr_string, "%d", nr);

    if ((santa = fork()) < 0) {
        printf("ERROR: Nepodarila sa operacia fork()");
        exitAbnormaly();
    }
    if (santa == 0) {
        if (execl(SANTA_PROGRAM, SANTA_PROGRAM, nr_string, NULL) < 0) {
            printf("ERROR: Nepodarila sa operacia execl()");
            exitAbnormaly();
        }
    }
}

void stopGenerating(){
    // Nastavi pocet novych elfov na 0
    ne = 0;
}



void initializeAll(){
    // Vycistenie subory s vystpom
    FILE *output = fopen(OUTPUT_FILE, "w");
    fclose(output);

    workshop       = sem_open(WORKSHOP, O_CREAT | O_EXCL, SEM_PERMS, WORKSHOP_INIT);
    waitroom       = sem_open(WAITROOM, O_CREAT | O_EXCL, SEM_PERMS, WAITROOM_INIT);
    empty_workshop = sem_open(EMPTY_WORKSHOP, O_CREAT | O_EXCL, SEM_PERMS, EMPTY_WORKSHOP_INIT);
    stables        = sem_open(STABLES, O_CREAT | O_EXCL, SEM_PERMS, STABLES_INIT);
    sleigh_ready   = sem_open(SLEIGH_READY, O_CREAT | O_EXCL, SEM_PERMS, SLEIGH_READY_INIT);
    work_for_santa = sem_open(WORK_FOR_SANTA, O_CREAT | O_EXCL, SEM_PERMS, WORK_FOR_SANTA_INIT);
    elf_mutex      = sem_open(ELF_MUTEX, O_CREAT | O_EXCL, SEM_PERMS, ELF_MUTEX_INIT);
    reindeer_mutex = sem_open(REINDEER_MUTEX, O_CREAT | O_EXCL, SEM_PERMS, REINDEER_MUTEX_INIT);
    log_file       = sem_open(LOG_FILE, O_CREAT | O_EXCL, SEM_PERMS, LOG_FILE_INIT);

    // V rodicovskom procese semafory nepotrebujeme
    sem_close(workshop);
    sem_close(waitroom);
    sem_close(empty_workshop);
    sem_close(stables);
    sem_close(sleigh_ready);
    sem_close(work_for_santa);
    sem_close(elf_mutex);
    sem_close(reindeer_mutex);
    sem_close(log_file);

    // Vytvorenie santovho domu (dielna, stajne)
    key_t key = ftok("./proj2", 'b');
    shmid = shmget(key, sizeof(struct santas_home), 0644|IPC_CREAT);
    struct santas_home *santas_home;
    santas_home = shmat(shmid, NULL, 0);
    if(santas_home == NULL){
        printf("ERROR: Nepodarilo sa vytvorit zdielany pamat\n");
        exitAbnormaly(1);
    }

    santas_home->master_process = getpid();
    santas_home->workshop.closed = 0;
    santas_home->workshop.inWaitroom = 0;
    santas_home->stables.reindeersInStables = 0;
    santas_home->action_id = 0;
}

void exitProperly(int exitCode){
    // Cakanie na potomkov
    for (int i = 0; i < numOfElves; i++){
       if (waitpid(elves[i], NULL, 0) < 0)
           printf("ERROR: Chyba pri cakani na dieta\n");
    }
    for (int i = 0; i < nr; i++){
        if (waitpid(reindeers[i], NULL, 0) < 0)
            printf("ERROR: Chyba pri cakani na dieta\n");
    }
    if (waitpid(santa, NULL, 0) < 0)
        printf("ERROR: Chyba pri cakani na dieta\n");

    free(elves);
    free(reindeers);

    // Cistenie semaforov
    sem_unlink(WORKSHOP);
    sem_unlink(WAITROOM);
    sem_unlink(EMPTY_WORKSHOP);
    sem_unlink(STABLES);
    sem_unlink(SLEIGH_READY);
    sem_unlink(WORK_FOR_SANTA);
    sem_unlink(ELF_MUTEX);
    sem_unlink(REINDEER_MUTEX);
    sem_unlink(LOG_FILE);

    // Cistenie zdielanej pamate
    shmdt(santas_home);
    shmctl(shmid, IPC_RMID, 0);

    exit(exitCode);
}

void exitAbnormaly(){
    for (int i = 0; i < numOfElves; i++)
       if (kill(elves[i], SIGQUIT) < 0)
           printf("ERROR: Chyba pri ukoncovani potomka\n");
    for (int i = 0; i < nr; i++)
       if (kill(elves[i], SIGQUIT) < 0)
           printf("ERROR: Chyba pri ukoncovani potomka\n");
    if (kill(santa, SIGQUIT) < 0)
        printf("ERROR: Chyba pri ukoncovani potomka\n");

    exitProperly(1);
}



void sig_usr(int signo){
    if (signo == SIGUSR1)
        elfGenerator();
    else if (signo == SIGUSR2)
        stopGenerating(); // Posiela santa ked zacinaju vianoce
    else{
        printf("ERROR: Neznamy signal %d\n", signo);
        exitAbnormaly(1);
    }
}



int main(int argc, char *argv[]) {
    // ./proj2 NE NR TE TR
    if(argc < 5){
        fprintf(stderr, "ERROR: Nepsravne zadane paramentre!\n");
        return 1;
    }

    int start = 0;
    char generateOnlyOnce = 1;
    if(argv[1][0] == '-' && argv[1][1] == 'b'){
        generateOnlyOnce = 0; 
        start++;
    }

    ne = strtol(argv[start+1], NULL, 10);//Number of elf
    nr = strtol(argv[start+2], NULL, 10);//Number of reindeers
    te_str = argv[start+3];// maximalny WAIT cas pre skriatkov
    tr_str = argv[start+4];// maximalny WAIT cas pre sobov
    int te = strtol(te_str, NULL, 10);
    int tr = strtol(tr_str, NULL, 10);

    if(ne <= 0 || ne >= 1000){
        printf("ERROR: NE musi byt z intervalu (0,1000)!\n");
        return 1;
    }
    if(nr <= 0 || nr >= 20){
        printf("ERROR: NR musi byt z intervalu (0,20)!\n");
        return 1;
    }
    if(te < 0 || te > 1000){
        printf("ERROR: TE musi byt z intervalu <0,1000>!\n");
        return 1;
    }
    if(tr < 0 || tr > 1000){
        printf("ERROR: RE musi byt z intervalu <0,1000>!\n");
        return 1;
    }

    if (signal(SIGUSR1, sig_usr) == SIG_ERR)
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIGUSR1");
    if (signal(SIGUSR2, sig_usr) == SIG_ERR)
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIGUSR2");
    if (signal(SIGQUIT, exitAbnormaly) == SIG_ERR)
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIQUIT");



    initializeAll(); 
    
    elfGenerator(); //Creating elves
    if(generateOnlyOnce)
        ne = 0;
    reindeerGenerator();//Create reindeers
    santaGenerator();//Creating santa
    
    exitProperly(0);
}
