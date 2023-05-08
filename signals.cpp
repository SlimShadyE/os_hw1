#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

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
  // TODO: Add your implementation
}

