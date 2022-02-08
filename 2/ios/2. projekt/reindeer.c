//reindeer.c
//Autor: Miroslav Harag (xharag02)
//Date 30.4.2021

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "santas_home.h"

FILE *output_file;
sem_t *stables;
sem_t *sleighReady; 
sem_t *workForSanta;
sem_t *reindeerMutex;
sem_t *output;
int shmid;
struct santas_home *santas_home;

void exitProperly(int errorCode){
    fclose(output_file);

    sem_close(stables);
    sem_close(sleighReady);
    sem_close(workForSanta);
    sem_close(reindeerMutex);
    sem_close(output);

    shmdt(santas_home);
    exit(errorCode);
}

int main(int argc, char *argv[]) {
    (void) argc;
    
    if (signal(SIGQUIT, exitProperly) == SIG_ERR){
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIGUSR1");
        return 1;
    }

    FILE *output_file = fopen(OUTPUT_FILE, "a");
    if(output_file == NULL){
        printf("ERROR: Nepodarilo sa otvorit log subor\n");
        return 1;
    }
   
    int tr = strtol(argv[2], NULL, 10);
    int nr = strtol(argv[3], NULL, 10);

    sem_t *stables = sem_open(STABLES, O_RDWR);
    sem_t *sleighReady = sem_open(SLEIGH_READY, O_RDWR);
    sem_t *workForSanta = sem_open(WORK_FOR_SANTA, O_RDWR);
    sem_t *reindeerMutex = sem_open(REINDEER_MUTEX, O_RDWR);
    sem_t *output = sem_open(LOG_FILE, O_RDWR);

    key_t key = ftok("./proj2", 'b');
    shmid = shmget(key, sizeof(struct santas_home), 0644|IPC_CREAT);
    struct santas_home *santas_home = shmat(shmid, NULL, 0);

    LOG_ME("%d: RD %s: rstarted\n", argv[1], output);

    // Sob je na letnej dovolenke 
    srand(clock());
    int working_time;
    if(tr < 2)
        working_time = 0;
    else
        working_time = rand() % (tr/2) + tr/2;
    usleep(working_time*1000);

    LOG_ME("%d: RD %s: return home\n", argv[1], output);

    sem_wait(reindeerMutex);
    if(++santas_home->stables.reindeersInStables == nr){
        sem_post(workForSanta);
    }
    sem_post(reindeerMutex);

    sem_wait(stables);
    LOG_ME("%d: RD %s: get hitched\n", argv[1], output);

    sem_wait(reindeerMutex);
    if(--(santas_home->stables.reindeersInStables) == 0){
        sem_post(sleighReady);
    }
    sem_post(reindeerMutex);


    exitProperly(0); 
    return 0;
}
