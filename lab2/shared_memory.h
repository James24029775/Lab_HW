void InitShm()
{
    if((shmID = shmget(SHMKEY, sizeof(ClientInfo)*LISTENQ, IPC_CREAT | 0600)) <0)
    {
        cerr<<"Fail to create shm space, errno: "<<errno<<endl;
        exit(1);
    }

    /* Attach a pointer to the first element of shm .*/
    if((shmStartAddr = (ClientInfo*)shmat(shmID, NULL, 0)) < 0)
    {
        cerr<<"Fail to attach shm, errno: "<<errno<<endl;
        exit(1);
    }
}

void InitClientTable()
{
    for(int i=0; i<LISTENQ; i++)
    {
        (shmStartAddr+i)->pid = -1;
        (shmStartAddr+i)->fd = -1;
        (shmStartAddr+i)->uid = -1;
        for(int j=0; j<LISTENQ; j++)
        {
            ((shmStartAddr+i)->sendInfo)[j] = -1;
            ((shmStartAddr+i)->recvInfo)[j] = -1;
        }
        strcpy((shmStartAddr + i)->msg,"");
    }
}

ClientInfo* SetClient()
{
    for(int i=0; i<LISTENQ; i++)
    {
        if((shmStartAddr+i)->pid == -1)
        {
            (shmStartAddr+i)->uid = i+1;
            return (shmStartAddr+i);
        }
    }
    return NULL;
}

void WelcomeUser(int sockfd)
{
    string msg = MOTD;
    write(sockfd, msg.c_str(), msg.length());
}

void _BroadCast(string input,BroadcastSign sign,int sendrecvuid)
{
    string temp = "";
    switch(sign) {
        case YELL:
            temp = "*** "+string(shmCurrentAddr->name)+" yelled ***: " + input ;
            break;
        case ENTER:
            temp = "*** User '(no name)' entered from "+string(shmCurrentAddr->ip)+":"+to_string(shmCurrentAddr->port)+". ***";
            break;
        case EXIT:
            temp = "*** User '"+string(shmCurrentAddr->name)+"' left. ***";
            break;
        case NAME:
            temp = "*** User from " +string(shmCurrentAddr->ip)+":"+to_string(shmCurrentAddr->port)+" is named '"+string(shmCurrentAddr->name)+"'. ***";
            break;
        case SEND:
            temp = "*** "+string(shmCurrentAddr->name)+" (#"+to_string(shmCurrentAddr->uid)+") just piped '"+input+"' to "+string(shmStartAddr[sendrecvuid-1].name)+" (#"+to_string(shmStartAddr[sendrecvuid-1].uid)+") ***";
            break;
        case RECV:
            temp = "*** "+string(shmCurrentAddr->name)+" (#"+to_string(shmCurrentAddr->uid)+") just received from "+string(shmStartAddr[sendrecvuid-1].name)+" (#"+to_string(shmStartAddr[sendrecvuid-1].uid)+") by '"+input+"' ***";
            break;
        default:
            break;
    }
    for(int i=0; i<LISTENQ; i++)
    {
        if( (shmStartAddr+i)->pid  != -1)
        {
            strcpy((shmStartAddr + i)->msg,temp.c_str());
            kill((shmStartAddr+i)->pid , SIGUSR1);
        }
    }
}

void Exit()
{
    _BroadCast("",EXIT,-1);
    shmCurrentAddr->pid = -1;
    shmCurrentAddr->uid = -1;
    ClearClientTable();
    ClearPipe();

    if(shmdt(shmStartAddr) == -1)
    {
        cerr<<"Fail to detach shm, errno: "<<errno<<endl;
        exit(1);
    }
    if (shmctl(shmID,IPC_RMID,0) == -1)
    {
        cerr<<"Fail to delete shm, errno: "<<errno<<endl;
        exit(1);
    }
}

void ClearClientTable()
{
    for(int i=0 ; i<LISTENQ ; i++)
    {
        (shmCurrentAddr->recvInfo)[i] = -1;
        (shmCurrentAddr->sendInfo)[i] = -1;
        
        if(shmStartAddr[i].pid != -1)
        {
            for(int j=0; j<LISTENQ; j++)
            {
                if((shmStartAddr[i].sendInfo)[j] == shmCurrentAddr->uid)
                    (shmStartAddr[i].sendInfo)[j] = -1;
                if((shmStartAddr[i].recvInfo)[j] == shmCurrentAddr->uid)
                    (shmStartAddr[i].recvInfo)[j] = -1;
            }
        }
    }
}

void ClearPipe()
{
    /*  close all pipe related to client    */
    vector<struct Pipe>::iterator iter = pipeV.begin();
    while(iter != pipeV.end())
    {
        if(iter->clientId == shmCurrentAddr->uid || iter->recverId == shmCurrentAddr->uid)
        {
            close(iter->pipefd[0]);
            close(iter->pipefd[1]);
            delete [] iter->pipefd;
            pipeV.erase(iter);
            continue;
        }
        iter++;
    }
}

int Shell(int sockfd)
{
    string cmdLine;
    string cmd;
    string path = getenv("PATH");
    vector<string> splitCmdLine;
    vector<string>::iterator iterLine;
    vector<string> pathV = SplitEnvPath(path, ':');
    vector<pid_t> pidV;

    int pipeNum;
    int writeId;
    int readId;

    bool isNumPipe;
    bool UserPipeInError;
    bool UserPipeOutError;

    PipeSign sign;

    WelcomeUser(sockfd);
    _BroadCast("",ENTER,-1);

    while(1)
    {
        // cout<<"% ";
        string msg = "% ";
        write(shmCurrentAddr->fd, msg.c_str(), msg.length());
        getline(cin, cmdLine);
        //!!!!!!!!!!!!!!從這開始要一股作氣開始看，要怎麼取代成自己的寫法 
        splitCmdLine = SplitCmdWithSpace(cmdLine);
        iterLine = splitCmdLine.begin();

        isNumPipe = false;
        pidV.clear();

        while(iterLine != splitCmdLine.end() && *iterLine != "\0")
        {
            Fd fd = {sockfd, sockfd, sockfd};
            vector<string> argV;
            sign = None;
            pipeNum = 0;
            writeId = 0;
            readId = 0;
            UserPipeInError = false;
            UserPipeOutError = false;
        
            IdentifyCmd(splitCmdLine, iterLine, cmd, argV, sign, pipeNum, writeId, readId);
            
            /*  Do buildin command  */
            if(BuildCmd(cmd))
            {
                int status = DoBuildinCmd(sockfd, cmd, argV, path, pathV);
                ClosePipe(readId);
                ReducePipe();
                if(status <0)
                    return -1;
                else
                    break;
            }

            /*  handle cmd <n   */
            if(readId != 0)
            {
                if(!HasClient(readId))
                {
                    // cout<<"*** Error: user #" + to_string(readId) + " does not exist yet. ***\n";
                    string msg = "*** Error: user #" + to_string(readId) + " does not exist yet. ***\n";
                    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                    UserPipeInError = true;
                }
                else if(!HasUserPipe(readId, shmCurrentAddr->uid))
                {
                    // cout<<"*** Error: the pipe #" + to_string(readId) + "->#" + to_string(shmCurrentAddr->uid) + " does not exist yet. ***\n";
                    string msg = "*** Error: the pipe #" + to_string(readId) + "->#" + to_string(shmCurrentAddr->uid) + " does not exist yet. ***\n";
                    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                    UserPipeInError = true;
                }
                else
                    _BroadCast(cmdLine, RECV, readId);
            }

            if(sign == Pipe)
            {
                CreatePipe(sign, pipeNum, shmCurrentAddr->uid, 0, 0);
            }
            else if(sign == NumberPipe || sign == ErrorPipe)
            {
                if(!HasNumberedPipe(pipeNum, sign, shmCurrentAddr->uid))
                    CreatePipe(sign, pipeNum, shmCurrentAddr->uid, 0, 0);
                isNumPipe = true;
            }
            else if(sign == Write)
            {
                string fileName = argV[argV.size()-1];
                argV.pop_back();
                FILE* file = fopen(fileName.c_str(), "w");
                
                if(file == NULL)
                {
                    cerr<<"Fail to open file"<<endl;
                    return -1;
                }
                
                fd.out = fileno(file);
            }
            else if(sign == WriteUserPipe)
            {
                isNumPipe = true;
                if(!HasClient(writeId))
                {
                    // cout<<"*** Error: user #" + to_string(writeId) + " does not exist yet. ***\n";
                    string msg = "*** Error: user #" + to_string(writeId) + " does not exist yet. ***\n";
                    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                    UserPipeOutError = true;
                }
                else if(!HasUserPipe(shmCurrentAddr->uid, writeId))
                {
                    _BroadCast(cmdLine, SEND, writeId);
                    CreatePipe(sign, -1, shmCurrentAddr->uid, shmCurrentAddr->uid, writeId);
                }
                else
                {
                    // cout<<"*** Error: the pipe #" + to_string(shmCurrentAddr->uid) + "->#" + to_string(writeId) + " already exists. ***\n";
                    string msg = "*** Error: the pipe #" + to_string(shmCurrentAddr->uid) + "->#" + to_string(writeId) + " already exists. ***\n";
                    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                    UserPipeOutError = true;
                }
            }

            SetStdInOut(sign, fd, pipeNum, writeId, readId, UserPipeInError, UserPipeOutError);
            
            pid_t pid;
            while((pid = fork()) <0)
            {
                /*  parent process no resource to fork child process, so wait for child exit and release recourse   */
                int status = 0;
                waitpid(-1, &status, 0);
            }
            if(pid == 0)
                DoCmd(cmd, argV, pathV, fd, sockfd);
            else if(pid > 0)
                pidV.push_back(pid);
            

            if(fd.in != sockfd)  //close read from pipe, the other entrance is closed in SetStdInOut
                close(fd.in);

            ClosePipe(readId);
            ReducePipe();
        }

        if(!isNumPipe)
        {
            vector<pid_t>::iterator iter = pidV.begin();
            while(iter != pidV.end())
            {
                int status;
                waitpid((*iter), &status, 0);
                iter++;
            }
        }

        int cmdSize = cmdLine.length();
        if(!SpaceLine(cmdLine, cmdSize))
            ReducePipeNum();
    }
    return 0;
}

vector<string> SplitEnvPath(string path, char delim)
{
    vector<string> pathV;
    string temp;
    stringstream ss(path);
    
    while(getline(ss, temp, delim))
    {
        pathV.push_back(temp);
    }

    return pathV;
}

vector<string> SplitCmdWithSpace(string cmdLine)
{
    istringstream ss(cmdLine);
    vector<string> splitCmdLine;
    string temp;

    while(ss>>temp)
    {
        splitCmdLine.push_back(temp);
    }
    
    return splitCmdLine;
}

void IdentifyCmd(vector<string> &splitCmdLine, vector<string>::iterator &iterLine, string &cmd, vector<string> &argV, PipeSign &sign, int &pipeNum, int &writeId, int &readId)
{
    string temp;
    bool isCmd = true;

    /*  who, tell yell and name are user cmd, and set all remaining iter as argument    */
    bool isBuildinCmd = BuildCmd(splitCmdLine[0]);

    while(iterLine != splitCmdLine.end())
    {
        temp = *iterLine;

        if(temp[0] == '|' && !isBuildinCmd)
        {
            if(temp.size() == 1)
            {
                sign = Pipe;
                pipeNum = 1;
            }
            else
            {
                sign = NumberPipe;
                string num;

                for(int i = 1; i < temp.size(); i++)
                    num += temp[i];

                pipeNum = stoi(num);
            }

            iterLine++;
            break;
        }
        else if(temp[0] == '!' && !isBuildinCmd)
        {
            sign = ErrorPipe;
            string num;

            for(int i = 1; i < temp.size(); i++)
                num += temp[i];

            pipeNum = stoi(num);
            
            iterLine++;
            break;
        }
        else if(temp[0] == '>' && temp.length() == 1 && !isBuildinCmd)
        {
            sign = Write;
            iterLine++;
            argV.push_back(*iterLine);

            iterLine++;
            break;
        }
        else if(temp[0] == '>' && !isBuildinCmd)
        {
            sign = WriteUserPipe;
            string id;

            for(int i=1; i<temp.size(); i++)
                id += temp[i];

            writeId = stoi(id);
            
            /*  check cat >2 <1 case    */
            if((iterLine+1) != splitCmdLine.end())
            {
                temp = *(iterLine+1);
                if(temp[0] == '<')
                {
                    iterLine++;
                    continue;
                }
            }
            iterLine++;
            break;
        }
        else if(temp[0] == '<' && !isBuildinCmd)
        {
            string id;
            for(int i=1; i<temp.size(); i++)
                id += temp[i];

            readId = stoi(id);
        }
        else if(isCmd)
        {
            cmd = temp;
            isCmd = false;
        }
        else
        {
            argV.push_back(temp);
        }
        
        iterLine++;
    }
}

bool BuildCmd(string cmd)
{
    if(cmd == "setenv" || cmd == "printenv" || cmd == "exit" || cmd == "EOF" || cmd == "who" || cmd == "tell" || cmd == "yell" || cmd == "name")
        return true;
    return false;
}

int DoBuildinCmd(int sockfd, string cmd, vector<string> argV, string &path, vector<string> &pathV)
{
    if(cmd == "printenv")
    {
        string env = argV[0];
        string msg = getenv(env.c_str());
        msg += "\n";
        write(shmCurrentAddr->fd, msg.c_str(), msg.length());
        // cout<<msg<<endl;
        return 0;
    }
    else if(cmd == "setenv")
    {
        string env, assign;

        env = argV[0];
        assign = argV[1];
        setenv(env.c_str(), assign.c_str(), 1);
        if(env == "PATH")
        {
            path = getenv("PATH");
            pathV.clear();
            pathV = SplitEnvPath(path, ':');
        }
        return 0;
    }
    else if(cmd == "yell")
    {
        string msg;
        for(int i=0; i<argV.size(); i++)
            msg += (" " + argV[i]);
        _BroadCast(msg, YELL, -1);
        return 0;
    }
    else if(cmd == "name")
    {
        string name = argV[0];
        
        for(int i=0; i<LISTENQ; i++)
        {
            if(shmStartAddr[i].name == name)
            {
                // cout << "*** User '"+name+"' already exists. ***" << endl;
                string msg  = "*** User '"+name+"' already exists. ***\n";
                write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                return 0;
            }
        }

        strcpy(shmCurrentAddr->name, name.c_str());
        _BroadCast("",NAME,-1);
        return 0;
    }
    else if(cmd == "tell")
    {
        int sendId = atoi(argV[0].c_str());
        string msg = "*** " + string(shmCurrentAddr->name) + " told you ***:";
        for(int i=1; i<argV.size(); i++)
            msg += (" " + argV[i]);

        if(HasClient(sendId))
        {
            for(int i=0; i<LISTENQ; i++)
            {
                if( (shmStartAddr+i)->uid  == sendId)
                {
                    strcpy((shmStartAddr + i)->msg, msg.c_str());
                    kill((shmStartAddr+i)->pid, SIGUSR1);
                    break;
                }
            }
        }
        else
        {
            string msg = "*** Error: user #" + to_string(sendId) + " does not exist yet. ***\n";
            write(shmCurrentAddr->fd, msg.c_str(), msg.length());
            // cout << "*** Error: user #" + to_string(sendId) + " does not exist yet. ***"<< endl;
        }
        return 0;
    }
    else if(cmd == "who")
    {
        string msg = "<ID> <nickname> <IP:port>  <indicate me>\n";
        write(shmCurrentAddr->fd, msg.c_str(), msg.length());
        for(int i=0; i<LISTENQ; i++)
        {
            if((shmStartAddr+i)->pid  != -1)
            {
                msg = to_string((shmStartAddr+i)->uid) + "    " + (shmStartAddr+i)->name + " " + (shmStartAddr+i)->ip + ":" + to_string((shmStartAddr+i)->port) + "   ";
                write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                if((shmStartAddr+i)->uid == shmCurrentAddr->uid) 
                {
                    msg = "<-me\n";
                    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
                }
                else
                    write(shmCurrentAddr->fd, "\n", strlen("\n"));
            }
        }

        return 0;   
    }
    else   // exit or EOF
        return -1;
}

void ClosePipe(int readId)
{
    vector<struct Pipe>::iterator iter = pipeV.begin();
    while(iter != pipeV.end())
    {
        /*  | cmd or |0 cmd */
        if(iter->pipeNum == 0 && iter->clientId == shmCurrentAddr->uid)
        {
            close(iter->pipefd[0]);
            close(iter->pipefd[1]);
            delete [] (*iter).pipefd;
            pipeV.erase(iter);
            
            break;
        }/* cmd <id */
        else if(iter->senderId == readId && iter->recverId == shmCurrentAddr->uid && iter->sign == WriteUserPipe)
        {
            close((*iter).pipefd[0]);    
            close((*iter).pipefd[1]);
            delete [] (*iter).pipefd;
            pipeV.erase(iter);

            break;
        }
        iter++;
    }
}

void ReducePipe()
{
    vector<struct Pipe>::iterator iter = pipeV.begin();
    while(iter != pipeV.end())
    {
        if((*iter).sign == Pipe && (*iter).clientId == shmCurrentAddr->uid)
            (*iter).pipeNum--;
        iter++;
    }
}

bool HasClient(int uid)
{
    for(int i=0; i<LISTENQ; i++)
    {
        if(shmStartAddr[i].uid == uid)
            return true;
    }
    return false; 
}

bool HasUserPipe(int senderId, int recverId)
{
    vector<struct Pipe>::iterator iter = pipeV.begin();

    while(iter != pipeV.end())
    {
        if((*iter).senderId == senderId && (*iter).recverId == recverId && (*iter).sign == WriteUserPipe)
            return true;

        iter++;
    }
    
    return false;
}

void CreatePipe(PipeSign sign, int pipeNum, int clientId, int senderId, int recverId)
{
    int* pipefd = new int [2];
    struct Pipe newPipe;

    if(pipe(pipefd)<0)
    {
        cerr<<"Pipe create fail"<<" eerrno:"<<errno<<endl;
        exit(1);
    }
    newPipe.pipefd = pipefd;
    newPipe.sign = sign;
    newPipe.pipeNum = pipeNum;
    newPipe.clientId = clientId;
    newPipe.senderId = senderId;
    newPipe.recverId = recverId;
    pipeV.push_back(newPipe);
}

bool HasNumberedPipe(int pipeNum, PipeSign sign, int clientId)
{
    vector<struct Pipe>::iterator iter = pipeV.begin();

    while(iter != pipeV.end())
    {
        if((*iter).pipeNum == pipeNum && ((*iter).sign == sign) && (*iter).clientId == clientId)
            return true;

        iter++;
    }
    
    return false;
}

void SetStdInOut(PipeSign sign, Fd &fd, int pipeNum, int writeId, int readId, bool UserPipeInError, bool UserPipeOutError)
{
    vector<struct Pipe>::iterator iter = pipeV.begin();
    bool setIn =false;
    bool setOut = false;

    if(sign == Pipe)
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        while(iter != pipeV.end())
        {
            
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            if(!setOut)
            {
                /*  cmd |   */
                if((*iter).pipeNum == 1 && (*iter).sign == Pipe && (*iter).clientId == shmCurrentAddr->uid)
                {
                    fd.out = (*iter).pipefd[1];
                    setOut = true;
                }
            }
            iter++;
        }
    }
    else if(sign == NumberPipe)
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        while(iter != pipeV.end())
        {
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            if(!setOut)
            {
                /*  cmd |n  */
                if((*iter).pipeNum == pipeNum && (*iter).sign == NumberPipe && (*iter).clientId == shmCurrentAddr->uid)
                {
                    fd.out = (*iter).pipefd[1];
                    setOut = true;
                }
            }
            iter++;
        }
    }
    else if(sign == ErrorPipe)
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        while(iter != pipeV.end())
        {
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            if(!setOut)
            {
                /*  cmd !n  */
                if((*iter).pipeNum == pipeNum && (*iter).sign == ErrorPipe && (*iter).clientId == shmCurrentAddr->uid)
                {
                    fd.out = (*iter).pipefd[1];
                    fd.error = (*iter).pipefd[1];
                    setOut = true;
                }
            }
            iter++;
        }
    }
    else if(sign == Write)
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        while(iter != pipeV.end())
        {
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            iter++;
        }
    }
    else if(sign == WriteUserPipe)
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        if(UserPipeOutError)
        {
            fd.out = -1;
            setOut = true;
        }
        while(iter != pipeV.end())
        {
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            if(!setOut)
            {
                if((*iter).senderId == shmCurrentAddr->uid && (*iter).recverId == writeId && (*iter).sign == WriteUserPipe)
                {
                    fd.out = (*iter).pipefd[1];
                    setOut = true;
                }
            }
            iter++;
        }
    }
    else
    {
        if(UserPipeInError)
        {
            fd.in = -1;
            setIn = true;
        }
        while(iter != pipeV.end())
        {
            if(!setIn)
            {   /*  cmd <n  */
                if((*iter).senderId == readId && (*iter).recverId == shmCurrentAddr->uid && (*iter).sign == WriteUserPipe)
                {
                    close((*iter).pipefd[1]);
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }/* |0  cmd or | cmd */
                else if((*iter).pipeNum == 0 && readId == 0 && (*iter).clientId == shmCurrentAddr->uid)
                {
                    close((*iter).pipefd[1]);   //close write to pipe
                    fd.in = (*iter).pipefd[0];
                    setIn = true;
                }
            }
            iter++;
        }
    }
    
}

void DoCmd(string cmd, vector<string> argV, vector<string> pathV, Fd fd, int sockfd)
{
    /*  Test legal command  */
    if(!LegalCmd(cmd, argV, pathV))
    {
        // cout<<"Unknown command: [" + cmd + "].\n";
        string msg = "Unknown command: [" + cmd + "].\n";
        write(shmCurrentAddr->fd, msg.c_str(), msg.length());
        exit(1);
    }

    int devNullIn, devNullOut;
    dup2(sockfd, 1);
    dup2(sockfd, 2);
    close(sockfd);
    if(fd.in != 0)
    {
        /*  user pipe in error  */
        if(fd.in == -1)
        {
            if((devNullIn = open("/dev/null", O_RDONLY)) <0)
                cout<<"Fail to redirect /dev/null, errno: "<<errno<<endl;
            if((dup2(devNullIn, 0)) <0)
                cout<<"Fail to dup2 /dev/null, errno: "<<errno<<endl;
        }
        else
            dup2(fd.in, 0);
    }
    if(fd.out != sockfd)
    {
        /*  user pipe out error */
        if(fd.out == -1)
        {
            if((devNullOut = open("/dev/null", O_WRONLY)) <0)
                cout<<"Fail to redirect dev/null, errno: "<<errno<<endl;
            if((dup2(devNullOut, 1)) <0)
                cout<<"Fail to dup2 /dev/null, errno: "<<errno<<endl;
        }
        else
            dup2(fd.out, 1);
    }
    if(fd.error != sockfd)
        dup2(fd.error, 2);
    if(fd.in != 0)
    {
        if(fd.in == -1)
            close(devNullIn);
        else
            close(fd.in);
    }
    if(fd.out != sockfd)
    {
        if(fd.in == -1)
            close(devNullOut);
        else
            close(fd.out);
    }
    if(fd.error != sockfd)
        close(fd.error);


    char **arg = SetArgv(cmd, argV);
    vector<string>::iterator iter = pathV.begin();
    while(iter != pathV.end())
    {
        string path = (*iter) + "/" + cmd;
        if((execv(path.c_str(), arg)) == -1)
            iter++;
    }
    
    cerr<<"Fail to exec"<<endl;
    exit(1);
}

bool SpaceLine(string cmdLine, int &cmdSize)
{
    int originSize = cmdSize;
    for(int i = originSize-1; i>=originSize-2; i--)
    {
        if(cmdLine[i] == '\r' || cmdLine[i] == '\n')
            cmdSize--;
    }
    if(cmdSize == 0)
        return true;
    else
        return false;
}

void ReducePipeNum()
{
    vector<struct Pipe>::iterator iter = pipeV.begin();
    while(iter != pipeV.end())
    {
        if((iter->sign == NumberPipe || iter->sign == ErrorPipe) && iter->clientId == shmCurrentAddr->uid)
            iter->pipeNum--;
        iter++;
    }
}

bool LegalCmd(string cmd, vector<string> argV, vector<string> pathV)
{
    string path;
    vector<string>::iterator iter;
    iter = pathV.begin();
    
    FILE* file;
    while(iter != pathV.end())
    {
        path = *iter + "/" + cmd;
        file = fopen(path.c_str(), "r");
        if(file != NULL)
        {
            fclose(file);
            return true;
        }
        iter++;
    }

    return false;
}

char** SetArgv(string cmd, vector<string> argV)
{   
    char ** argv = new char* [argV.size()+2];
    argv[0] = new char [cmd.size()+1];
    strcpy(argv[0], cmd.c_str());
    for(int i=0; i<argV.size(); i++)
    {
        argv[i+1] = new char [argV[i].size()+1];
        strcpy(argv[i+1], argV[i].c_str());
    }
    argv[argV.size()+1] = NULL;
    return argv;
}

int PassiveTcp(int port)
{
    const char* protocal  = "TCP";
    struct sockaddr_in servAddr;
    struct protoent *proEntry;
    int mSock, type;

    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    if((proEntry = getprotobyname(protocal) ) == NULL)
    {
        cerr<<"Fail to get protocal entry"<<endl;
        exit(1);
    }

    type = SOCK_STREAM;
    
    if((mSock = socket(PF_INET, type, proEntry->p_proto)) <0)
    {
        cerr<<"Fail to create master socket, errno: "<<errno<<endl;
        exit(1);
    }

	int optval = 1;
	if (setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) 
    {
		cout<<"Error: Set socket failed"<<endl;
		exit(1);
	}

    if(::bind(mSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) <0)
    {
        cerr<<"Fail to bind master socket, errno: "<<errno<<endl;
        exit(1);
    }

    if(listen(mSock, 5) <0)
    {
        cerr<<"Fail to listen master socket, errno: "<<errno<<endl;
        exit(1);
    }

    return mSock;
}

void SigClient(int signo)
{
    // cout<<shmCurrentAddr->msg<<endl;
    string msg(shmCurrentAddr->msg);
    msg += "\n";
    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
    strcpy(shmCurrentAddr->msg,"");
}