// ERROR Case -> 有空再處理
// 輸入錯的環境變數再輸入指令會報錯
int shell_function(string instruction, int sockfd, int cli, bool pipe_in_flg, bool pipe_out_flg, int in_pipe_num, int out_pipe_num) {
    char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
    CMD_dict valid_cmd_dict;
    int cmd_amt, ttl, OP_len, np_cnt, i, start_index, ith[EX_LIMIT];
    bool ifNumberPipe, first_flg, ex_flg;
    
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
            if (in_pipe_num > 0) {
                in_pipe_num_v[cli].push_back(in_pipe_num);
                tmp_op.in_pipe_num = in_pipe_num;
            }
            if (start_index == -1) {
                break;
            }

            if (!check_valid(&tmp_op, illegal_term, &ifNumberPipe, &OP_len,
                                &valid_cmd_dict)) {
                string msg = "Unknown command: [" + string(illegal_term) + "].\n";
                write(sockfd, msg.c_str(), msg.length());
                return 0;
            }
            cout << "1.0------------0" << endl;
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
                    cout << "1.1------------1" << endl;
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
                    cout << "1.1------------2" << endl;
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
                cout << "1.2------------1" << endl;
                newNode[cli] = createNode(&knowNode[cli]);
                // cout << "1.2.-0" << endl;
                addInfo2ExistNode(newNode[cli], &tmp_op, OP_len, ith, i);
                // cout << "1.2.-1" << endl;
                adoptChild(newNode[cli], targetNode[cli]);
                // cout << "1.2.-2" << endl;
                bin_func(&tmp_op, filename, newNode[cli], sockfd, cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num);
                // cout << "1.2.-3" << endl;
            }
        }
    } else {
        if (!check_valid(&current_op, illegal_term, &ifNumberPipe, &OP_len,
                            &valid_cmd_dict)) {
            cout << "Y111" << endl;
            string msg = "Unknown command: [" + string(illegal_term) + "].\n";
            write(sockfd, msg.c_str(), msg.length());
            return 0;
        }
        // 2.
        if (targetNode[cli]) {
            cout << "Y222" << endl;
            newNode[cli] = createNode(&knowNode[cli]);
            addInfo2ExistNode(newNode[cli], &current_op, OP_len, ith, -2);
            adoptChild(newNode[cli], targetNode[cli]);
            bin_func(&current_op, filename, newNode[cli], sockfd, cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num);
        }
        // 3.
        else {
            cout << "Y333" << endl;
            localCmd = &(current_op.cmds[0]);
            if (!strcmp(localCmd->terms[0], "printenv")) {
                cout << "Y333-1" << endl;
                // Error condition check, if need?
                string msg = env[cli][localCmd->terms[1]] + "\n";
                write(sockfd, msg.c_str(), msg.length());
            } else if (!strcmp(localCmd->terms[0], "setenv")) {
                cout << "Y333-2" << endl;
                // Error condition check, if need?
                env[cli][localCmd->terms[1]] = localCmd->terms[2];
                reset_dict(&valid_cmd_dict);
            } else {
                cout << "Y333-3" << endl;
                bin_func(&current_op, filename, newNode[cli], sockfd, cli, pipe_in_flg, pipe_out_flg, in_pipe_num, out_pipe_num);
            }
        }
    }

    // show_node(&knowNode[cli]);
}

int tran2number(char *term) {
    char *p = term;
    int num = 0;
    while (*p) {
        num *= 10;
        num += (*p - '0');
        p++;
    }
    return num;
}

// Return the number of number_pipe_t
int split_terms(char *input, ordinary_pipe_t *op, char *filename) {
    char d[10];
    char *p, *commands;
    char *command[LENGTH_LIMIT];
    char local_input[LENGTH_LIMIT];
    cmd *localCmd;
    int i, j, k, np_cnt;

    i = 0;
    memset(d, 0, 10);
    strcpy(d, ">");
    strcpy(local_input, input);
    local_input[strlen(local_input) - 1] = 0;
    p = strtok(local_input, d);
    while (p != NULL) {
        if (i++ == 0) {
            commands = p;
        } else {
            strcpy(filename, p);
            trim(filename);
        }
        // try strcpy, and see term's address if is same as p.
        p = strtok(NULL, d);
    }

    i = 0;
    memset(d, 0, 10);
    strcpy(d, "|");
    p = strtok(commands, d);
    while (p != NULL) {
        *(command + i) = p;
        i++;
        p = strtok(NULL, d);
    }
    op->cnt = i;

    i = 0;
    memset(d, 0, 10);
    strcpy(d, "  ");

    //! I just can't figure out why the last time array value would be
    //! kept.!!!!!!!!!!!!!!!!!!!!!!!
    char **t = command;
    while (*t) {
        j = 0;
        p = strtok(*t, d);
        localCmd = &(op->cmds[i]);
        while (p != NULL) {
            // I don't surely know why replace assign for strcpy would be wrong.
            strcpy(localCmd->terms[j++], p);
            localCmd->cnt++;
            p = strtok(NULL, d);
        }
        i++;
        t++;
    }

    np_cnt = 0;
    for (i = 0; i < op->cnt; i++) {
        if (isdigit_myself(op->cmds[i].terms[0])) {
            np_cnt++;
        }
    }
    return np_cnt;
}

// Return end index
int copy_ith_op(ordinary_pipe_t *tmp_op, ordinary_pipe_t *current_op,
                int start_index) {
    int i, j;
    bool first_flg;
    cmd *tmpCmd, *localCmd;

    first_flg = true;
    if (start_index == 0) {
        for (i = 0; i < current_op->cnt; i++) {
            localCmd = &(current_op->cmds[i]);
            tmpCmd = &(tmp_op->cmds[i]);
            if (isdigit_myself(localCmd->terms[0]) && !first_flg) {
                strcpy(tmpCmd->terms[0], localCmd->terms[0]);
                tmpCmd->cnt++;
                tmp_op->cnt++;
                return i;
            }
            for (j = 0; j < localCmd->cnt; j++) {
                strcpy(tmpCmd->terms[j], localCmd->terms[j]);
                tmpCmd->cnt++;
            }
            first_flg = false;
            tmp_op->cnt++;
        }
    } else {
        for (i = start_index; i < current_op->cnt; i++) {
            localCmd = &(current_op->cmds[i]);
            tmpCmd = &(tmp_op->cmds[i - start_index]);

            if (strlen(localCmd->terms[1]) == 0 && first_flg) {
                return -1;
            }

            if (isdigit_myself(localCmd->terms[0]) && !first_flg) {
                strcpy(tmpCmd->terms[0], localCmd->terms[0]);
                tmpCmd->cnt++;
                tmp_op->cnt++;
                return i;
            }
            if (first_flg) {
                for (j = 0; j < localCmd->cnt - 1; j++) {
                    strcpy(tmpCmd->terms[j], localCmd->terms[j + 1]);
                    tmpCmd->cnt++;
                }
            } else {
                for (j = 0; j < localCmd->cnt; j++) {
                    strcpy(tmpCmd->terms[j], localCmd->terms[j]);
                    tmpCmd->cnt++;
                }
            }
            // I just can not figure out why current_op save last time records, but i
            // have already do memset or reset_OP. It took me 4 hours but was still
            // unsolved.
            // show(current_op);
            first_flg = false;
            tmp_op->cnt++;
        }
    }
}

void sig_handler(){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){}
    return;
}

void scan(char *input, int *ith) {
    int idx, i, j;
    idx = j = 0;
    *ith = -1;
    for (i = 1; i < strlen(input); i++) {
        if (isdigit(input[i])) {
            if (input[i - 1] == '|') {
                idx++;
            } else if (input[i - 1] == '!') {
                ith[j++] = idx;
                idx++;
                input[i - 1] = '|';
            }
        }
    }
}

void reset_dict(CMD_dict *valid_cmd_dict) {
    int i;
    char *s, *p;
    char d[10], local_input[LENGTH_LIMIT];
    DIR *dir;
    struct dirent *ent;
    memset(valid_cmd_dict, 0, sizeof(CMD_dict));

    s = getenv("PATH");

    memset(d, 0, 10);
    strcpy(d, ":");
    strcpy(local_input, s);
    p = strtok(local_input, d);
    i = 0;
    while (p != NULL) {
        dir = opendir(p);
        if (dir == NULL) {
            break;
        }
        while ((ent = readdir(dir)) != NULL) {
            strcpy(valid_cmd_dict->dict[i++], ent->d_name);
            valid_cmd_dict->cnt++;
        }
        closedir(dir);
        p = strtok(NULL, d);
    }
}

void exec_cmds(cmd *localCmd, bool pipe_in_flg) {
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

void reset(ordinary_pipe_t *current_op, int *ith) {
    cmd *localCmd;
    int i, j;
    for (i = 0; i < current_op->cnt; i++) {
        localCmd = &(current_op->cmds[i]);
        for (j = 0; j < localCmd->cnt; j++) {
            memset(localCmd->terms[j], 0, sizeof(localCmd->terms[j]));
        }
        localCmd->cnt = 0;
    }
    current_op->cnt = 0;

    for (i = 0; i < EX_LIMIT; i++) {
        ith[i] = -1;
    }
}

void show(ordinary_pipe_t *localOP) {
    int x, y;
    cmd *localCmd;
    printf("Commands amount: %d\tEX flg:%d\n", localOP->cnt, localOP->ex_flg);
    cout << "USER PIPE IN: " << localOP->in_pipe_num << endl;

    for (x = 0; x < 3; x++) {
        localCmd = &(localOP->cmds[x]);
        printf("%d\t", localCmd->cnt);
        for (y = 0; y < localCmd->cnt; y++) {
            printf("%s\t", localCmd->terms[y]);
        }
        printf("\n");
    }
    printf("\n");
}

void printenv(char terms[TERM_AMT_LIMIT][CHAR_AMT_LIMIT], int cmd_amt) {
    if (cmd_amt == 1) {
        printf("Usage: printenv [var]");
    } else {
        char *s = getenv(*(terms + 1));
        if (s) {
            printf("%s\n", s);
        }
    }
}

void bin_func(ordinary_pipe_t *localOP, char *filename, number_pipe_t *Node, int sockfd, int cli, bool pipe_in_flg, bool pipe_out_flg, int in_pipe_num, int out_pipe_num) {
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
            } else {
                dup2(sockfd, 1);
                dup2(sockfd, 2);
            }
            // cout << "HERE" << endl;
            
            if (Node) {
                // cout << "HERE-1" << endl;
                numbered_pipe(Node, 0, cli, out_pipe_num);
            } else if (in_pipe_num >= 0) {
                if (localOP->cnt == 1) {
                    // cout << "HERE-2" << endl;
                    user_pipe_redirect(in_pipe_num, out_pipe_num, cli);
                    exec_cmds(localOP->cmds, pipe_in_flg);
                } else {
                    // cout << "HERE-3" << endl;
                    ordinary_pipe(localOP, cli, in_pipe_num, out_pipe_num);
                }
            } else {
                exit(EXIT_SUCCESS);
            }

        default:
            cout << "FFFFFFFFFFFFFFFFUC<MKK11111111111111" << endl;
            if(in_pipe_num > 0){
                cout << "INNER11111111111111" << endl;
                close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
            } else if (Node) {
                for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                    in_pipe_num = in_pipe_num_v[cli][i];
                    close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                    close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                }
            }
            wait(NULL);
            cout << "FFFFFFFFFFFFFFFFUC<MKK222222222222" << endl;
            if(in_pipe_num > 0){
                cout << "INNER222222222222222222" << endl;
                used_user_pipe[in_pipe_num][cli].user_pipe[0] = -1;
                used_user_pipe[in_pipe_num][cli].user_pipe[1] = -1;
            } else if (Node) {
                for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                    in_pipe_num = in_pipe_num_v[cli][i];
                    used_user_pipe[in_pipe_num][cli].user_pipe[0] = -1;
                    used_user_pipe[in_pipe_num][cli].user_pipe[1] = -1;
                }
                in_pipe_num_v[cli].clear();
            }
            break;
    }
}

void user_pipe_redirect(int in_pipe_num, int out_pipe_num, int cli) {
    // Redirect pipe in
    if (in_pipe_num > 0) {
        close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
        dup2(used_user_pipe[in_pipe_num][cli].user_pipe[0], 0);
        close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
    }
    else if (in_pipe_num < 0)
        dup2(DEVNULLI, 0);
    
    // Redirect pipe out
    if (out_pipe_num > 0) {
        close(used_user_pipe[cli][out_pipe_num].user_pipe[0]);
        dup2(used_user_pipe[cli][out_pipe_num].user_pipe[1], 1);
        close(used_user_pipe[cli][out_pipe_num].user_pipe[1]);
    }
    else if (out_pipe_num < 0)
        dup2(DEVNULLO, 1);
}

void trim(char *str) {
    int cnt;
    char *p;
    cnt = 0;
    p = str;
    while (*p == ' ') {
        cnt++;
        p++;
    }
    memmove(str, p, strlen(str) - cnt + 1);
    p = str + strlen(str) - 1;
    while (*p == ' ') {
        *p = '\0';
        p--;
    }
}

void ordinary_pipe(ordinary_pipe_t *op, int cli, int in_pipe_num, int out_pipe_num) {
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
                if (in_pipe_num > 0) {
                    close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                    close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                }
                if (dup2(fds[(i - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(1);
                };
            }
            else if (in_pipe_num > 0){
                close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                dup2(used_user_pipe[in_pipe_num][cli].user_pipe[0], 0);
                close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
            }
            
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
    if (in_pipe_num > 0) {
        close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
        close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
    }

    if (dup2(fds[((cmd_cnt-1) - 1) * 2], STDIN_FILENO) < 0) {
        perror("dup2");
        exit(1);
    };

    // Redirect pipe out
    if (out_pipe_num > 0) {
        close(used_user_pipe[cli][out_pipe_num].user_pipe[0]);
        dup2(used_user_pipe[cli][out_pipe_num].user_pipe[1], 1);
        close(used_user_pipe[cli][out_pipe_num].user_pipe[1]);
    }
    else if (out_pipe_num < 0)
        dup2(DEVNULLO, 1);

    for (j = 0; j < fds_size; j++) {
        close(fds[j]);
    }

    free(fds);
    exec_cmds(&(op->cmds[cmd_cnt - 1]), UNKNOWN);
}

void minus_ttl(know_node *knowNode) {
    int i;
    number_pipe_t *localNode;
    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];
        if (localNode->ttl != -1) {
            localNode->ttl--;
        }
    }
}

void show_tree(number_pipe_t *Node) {
    int i;
    number_pipe_t *childNode;
    if (Node->chd_cnt) {
        for (i = 0; i < Node->chd_cnt; i++) {
            childNode = Node->child[i];
            show_tree(childNode);
        }
    }
    printf("%d ", Node->id);
}

void adoptChild(number_pipe_t *parent, number_pipe_t *child) {
    parent->child[parent->op_cnt-1] = child;
    parent->chd_cnt = parent->op_cnt;
    child->parent = parent;
}

//! cat <2 >2會出問題
//! cat <2 |1會錯，number pipe function還沒改
void numbered_pipe(number_pipe_t *Node, int op_idx, int cli, int out_pipe_num) {
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
                exec_cmds(&(op->cmds[0]), UNKNOWN);
            }
            else{
                ordinary_pipe(op, cli, in_pipe_num, out_pipe_num);
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
                if (localNode->ops[j].in_pipe_num <= 0) {
                    for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                        in_pipe_num = in_pipe_num_v[cli][i];
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                    }
                } else {
                    // Redirect pipe in
                    in_pipe_num = localNode->ops[j].in_pipe_num;
                    if (in_pipe_num > 0) {
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                        dup2(used_user_pipe[in_pipe_num][cli].user_pipe[0], 0);
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                    }
                }
                if (localNode->ops[j].ex_flg && localNode->ops[j].cnt == 1) {
                    dup2(fds[1], STDERR_FILENO);
                }
                dup2(fds[1], STDOUT_FILENO);
                close(fds[0]);
                close(fds[1]);
                
                numbered_pipe(localNode, j, cli, out_pipe_num);
                break;
            default:
                if (first) {
                    //! 這裡應該要關掉的是那些曾經in_pipe_num不為0的
                    for (int i = 0 ; i < in_pipe_num_v[cli].size(); i++) {
                        in_pipe_num = in_pipe_num_v[cli][i];
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[1]);
                        close(used_user_pipe[in_pipe_num][cli].user_pipe[0]);
                    }

                    if (!Node->parent && out_pipe_num > 0) {
                        close(used_user_pipe[cli][out_pipe_num].user_pipe[0]);
                        dup2(used_user_pipe[cli][out_pipe_num].user_pipe[1], 1);
                        close(used_user_pipe[cli][out_pipe_num].user_pipe[1]);
                    }
                }
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
    // exec_cmds(&(Node->ops[op_idx].cmds[0]), UNKNOWN);
    if (op->cnt == 1){
        exec_cmds(&(Node->ops[op_idx].cmds[0]), UNKNOWN);
    }
    else{
        ordinary_pipe(&(Node->ops[op_idx]), cli, in_pipe_num, out_pipe_num);
    }
}

void freeKnowNode(know_node *knowNode) {
    int i;
    for (i = 0; i < knowNode->cnt; i++) {
        free(knowNode->record[i]);
    }
}

void addInfo2ExistNode(number_pipe_t *Node, ordinary_pipe_t *current_op,
                       int op_len, int *ith, int idx) {
    ordinary_pipe_t *localOP;
    cmd *localCmd, *currentCmd;
    int op_idx, i, j;

    op_idx = Node->op_cnt;
    localOP = &(Node->ops[op_idx]);
    for (i = 0; i < op_len; i++) {
        localCmd = &(localOP->cmds[i]);
        currentCmd = &(current_op->cmds[i]);
        for (j = 0; j < currentCmd->cnt; j++) {
            strcpy(localCmd->terms[j], currentCmd->terms[j]);
            localCmd->cnt++;
        }
        localOP->cnt++;
    }
    localOP->in_pipe_num = current_op->in_pipe_num;

    //! Maybe write it better.
    if (current_op->cnt != 1) {
        Node->ttl = tran2number(current_op->cmds[op_len].terms[0]);
    }
    Node->op_cnt++;

    // set ex_flg
    for (i = 0; i < EX_LIMIT; i++) {
        if (ith[i] == idx) {
            localOP->ex_flg = true;
            break;
        }
    }
}

void show_node(know_node *knowNode) {
    int i, j, k, z;
    number_pipe_t *localNode;
    ordinary_pipe_t *localOP;
    cmd *localCmd;

    printf("***********************************************\n");
    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];
        printf("ttl: %d\n", localNode->ttl);
        for (j = 0; j < localNode->op_cnt; j++) {
            localOP = &(localNode->ops[j]);
            for (k = 0; k < localOP->cnt; k++) {
                localCmd = &(localOP->cmds[k]);
                for (z = 0; z < localCmd->cnt; z++) {
                    printf("%s\t", localCmd->terms[z]);
                }
                printf("|\t");
            }
            printf("\t%d", localOP->ex_flg);
            printf("\n");
        }
    }
    printf("***********************************************\n");
}

bool check_valid(ordinary_pipe_t *op, char *illegal_term, bool *ifNumberPipe,
                 int *OP_len, CMD_dict *valid_cmd_dict) {
    int i, j, code;
    bool legal_flg;
    char *valid_cmd[VALID_CMD_NUM] = {VALID_CMD};
    cmd *localCmd;

    for (i = 0; i < op->cnt; i++) {
        legal_flg = false;
        localCmd = &(op->cmds[i]);
        *OP_len = i + 1;
        for (j = 0; j < valid_cmd_dict->cnt; j++) {
            if (!strcmp(localCmd->terms[0], valid_cmd_dict->dict[j])) {
                legal_flg = true;
                break;
            }
            if (isdigit_myself(localCmd->terms[0])) {
                legal_flg = true;
                *OP_len = i;
                *ifNumberPipe = true;
                break;
            }
        }
        for (j = 0; j < VALID_CMD_NUM; j++) {
            if (!strcmp(localCmd->terms[0], valid_cmd[j])) {
                legal_flg = true;
                break;
            }
            // cout << "FFFFFFFFFFFF" << localCmd->terms[0] << endl;
            if (isdigit_myself(localCmd->terms[0]) &&
                tran2number(localCmd->terms[0]) != IMPOSSIBLE) {
                legal_flg = true;
                *OP_len = i;
                *ifNumberPipe = true;
                break;
            }
        }
        if (!legal_flg) {
            strcpy(illegal_term, localCmd->terms[0]);
            return false;
        }
    }
    return true;
}

bool isdigit_myself(char *term) {
    int i;
    bool flg;
    flg = true;
    for (i = 0; i < strlen(term); i++) {
        if (!isdigit(term[i])) {
            flg = false;
        }
    }
    return flg;
}

number_pipe_t *createNode(know_node *knowNode) {
    number_pipe_t *AnewNode;
    int *idx;

    AnewNode = (number_pipe_t*)malloc(sizeof(number_pipe_t));
    memset(AnewNode, 0, sizeof(number_pipe_t));

    idx = &knowNode->cnt;
    knowNode->record[*idx] = AnewNode;
    (*idx)++;

    AnewNode->id = ID;
    ID++;

    return AnewNode;
}

number_pipe_t *getSpecificNode(know_node *knowNode, int ttl) {
    int i;
    for (i = 0; i < knowNode->cnt; i++) {
        if (knowNode->record[i]->ttl == ttl) {
            return knowNode->record[i];
        }
    }
    return NULL;
}

number_pipe_t *getParentNode(know_node *knowNode) {
    int i;
    number_pipe_t *localNode;
    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];

        if (!localNode->parent) {
            return localNode;
        }
    }
}