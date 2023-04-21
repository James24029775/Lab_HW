void rwg_func(int sockfd, int cli) {
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
        minus_ttl(&knowNode[cli]);
        if (!choose_rwg_cmd(args, cli))
            choose_shell_cmd(args, msg, cli, sockfd);
    }
}

void choose_shell_cmd(vector<string> &arg, string input, int cli, int sockfd){
    int arg_num = arg.size();
    signal(SIGCHLD, SIG_IGN);

    // Environment argument setup
    for(auto iter=env[cli].begin(); iter != env[cli].end(); iter++) {
        setenv(iter->first.c_str(), iter->second.c_str(), 1);
    }

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
                if (user_pipe_num > LISTENQ || client[user_pipe_num] < 0) {
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
                    if (user_pipe_num > LISTENQ || (used_user_pipe[user_pipe_num][cli].user_pipe[0] == -1 && used_user_pipe[user_pipe_num][cli].user_pipe[1] == -1)){
                        in_pipe_exist_flg = false;
                        in_user_pipe_num = -1 * user_pipe_num;
                    }
                    else {
                        in_pipe_exist_flg = true;
                        in_user_pipe_num = user_pipe_num;
                    }
                }
                // Check pipe out
                else if (arg[i][0] == '>') {
                    if (used_user_pipe[cli][user_pipe_num].user_pipe[0] == -1 && used_user_pipe[cli][user_pipe_num].user_pipe[1] == -1) {
                        struct user_pipe tmp_user_pipe;
                        pipe(tmp_user_pipe.user_pipe);
                        used_user_pipe[cli][user_pipe_num] = tmp_user_pipe;
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
    cout << "WAAAAAAAAAIT: " << input << "  " << endl;
    cout << "IN num " << in_user_pipe_num << " OUT num " << out_user_pipe_num << endl;
    cout << "user_pipe_num " << user_pipe_num << " cli " << cli << endl;
    
    bool pipe_in_flg = false, pipe_out_flg = false;
    if (in_user_exist_flg && in_pipe_exist_flg && in_user_pipe_num > 0) pipe_in_flg = true;
    if (out_user_exist_flg && !out_pipe_dup_flg && out_user_pipe_num > 0) pipe_out_flg = true;

    // Make pipe-in msg
    if (!in_user_exist_flg) {
        int err_user_num = -1 * in_user_pipe_num;
        string errmsg = "*** Error: user #" + to_string(err_user_num) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (!in_pipe_exist_flg) {
        int err_user_num = -1 * in_user_pipe_num;
        string errmsg = "*** Error: the pipe #" + to_string(err_user_num) + "->#" + to_string(cli) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (pipe_in_flg) {
        string msg = "*** " + name[cli] + " (#" + to_string(cli) + ") just received from " + name[in_user_pipe_num] + " (#" + to_string(in_user_pipe_num) + ") by \'" + input + "' ***\n";
        broadcast(msg);
    }

    // Make pipe-out msg 
    if (!out_user_exist_flg) {
        int err_user_num = -1 * out_user_pipe_num;
        string errmsg = "*** Error: user #" + to_string(err_user_num) + " does not exist yet. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (out_pipe_dup_flg) {
        int err_user_num = -1 * out_user_pipe_num;
        string errmsg = "*** Error: the pipe #" + to_string(cli) + "->#" + to_string(err_user_num) + " already exists. ***\n";
        write(sockfd, errmsg.c_str(), errmsg.length());
    } else if (pipe_out_flg) {
        string msg = "*** " + name[cli] + " (#" + to_string(cli) + ") just piped '" + input + "' to " + name[out_user_pipe_num] + " (#" + to_string(out_user_pipe_num) + ") ***\n";
        broadcast(msg);
    }

    cout << "**********************************************1" << endl;

    // Shell commands 
    string instruction = "";
    for (int i=0; i<arg_num; i++) {
        instruction = instruction + arg[i] + " ";
    }
    instruction += "\n";
    cout << "**********************************************1.5" << endl;
    shell_function(instruction, sockfd, cli, pipe_in_flg, pipe_out_flg, in_user_pipe_num, out_user_pipe_num);
    
    cout << "**********************************************2" << endl;

    // Reset environment argument
    for(auto iter=env[cli].begin(); iter != env[cli].end(); iter++)
        setenv(iter->first.c_str(), "", 1);

    cout << "**********************************************3" << endl;
    
}

bool choose_rwg_cmd(vector<string> args, int cli) {
    if (args[0] == "who"){
        rwg_who(args, cli);
        return true;
    } else if (args[0] == "tell") {
        rwg_tell(args, cli);
        return true;
    } else if (args[0] == "yell") {
        rwg_yell(args, cli);
        return true;
    } else if (args[0] == "name") {
        rwg_name(args, cli);
        return true;
    } else if (args[0] == "exit") {
        rwg_exit(args, cli);
        return true;
    }
    return false;
}

void rwg_who(vector<string> args, int cli) {
    string msg = WHO_COLUMN;
    for (int i = 0; i < LISTENQ; i++) {
        if (client[i] > 0){
            msg += to_string(i) + '\t' + name[i] + '\t' + ipaddr[i] + ':' + to_string(port[i]);
            if (i == cli){
                msg +=  "\t<-me";
            }
            msg += '\n';
        }
    }
    
    if (write(client[cli], msg.c_str(), msg.size()) == -1) {
        cerr << "Error: Failed to write data to socket.\n";
        exit(EXIT_FAILURE);
    }
}

void rwg_tell(vector<string> args, int cli) {
    int towhom;
    istringstream iss(args[1]);
    iss >> towhom;

    // If rcv doesn't exist
    if (client[towhom] == -1) {
        string errmsg = "*** Error: user #"+ to_string(towhom) +" does not exist yet. ***\n";
        if (write(client[cli], errmsg.c_str(), errmsg.size()) == -1) {
            cerr << "Error: Failed to write data to socket.\n";
            exit(EXIT_FAILURE);
        }
        return;
    }

    // If rcv exists
    string msg = "*** " + name[cli] + " told you ***: ";
    for (int i = 2; i < args.size(); i++) {
        msg += args[i] + " ";
    }
    msg += "\n";

    if (write(client[towhom], msg.c_str(), msg.size()) == -1) {
        cerr << "Error: Failed to write data to socket.\n";
        exit(EXIT_FAILURE);
    }
}

void rwg_yell(vector<string> args, int cli) {
    string msg = "*** "+name[cli]+" yelled ***: ";
    for (int i = 1; i < args.size(); i++) {
        msg += args[i] + " ";
    }
    msg += "\n";

    broadcast(msg);
}

void rwg_name(vector<string> args, int cli) {
    bool dupFlg = false;
    for (int i = 0 ; i < LISTENQ; i++) {
        if (client[i] != -1){
            if (name[i] == args[1]) {
                dupFlg = true;
                break;
            }
        }
    }
    
    string msg;
    if (dupFlg) {
        msg = "*** User \'" + args[1] + "\' already exists. ***\n";
        if (write(client[cli], msg.c_str(), msg.size()) == -1) {
            cerr << "Error: Failed to write data to socket.\n";
            exit(EXIT_FAILURE);
        }
    } else {
        name[cli] = args[1];
        msg = "*** User from " + ipaddr[cli] + ":" + to_string(port[cli]) + " is named \'"+ name[cli] +"\'. ***\n";
        broadcast(msg);
    }
}

void rwg_exit(vector<string> args, int cli) {
    string msg;
    msg = "*** User \'" + name[cli] + "\' left. ***\n";
    broadcast(msg);

    // Clean user pipe
    for (int i = 0 ; i < LISTENQ ; i++){
        used_user_pipe[i][cli].user_pipe[0] = -1;
        used_user_pipe[i][cli].user_pipe[1] = -1;
        used_user_pipe[cli][i].user_pipe[0] = -1;
        used_user_pipe[cli][i].user_pipe[1] = -1;
    }
    in_pipe_num_v[cli].clear();
    for(int j = 0; j < NODE_LIMIT; j++) {
        knowNode[cli].record[j] = NULL;
    }
    knowNode[cli].cnt = 0;
    newNode[cli] = NULL;
    targetNode[cli] = NULL;
    parentNode[cli] = NULL;


    // Reset and close connection
    close(client[cli]);
    FD_CLR(client[cli], &allset);
    client[cli] = -1;
    name[cli] = "(no name)";
    ipaddr[cli] = "";
    port[cli] = 0;
    process_line[cli] = 0;
    env[cli].clear();
    env[cli]["PATH"] = "bin:.";
}

void broadcast(string msg){
    for (int i = 0; i < LISTENQ; i++) {
        if (client[i] != -1){
            if (write(client[i], msg.c_str(), msg.size()) == -1) {
                cerr << "Error: Failed to write data to socket.\n";
                exit(EXIT_FAILURE);
            }
        }
    }
}