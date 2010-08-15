var dht = require('../dht');

var max_nodes = 100;
var port = 10000;
var nodes = [];

var rootNode = dht.createNode(port).setGlobal();
var rootNode2 = dht.createNode(port+1).setGlobal();

var makeBuffer = function (n) {
  return new Buffer(n.toString(), encoding="ascii");
}

rootNode2.join("localhost", port, function(ok){

  if (!ok) return;
  var n = 0;

  for (var i = 0; i < max_nodes; i++) {
    // Create a new node
    var node = dht.createNode(port + 2 + i)

    // Join the root node on port 10000
    node.join("localhost", port, function(result) {
      n++;
      var node = this; 
      node.n = n;

      console.log("node", node.n, result ? "joined" : "failed to join");

      // Bail out if we couldn't join the network
      // Alternately we could set up a callback and try
      // again at exponential backoff, or something similar
      if (!result)
        return;

      var key = makeBuffer(n);
      var value = makeBuffer("ride the dht train! (" + n + ")");

      // Put stuff into the network after about 2 seconds
      setTimeout(function() {
        console.log("node", node.n, 
          "sending data into the network: '" + value + "'");
        node.put(key, value, 30000);
      }, 2000);

      // Have the node periodically get stuff from the network
      setInterval(function() {
        var rndKey = node.n;
        var key = makeBuffer(rndKey);

        node.get(key, function (success, buffers) {
          console.log("key lookup from node", node.n,
            success ? "succeded" : "failed", "for key", key,
            success ? "data: '" + buffers + "'" : "");
        });

      }, 15000);
    });

  }
});


