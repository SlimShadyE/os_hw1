#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/wait.h>


using namespace std;

void ctrlZHandler(int sig_num) {
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();
    int curr_job_id = small_shell.getCurrentJobID();
    int curr_job_pid = small_shell.getCurrentJobPID();
    char* curr_job_cmd_line = small_shell.getCurrentJobCmdLine();

    cout << "small_shell: got ctrl-Z" << endl;

    if(curr_job_pid < 0){
        return;
    }

    Command* current_command = new ExternalCommand(curr_job_cmd_line);

    cout << "rami fc" << endl;

    jobs_list->addJob(current_command,curr_job_id,curr_job_pid,true);

    if(kill(curr_job_pid,SIGSTOP) == -1){
        perror("smash error: kill failed");
        jobs_list->removeJobById(curr_job_id);
        delete current_command;
        return;
    }
    cout<< "smash: process "<< curr_job_pid <<" was stopped"<<endl;

    delete current_command;

    small_shell.NullifyCurrentProcess();
}

void ctrlCHandler(int sig_num) {
    SmallShell& small_shell = SmallShell::getInstance();
    int curr_job_pid = small_shell.getCurrentJobPID();

    cout << "smash: got ctrl-C" << endl;

    if(curr_job_pid <= 0){
        return;
    }

    if(kill(curr_job_pid,SIGKILL) == -1)
    {
        perror("smash error: kill failed");
        return;
    }

    cout << "smash: process " << curr_job_pid << " was killed" << endl;

    small_shell.NullifyCurrentProcess();
}

void alarmHandler(int sig_num) {
    cout << "smash: got an alarm" << endl;

    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();
    JobsList* timeout_jobs_list = small_shell.getTimeOutJobsList();

    /// lazm front bs pop_front msh zabta f7ttet back w pop back
    JobsList::JobEntry* timeout_job = timeout_jobs_list->getJobsVector()->back();

    jobs_list->removeFinishedJobs();

//
//    time_t duration = timeout_job->getDuration();
//    time_t entry_time = timeout_job->getTime();
//
//    time_t current_time;
//    if((int)time(&current_time) == -1){
//        perror("smash error: time failed");
//        return;
//    }
//
//    time_t time_left = entry_time - current_time + duration

    int pid = timeout_job->getPID();

    //send SIGKILL if job not finished
    if(waitpid(pid, NULL , WNOHANG) == 0){
        if(kill(pid,SIGKILL) == -1){
            perror("smash error: kill failed");
        } else{
            cout << "smash: " << timeout_job->getCmdLine() << " timed out!" << endl;
        }
    }

    /// lazm pop_front bs msh zabta
    timeout_jobs_list->getJobsVector()->pop_back();
}

