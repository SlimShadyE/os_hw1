#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
//#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

/**asd*/
BuiltInCommand::BuiltInCommand(const char* cmd_line): Command(cmd_line){}


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
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

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

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }

  if(firstWord.compare("cd") == 0){
      return new ChangeDirCommand(cmd_line, NULL);
  }
  /*
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
   Command* cmd = CreateCommand(cmd_line);
   cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

Command::Command(const char *cmd_line) {
    args = new char*[COMMAND_MAX_ARGS];
    curr_cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(curr_cmd_line, cmd_line);
    num_of_args = _parseCommandLine(cmd_line, args); /** change cmd_line to curr_cmd_line*/
}

Command::~Command() {
    delete args;
    delete curr_cmd_line;
}

int Command::getNumOfArguments() const {
    return num_of_args;
}

char *Command::getCurrCmdLine() {
    return curr_cmd_line;
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

void Command::DeleteLastPwd_ptr() {
    delete plast_pwd;
    plast_pwd = NULL;
}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){
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

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd):BuiltInCommand(cmd_line) {

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
