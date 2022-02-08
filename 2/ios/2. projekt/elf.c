//elf.c
//Autor: Miroslav Harag (xharag02)
//Date 2.5.2021

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "santas_home.h"

FILE *output_file;
sem_t *waitroom;
sem_t *workshop; 
sem_t *emptyWorkshop;
sem_t *workForSanta;
sem_t *elfMutex;
sem_t *output;
int shmid;
struct santas_home *santas_home;

void exitProperly(int exitCode){
    fclose(output_file);

    sem_close(waitroom);
    sem_close(workshop);
    sem_close(emptyWorkshop);
    sem_close(workForSanta);
    sem_close(elfMutex);
    sem_close(output);

    shmdt(santas_home);
    exit(exitCode);
}

int main(int argc, char *argv[]) {
    (void)argc;
    int te = strtol(argv[2], NULL, 10);

    if (signal(SIGQUIT, exitProperly) == SIG_ERR){
        printf("ERROR: nepodarilo sa nastavit sig_handler pre SIGUSR1");
        return 1;
    }

    FILE *output_file = fopen(OUTPUT_FILE, "a");
    if(output_file == NULL){
        printf("ERROR: Nepodarilo sa otvorit log subor\n");
        return 1;
    }

    waitroom = sem_open(WAITROOM, O_RDWR);
    workshop = sem_open(WORKSHOP, O_RDWR);
    emptyWorkshop = sem_open(EMPTY_WORKSHOP, O_RDWR);
    workForSanta = sem_open(WORK_FOR_SANTA, O_RDWR);
    elfMutex = sem_open(ELF_MUTEX, O_RDWR);
    output = sem_open(LOG_FILE, O_RDWR);

    key_t key = ftok("./proj2", 'b');
    shmid = shmget(key, sizeof(struct santas_home), 0644|IPC_CREAT);
    santas_home = shmat(shmid, NULL, 0);

    LOG_ME("%d: Elf %s: started\n", argv[1], output);
    while(1){
        //pracuje
        srand(clock());
        int working_time;
        if(te < 2) 
            working_time = 0;
        else
            working_time = rand() % (te/2) + te/2;
        usleep(working_time*1000);

        LOG_ME("%d: Elf %s: need help\n", argv[1], output);
        //vstup do cakarne
        sem_wait(waitroom); //elf vstupuje do cakarne - max 3. elfovia
        
        sem_wait(elfMutex);
        santas_home->workshop.inWaitroom++; //pocet elfov v cakarni
        if(santas_home->workshop.inWaitroom ==  3) // ak je cakaren plna idu zobudit santu
            sem_post(workForSanta);
        else
            sem_post(waitroom); // cakaren nie je plna - moze vstupit dalsi elf
        sem_post(elfMutex);

        char taking_holiday = 0;
        if(santas_home->workshop.closed){
            LOG_ME("%d: Elf %s: taking holidays\n", argv[1], output);
            taking_holiday = 1;
        }
        else{
            sem_wait(workshop); // elfovia cakaju pred dverami kym santa neotvori
            if(santas_home->workshop.closed){
                LOG_ME("%d: Elf %s: taking holidays\n", argv[1], output);
                taking_holiday = 1;
            }
            else{
                LOG_ME("%d: Elf %s: get help\n", argv[1], output);
            }
        }

        //vystup z dielne
        sem_wait(elfMutex);
        santas_home->workshop.inWaitroom--;
        if(santas_home->workshop.inWaitroom == 0){ //posledny pusti dalsich elfov do cakarne
            sem_post(emptyWorkshop);
            sem_post(waitroom);
        }
        sem_post(elfMutex);

        if(taking_holiday)
            break;
    }

    //ukoncenie procesu
    
    exitProperly(0);
    return 0;
}
