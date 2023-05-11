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


/*** ADDED HELPING FUNCTIONS ***/

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

/*** SMALLSHELL FUNCTIONS ***/


SmallShell::SmallShell():prompt("smash")
{
    // TODO: add your implementation
    this->shell_pid = getpid();

    if(shell_pid == -1)
    {
        perror("smash error: getpid failed");
        return;
    }

    jobs_list = new JobsList();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
    delete jobs_list;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if(cmd_s.find(">")!=string::npos) {
        return new RedirectionCommand(cmd_line);
    }
    if(cmd_s.find("|")!=string::npos){
        return new PipeCommand(cmd_line);
    }
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
    else if(firstWord.compare("jobs")== 0){
        return new JobsCommand(cmd_line);
    }
    else if(firstWord.compare("quit")==0){
        return new QuitCommand(cmd_line);
    }
    else if(firstWord.compare("setcore")==0){
        return new SetcoreCommand(cmd_line);
    }
    else if(firstWord.compare("getfileinfo")==0){
        return new GetFileTypeCommand(cmd_line);
    }
    else{
        return new ExternalCommand(cmd_line);
    }


    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    jobs_list->removeFinishedJobs();
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
}

std::string SmallShell::getPrompt() {
    return prompt;
}

void SmallShell::setPrompt(std::string& new_prompt) {
    prompt=new_prompt;
}

int SmallShell::getPid(){
    return shell_pid;
}

JobsList* SmallShell::getJobsList(){
    return jobs_list;
}

char* SmallShell::getCurrentJobCmdLine() {
    return current_job_cmd_line;
}

int SmallShell::getCurrentJobID() const {
    return current_job_id;
}

int SmallShell::getCurrentJobPID() const {
    return current_job_pid;
}

void SmallShell::NullifyCurrentProcess(){
    current_job_cmd_line = nullptr;
    current_job_id = -1;
    current_job_pid = -1;
}

void SmallShell::UpdateCurrentProcess(int job_id, int job_pid, char* job_cmd_line) {
    current_job_cmd_line = job_cmd_line;
    current_job_id = job_id;
    current_job_pid = job_pid;
}

/*********************************************************************************************************************/

/*** COMMAND FUNCTIONS ***/


Command::Command(const char *cmd_line) {
    args = new char*[COMMAND_MAX_ARGS];
    real_cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    this->cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(real_cmd_line, cmd_line);
    strcpy(this->cmd_line, cmd_line);

    is_background_cmd = _isBackgroundComamnd(cmd_line);
    _removeBackgroundSign(this->cmd_line); ///zdt hay
    num_of_args = _parseCommandLine(this->cmd_line, args); /** 3'yrt mn cmd_line la cmd_line */
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

/*********************************************************************************************************************/

/*** EXECUTE FUNCTIONS ***/


void ChpromptCommand::execute() {
    SmallShell& smallshell = SmallShell::getInstance();
    if(getNumOfArguments()==1){
        std::string smash = "smash";
        smallshell.setPrompt(smash);
    }
    else{
        std::string new_prompt = getArgsArray()[1];
        smallshell.setPrompt(new_prompt);
    }
}

void ShowPidCommand::execute()
{
    int pid = SmallShell::getInstance().getPid();
    std::cout<< "smash shell_pid is "+ to_string(pid)  << std::endl;
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

void JobsCommand::execute(){
    SmallShell& small_shell = SmallShell::getInstance();
    JobsList* jobs_list = small_shell.getJobsList();
    jobs_list->printJobsList();
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

    small_shell.UpdateCurrentProcess(job->getID(),job->getPID(),cmd_line);

    jobs_list->removeJobById(job->getID());

    int res = waitpid(job->getPID(),NULL,WUNTRACED);
    if(res == -1){
        perror("smash error: waitpid failed");
        return;
    }

    small_shell.NullifyCurrentProcess();
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

    if(getNumOfArguments()>1){
        if(strcmp("kill", args[1]) == 0){
            jobs_list->killAllJobs();
        }
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
            cout << "signal number " << signum << " was sent to shell_pid " << job->getPID() << endl;
        }
        /*** Do we need to add SIGSTOPPED and SIGCONT to stop and continue jobs ??? */
    }
    else{
        cerr<< "smash error: kill: invalid arguments"<<endl;
    };
}

/** BG Command */

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
        return;
    }

    if(is_bg_cmd){
        //printf("IS BG");
        jobs_list->addJob(this,  jobs_list->maxJobId()+1, pid);
    } else{

        //the job id is set to -1 because it is irrelevant in this case
        small_shell.UpdateCurrentProcess(-1,pid,cmd_line);

        if(waitpid(pid,&status,WUNTRACED) == -1)
        {
            perror("smash error: waitpid failed");
            return;
        }
        /// msh lazm atapel bl process el current abel el3amaleyye?
        small_shell.NullifyCurrentProcess();
    }
}

void PipeCommand::execute() {
    //printf("In pipe \n");
    int new_pipe[2];
    SmallShell& smash = SmallShell::getInstance();
    string cmd_line = getCmdLine();
    //pipe_string contains '| ' or '|&'
    string pipe_string = cmd_line.substr(cmd_line.find_first_of('|'),2);
    string after_pipe = cmd_line.substr( cmd_line.find_first_of('|')+2);
    string before_pipe = cmd_line.substr( 0, cmd_line.find_first_of('|')-1);
//    cout<< "PIPE :" << pipe_string << endl;
//    cout<< "AFTER PIPE :" << after_pipe << endl;
//    cout<< "BEFORE PIPE :" << before_pipe << endl;


//    cout << after_pipe << endl;
    if(pipe(new_pipe)==-1){
        perror("smash error: pipe failed");
        return;
    }
    char** args_array = getArgsArray();
    char last_char = pipe_string.back();//to check if & is there

    int save_stdin = dup(STDIN_FILENO); //To save the stdin file object for later
    pid_t pid=fork();
    if(pid<0){
        perror("smash error: fork failed");
        return;
    }
    /** Check if the arguments are of a valid form
     * AND CHECK IF WE SHOULD REPLACE PERROR WITH CERR << **/
    if(pid==0){ //Child
        setpgrp();
        //printf("CHILD \n");
        if(last_char == '&'){
            if(close(new_pipe[0]) || close(2)){
                perror("smash error: close failed");
                return;
            }
            if(dup2(new_pipe[1],STDERR_FILENO)==-1){
                perror("smash error: dup2 failed");
                return;
            }
        }
        else{

            if(close(new_pipe[0]) || close(1)){
                perror("smash error: close failed");
                return;
            }

            if(dup2(new_pipe[1],STDOUT_FILENO)==-1){
                perror("smash error: dup2 failed");
                return;
            }
        }
        Command* command = SmallShell::getInstance().CreateCommand(before_pipe.c_str());
        command->execute();
        delete command;
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
        //printf("After pipe %s \n", after_pipe.c_str());
        Command* command = SmallShell::getInstance().CreateCommand(after_pipe.c_str());
        command->execute();
        int wait = waitpid(pid,nullptr,0);
        if(wait==-1){
            perror("smash error: wait failed");
            delete command;
            return;
        }
        delete command;
        exit(0);
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
    cout<<"CORE NUMBER " << core_id <<endl;
    cpu_set_t core;
    CPU_ZERO(&core); //zeros out all bits of the CPU's (none active)
    CPU_SET(core_id,&core); // turns on requested CPU
    cout<<"PID NUMBER " << job->getPID() <<endl;

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

    // Convert the permissions string to an integer
    mode_t permissions = strtol(args[1], NULL, 8);

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

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){
}

ChpromptCommand::ChpromptCommand(const char* cmd_line): BuiltInCommand(cmd_line){
}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd):BuiltInCommand(cmd_line) {
}

/*********************************************************************************************************************/

/*** JOBLIST FUNCTIONS ***/

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

const char* JobsList::JobEntry::getRealCmdLine() const {
    return real_cmd_line;
}

void JobsList::JobEntry::Stop() {
    is_stopped = true;
}

void JobsList::JobEntry::Run() {
    is_stopped = false;
}

JobsList::JobEntry::JobEntry(char* real_cmd_line,char* cmd_line, int id, int pid, bool is_stopped, time_t time)
: real_cmd_line(real_cmd_line), cmd_line(cmd_line), is_stopped(is_stopped), id(id), pid(pid), time(time){}

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


/********************** EM7A!!!!!!!!!!!!!!!!!! ***************************/
void JobsList::addJob(Command *cmd, int id, int pid, bool is_stopped) {
    removeFinishedJobs();
//    int job_id;
//    if(shell_pid<=0){
//        job_id = this->maxJobId()+1;
//    }
//    else{
//        job_id = id;
//    }
    time_t curr_time;
    time(&curr_time);

    auto* new_job = new JobEntry(cmd->getRealCmdLine(),cmd->getCmdLine()
                                 , id, pid, is_stopped, curr_time);

    jobs.push_back(new_job);

}

void JobsList::killAllJobs() {
    cout<< "sending SIGKILL signal to " << jobs.size() <<" jobs:" << endl;
    for(int i=0 ; i<jobs.size() ; i++ ){
        if(kill(jobs[i]->getPID(),SIGKILL)){
            perror("smash error: kill failed");
            return;
        }
        cout << jobs[i]->getID() << ": " << jobs[i]->getRealCmdLine() << endl;
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

void RedirectionCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    string cmd_line = getCmdLine();
    string command = cmd_line.substr(0, cmd_line.find_first_of('>'));
    /**CAN ARGS BE GREATER THAN 3 ? IF NO CHECK NUM OF ARGS (ANSWER:NO)
     * replace stdout_fileno with numbers*/
    char** args_array = getArgsArray();
    int i=0;

    while(strcmp(args_array[i] ,">")!=0  && strcmp(args_array[i] , ">>") != 0){
        i++;
    }
    string redirection_char = args_array[i];

    i++;
    char* destination = args_array[i];
    int save_stdout = dup(STDOUT_FILENO);
    close(STDOUT_FILENO);
    int new_fd;

    int flags = (redirection_char == ">") ? (O_WRONLY | O_CREAT | O_TRUNC) :
                (redirection_char == ">>") ? (O_WRONLY | O_CREAT | O_APPEND) :
                -1;

    if(flags == -1){ //if there are more than two >>
        perror("smash error: invalid arguments");
        return;
    }

    new_fd = open(destination,flags, 0655);
    if(new_fd < 0){
        perror("smash error: open failed");
        dup2(save_stdout,STDOUT_FILENO);
        close(save_stdout);
        return;
    }
    smash.executeCommand(command.c_str());
    //Restores fd after done with the redirection
    close(new_fd);
    dup2(save_stdout,STDOUT_FILENO);
    close(save_stdout);
}
