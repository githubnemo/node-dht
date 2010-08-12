var binding = require('./dht.node');
var DHT = binding.DHT;

DHT.prototype.natState = function() {
  var state = this._natState();
  switch (state) {
    default:
    case binding.NODE_UNDEFINED:
      return "undefined";
    case binding.NODE_NAT:
      return "nat";
    case binding.NODE_CONE:
      return "cone";
    case binding.NODE_SYMMETRIC:
      return "symmetric";
    case binding.NODE_GLOBAL:
      return "global";
  }
}

exports.createNode = function (port) {
  var dht = new DHT();
  dht.open(binding.PF_INET, port, true);
  return dht;
}

