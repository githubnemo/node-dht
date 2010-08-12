var dht = require('../dht');

var max_nodes = 100;
var rootPort = 10000;
var port = rootPort;

// start the bootstrap node

var rootNode = dht.createNode(port++).setGlobal();
var bootstrapNode = dht.createNode(port++).setGlobal();

var n = 0;
var onJoin = function(result) {
  console.log("Join", result ? "succeeded" : "failed");

  n++;
  if (n < max_nodes) {
    dht.createNode(port++)
       .setGlobal()
       .join("localhost", rootPort, onJoin);
  }
}

bootstrapNode.join("localhost", rootPort, onJoin);
