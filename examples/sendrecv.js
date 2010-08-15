var dht = require('../dht');
var port = 10000;

var global = dht.createNode(port, false).setGlobal();

var bobNode = dht.createNode(port + 1, /* dtun */ false);
var aliceNode = dht.createNode(port + 2, /* dtun */ false);
var chuckNode = dht.createNode(port + 3, /* dtun */ false);

var mb = function (s) {
  return new Buffer(s.toString(), "ascii");
}

// Bob's away from the computer message handler
bobNode.recv(function (data, from) {
  console.log("(Bob) Bob received:", data);
  var msg = mb("Hey I'm not here right now, so go away");
  bobNode.send(from, msg);
});

// Bob joins the global node
bobNode.join("localhost", port, function (ok) {

  // Alice joins the global node after bob
  aliceNode.join("localhost", port, function (ok) {

    // Alice eagerly awaits messages from anyone!
    aliceNode.recv(function (data, from) {
      if (from == bobNode.id)
        console.log("(Alice) Bob sends a message:", data);
      else
        console.log("(Alice) A message from a stranger:", data);
    });

    // Send a message to bob!
    aliceNode.send(bobNode.id, mb("Hi Bob!"))
  });
});

var creepy = [
  "Where are you alice?",
  "What are you wearing alice?",
  "I'm wearing your dress alice",
  "Alice?",
  "Alice!!??!?!?!?",
  "Stop ignoring me."
];

chuckNode.join("localhost", port, function (ok) {
  if (!ok) return;

  setInterval(function () {
    var ind = Math.floor(Math.random() * creepy.length);
    chuckNode.send(aliceNode.id, mb(creepy[ind]));
  }, 1000);
});
