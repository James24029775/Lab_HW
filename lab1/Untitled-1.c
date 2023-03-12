ifNumberPipe = true;
minus_ttl(&knowNode);
targetNode = getSpecificNode(&knowNode, 0);
if (ifNumberPipe) {
    ttl = tran2number(cmd_2[op_len][0]);
    if (targetNode) {
        parentNode = getSpecificNode(&knowNode, ttl);
        if (parentNode) {
            addInfo2ExistNode(parentNode, cmd_2, terms_amt_2, cmd_amt_2, op_len);
            adoptChild(parentNode, targetNode);
        } else {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2, op_len);
            adoptChild(newNode, targetNode);
        }
    } else {
        targetNode = getSpecificNode(&knowNode, ttl);
        if (targetNode) {
            addInfo2ExistNode(targetNode, cmd_2, terms_amt_2, cmd_amt_2, op_len);
        } else {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2, op_len);
        }
    }
} else if (targetNode) {
    newNode = createNode(&knowNode);
    addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2, op_len);
    adoptChild(newNode, targetNode);
    bin_func(cmd_2, cmd_amt_2, terms_amt_2, filename, ifNumberPipe, newNode);
} else {
}