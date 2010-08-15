var dht = require('../dht');

var max_nodes = 20;
var port = 10000;
var n = 1;
var nodes = [];

// start the bootstrap nodes
var rootNode = dht.createNode(port).setGlobal();
var bootstrapNode = dht.createNode(port+n).setGlobal();

nodes.push(rootNode);
nodes.push(bootstrapNode);

rootNode.n = 0;
bootstrapNode.n = 1;

var makeBuffer = function (n) {
  return new Buffer(n.toString(), encoding="ascii");
}

var onJoin = function(result) {
  var node = this;

  console.log("join", result ? "succeeded" : "failed");
  n++;

  if (n < max_nodes) {

    var newNode = dht.createNode(port + n);
    newNode.n = n;
    nodes.push(newNode);
    newNode.join("localhost", port, onJoin);

  } else {

    for (var i = 0; i < max_nodes; i++) {
      var key = makeBuffer(i);
      nodes[0].put(key, key, 30000);
    }

    setInterval(function () {
      var chosenNode = Math.floor(Math.random() * max_nodes);
      var chosenKey = Math.floor(Math.random() * max_nodes);
      var key = makeBuffer(chosenKey);

      nodes[chosenNode].get(key, function(success, results) {
        var node = this;
        console.log("[node", node.n + "]", "get '" + key + "'", 
          success ? "succeeded!" : "failed",
          success ? "data: '" + results + "'" : "");
      });
    }, 1000);
  }
}

// the bootstrap node first joins the root node. after it
// has joined, another node is created and attempts to join
// the root node, etc recursively
bootstrapNode.join("localhost", port, onJoin);
