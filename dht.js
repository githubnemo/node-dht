var binding = require('./dht.node');
var DHT = binding.DHT;

var natStr = function(state) {
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

DHT.prototype.recv = function (fn) {

  if (arguments.length != 1)
    throw new Error("Requires at least 1 argument (function)");

  if (typeof fn != "function")
    throw new Error("First argument must be a function");

  this._setDgramCallback(fn);
  return this;
}

DHT.prototype.send = function (id, buffer) {
  if (arguments.length != 2)
    throw new Error("Requires at least 2 arguments");

  if (typeof id != "string")
    throw new Error("First argument must be a string (node id)");

  if (! (buffer instanceof Buffer))
    throw new Error("Second argument must be a buffer");
   
  this._sendDgram(id, buffer);
  return this;
}

DHT.prototype.setGlobal = function () {
  this._setGlobal();
  this.natState = natStr(this._natState());
  return this;
}

DHT.prototype.get = function (key, fn) {
  this._get(key, function (success, lengths) {
    var buffers = [];
    for (var i = 0; i < lengths.length; i++) {
      var buffer = new Buffer(lengths[i]);
      buffers.push(buffer);
    }
    this._fillGetBuffers(buffers);
    fn.call(this, success, buffers);
  });
  return this;
}

exports.createNode = function (port, dtun, inet) {
  var dht = new DHT();
  inet = inet || binding.PF_INET;
  dtun = dtun || true;
  dht.open(inet, port, dtun);
  return dht;
}

