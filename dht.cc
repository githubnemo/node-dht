
#include "dht.h"
#include "node_util.h"

using namespace v8;
using namespace node;
using namespace node_util;

namespace node_dht {

Persistent<String> join_symbol;

DHT::DHT() : EventEmitter() {}

Handle<Value> DHT::Join(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);
  return Integer::New(42);
}

Handle<Value> DHT::New(const Arguments& args) {
  HandleScope scope;
  DHT* dht = new DHT();
  dht->Wrap(args.Holder());
  return args.This();
}

void DHT::Initialize(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(DHT::New);
  t->Inherit(EventEmitter::constructor_template);
  t->InstanceTemplate()->SetInternalFieldCount(1);

  join_symbol = NODE_PSYMBOL("join");
  NODE_SET_PROTOTYPE_METHOD(t, "join", DHT::Join);

  target->Set(String::NewSymbol("DHT"), t->GetFunction());
  return;
}

} /* namespace node_dht */

extern "C" void 
init (Handle<Object> target) {
  HandleScope scope;
  node_dht::DHT::Initialize(target);
}
  
