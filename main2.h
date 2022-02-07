#ifndef CPUSIM_MAIN2_H
#define CPUSIM_MAIN2_H

#define MAXJOB 10
#define TIMEOFIO 10
int inline my_assert_check();
enum PROC_STATE {READY, RUNNING, BLOCKED, COMPLETE};
int quantum = 20;
struct PCB{
    int time_real;
    int time_round;
    int time_start;
    PROC_STATE state;
    int job_ident;
    int bursts_count;
    int bursts_time;
    int time_left_io;
    int arrival_time;
    int time_left_rr;
    int time_end;
    bool RRFlag;
    std::vector<int> bursts_tasks;
};
typedef std::shared_ptr<PCB> PCB_PTR;
typedef std::vector<PCB_PTR> PCB_QUEUE;

#endif //CPUSIM_MAIN2_H
