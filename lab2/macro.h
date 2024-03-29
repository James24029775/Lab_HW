
// npshell
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// rwg system include
#include<bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// rwg system define
#define SA struct sockaddr
#define MOTD "****************************************\n** Welcome to the information server. **\n****************************************\n"
#define WHO_COLUMN "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n"
#define LISTENQ 35
#define MAXLEN 15001

using namespace std;

// shell define
#include <sys/types.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define UNKNOWN true
#define LENGTH_LIMIT 15000
#define DICT_LEN 1000
#define CHAR_AMT_LIMIT 100
#define TERM_AMT_LIMIT 5
#define OP_PER_NODE_LIMIT 100
#define CMD_PER_OP_LIMIT 5000
#define CHILD_LIMIT 100
#define NODE_LIMIT 5000
#define EX_LIMIT 100
#define VALID_CMD "printenv", "setenv", "exit"
#define VALID_CMD_NUM 3
#define bool int
#define IMPOSSIBLE 2147483647
#define true 1
#define false 0

// shared memory define
#define SHMKEY ((key_t)777)

// shared memory global variables
struct ClientInfo
{
    int fd;
    int pid;
    int uid;
    char name[20];                          
    char msg[1025];                        
    char ip[20];
    unsigned short port;
    int sendInfo[LISTENQ];     
    int recvInfo[LISTENQ];   
};

enum BroadcastSign
{
    YELL,
    NAME,
    EXIT,
    SEND,
    RECV,
    ENTER
};


// shell global variables
const int TIMEOUT = 10;
static int ID = 0;
int DEVNULLO = fileno(fopen("/dev/null", "w"));
int DEVNULLI = fileno(fopen("/dev/null", "r"));

typedef struct cmd {
    char terms[TERM_AMT_LIMIT][CHAR_AMT_LIMIT];
    int cnt;
} cmd;

typedef struct ordinary_pipe_t {
    cmd cmds[CMD_PER_OP_LIMIT];
    int cnt;
    bool ex_flg;
    int in_pipe_num;
    int out_pipe_num;
} ordinary_pipe_t;

typedef struct number_pipe_t {
    struct number_pipe_t *child[CHILD_LIMIT];
    struct number_pipe_t *parent;
    ordinary_pipe_t ops[OP_PER_NODE_LIMIT];
    int ttl;
    int op_cnt;
    int chd_cnt;
    int id;
} number_pipe_t;

typedef struct know_node {
    number_pipe_t *record[NODE_LIMIT];
    int cnt;
} know_node;

typedef struct CMD_dict {
    char dict[DICT_LEN][DICT_LEN];
    int cnt;
} CMD_dict;

int tran2number(char *);
int split_terms(char *, ordinary_pipe_t *, char *);
int copy_ith_op(ordinary_pipe_t *, ordinary_pipe_t *, int);
void sig_handler();
void reset_dict(CMD_dict *);
void scan(char *, int *);
void exec_cmds(cmd *, bool);
void reset(ordinary_pipe_t *, int *);
void show(ordinary_pipe_t *);
void printenv(char[TERM_AMT_LIMIT][CHAR_AMT_LIMIT], int);
void bin_func(ordinary_pipe_t *, char *, number_pipe_t *, int, int, bool, bool, int, int);
void trim(char *);
void show_node(know_node *);
void minus_ttl(know_node *);
void show_tree(number_pipe_t *);
void adoptChild(number_pipe_t *, number_pipe_t *);
void numbered_pipe(number_pipe_t *, int, int, int);
void freeKnowNode(know_node *);
void addInfo2ExistNode(number_pipe_t *, ordinary_pipe_t *, int, int *, int);
void ordinary_pipe(ordinary_pipe_t *, int, int, int);
void user_pipe_redirect(int, int, int);
bool check_valid(ordinary_pipe_t *, char *, bool *, int *, CMD_dict *);
bool isdigit_myself(char *);
number_pipe_t *createNode(know_node *);
number_pipe_t *getSpecificNode(know_node *, int);
number_pipe_t *getParentNode(know_node *);

// shell global variables
vector<int> in_pipe_num_v[LISTENQ];

// rwg system global variables
vector<string> name(LISTENQ);
vector<string> ipaddr(LISTENQ);
vector<int> port(LISTENQ);
vector<int> client(LISTENQ);
vector<int> process_line(LISTENQ);
vector<map<string, string>> env(LISTENQ);
struct user_pipe { int user_pipe[2]; };
struct user_pipe used_user_pipe[LISTENQ][LISTENQ];
char buf[MAXLEN];
know_node knowNode[LISTENQ];
number_pipe_t *newNode[LISTENQ];
number_pipe_t *targetNode[LISTENQ];
number_pipe_t *parentNode[LISTENQ];

fd_set allset, rset;

void rwg_func(int, int);
bool choose_rwg_cmd(vector<string>, int);
void choose_shell_cmd(vector<string> &, string, int, int);
void rwg_who(vector<string>, int);
void rwg_tell(vector<string>, int);
void rwg_yell(vector<string>, int);
void rwg_name(vector<string>, int);
void rwg_exit(vector<string>, int);
int shell_function(string, int, int, bool, bool, int, int);
void broadcast(string);

// Shared memory global variables
int shmID;
ClientInfo *shmStartAddr;
ClientInfo *shmCurrentAddr;
//! SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS 
enum PipeSign
{
    Pipe,
    NumberPipe,
    ErrorPipe,
    Write,
    WriteUserPipe,
    None
};
struct Pipe
{
    int pipeNum;
    int senderId;
    int recverId;
    int clientId;
    int * pipefd;
    PipeSign sign;
};
// 因為不是shm，所以不同process的pipev都不一樣
vector<struct Pipe> pipeV;

// Shared memory functions
void InitpipeV();
void InitShm();
void InitClientTable();
ClientInfo* SetClient();
void WelcomeUser(int);
void _BroadCast(string ,BroadcastSign ,int);
void _Exit();
void ClearClientTable();
void ClearPipe();
int Shell(int);
void sm_broadcast(string msg);
void sm_rwg_who(vector<string> args);
bool sm_choose_rwg_cmd(vector<string> args);
void sm_rwg_func(int sockfd);
void sm_rwg_tell(vector<string> args);
void sm_rwg_yell(vector<string> args);
void sm_rwg_name(vector<string> args);
void sm_rwg_exit(vector<string> args);
void sm_choose_shell_cmd(vector<string> &arg, string input, int sockfd);
int sm_shell_function(string instruction, int sockfd);
void sm_exec_cmds(cmd *localCmd, bool pipe_in_flg);
void sm_bin_func(ordinary_pipe_t *localOP, char *filename, number_pipe_t *Node, int sockfd);
void sm_ordinary_pipe(ordinary_pipe_t *op);
void sm_numbered_pipe(number_pipe_t *Node, int op_idx);
void sm_CreatePipe(int clientId, int senderId, int recverId);

vector<string> SplitEnvPath(string path, char delim);
vector<string> SplitCmdWithSpace(string cmdLine);
void IdentifyCmd(vector<string> &splitCmdLine, vector<string>::iterator &iterLine, string &cmd, vector<string> &argV, PipeSign &sign, int &pipeNum, int &writeId, int &readId);
struct Fd
{
    int in;
    int out;
    int error;
};
bool BuildCmd(string cmd);
int DoBuildinCmd(int sSock, string cmd, vector<string> argV, string &path, vector<string> &pathV);
void ClosePipe(int readId);
void ReducePipe();
bool HasClient(int uid);
bool HasUserPipe(int senderId, int recverId);
void CreatePipe(PipeSign sign, int pipeNum, int clientId, int senderId, int recverId);
bool HasNumberedPipe(int pipeNum, PipeSign sign, int clientId);
void SetStdInOut(PipeSign sign, Fd &fd, int pipeNum, int writeId, int readId, bool UserPipeInError, bool UserPipeOutError);
void DoCmd(string cmd, vector<string> argV, vector<string> pathV, Fd fd, int sSock);
bool SpaceLine(string cmdLine, int &cmdSize);
void ReducePipeNum();
bool LegalCmd(string cmd, vector<string> argV, vector<string> pathV);
char** SetArgv(string cmd, vector<string> argV);
int PassiveTcp(int port);
void SigClient(int signo);