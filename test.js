var dht = require('./dht');

var node = dht.createNode(10001)
              .setGlobal();

node.join("localhost", 10000, function() {
  console.log("Joined!");
  console.log("Nat state: ", this.natState());
});
