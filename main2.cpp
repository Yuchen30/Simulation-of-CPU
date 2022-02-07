#include <cassert>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <memory>
#include "main2.h"

// this function, load job from disk file, and put them into job queue.
void load_job(PCB_QUEUE & queue_job, PCB_QUEUE & all_task, int cputime){
    auto task = all_task.begin();
    //MACJOB 10
    while (queue_job.size() < MAXJOB && task != all_task.end()){
        if( (*task)->arrival_time > cputime)
            task++;
        else {
            my_assert_check();
            (*task)->time_round = cputime;
            queue_job.push_back(*task);
            all_task.erase(task);
        };
    }
}

class scheduler{
private:
    int cputime;
    int task_count;
    PCB_QUEUE  complete_tasts;
    PCB_QUEUE  &tasks;
    PCB_QUEUE  queue_job;
    PCB_QUEUE  queue_ready;
    PCB_QUEUE  queue_blocked;
    PCB_PTR cur_job;
public:
    scheduler(PCB_QUEUE &allTasks) : tasks(allTasks),task_count(allTasks.size()) {};

    // a function for comlete io
    void complete_io(PCB_PTR pcb);

    // a function for start io.
    void start_io(PCB_PTR pcb);

    // different tasks
    PCB_PTR get_cur_task_FCFS();
    PCB_PTR get_cur_task_SJF();
    PCB_PTR get_cur_task_RR();



    // mark a task's state to completed
    void finish_task(PCB_PTR task);

    // emulate timer int.
    void timer_intr(int t);

    // check if whether all tasks are completed.
    bool task_all_completed();

    // some helper functions for access private members.
    PCB_QUEUE & get_blocked_task();
    PCB_QUEUE & get_ready_task();
    PCB_QUEUE & get_finished_task();
};

// assert check, slow down the emulation so that i can watch the process of output.
int inline my_assert_check(){
    int sum = 0;
    for(int i = 0; i < quantum; i++){
        sum += i;
    }
    PCB *pcb = new PCB;
    pcb->bursts_tasks.push_back(sum);
    delete pcb;
    return sum;
}

// the implementaion of FCFS
PCB_PTR scheduler::get_cur_task_FCFS() {

    if( cur_job && cur_job->state == RUNNING)
        return cur_job;
    // load job or not?
    load_job(this->queue_job, this->tasks, this->cputime);
    PCB_PTR new_task;
    if(this->queue_ready.empty()){
        if(this->queue_job.empty())
            return nullptr;
        auto minJob = queue_job[0];
        my_assert_check();
        for(auto job: this->queue_job){
            if(job->state == READY){
                minJob = job;
                break;
            }
        }
        for(auto job:this->queue_job){
            if((job->arrival_time < minJob->arrival_time) && job->state == READY){
                minJob = job;
            }
        }
        if(minJob->state != READY)
            return nullptr;

        new_task = minJob;
        new_task->time_start = this->cputime;
    } else {
        auto first = queue_ready.begin();
        my_assert_check();
        new_task = * first;
        queue_ready.erase(first);
    }
    // set some state.
    new_task->bursts_time = 0;
    new_task->bursts_count = new_task->bursts_tasks[0];
    new_task->bursts_tasks.erase(new_task->bursts_tasks.begin());
    cur_job = new_task;
    cur_job->state = RUNNING;
    return cur_job;
}

void scheduler::start_io(PCB_PTR pcb) {
    assert(pcb->state == RUNNING);
    pcb -> time_left_io = TIMEOFIO;
    pcb -> state = BLOCKED;
    this->queue_blocked.push_back(pcb);
}

void scheduler::complete_io(PCB_PTR pcb) {
    my_assert_check();
    assert( pcb->state == BLOCKED);
    pcb->state = READY;
    this->queue_ready.push_back(pcb);
}

void scheduler::finish_task(PCB_PTR task) {
    task -> state = COMPLETE;
    this -> cur_job = nullptr;
    this -> complete_tasts.push_back(task);
    auto job_iter = this->queue_job.begin();
    while (job_iter != this->queue_job.end()){
        if( *job_iter == task){
            this->queue_job.erase(job_iter);
            break;
        }
        job_iter++;
    }
    my_assert_check();
    load_job(this->queue_job, this->tasks, MAXJOB);
}

void scheduler::timer_intr(int t) {
    this->cputime = t;
}

bool scheduler::task_all_completed() {
    return complete_tasts.size() == this->task_count;
}

PCB_QUEUE &scheduler::get_blocked_task() {
    return this->queue_blocked;
}

PCB_QUEUE &scheduler::get_ready_task() {
    return this->queue_ready;
}

PCB_QUEUE &scheduler::get_finished_task() {
    return this->complete_tasts;
}

PCB_PTR scheduler::get_cur_task_SJF() {
    if( cur_job && cur_job->state == RUNNING)
        return cur_job;

    load_job(this->queue_job, this->tasks, this->cputime);
    PCB_PTR new_task;
    if(this->queue_ready.empty()){
        if(this->queue_job.empty())
            return nullptr;
        auto min_job = queue_job[0];
        for(auto job: this->queue_job){
            if(job->state == READY){
                min_job = job;
                break;
            }
        }
        for(auto job:this->queue_job){

            if(job->bursts_tasks.size() > 0 && job->state == READY){
                if(job->bursts_tasks[0] < min_job->bursts_tasks[0])
                    min_job = job;
            }
        }
        if(min_job->state != READY)
            return nullptr;

        new_task = min_job;
        new_task->time_start = this->cputime;
    } else {
        auto first = queue_ready.begin();
        auto minIter  = first;
        my_assert_check();
        while (first != queue_ready.end()){
            assert((*first) -> bursts_tasks.size() > 0);
            if( (*first)->bursts_tasks[0] < (*minIter)->bursts_tasks[0])
                minIter = first;
            first++;
        }
        new_task = *minIter;
        queue_ready.erase(minIter);

    }
    new_task->bursts_time = 0;
    new_task->bursts_count = new_task->bursts_tasks[0];
    new_task->bursts_tasks.erase(new_task->bursts_tasks.begin());
    cur_job = new_task;
    cur_job->state = RUNNING;
    return cur_job;
}

PCB_PTR scheduler::get_cur_task_RR() {
    if( cur_job && cur_job->state == RUNNING){
        if(cur_job->time_left_rr != 0){
            cur_job->time_left_rr -= 1;
            return cur_job;
        }else {
            my_assert_check();
            cur_job->state = READY;
            queue_ready.push_back(cur_job);
        }
    }

    load_job(this->queue_job, this->tasks, this->cputime);

    // get new task

    PCB_PTR new_task;
    if(!this->queue_job.empty()){
        auto job_begin = this->queue_job.begin();
        while (job_begin != this->queue_job.end()){
            PCB_PTR job = *job_begin;
            if(job->arrival_time <= this->cputime && !job->RRFlag){
                my_assert_check();
                this->queue_ready.insert(this->queue_ready.begin(), job);
                job->RRFlag = true;
            }
            job_begin ++;
        }
    }
    if(queue_ready.empty())
        return nullptr;

    auto first = queue_ready.begin();
    new_task = * first;
    queue_ready.erase(first);
    if(!new_task->time_left_rr){
        new_task->time_left_rr = quantum - 1;
    }
    if(new_task->bursts_time  == new_task->bursts_count){
        my_assert_check();
        new_task->bursts_count = new_task->bursts_tasks[0];
        new_task->bursts_tasks.erase(new_task->bursts_tasks.begin());
        new_task->bursts_time = 0;
    }
    cur_job = new_task;
    cur_job->state = RUNNING;
    return cur_job;
}
std::string alg;
std::string inputFile;
PCB_QUEUE tasks;
void parser_command_to_task(int argc, char *argv[]){
    alg = argv[1];
    if(alg == "RR")
    {
        quantum = atoi(argv[2]);
        inputFile = argv[3];
    }else
        inputFile = argv[2];

    FILE  * fp = fopen(inputFile.c_str(),"r");

    while (!feof(fp)){
        int io_timer1;
        PCB_PTR cur_pcb(new PCB());
        if(-1 == fscanf(fp, "%d %d %d", &cur_pcb->job_ident, &cur_pcb->arrival_time, &io_timer1))
            break;
        int rtime = 0;
        for (int i = 0; i < io_timer1; ++i) {
            int bursts_time1 = 0;
            fscanf(fp, "%d", &bursts_time1);
            rtime += bursts_time1;
            cur_pcb->bursts_tasks.push_back(bursts_time1);
        }
        rtime += (io_timer1  - 1) * 10;
        cur_pcb->time_real = rtime;
        cur_pcb->state = READY;
        tasks.push_back(cur_pcb);
    }
}
int main(int argc, char *argv[]) {
    parser_command_to_task(argc, argv);
    std::shared_ptr<scheduler> scheduler_obj(new scheduler(tasks));
    int time_now;
    for(time_now = 0; !scheduler_obj->task_all_completed(); time_now += 1){
        if(time_now % 200 == 0){
            printf("************************************************************************************************\n");
            printf(" Time : %d\n", time_now);
            printf(" number of jobs in the ReadyQueue: %lu\n", scheduler_obj->get_ready_task().size());
            printf(" number of jobs in the BlockedQueue:%lu\n", scheduler_obj->get_blocked_task().size());
            printf(" number of jobs completed:%lu\n", scheduler_obj->get_finished_task().size());
        }
        scheduler_obj->timer_intr(time_now);

        PCB_PTR cur_task;
        // select a right scheduler.
        if(alg == "FCFS")
            cur_task  = scheduler_obj->get_cur_task_FCFS();
        else if (alg == "RR")
            cur_task  = scheduler_obj->get_cur_task_RR();
        else if(alg == "SJF")
            cur_task  = scheduler_obj->get_cur_task_SJF();
        else
            printf("Error.");
        
        if(cur_task)
        {
            cur_task->bursts_time += 1;
            if (cur_task->bursts_time == cur_task->bursts_count ){
                if(cur_task->bursts_tasks.empty())
                {
                    cur_task->time_end = time_now;
                    my_assert_check();
                    printf("**************** task finished ****************\n id:%d\n completion time:%d\n arrival time:%d\n processing time:%d\n turnaround time:%d\n",
                           cur_task->job_ident, time_now, cur_task->time_start, cur_task->time_real, cur_task->time_end - cur_task->time_round);
                    cur_task->state = COMPLETE;
                    scheduler_obj -> finish_task(cur_task);
                }else{
                    scheduler_obj -> start_io(cur_task);
                    cur_task->time_left_io += 1;
                }
            }
        }
        PCB_QUEUE & blocked1 = scheduler_obj->get_blocked_task();
        if(!blocked1.empty())
        {
            auto task = blocked1.begin();
            while (task != blocked1.end()){
                assert((*task)->state == BLOCKED);
                (*task)->time_left_io -= 1;
                if((*task)->time_left_io == 0){
                    scheduler_obj->complete_io(*task);
                    blocked1.erase(task);
                }else
                    task ++;
            }
        }
    }

    // total
    int process_time = 0;
    int turnaround = 0;
    int wait_time = 0;
    for(auto task : scheduler_obj->get_finished_task()){
        my_assert_check();
        process_time  += task->time_real;
        turnaround += (task->time_end - task->time_round + 1);
        wait_time += (task->time_end - task->time_round - task->time_real + 1);
    }
    process_time /= scheduler_obj->get_finished_task().size();
    turnaround /= scheduler_obj->get_finished_task().size();
    wait_time /= scheduler_obj->get_finished_task().size();
    printf("**************** all task complete ****************\n");
    printf(" scheduling algorithm used:%s \n", alg.c_str());
    printf(" current CPU clock value:%d  \n", time_now);
    printf(" average processing time:%d \n", process_time);
    printf(" average waiting time:%d \n", wait_time);
    printf(" average turnaround time:%d \n", turnaround);
}
