ONE {
  ifNumberPipe = true;
  minus_ttl(&knowNode);
  targetNode = getSpecificNode(&knowNode, 0);
  if (ifNumberPipe) {
    ttl = tran2number(cmd_1[1][0]);
    if (targetNode) {
      parentNode = getSpecificNode(&knowNode, ttl);
      if (parentNode) {
        addInfo2ExistNode(parentNode, cmd_1, terms_amt_1, cmd_amt_1);
        adoptChild(parentNode, targetNode);
      } else {
        newNode = createNode(&knowNode);
        addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);
        adoptChild(newNode, targetNode);
      }
    } else {
      targetNode = getSpecificNode(&knowNode, ttl);
      if (targetNode) {
        addInfo2ExistNode(targetNode, cmd_1, terms_amt_1, cmd_amt_1);
      } else {
        newNode = createNode(&knowNode);
        addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);
      }
    }
  } else if (targetNode) {
    newNode = createNode(&knowNode);
    addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);
    adoptChild(newNode, targetNode);
    bin_func(cmd_1, cmd_amt_1, terms_amt_1, filename, ifNumberPipe, newNode);
  } else {
  }
  show_node(&knowNode);
}
