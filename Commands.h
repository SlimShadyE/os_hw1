#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
    char** args;
    char* real_cmd_line;
    char* cmd_line; ///bdon el background sign
    char* plast_pwd;
    int num_of_args; ///bdon el background sign
    bool is_background_cmd;

public:
    Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    int getNumOfArguments() const;
    char* getCmdLine();
    char* getRealCmdLine();
    char* getLastPwd();
    char** getArgsArray();
    void setLastPwd(char* new_path);
    void DeleteLastPwd_ptr();
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed

    bool isBackgroundCommand();
};



/*********************************************************************************************************************/



class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line):Command(cmd_line){}
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line):Command(cmd_line){}
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line):Command(cmd_line){}
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

/*********************************************************************************************************************/

///we added this class
class ChpromptCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    ChpromptCommand(const char* cmd_line);
    virtual ~ChpromptCommand() {}
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~ShowPidCommand() {}
    void execute() override;
};


class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~QuitCommand() {}
    void execute() override;
};

/*********************************************************************************************************************/

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members

        const char* cmd_line;
        const char* real_cmd_line;
        int id;
        int pid;
        bool is_stopped;
        time_t time;

    public:
        JobEntry(char* real_cmd_line,char* cmd_line, int id, int pid, bool is_stopped, time_t time);
        int getID() const;
        int getPID() const;
        bool isStopped() const;
        void setIsStopped(bool);
        time_t getTime() const;
        const char* getCmdLine() const;
        const char* getRealCmdLine() const;
        void Stop();
        void Run();
        bool operator<(const JobEntry& other) const;
        bool operator>(const JobEntry& other) const;
    };
    // TODO: Add your data members

    std::vector<JobEntry*> jobs;
public:
    JobsList() = default;

    ~JobsList() = default;
    void addJob(Command* cmd, int id, int pid, bool is_stopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
    int maxJobId() const;
    void sort();
    std::vector<JobsList::JobEntry*>* getJobsVector();

};

/*********************************************************************************************************************/


class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~JobsCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
public:
    explicit TimeoutCommand(const char* cmd_line);
    virtual ~TimeoutCommand() {}
    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ChmodCommand(const char* cmd_line);
    virtual ~ChmodCommand() {}
    void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    GetFileTypeCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~GetFileTypeCommand() {}
    void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    SetcoreCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line){}
    virtual ~KillCommand() {}
    void execute() override;
};

/*********************************************************************************************************************/

class SmallShell {
private:
    // TODO: Add your data members
    std::string prompt;
    int shell_pid;
    JobsList* jobs_list;

    char* current_job_cmd_line;
    int current_job_id;
    int current_job_pid;

    SmallShell();
public:
    Command* CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);

    std::string getPrompt();
    void setPrompt(std::string&);
    int getPid();
    JobsList* getJobsList();
    char* getCurrentJobCmdLine();
    int getCurrentJobID() const;
    int getCurrentJobPID() const;
    void NullifyCurrentProcess();
    void UpdateCurrentProcess(int job_id,int job_pid, char* job_cmd_line);
};

#endif //SMASH_COMMAND_H_