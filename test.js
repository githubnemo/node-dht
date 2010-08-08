var binding = require('./dht');

var DHT = binding.DHT;

var dht = new DHT();
var ret = dht.join();

console.log(ret);
