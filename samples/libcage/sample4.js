
var dht = require('../../dht');

var max_nodes = 20;
var port = 10000;
var n = 1;
var nodes = [];

// start the bootstrap nodes
var globalNode1 = dht.createNode(port).setGlobal();
var globalNode2 = dht.createNode(port+n).setGlobal();

nodes.push(globalNode1);
nodes.push(globalNode2);

globalNode1.n = 0;
globalNode2.n = 1;

var makeBuffer = function (n) {
  return new Buffer(n.toString(), "ascii");
}


var onJoin = function(result) {
  var node = this;
  console.log("join", result ? "succeeded" : "failed");

  n++;

  if (n < max_nodes) {

    var newNode = dht.createNode(port + n);
    newNode.n = n;
    nodes.push(newNode);

    newNode.join("localhost", port, function (result) {
      // set up a receive function for our new node
      newNode.recv(function (data, from) {
        console.log(this.id, "received '" + data + "' from", from);
      });
      onJoin(arguments);
    });

  } else {
    var j = 0;
    setInterval(function () {
      var sendNode = nodes[Math.floor(Math.random() * max_nodes)];
      var recvNode = nodes[Math.floor(Math.random() * max_nodes)];
      var data = makeBuffer("Hello, world");
      sendNode.send(recvNode.id, data);
    }, 1000);
  }
}

globalNode2.join("localhost", port, onJoin);
