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

var makeKey = function (n) {
  return new Buffer(n.toString(), encoding="ascii");
}

var onJoin = function(result) {
  console.log("Join", result ? "succeeded" : "failed");
  if (!result)
    return;

  n++;
  if (n < max_nodes) {
    var key = makeKey(n);
    var value = new Buffer("ride the dht train! (" + n + ")", 
                           encoding="ascii");

    var node = dht.createNode(port + n)
                  .setGlobal()
                  .join("localhost", port, onJoin)
                  .put(key, value, 30000);

    nodes.push(node);

  } else {
    console.log("Setting timeout...");
    setInterval(function () {
      var chosenNode = Math.floor(Math.random() * max_nodes);
      var chosenKey = Math.floor(Math.random() * max_nodes);
      var key = makeKey(chosenKey);

      console.log("Attempting to get...", chosenKey);

      nodes[chosenNode].get(key, function(success, results) {
        console.log("Get", success ? "succeeded!" : "failed");
      }, 1000);
    });
  }
}

// the bootstrap node first joins the root node. after it
// has joined, another node is created and attempts to join
// the root node, etc recursively
bootstrapNode.join("localhost", port, onJoin);
