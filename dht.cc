
#include "dht.h"
#include "node_util.h"

using namespace v8;
using namespace node;
using namespace node_util;

namespace node_dht {

// Helper functions {{{

inline Handle<Value> ThrowTypeError(const char* err) {
  return ThrowException(Exception::TypeError(String::New(err)));
}
  
inline Handle<Value> ThrowError(const char* err) {
  return ThrowException(Exception::Error(String::New(err)));
}

// }}}


Persistent<FunctionTemplate> DHT::constructor_template;
Persistent<String> join_symbol;

DHT::DHT() {
  cage = new libcage::cage;
}

DHT::~DHT() {
  delete cage;
}

Handle<Value> DHT::Open(const Arguments& args) {
  HandleScope scope;

  if (!(args.Length() <= 2))
    return ThrowTypeError("requires at least 2 arguments");

  if (!args[0]->IsUint32() || 
      (args[0]->ToInteger()->Value() != PF_INET &&
      args[0]->ToInteger()->Value() != PF_INET6))
    return ThrowTypeError(
      "first argument must be either binding.PF_INET or binding.PF_INET6");

  if (!args[1]->IsUint32())
    return ThrowTypeError("second argument must be a valid port");

  int domain = args[0]->ToInteger()->Value();
  int port = args[1]->ToInteger()->Value();
  bool dtun = (args.Length() >= 3 && args[3]->IsBoolean()) ? 
              args[3]->ToBoolean()->Value() : true;

  DHT* dht = UnwrapThis<DHT>(args);

  if (! dht->cage->open(domain, port, dtun)) {
    return ThrowError("Could not open dht connection on specific port");
  }

  return Undefined();
}

Handle<Value> DHT::Join(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 3) 
    return ThrowTypeError("requires 3 arguments (host, port, callback)");

  if (!args[0]->IsString()) 
    return ThrowTypeError("first argument must be a string (hostname)");

  if (!args[1]->IsUint32()) 
    return ThrowTypeError("second argument must be an int (port)");

  if (!args[2]->IsFunction()) 
    return ThrowTypeError("third argument must be a function");

  String::AsciiValue asciiHost(args[0]->ToString());
  std::string host(*asciiHost);
  int port = static_cast<int>(args[1]->ToInteger()->Value());

  DHT* dht = UnwrapThis<DHT>(args);
  dht->cage->join(host, port, JoinCallback);

  return String::New("Success!!");
}

void JoinCallback(bool unk) {
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

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("DHT"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "open", DHT::Open);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "join", DHT::Join);

  target->Set(String::NewSymbol("PF_INET"), Integer::New(PF_INET));
  target->Set(String::NewSymbol("PF_INET6"), Integer::New(PF_INET6));
  target->Set(String::NewSymbol("DHT"), constructor_template->GetFunction());
  return;
}

extern "C" void 
init (Handle<Object> target) {
  HandleScope scope;
  DHT::Initialize(target);
}

} /* namespace node_dht */

