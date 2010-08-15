var dht = require('../../dht');

var max_nodes = 100;
var rootPort = 10000;
var port = rootPort;

// start the bootstrap nodes
var rootNode = dht.createNode(port++).setGlobal();
var bootstrapNode = dht.createNode(port++).setGlobal();

var n = 0;
var onJoin = function(result) {
  console.log("Join", result ? "succeeded" : "failed");

  n++;
  if (n < max_nodes) {
    var node = dht.createNode(port++)
                   .setGlobal()
                   .join("localhost", rootPort, onJoin);
    console.log(node);
  } else {
    console.log("Joined", n, "nodes");
  }
}

// the bootstrap node first joins the root node. after it
// has joined, another node is created and attempts to join
// the root node, etc recursively
bootstrapNode.join("localhost", rootPort, onJoin);
