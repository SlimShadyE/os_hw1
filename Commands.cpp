#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <algorithm>
#include "Commands.h"
#include <sys/stat.h>
#include "fcntl.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << _PRETTY_FUNCTION_ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << _PRETTY_FUNCTION_ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


/*********************************************************************************************************************/

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (is_background_cmd sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

/*********************************************************************************************************************/

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if(firstWord.compare("chprompt") == 0){
        return new ChpromptCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    }
    else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    }
    else if(firstWord.compare("cd") == 0){
        return new ChangeDirCommand(cmd_line, NULL);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}


/*** ADDED FUNCTIONS ***/

std::string SmallShell::getPrompt() {
    return prompt;
}

void SmallShell::setPrompt(std::string& new_prompt) {
    prompt=new_prompt;
}

int SmallShell::getPid(){
    return pid;
}

JobsList* SmallShell::getJobsList(){
    return jobs_list;
}

bool is_number(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

bool ContainsNumber(const string &s){
    for (char const &i : s){
        if (std::isdigit(i) == 0){
            return false;
        }
    }
    return true;
}

/*********************************************************************************************************************/

Command::Command(const char *cmd_line) {
    args = new char*[COMMAND_MAX_ARGS];
    real_cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    this->cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(real_cmd_line, cmd_line);
    strcpy(this->cmd_line, cmd_line);

    is_background_cmd = _isBackgroundComamnd(cmd_line);
    _removeBackgroundSign(this->cmd_line); ///zdt hay
    num_of_args = _parseCommandLine(cmd_line, args); /** 3'yrt mn cmd_line la cmd_line */
}

Command::~Command() {
    delete args; /// le msh delete[] ?
    delete cmd_line;
    delete real_cmd_line;
    /// le msh 3aml delete plastpwd ?
}

int Command::getNumOfArguments() const {
    return num_of_args;
}

char *Command::getCmdLine() {
    return cmd_line;
}

char* Command::getRealCmdLine(){
    return real_cmd_line;
}

char * Command::getLastPwd() {
    return plast_pwd;
}

char **Command::getArgsArray() {
    return args;
}

void Command::setLastPwd(char *new_path) {
    char* path = new char[strlen(new_path)+1];
    strcpy(path,new_path);
    plast_pwd = path;
}

bool Command::isBackgroundCommand() {
    return is_background_cmd;
}

void Command::DeleteLastPwd_ptr() {
    delete plast_pwd;
    plast_pwd = nullptr; /// 3'yrt mn NULL la nullptr
}

void ChpromptCommand::execute() {
    SmallShell& smallshell = SmallShell::getInstance();
    if(getNumOfArguments()==1)
    {
        smallshell.setPrompt((string &) "smash");
    } else{
        std::string new_prompt = getArgsArray()[1];
        smallshell.setPrompt(new_prompt);
    }
}

void ShowPidCommand::execute()
{
    int pid = SmallShell::getInstance().getPid();
    std::cout<< "smash pid is "+ to_string(pid)  << std::endl;
}

void GetCurrDirCommand::execute() {
    SmallShell& smallshell = SmallShell::getInstance();
    char* dir = getcwd(NULL,0);
    if(dir){
        cout << dir << endl;
    }
    else{
        perror("smash error: getcwd failed ");
    }
}

void ChangeDirCommand::execute() {
    int chdir_res;
    char** args_array = getArgsArray();
    if(getNumOfArguments() > 2){
        cout << "smash error: cd: too many arguments" << endl;
        return;
    }
    if(strcmp(args_array[1],"-")==0){
        if(!getLastPwd()){
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        char* temp = getcwd(NULL, 0);
        DeleteLastPwd_ptr();
        setLastPwd(temp);
        chdir_res = chdir(args_array[1]);
        if(!chdir_res){
            perror("smash error: chdir failed ");
            return;
        }

    }
    char* temp = getcwd(NULL, 0);
    chdir_res = chdir(args_array[1]);
    cout << getcwd(NULL,0) << endl;
    if(!chdir_res){
        perror("smash error: chdir failed ");
        return;
    }
    DeleteLastPwd_ptr();
    setLastPwd(temp);
}

void ForegroundCommand::execute(){
    char* cmd_line = getCmdLine();
    char** args = getArgsArray();
    int num_of_args = getNumOfArguments();
    JobsList::JobEntry* job = nullptr;
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();

    if(num_of_args == 1){
        // unspecified job

        if(jobs_list->getJobsVector()->empty()){
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }

        int max_job_id = jobs_list->maxJobId();
        job = jobs_list->getJobById(max_job_id);

    }else if(num_of_args == 2){
        // specified job
        if(ContainsNumber(string(args[1]))){
            int job_id = stoi(args[1]);
            job = jobs_list->getJobById(job_id);

            if(job == nullptr){
                // if the job doesn't exist
                cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
                return;
            }

        } else{
            // if the second argument  isn't a number
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
    } else{
        // if contains more than two arguments or none at all
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }

    if(job->isStopped()){
        int res = kill(job->getPID(),SIGCONT);
        if(res == -1){
            perror("smash error: kill failed");
            return;
        }
        job->Run();
    }

    cout << job->getCmdLine() <<" : " << job->getPID() << endl;

    jobs_list->removeJobById(job->getID());

    int res = waitpid(job->getPID(),NULL,WUNTRACED);
    if(res == -1){
        perror("smash error: waitpid failed");
        return;
    }

    /// UPDATE CURRENT JOB IN SHELL
}

void BackgroundCommand::execute() {
    int* id;
    JobsList::JobEntry* job;
    char** args_array = getArgsArray();
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();

    if(getNumOfArguments()==0){
        job = jobs_list->getLastStoppedJob(id);
        if(*id==0){
            cerr<<"smash error: bg: there is no stopped jobs to resume" <<endl;
            return;
        }
    }
    else {
        string string1=args_array[1];
        if (!is_number(string1) || getNumOfArguments() > 1) {
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
        int arg1 = stoi(string1);
        JobsList::JobEntry *job = jobs_list->getJobById(arg1);
        if(!job){
            cerr<<"smash error: bg: job-id "<< id <<" does not exist"<<endl;
            return;
        }
        if(!job->isStopped()){
            cerr<<"smash error: bg: job-id " << id << " is already running in the background" <<endl;
            return;
        }
        cout<< job->getCmdLine() << " : " << job->getPID() << endl;
        kill(job->getPID(), SIGCONT);
        job->setIsStopped(false);
    }
}

void QuitCommand::execute(){
    char** args = getArgsArray();
    int num_of_args = getNumOfArguments();
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();

    if(strcmp("kill", args[1]) == 0){
        jobs_list->killAllJobs();
    }
    exit(0);
}

void KillCommand::execute() {
    char** args_array=getArgsArray();
    string first_arg = args_array[1], second_arg;
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();

    /***ARGS ARE PASSED CHAR* Convert to int*/

    if(first_arg.substr(0,1) == "-"){
        first_arg = first_arg.substr(1);
        if(first_arg.find_first_of(" \n")==string::npos){
            cerr << "smash error: kill: invalid arguments" <<  endl;
            return;
        }
        if(!is_number(first_arg)){
            cerr << "smash error: kill: invalid arguments" <<  endl;
            return;
        }
        int signum = stoi(first_arg);
        if(signum < 1 || signum > 64){
            cerr << "smash error: kill: invalid arguments" <<  endl;
            return;
        }
        second_arg = args_array[2];
        if(second_arg.length()==0){
            cerr << "smash error: kill: invalid arguments" <<  endl;
            return;
        }
        if(!is_number(second_arg)){
            cerr << "smash error: kill: invalid arguments" <<  endl;
            return;
        }
        int job_id = stoi(second_arg);
        JobsList::JobEntry* job = jobs_list->getJobById(job_id);
        if(!job){
            cerr<< "smash error: kill: job-id "<< job_id << " does not exist" << endl;
            return;
        }
        if(-1!=kill(job->getPID(),signum)){
            cout << "signal number " << signum << " was sent to pid " << job->getPID() << endl;
        }
        /*** Do we need to add SIGSTOPPED and SIGCONT to stop and continue jobs ??? */
    }
    else{
        cerr<< "smash error: kill: invalid arguments"<<endl;
    };
}

void ExternalCommand::execute(){
    char* cmd_line = getCmdLine();
    char** args = getArgsArray();
    int num_of_args = getNumOfArguments();
    bool is_bg_cmd = isBackgroundCommand();
    SmallShell &small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();


    string cmd_line_string = string(cmd_line);

    bool is_complex =
            ((cmd_line_string.find("*") != std::string::npos) || (cmd_line_string.find("?") != std::string::npos));

    int status;
    int pid = fork();

    if(pid == -1){
        perror("smash error: fork failed");
        return;
    }

    if(pid == 0){
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            return;
        }
        if(execvp(args[0],args) == -1){
            perror("smash error: execvp failed");
            return;
        }
    }

    if(is_bg_cmd){
        jobs_list->addJob(this, pid, -1);
    } else{
        if(waitpid(pid,&status,WUNTRACED) == -1)
        {
            perror("smash error: waitpid failed");
            return;
        }
    }
}

void PipeCommand::execute() {
    int new_pipe[2];
    if(pipe(new_pipe)==-1){
        perror("smash error: pipe failed");
        return;
    }
    char** args_array = getArgsArray();
    string pipe_argument = args_array[1];
    char last_char = pipe_argument.back();//to check if & is there

    int save_stdin = dup(0); //To save the stdin file object for later
    pid_t pid=fork();
    if(pid<0){
        perror("smash error: fork failed");
        return;
    }
    /** Check if the arguments are of a valid form
     * AND CHECK IF WE SHOULD REPLACE PERROR WITH CERR << **/
    if(pid==0){ //Child
        setpgrp();
        if(last_char == '&'){
            if(close(new_pipe[0]) || close(2)){
                perror("smash error: close failed");
                return;
            }
            if(dup2(new_pipe[1],2)==-1){
                perror("smash error: dup2 failed");
                return;
            }
        }
        else{
            if(close(new_pipe[0]) || close(1)){
                perror("smash error: close failed");
                return;
            }
            if(dup2(new_pipe[1],1)==-1){
                perror("smash error: dup2 failed");
                return;
            }
        }
        Command* command = SmallShell::getInstance().CreateCommand(args_array[0]);
        command->execute();
        delete command; //Why do we need to delete when there's no new?
        exit(0);
    }
    else{
        if(close(new_pipe[1]) || close(0)){
            perror("smash error: close failed");
            return;
        }
        if(dup2(new_pipe[0], 0)==-1){
            perror("smash error: dup2 failed");
        }
        int wait = waitpid(pid,nullptr,0);
        if(wait==-1){
            perror("smash error: wait failed");
            return;
        }
    }
}

void SetcoreCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    char** args_array = getArgsArray();
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();

    if(getNumOfArguments() != 3){
        perror("smash error: setcore: invalid arguments");
        return;
    }
    /*** DO WE HAVE TO CHECK IF THE ARGUMENTS ARE VALID? (NUMBERS AND NOT CHAR) ? */
    int job_id = stoi(args_array[1]), core_id = stoi(args_array[2]);
    JobsList::JobEntry* job = jobs_list->getJobById(job_id);
    if(!job){
        cerr << "smash error: setcore job-id" << job_id << " does not exist" << endl;
        return;
    }
    cpu_set_t core;
    CPU_ZERO(&core); //zeros out all bits of the CPU's (none active)
    CPU_SET(core_id,&core); // turns on requested CPU
    if(sched_setaffinity(job->getPID(),sizeof(cpu_set_t), &core)==-1){
        cerr<< "smash error: setcore: invalid core number" << endl;
        return;
    }
}

void GetFileTypeCommand::execute() {
    char** args_array = getArgsArray();
    if(getNumOfArguments() != 2){
        perror("smash error: gettype: invalid aruments");
        return;
    }
    struct stat fileStat;

    if (stat(args_array[1], &fileStat) == -1) {
        cerr << "smash error: getfiletype: stat failed " << args_array[1] << endl;
        return;
    }

    string type;
    switch (fileStat.st_mode & S_IFMT) {
        case S_IFREG:
            type = "regular file";
            break;
        case S_IFDIR:
            type = "directory";
            break;
        case S_IFCHR:
            type = "character device";
            break;
        case S_IFBLK:
            type = "block device";
            break;
        case S_IFIFO:
            type = "FIFO";
            break;
        case S_IFLNK:
            type = "symbolic link";
            break;
        case S_IFSOCK:
            type = "socket";
            break;
    }
    cout <<  args_array[1] << "s type is " << type << " and takes up " << (int) fileStat.st_size << " bytes" << endl;
}

void ChmodCommand::execute(){
    char* cmd_line = getCmdLine();
    char** args = getArgsArray();
    int num_of_args = getNumOfArguments();

    char* path = args[2];

    if(num_of_args != 2 || path == NULL){
        perror("smash error: gettype: invalid aruments");
        return;
    }

    char* cmd_line_cpy = strdup(cmd_line);

    mode_t permissions = strtol(args[1], NULL, 8); // Convert the permissions string to an integer

    // Open the file for read/write access
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        free(cmd_line_cpy);
        return;
    }

    // Get the current file permissions
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        free(cmd_line_cpy);
        return;
    }

    // Set the new file permissions
    st.st_mode = (st.st_mode & ~0777) | permissions;
    if (fchmod(fd, st.st_mode) < 0) {
        close(fd);
        free(cmd_line_cpy);
        free(cmd_line_cpy);
        return;
    }

    close(fd);
    free(cmd_line_cpy);
}

void TimeoutCommand::execute(){

}

/*********************************************************************************************************************/

BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line){}

/// ymkn fsh 7aji elhn 3shan elsater elle fo2
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){
}

ChpromptCommand::ChpromptCommand(const char* cmd_line): BuiltInCommand(cmd_line){
}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd):BuiltInCommand(cmd_line) {
}

/*********************************************************************************************************************/

int JobsList::JobEntry::getID() const {
    return  id;
}

int JobsList::JobEntry::getPID() const {
    return  pid;
}

bool JobsList::JobEntry::isStopped() const {
    return is_stopped;
}

time_t JobsList::JobEntry::getTime() const {
    return time;
}

const char* JobsList::JobEntry::getCmdLine() const {
    return cmd_line;
}

void JobsList::JobEntry::Stop() {
    is_stopped = true;
}

void JobsList::JobEntry::Run() {
    is_stopped = false;
}

JobsList::JobEntry::JobEntry(char *cmd_line, int id, int pid, bool is_stopped, time_t time) : cmd_line(cmd_line)
        , is_stopped(is_stopped), id(id), pid(pid), time(time){}

void JobsList::JobEntry::setIsStopped(bool flag) {
    is_stopped=flag;
}

bool JobsList::JobEntry::operator<(const JobsList::JobEntry &other) const {
    return id < other.id;
}

bool JobsList::JobEntry::operator>(const JobsList::JobEntry &other) const {
    return id > other.id;
}

void JobsList::removeFinishedJobs()
{
    int res;
    for(unsigned int i=0; i<jobs.size(); i++)
    {
        res = waitpid(jobs[i]->getPID(), NULL, WNOHANG);
        if(res > 0)
        {
            jobs.erase(jobs.begin()+i);
            i--;
        }
    }
}

void JobsList::printJobsList(){
    removeFinishedJobs();
    sort();

    time_t time_diff;
    time_t curr_time;
    time(&curr_time);

    for(auto & job : jobs){
        time_diff = difftime(curr_time, job->getTime());

        cout << "[" << to_string(job->getID()) << "] ";
        cout << job->getCmdLine() << " : " << job->getPID() << " " << time_diff << " secs";

        if(job->isStopped())
        {
            cout<<" (stopped)";
        }

        cout << endl;
    }
}

void JobsList::sort() {
    std::sort(jobs.begin(), jobs.end());
}

void JobsList::removeJobById(int jobId){
    unsigned int num;
    for(unsigned int i=0; i<jobs.size(); i++)
    {
        if (jobs[i]->getID() == jobId) {
            num = i;
        }
    }
    jobs.erase(jobs.begin() + num);
}

int JobsList::maxJobId() const {
    int temp=0;
    for(auto & job : jobs){
        if(job->getID() > temp){
            temp = job->getID();
        }
    }
    return temp;
}

void JobsList::addJob(Command *cmd, int id, int pid, bool is_stopped) {
    int job_id;
    if(pid<=0){
        job_id = this->maxJobId()+1;
    }
    else{
        job_id = id;
    }
    time_t curr_time;
    time(&curr_time);
    auto* new_job = new JobEntry(cmd->getCmdLine(), job_id, pid, is_stopped, curr_time);
    jobs.push_back(new_job);
}

void JobsList::killAllJobs() {
    cout<< "sending SIGKILL signal to " << jobs.size() <<"jobs:" << endl;
    for(int i=0 ; i<jobs.size() ; i++ ){
        if(kill(jobs[i]->getPID(),SIGKILL)){
            perror("smash error: kill failed");
            return;
        }
        cout << jobs[i]->getID() << ": " << jobs[i]->getCmdLine() << endl;
    }
}

JobsList::JobEntry * JobsList::getJobById(int jobId)
{
    for(auto & job : jobs){
        if(job->getID() == jobId){
            return job;
        }
    }
    return nullptr;
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    if(jobs.empty()){
        *lastJobId=0;
        return nullptr;
    }
    *lastJobId = jobs.back()->getID();
    return jobs.back();
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    int last_stopped_job_id=0;
    JobEntry* last_stopped_job = nullptr;
    for(auto & job : jobs){
        if(job->getID() > last_stopped_job_id && job->isStopped()){
            last_stopped_job_id = job->getID();
            last_stopped_job = job;
        }
    }
    *jobId= last_stopped_job_id;
    return last_stopped_job;

}

std::vector<JobsList::JobEntry *> *JobsList::getJobsVector() {
    return &jobs;
}