var dht = require('./dht');

var node = dht.createNode(10000)
              .setGlobal();

console.log("Joining...");
node.join("localhost", 10000, function(result) {
  console.log("Join", result ? "succeeded!" : "failed");
  console.log("Nat state: ", this.natState());
});
