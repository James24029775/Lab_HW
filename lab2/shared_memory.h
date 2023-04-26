void sm_rwg_func(int sockfd) {
    memset(buf, 0, MAXLEN);
    ssize_t bytes_read = read(sockfd, buf, MAXLEN);
    
    if (bytes_read == -1){
        cerr << "Error reading from socket." << endl;
        exit(EXIT_FAILURE);
    }

    string msg = string(buf);
    for(int i=0; i<msg.length(); i++){
        if(msg[i] == '\r' || msg[i] == '\n'){
            msg.erase(msg.begin()+i);
            i --;
        }
    }

    stringstream ss(buf);
    string arg;
    vector<string> args;
    while(ss >> arg){
        args.push_back(arg);
    }

    // Minus every nodes' ttl to maintan number pipe DS.
    if (msg != "") {
        // cout << msg << endl;
        int cli = shmCurrentAddr->uid;
        minus_ttl(&knowNode[cli]);
        if (!sm_choose_rwg_cmd(args))
            sm_choose_shell_cmd(args, msg, sockfd);
    }

    // cout << "ATTTTTTTTTTTTTTTTTTTTTTENTION" << endl;
    
    // vector<struct Pipe>::iterator iter = pipeV.begin();
    // while(iter != pipeV.end())
    // {
    //     cout << (*iter).senderId << " " << (*iter).recverId << " " << (*iter).clientId << " " << endl;
    //     iter++;
    // }
}

void sm_choose_shell_cmd(vector<string> &arg, string input, int sockfd){
    int arg_num = arg.size();
    signal(SIGCHLD, SIG_IGN);
    // cout << "??????????????????" << endl;

    // Check user pipe
    int user_pipe_num = 0, in_user_pipe_num = 0, out_user_pipe_num = 0;
    bool in_user_exist_flg = true, out_user_exist_flg = true;
    bool in_pipe_exist_flg = true, out_pipe_dup_flg = false;
    for (int i = 0; i < arg_num; i++){
        if ((arg[i][0] == '<' || arg[i][0] == '>') && arg[i].length() > 1) {
            user_pipe_num = stoi(&arg[i][1]);
            for (int j = 1; j < arg[i].length(); j++) {
                if (arg[i][j] < '0' || arg[i][j] > '9') {
                    user_pipe_num = 0;
                    break;
                }
            }

            if (user_pipe_num > 0) {
                // Check user exist
                
                // if (user_pipe_num > LISTENQ || client[user_pipe_num] < 0) {
                if (user_pipe_num > LISTENQ || !HasClient(user_pipe_num)) {
                    if (arg[i][0] == '<') {
                        in_user_exist_flg = false;
                        in_user_pipe_num = -1 * user_pipe_num;
                    }
                    else if (arg[i][0] == '>') {
                        out_user_exist_flg = false;
                        out_user_pipe_num = -1 * user_pipe_num;
                    }
                    arg.erase(arg.begin()+i);
                    i--;
                    arg_num--;
                    continue;
                }

                // Check pipe in
                if (arg[i][0] == '<') {
                    // if (user_pipe_num > LISTENQ || (used_user_pipe[user_pipe_num][cli].user_pipe[0] == -1 && used_user_pipe[user_pipe_num][cli].user_pipe[1] == -1)){
                    if (HasUserPipe(user_pipe_num, shmCurrentAddr->uid)) {
                        in_pipe_exist_flg = true;
                        in_user_pipe_num = user_pipe_num;
                    } else {
                        in_pipe_exist_flg = false;
                        in_user_pipe_num = -1 * user_pipe_num;
                    }
                    // if (user_pipe_num > LISTENQ || (!HasUserPipe(user_pipe_num, shmCurrentAddr->uid))){
                    //     in_pipe_exist_flg = false;
                    //     in_user_pipe_num = -1 * user_pipe_num;
                    // }
                    // else {
                    //     in_pipe_exist_flg = true;
                    //     in_user_pipe_num = user_pipe_num;
                    // }
                }
                // Check pipe out
                else if (arg[i][0] == '>') {
                    // if (used_user_pipe[cli][user_pipe_num].user_pipe[0] == -1 && used_user_pipe[cli][user_pipe_num].user_pipe[1] == -1) {
                    if (!HasUserPipe(shmCurrentAddr->uid, user_pipe_num)){
                        struct user_pipe tmp_user_pipe;
                        // FOCUSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs
                        // pipe(tmp_user_pipe.user_pipe);
                        // used_user_pipe[cli][user_pipe_num] = tmp_user_pipe;
                        sm_CreatePipe(shmCurrentAddr->uid, shmCurrentAddr->uid, user_pipe_num);
                        out_pipe_dup_flg = false;
                        out_user_pipe_num = user_pipe_num;
                    }
                    else {
                        out_pipe_dup_flg = true;
                        out_user_pipe_num = -1 * user_pipe_num;
                    }
                }
                arg.erase(arg.begin()+i);
                i--;
                arg_num--;
            } 
            
        }
    }
    // cout << "WAAAAAAAAAIT: " << input << "  " << endl;
    // cout << "IN num " << in_user_pipe_num << " OUT num " << out_user_pipe_num << endl;
    // cout << "user_pipe_num " << user_pipe_num << " cli " << shmCurrentAddr->uid << endl;
    
    bool pipe_in_flg = false, pipe_out_flg = false;
    if (in_user_exist_flg && in_pipe_exist_flg && in_user_pipe_num > 0) pipe_in_flg = true;
    if (out_user_exist_flg && !out_pipe_dup_flg && out_user_pipe_num > 0) pipe_out_flg = true;

    // Make pipe-in msg
    if (!in_user_exist_flg) {
        int err_user_num = -1 * in_user_pipe_num;
        string errmsg = "*** Error: user #" + to_string(err_user_num) + " does not exist yet. ***\n";
        // write(sockfd, errmsg.c_str(), errmsg.length());
        write(shmCurrentAddr->fd, errmsg.c_str(), errmsg.length());
    } else if (!in_pipe_exist_flg) {
        int err_user_num = -1 * in_user_pipe_num;
        string errmsg = "*** Error: the pipe #" + to_string(err_user_num) + "->#" + to_string(shmCurrentAddr->uid) + " does not exist yet. ***\n";
        // write(sockfd, errmsg.c_str(), errmsg.length());
        write(shmCurrentAddr->fd, errmsg.c_str(), errmsg.length());
    } else if (pipe_in_flg) {
        string msg = "*** " + string(shmCurrentAddr->name) + " (#" + to_string(shmCurrentAddr->uid) + ") just received from " + string(shmStartAddr[in_user_pipe_num-1].name) + " (#" + to_string(shmStartAddr[in_user_pipe_num-1].uid) + ") by \'" + input + "' ***\n";
        // temp =       "*** " + string(shmCurrentAddr->name) + " (#" + to_string(shmCurrentAddr->uid) + ") just received from " + string(shmStartAddr[sendrecvuid-1].name) + " (#" + to_string(shmStartAddr[sendrecvuid-1].uid) + ") by '" + input + "' ***";
        sm_broadcast(msg);
    }

    // Make pipe-out msg 
    if (!out_user_exist_flg) {
        int err_user_num = -1 * out_user_pipe_num;
        string errmsg = "*** Error: user #" + to_string(err_user_num) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (out_pipe_dup_flg) {
        int err_user_num = -1 * out_user_pipe_num;
        string errmsg = "*** Error: the pipe #" + to_string(shmCurrentAddr->uid) + "->#" + to_string(err_user_num) + " already exists. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (pipe_out_flg) {
        string msg = "*** " + string(shmCurrentAddr->name) + " (#" + to_string(shmCurrentAddr->uid) + ") just piped '" + input + "' to " + string(shmStartAddr[out_user_pipe_num-1].name) + " (#" + to_string(shmStartAddr[out_user_pipe_num-1].uid) + ") ***\n";
        //   temp = "*** " + string(shmCurrentAddr->name) + " (#" + to_string(shmCurrentAddr->uid) + ") just piped '" + input + "' to " + string(shmStartAddr[sendrecvuid-1].name) + " (#" + to_string(shmStartAddr[sendrecvuid-1].uid)+") ***";
        sm_broadcast(msg);
    }

    // cout << "**********************************************1" << endl;

    // Shell commands 
    string instruction = "";
    for (int i=0; i<arg_num; i++) {
        instruction = instruction + arg[i] + " ";
    }
    instruction += "\n";
    // cout << "**********************************************1.5" << endl;
    sm_shell_function(instruction, sockfd);
    
    // cout << "**********************************************2" << endl;

}

// , bool pipe_in_flg, bool pipe_out_flg, int in_pipe_num, int out_pipe_num
int sm_shell_function(string instruction, int sockfd) {
    char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
    CMD_dict valid_cmd_dict;
    int cmd_amt, ttl, OP_len, np_cnt, i, start_index, ith[EX_LIMIT];
    bool ifNumberPipe, first_flg, ex_flg;
    int cli = shmCurrentAddr->uid;
    cmd *localCmd;
    ordinary_pipe_t current_op, tmp_op;

    reset_dict(&valid_cmd_dict);

    memcpy(input, instruction.c_str(), instruction.size());

    targetNode[cli] = newNode[cli] = parentNode[cli] = NULL;
    // Original while
    if (!strcmp(input, "\n")) {
        return 0;
    }
    // reset
    char *terms[LENGTH_LIMIT][TERM_AMT_LIMIT];
    cmd_amt = OP_len = start_index = 0;
    ifNumberPipe = ex_flg = false;
    memset(filename, 0, LENGTH_LIMIT);
    reset(&current_op, ith);
    memset(&current_op, 0, sizeof(ordinary_pipe_t));
    scan(input, ith);
    // Parse input and store info into a ordinary_pipe_t
    np_cnt = split_terms(input, &current_op, filename);
    memset(input, 0, LENGTH_LIMIT);


    // // Minus every nodes' ttl to maintan number pipe DS.
    // minus_ttl(&knowNode[cli]);
    targetNode[cli] = getSpecificNode(&knowNode[cli], 0);

    /*
    Priority: store info into a number_pipe_t -> end a number pipe request ->
    other requests
    1. when np_cnt > 0, which means there are some info should be saved into
    number_pipe_t, or even be ended (number test.html |1 number).
        1.1. if ifNumberPipe is true, means the cmd should be saved.
        1.2. else do the same things like 2.
    2. when targetNode isn't empty and current cmd isn't also number pipe,
    it's time to end a number pipe request.
    3. Tackle remaining requests, e.g. ordinary pipe, normal commands, etc.
    */

    // 1.
    if (np_cnt) {
        first_flg = true;
        for (i = 0; i < np_cnt + 1; i++) {
            targetNode[cli] = newNode[cli] = parentNode[cli] = NULL;
            memset(&tmp_op, 0, sizeof(ordinary_pipe_t));
            start_index = copy_ith_op(&tmp_op, &current_op, start_index);
            // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
            // if (in_pipe_num > 0) {
            //     in_pipe_num_v[cli].push_back(in_pipe_num);
            //     tmp_op.in_pipe_num = in_pipe_num;
            // }
            if (start_index == -1) {
                break;
            }

            if (!check_valid(&tmp_op, illegal_term, &ifNumberPipe, &OP_len,
                                &valid_cmd_dict)) {
                string msg = "Unknown command: [" + string(illegal_term) + "].\n";
                write(sockfd, msg.c_str(), msg.length());
                return 0;
            }
            // cout << "1.0------------0" << endl;
            if (strlen(tmp_op.cmds[OP_len].terms[0]) == 0) {
                ifNumberPipe = false;
            }

            if (first_flg) {
                first_flg = false;
            } else {
                minus_ttl(&knowNode[cli]);
            }
            targetNode[cli] = getSpecificNode(&knowNode[cli], 0);
            // 1.1.
            if (ifNumberPipe) {
                ttl = tran2number(tmp_op.cmds[OP_len].terms[0]);
                if (targetNode[cli]) {
                    // cout << "1.1------------1" << endl;
                    parentNode[cli] = getSpecificNode(&knowNode[cli], ttl);
                    if (parentNode[cli]) {
                        addInfo2ExistNode(parentNode[cli], &tmp_op, OP_len, ith, i);
                        adoptChild(parentNode[cli], targetNode[cli]);
                    } else {
                        newNode[cli] = createNode(&knowNode[cli]);
                        addInfo2ExistNode(newNode[cli], &tmp_op, OP_len, ith, i);
                        adoptChild(newNode[cli], targetNode[cli]);
                    }
                } else {
                    // cout << "1.1------------2" << endl;
                    targetNode[cli] = getSpecificNode(&knowNode[cli], ttl);
                    if (targetNode[cli]) {
                        addInfo2ExistNode(targetNode[cli], &tmp_op, OP_len, ith, i);
                    } else {
                        newNode[cli] = createNode(&knowNode[cli]);
                        addInfo2ExistNode(newNode[cli], &tmp_op, OP_len, ith, i);
                    }
                }
            }
            // 1.2.
            else if (targetNode[cli]) {
                // cout << "1.2------------1" << endl;
                newNode[cli] = createNode(&knowNode[cli]);
                // cout << "1.2.-0" << endl;
                addInfo2ExistNode(newNode[cli], &tmp_op, OP_len, ith, i);
                // cout << "1.2.-1" << endl;
                adoptChild(newNode[cli], targetNode[cli]);
                // cout << "1.2.-2" << endl;
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // , cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num
                sm_bin_func(&tmp_op, filename, newNode[cli], sockfd);
                // cout << "1.2.-3" << endl;
            }
        }
    } else {
        if (!check_valid(&current_op, illegal_term, &ifNumberPipe, &OP_len,
                            &valid_cmd_dict)) {
            // cout << "Y111" << endl;
            string msg = "Unknown command: [" + string(illegal_term) + "].\n";
            write(sockfd, msg.c_str(), msg.length());
            return 0;
        }
        // 2.
        if (targetNode[cli]) {
            // cout << "Y222" << endl;
            newNode[cli] = createNode(&knowNode[cli]);
            addInfo2ExistNode(newNode[cli], &current_op, OP_len, ith, -2);
            adoptChild(newNode[cli], targetNode[cli]);
            // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
            // bin_func(&current_op, filename, newNode[cli], sockfd, cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num);
            sm_bin_func(&current_op, filename, newNode[cli], sockfd);
        }
        // 3.
        else {
            // cout << "Y333" << endl;
            localCmd = &(current_op.cmds[0]);
            if (!strcmp(localCmd->terms[0], "printenv")) {
                // cout << "Y333-1" << endl;
                // Error condition check, if need?
                string env = localCmd->terms[1];
                string msg = getenv(env.c_str()) + string("\n");
                write(shmCurrentAddr->fd, msg.c_str(), msg.size());
            } else if (!strcmp(localCmd->terms[0], "setenv")) {
                // cout << "Y333-2" << endl;
                // Error condition check, if need?
                string env = localCmd->terms[1];
                string assign = localCmd->terms[2];
                setenv(env.c_str(), assign.c_str(), 1);
                reset_dict(&valid_cmd_dict);
            } else {
                // cout << "Y333-3" << endl;
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // bin_func(&current_op, filename, newNode[cli], sockfd, cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num);
                // , cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num
                sm_bin_func(&current_op, filename, newNode[cli], sockfd);
            }
        }
    }
    // show_node(&knowNode[cli]);
}

// HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
// , int cli, bool pipe_in_flg, bool pipe_out_flg, int in_pipe_num, int out_pipe_num
void sm_bin_func(ordinary_pipe_t *localOP, char *filename, number_pipe_t *Node, int sockfd) {
    int fd;
    cmd *localCmd;
    pid_t pid;
    pid = fork();

    switch (pid) {
        case -1:
            perror("Error");
            exit(EXIT_FAILURE);
        case 0:
            // Open redirection mechanism
            if (*filename) {
                fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                if (fd == -1) {
                    perror("Error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            } 
            // cout << "HERE" << endl;
            
            if (Node) {
                // cout << "HERE-1" << endl;
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // , cli, out_pipe_num
                sm_numbered_pipe(Node, 0);
            // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
            // } else if (in_pipe_num >= 0) {
            } else if (true) {
                if (localOP->cnt == 1) {
                    // cout << "HERE-2" << endl;
                    // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                    // user_pipe_redirect(in_pipe_num, out_pipe_num, cli);
                    // sm_exec_cmds(localOP->cmds, pipe_in_flg);
                    // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                    sm_exec_cmds(localOP->cmds, true);
                } else {
                    // cout << "HERE-3" << endl;
                    // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                    // , cli, in_pipe_num, out_pipe_num
                    sm_ordinary_pipe(localOP);
                }
            } else {
                exit(EXIT_SUCCESS);
            }

        default:
            // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
            // cout << "FFFFFFFFFFFFFFFFUC<MKK11111111111111" << endl;
            // if(in_pipe_num > 0){
            //     cout << "INNER11111111111111" << endl;
            //     close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
            //     close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
            // } else if (Node) {
            //     for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
            //         in_pipe_num = in_pipe_num_v[cli][i];
            //         close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
            //         close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
            //     }
            // }
            wait(NULL);
            // cout << "FFFFFFFFFFFFFFFFUC<MKK222222222222" << endl;
            // if(in_pipe_num > 0){
            //     cout << "INNER222222222222222222" << endl;
            //     used_user_pipe[in_pipe_num][cli].user_pipe[0] = -1;
            //     used_user_pipe[in_pipe_num][cli].user_pipe[1] = -1;
            // } else if (Node) {
            //     for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
            //         in_pipe_num = in_pipe_num_v[cli][i];
            //         used_user_pipe[in_pipe_num][cli].user_pipe[0] = -1;
            //         used_user_pipe[in_pipe_num][cli].user_pipe[1] = -1;
            //     }
            //     in_pipe_num_v[cli].clear();
            // }
            break;
    }
}

// HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
// , int cli, int out_pipe_num
void sm_numbered_pipe(number_pipe_t *Node, int op_idx) {
    pid_t pid;
    int fds[2], i, j, stat;
    int in_pipe_num, status;
    bool first = true;
    number_pipe_t *localNode;
    cmd *localCmd;
    ordinary_pipe_t *op;
    op = &(Node->ops[op_idx]);
    in_pipe_num = op->in_pipe_num;

    if (!Node->child[op_idx]) {
        op = &(Node->ops[op_idx]);
        in_pipe_num = op->in_pipe_num;
        if (in_pipe_num >= 0) {
            if (op->cnt == 1){
                sm_exec_cmds(&(op->cmds[0]), UNKNOWN);
            }
            else{
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // , cli, in_pipe_num, out_pipe_num
                sm_ordinary_pipe(op);
            }
        }
    }

    if (pipe(fds) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    localNode = Node->child[op_idx];

    for (j = 0; j < localNode->op_cnt; j++) {
        pid = fork();
        switch (pid) {
            case -1:
                perror("Error");
                exit(EXIT_FAILURE);
            case 0:
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // if (localNode->ops[j].in_pipe_num <= 0) {
                //     for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                //         in_pipe_num = in_pipe_num_v[cli][i];
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                //     }
                // } else {
                //     // Redirect pipe in
                //     in_pipe_num = localNode->ops[j].in_pipe_num;
                //     if (in_pipe_num > 0) {
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                //         dup2(used_user_pipe[in_pipe_num][cli].user_pipe[0], 0);
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                //     }
                // }
                if (localNode->ops[j].ex_flg && localNode->ops[j].cnt == 1) {
                    dup2(fds[1], STDERR_FILENO);
                }
                dup2(fds[1], STDOUT_FILENO);
                close(fds[0]);
                close(fds[1]);
                
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // , cli, out_pipe_num
                sm_numbered_pipe(localNode, j);
                break;
            default:
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // if (first) {
                //     //! 這裡應該要關掉的是那些曾經in_pipe_num不為0的
                //     for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                //         in_pipe_num = in_pipe_num_v[cli][i];
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                //         close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                //     }

                //     if (!Node->parent && out_pipe_num > 0) {
                //         close(used_user_pipe[cli][out_pipe_num].user_pipe[0]);
                //         dup2(used_user_pipe[cli][out_pipe_num].user_pipe[1], 1);
                //         close(used_user_pipe[cli][out_pipe_num].user_pipe[1]);
                //     }
                // }
                first = false;
                wait(NULL);
                break;
        }
    }
    // waitpid(pid, &status, 0);
    // wait(NULL);
    
    
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);
    op = &(Node->ops[op_idx]);
    if (op->cnt == 1){
        // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
        sm_exec_cmds(&(Node->ops[op_idx].cmds[0]), UNKNOWN);
    }
    else{
        // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
        // , cli, in_pipe_num, out_pipe_num
        sm_ordinary_pipe(&(Node->ops[op_idx]));
    }
}

void sm_exec_cmds(cmd *localCmd, bool pipe_in_flg) {
    if (localCmd->cnt == 1 && strcmp(localCmd->terms[0], "ls") && !pipe_in_flg){
        exit(EXIT_FAILURE);
    }
    char *argv[localCmd->cnt+1];
    for(int i=0; i<localCmd->cnt; i++){
        char tmp_buf[MAXLEN];
        strcpy(tmp_buf, localCmd->terms[i]);
        argv[i] = strdup(tmp_buf);
    }
    argv[localCmd->cnt] = NULL;
    int err = execvp(argv[0], argv);
    if(err != 0){
        printf("Unknown command: [%s].\n", localCmd->terms[0]);
        exit(EXIT_FAILURE);
    }
}

// , int cli, int in_pipe_num, int out_pipe_num
void sm_ordinary_pipe(ordinary_pipe_t *op) {
    int *fds, fds_size;
    int cmd_cnt = op->cnt, status;
    int i, j;
    pid_t pid;
    
    // register signal
    signal(SIGCHLD, SIG_IGN);

    fds_size = 2 * (op->cnt - 1);
    fds = (int *)malloc(fds_size * sizeof(int));

    for (i = 0; i < cmd_cnt - 1; i++) {
        if (pipe(fds + i * 2) < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        };
    }
    for (i = 0; i < cmd_cnt - 1; i++) {
        pid = fork();
        if (pid == -1) {
            printf("fork failed\n");
            exit(1);
        }

        if (pid == 0) {
            if (i != 0) {
                // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
                // if (in_pipe_num > 0) {
                //     close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                //     close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                // }
                if (dup2(fds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(1);
                };
            }
            // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
            // else if (in_pipe_num > 0){
            //     close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
            //     dup2(used_user_pipe[in_pipe_num][cli].user_pipe[0], 0);
            //     close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
            // }
            
            if (dup2(fds[2 * i + 1], STDOUT_FILENO) < 0) {
                perror("dup2");
                exit(1);
            };
            for (j = 0; j < fds_size; j++) {
                close(fds[j]);
            }
            
            exec_cmds(&(op->cmds[i]), UNKNOWN);
        }
    }
    // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
    // if (in_pipe_num > 0) {
    //     close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
    //     close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
    // }

    if (dup2(fds[((cmd_cnt-1) - 1) * 2], STDIN_FILENO) < 0) {
        perror("dup2");
        exit(1);
    };

    // HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
    // Redirect pipe out
    // if (out_pipe_num > 0) {
    //     close(used_user_pipe[cli][out_pipe_num].user_pipe[0]);
    //     dup2(used_user_pipe[cli][out_pipe_num].user_pipe[1], 1);
    //     close(used_user_pipe[cli][out_pipe_num].user_pipe[1]);
    // }
    // else if (out_pipe_num < 0)
    //     dup2(DEVNULLO, 1);

    for (j = 0; j < fds_size; j++) {
        close(fds[j]);
    }

    free(fds);
    exec_cmds(&(op->cmds[cmd_cnt - 1]), UNKNOWN);
}

bool sm_choose_rwg_cmd(vector<string> args) {
    if (args[0] == "who"){
        sm_rwg_who(args);
        return true;
    } else if (args[0] == "tell") {
        sm_rwg_tell(args);
        return true;
    } else if (args[0] == "yell") {
        sm_rwg_yell(args);
        return true;
    } else if (args[0] == "name") {
        sm_rwg_name(args);
        return true;
    } else if (args[0] == "exit") {
        sm_rwg_exit(args);
        return true;
    }
    return false;
}

void sm_rwg_exit(vector<string> args) {
    string msg;
    msg = "*** User \'" + string(shmCurrentAddr->name) + "\' left. ***";
    sm_broadcast(msg);

    // // Clean user pipe
    // for (int i = 0 ; i < LISTENQ ; i++){
    //     used_user_pipe[i][cli].user_pipe[0] = -1;
    //     used_user_pipe[i][cli].user_pipe[1] = -1;
    //     used_user_pipe[cli][i].user_pipe[0] = -1;
    //     used_user_pipe[cli][i].user_pipe[1] = -1;
    // }
    // in_pipe_num_v[cli].clear();
    // for(int j = 0; j < NODE_LIMIT; j++) {
    //     knowNode[cli].record[j] = NULL;
    // }
    // knowNode[cli].cnt = 0;
    // newNode[cli] = NULL;
    // targetNode[cli] = NULL;
    // parentNode[cli] = NULL;

    // Reset and close connection
    shmCurrentAddr->pid = -1;
    shmCurrentAddr->fd = -1;
    shmCurrentAddr->uid = -1;
    for(int j=0; j<LISTENQ; j++)
    {
        (shmCurrentAddr->sendInfo)[j] = -1;
        (shmCurrentAddr->recvInfo)[j] = -1;
    }
    strcpy(shmCurrentAddr->msg,"");
    
    ClearClientTable();
    ClearPipe();

    if(shmdt(shmStartAddr) == -1)
    {
        cerr<<"Fail to detach shm, errno: "<<errno<<endl;
        exit(EXIT_SUCCESS);
    }
    if (shmctl(shmID,IPC_RMID,0) == -1)
    {
        cerr<<"Fail to delete shm, errno: "<<errno<<endl;
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_SUCCESS);
}

void sm_rwg_name(vector<string> args) {
    bool dupFlg = false;
    for (int i = 0 ; i < LISTENQ; i++) {
        if ((shmStartAddr+i)->name == args[1]){
            dupFlg = true;
            break;
        }
    }
    
    string msg;
    if (dupFlg) {
        msg = "*** User \'" + args[1] + "\' already exists. ***\n";
        if (write(shmCurrentAddr->fd, msg.c_str(), msg.size()) == -1) {
            cerr << "Error: Failed to write data to socket.\n";
            exit(EXIT_FAILURE);
        }
    } else {
        strcpy(shmCurrentAddr->name, args[1].c_str());
        msg = "*** User from " + string(shmCurrentAddr->ip) + ":" + to_string(shmCurrentAddr->port) + " is named \'"+ string(shmCurrentAddr->name) +"\'. ***";
        sm_broadcast(msg);
    }
}

void sm_rwg_yell(vector<string> args) {
    string msg = "*** "+string(shmCurrentAddr->name)+" yelled ***: ";
    for (int i = 1; i < args.size(); i++) {
        msg += args[i] + " ";
    }
    // msg += "\n";

    sm_broadcast(msg);
}

void sm_rwg_tell(vector<string> args) {
    int towhom;
    istringstream iss(args[1]);
    iss >> towhom;

    // If rcv doesn't exist
    if (!HasClient(towhom)) {
        string errmsg = "*** Error: user #"+ to_string(towhom) +" does not exist yet. ***\n";
        if (write(shmCurrentAddr->fd, errmsg.c_str(), errmsg.length()) == -1) {
            cerr << "Error: Failed to write data to socket.\n";
            exit(EXIT_FAILURE);
        }
        return;
    }

    // If rcv exists
    string msg = "*** " + string(shmCurrentAddr->name) + " told you ***: ";
    for (int i = 2; i < args.size(); i++) {
        msg += args[i] + " ";
    }
    // msg += "\n";

    // if (write(client[towhom], msg.c_str(), msg.size()) == -1) {
    //     cerr << "Error: Failed to write data to socket.\n";
    //     exit(EXIT_FAILURE);
    // }

    for(int i=0; i<LISTENQ; i++)
    {
        if( (shmStartAddr+i)->uid  == towhom)
        {
            strcpy((shmStartAddr + i)->msg, msg.c_str());
            kill((shmStartAddr+i)->pid, SIGUSR1);
            break;
        }
    }
}

void sm_rwg_who(vector<string> args) {
    string msg = WHO_COLUMN;
    for (int i = 0; i < LISTENQ; i++) {
        if ((shmStartAddr+i)->pid != -1){
            msg += to_string(i+1) + '\t' + string((shmStartAddr+i)->name) + '\t' + string((shmStartAddr+i)->ip) + ':' + to_string((shmStartAddr+i)->port);
            if((shmStartAddr+i)->uid == shmCurrentAddr->uid) {
                msg += "\t<-me";
            }
            msg += '\n';
        }
    }
    
    if (write(shmCurrentAddr->fd, msg.c_str(), msg.length()) == -1) {
        cerr << "Error: Failed to write data to socket.\n";
        exit(EXIT_FAILURE);
    }
}

void InitpipeV() {
    // 設定共享記憶體的 key
    key_t key = ftok("shared_memory_key", 1);
    if (key == -1) {
        std::cerr << "ftok failed" << std::endl;
        return;
    }

    // 創建共享記憶體
    int shmid = shmget(key, sizeof(std::vector<struct Pipe>), IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cerr << "shmget failed" << std::endl;
        return;
    }

    // 附加共享記憶體
    void* shm_pipeV = shmat(shmid, nullptr, 0);
    if (shm_pipeV == (void*)-1) {
        std::cerr << "shmat failed" << std::endl;
        return;
    }

    // 將 vector 拷貝到共享記憶體
    memcpy(shm_pipeV, &pipeV, sizeof(std::vector<struct Pipe>));
}

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

void sm_broadcast(string msg){
    for(int i=0; i<LISTENQ; i++)
    {
        if( (shmStartAddr+i)->pid  != -1)
        {
            strcpy((shmStartAddr + i)->msg, msg.c_str());
            //! 為何
            kill((shmStartAddr+i)->pid , SIGUSR1);
        }
    }
}

void _Exit()
{
    // _BroadCast("",EXIT,-1);
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
    
    string newer_msg = MOTD;
    string msg = "*** User '" + string(shmCurrentAddr->name)  + "' entered from " + string(shmCurrentAddr->ip) + ":" + to_string(shmCurrentAddr->port) + ". ***";
    write(sockfd, newer_msg.c_str(), newer_msg.length());
    sm_broadcast(msg);
    write(sockfd, "% ", strlen("% "));

    while(1) {
        sm_rwg_func(sockfd);
        write(sockfd, "% ", strlen("% "));
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

    // cout << "AAAAAAAAAAAAAAAAATTTTTTTTTTTTTTENTION" << endl;
    while(iter != pipeV.end())
    {
        // cout << (*iter).senderId << "  " << (*iter).recverId << endl;

        if((*iter).senderId == senderId && (*iter).recverId == recverId)
            return true;
            
        iter++;
    }
    // cout << endl << endl;
    
    return false;
}

void sm_CreatePipe(int clientId, int senderId, int recverId)
{
    int* pipefd = new int [2];
    struct Pipe newPipe;

    if(pipe(pipefd)<0)
    {
        cerr<<"Pipe create fail"<<" eerrno:"<<errno<<endl;
        exit(1);
    }
    newPipe.pipefd = pipefd;
    // newPipe.sign = sign;
    // newPipe.pipeNum = pipeNum;
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

void SigClient(int signo)
{
    // cout<<shmCurrentAddr->msg<<endl;
    string msg(shmCurrentAddr->msg);
    msg += "\n";
    write(shmCurrentAddr->fd, msg.c_str(), msg.length());
    strcpy(shmCurrentAddr->msg,"");
}