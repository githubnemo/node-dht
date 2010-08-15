var dht = require('../dht')
var port = 10000;

// create our initial nodes
var node1 = dht.createNode(port).setGlobal();
var node2 = dht.createNode(port + 1).setGlobal();

var makeBuffer = function (s) { 
  return new Buffer(s.toString(), encoding="ascii"); 
}

// node2 -> join -> node
node2.join("localhost", port, function (success) {
  var node = this;
  console.log(node, "join", success ? "succeed" : "failed");

  var key = makeBuffer("myKey");
  var value = makeBuffer(1234);

  // put data into the network
  node.put(key, value, /* ttl */ 30000);

  // get data from the network
  node.get(key, function (success, results) {
    console.log("get", success ? "succeeded" : "failed");
  });
});

