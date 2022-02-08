//santa.c
//Autor: Miroslav Harag (xharag02)
//Date 30.4.2021

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#include "santas_home.h"

FILE *output_file;
sem_t *workshop;
sem_t *emptyWorkshop;
sem_t *stables;
sem_t *sleighReady;
sem_t *workForSanta;
sem_t *output;
sem_t *elfMutex;
int shmid;
struct santas_home *santas_home;

void exitProperly(int errorCode){
    fclose(output_file);

    sem_close(workshop);
    sem_close(emptyWorkshop);
    sem_close(stables);
    sem_close(sleighReady);
    sem_close(workForSanta);
    sem_close(output);
    sem_close(elfMutex);
    shmdt(santas_home);

    exit(errorCode);
}

int main(int argc, char *argv[]) {
    (void) argc;
    int nr = strtol(argv[1], NULL, 10);

    if (signal(SIGQUIT, exitProperly) == SIG_ERR){
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIGUSR1");
        return 1;
    }

    FILE *output_file = fopen(OUTPUT_FILE, "a");
    if(output_file == NULL){
        printf("ERROR: Nepodarilo sa otvorit log subor\n");
        return 1;
    }

    sem_t *workshop = sem_open(WORKSHOP, O_RDWR);
    sem_t *emptyWorkshop = sem_open(EMPTY_WORKSHOP, O_RDWR);
    sem_t *stables = sem_open(STABLES, O_RDWR);
    sem_t *sleighReady = sem_open(SLEIGH_READY, O_RDWR);
    sem_t *workForSanta = sem_open(WORK_FOR_SANTA, O_RDWR);
    sem_t *output = sem_open(LOG_FILE, O_RDWR);
    sem_t *elfMutex = sem_open(ELF_MUTEX, O_RDWR);

    key_t key = ftok("./proj2", 'b');
    int shmid = shmget(key, sizeof(struct santas_home), 0644|IPC_CREAT);
    struct santas_home *santas_home;
    santas_home = shmat(shmid, NULL, 0);


    while(1){
        LOG_SANTA("%d: Santa: going to sleep\n", output);

        sem_wait(workForSanta);
        if(santas_home->workshop.inWaitroom == 3){
            LOG_SANTA("%d: Santa: helping elves\n", output);

            sem_post(workshop);
            sem_post(workshop);
            sem_post(workshop);

            sem_wait(emptyWorkshop);
        }
        else if(santas_home->stables.reindeersInStables == nr){
            LOG_SANTA("%d: Santa: closing workshop\n", output);
            kill(santas_home->master_process, SIGUSR2);

            sem_wait(elfMutex);
            santas_home->workshop.closed = 1;
            for(int i = 0; i < santas_home->workshop.inWaitroom; i++){
                sem_post(workshop);
            }
            sem_post(elfMutex);

            for(int i = 0; i < nr; i++){
                sem_post(stables);
            } 

            sem_wait(sleighReady);
            LOG_SANTA("%d: Santa: Christmas started\n", output);
            break;
        }
    }

    exitProperly(0);
}
