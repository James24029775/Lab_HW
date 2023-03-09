
{
  ifNumberPipe = false;
  minus_ttl(&knowNode);
  targetNode = getSpecificNode(&knowNode, 0);
  if (ifNumberPipe) {
    ttl = tran2number(cmd_6[1][0]);
    // If targetNode isn't Null, it means there is a node with ttl == 0.
    // And the node should point to a parent node (maybe has not created).
    // Or not store the cmd into a normal node (maybe has not created).
    if (targetNode) {
      parentNode = getSpecificNode(&knowNode, ttl);
      // If parentNode isn't Null, then it exist.
      if (parentNode) {
        addInfo2ExistNode(parentNode, cmd_6, terms_amt_6, cmd_amt_6);
        adoptChild(parentNode, targetNode);
      } else {
        newNode = createNode(&knowNode);
        addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
        adoptChild(newNode, targetNode);
      }

    } else {
      targetNode = getSpecificNode(&knowNode, ttl);
      // If targetNode isn't Null, then it exist.
      if (targetNode) {
        addInfo2ExistNode(targetNode, cmd_6, terms_amt_6, cmd_amt_6);
      } else {
        newNode = createNode(&knowNode);
        addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
      }
    }
  } else if (targetNode) {
    newNode = createNode(&knowNode);
    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
    adoptChild(newNode, targetNode);

    bin_func(cmd_6, cmd_amt_6, terms_amt_6, filename, ifNumberPipe, newNode);
  } else {
  }
  // show_node(&knowNode);
}