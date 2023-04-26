#include "macro.h"
#include "function.h"
#include "shell.h"
#include "shared_memory.h"

int main(int argc, char* const argv[])
{
    // Check argument number
    if(argc != 2){
        cerr << "./server [port]" << endl;
        return -1;
    }

    // Server prepare
    int listenfd, connfd;
    int nready, pid, flag;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

    bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    /*  kill zombie */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGUSR1, SigClient);

    InitShm();
    InitClientTable();

    for(int i=0; i<LISTENQ; i++){
        for(int j = 0; j < NODE_LIMIT; j++) {
            knowNode[i].record[j] = NULL;
        }
        knowNode[i].cnt = 0;
        newNode[i] = NULL;
        targetNode[i] = NULL;
        parentNode[i] = NULL;
    }

    while(1)
    {
        clilen = sizeof(cliaddr);
        
        if((connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) <0)
        {
            /*  there is not client connect */
            if(errno == EINTR)
                continue;
            cerr<<"Server fail to accept, errno: "<<errno<<endl;
            exit(1);
        }

        if((pid = fork()) <0)
        {
            cerr<<"Server fail to fork"<<endl;
            exit(1);
        }
        else if(pid == 0)
        {
            close(listenfd);
            dup2(connfd, 0);
            dup2(connfd, 1);
            dup2(connfd, 2);
            setenv("PATH", "bin:.", 1);

            // 各自的process shmCurrentAddr會存自己在shared memory的位置
            shmCurrentAddr = SetClient();
            shmCurrentAddr->fd = connfd;
            //! 確認PID的功能
            shmCurrentAddr->pid = getpid();
            strcpy(shmCurrentAddr->name,"(no name)");
            strcpy(shmCurrentAddr->ip,inet_ntoa(cliaddr.sin_addr));
            shmCurrentAddr->port = ntohs(cliaddr.sin_port);
            
            Shell(connfd);
            _Exit();
            close(connfd);
            exit(0);
        }
        else
            close(connfd);
         
    }

    return 0;
}