var binding = require('./dht.node');

var DHT = binding.DHT;

var dht = new DHT();

dht.open(binding.PF_INET, 10000);

var ret = dht.join("localhost", 10000, function() {
  console.log("join seemed to work");
});

console.log(ret);
