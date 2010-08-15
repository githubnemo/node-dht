var dht = require('../dht')
var port = 10000;

// create our initial nodes
var node = dht.createNode(port).setGlobal();
var node2 = dht.createNode(port + 1).setGlobal();
node2.join("localhost", port, function () {});

// node2 -> join -> node
node2.join("localhost", port, function (success) {
  console.log("join", success ? "succeed" : "failed");

  node.printState();
  node2.printState();
});
