//santas_home.h
//Autor: Miroslav Harag (xharag02)
//Date 15.4.2021

//Fronta pred santovou dielnou
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define WORKSHOP "/xharag02_workshop"
#define WORKSHOP_INIT 0
#define WAITROOM "/xharag02_waitroom"
#define WAITROOM_INIT 1
#define EMPTY_WORKSHOP "/xharag02_empty_workshop"
#define EMPTY_WORKSHOP_INIT 0

#define STABLES "/xharag02_stables"
#define STABLES_INIT 0
#define SLEIGH_READY "/xharag02_sleigh_ready"
#define SLEIGH_READY_INIT 0

#define WORK_FOR_SANTA "/xharag02_work_for_santa"
#define WORK_FOR_SANTA_INIT 0

#define ELF_MUTEX "/xharag02_elfmutex"
#define ELF_MUTEX_INIT 1
#define REINDEER_MUTEX "/xharag02_rdmutex"
#define REINDEER_MUTEX_INIT 1
#define LOG_FILE "/xharag02_logfile"
#define LOG_FILE_INIT 1


#define OUTPUT_FILE "./proj2.out"


#define LOG_SANTA(message,sem) sem_wait(sem); fprintf(output_file, message, ++santas_home->action_id);   fflush(output_file); sem_post(sem)
#define LOG_ME(message,id,sem) sem_wait(sem); fprintf(output_file, message, ++santas_home->action_id,id);fflush(output_file); sem_post(sem)

struct workshop {
    int closed;
    int inWaitroom;
};
struct stables {
    int reindeersInStables;
};
struct santas_home {
    // hlavny proces
    pid_t master_process;

    struct workshop workshop;
    struct stables stables;

    // Cislo v log subore
    int action_id;
};

