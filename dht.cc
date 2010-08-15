
#include "dht.h"
#include <ev.h>
#include <node_buffer.h>
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

Local<String> IdToString(uint8_t* id) {
  HandleScope scope;
  char buf[CAGE_ID_LEN+1] = {0};
  sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
               "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               id[ 0], id[ 1], id[ 2], id[ 3], id[ 4],
               id[ 5], id[ 6], id[ 7], id[ 8], id[ 9],
               id[10], id[11], id[12], id[13], id[14],
               id[15], id[16], id[17], id[18], id[19]);
  return String::New(buf);
}

void StringToId(Handle<String> str, uint8_t* id) {
  String::AsciiValue value(str);
  uint8_t* data = (uint8_t*)*value;
  uint8_t lshifted = '\0';
  for (int i = 0; i < (CAGE_ID_LEN * 2); i++) {
    uint8_t val = data[i];

    if (val >= '0' && val <= '9') val = val - '0';
    else if (val >= 'A' && val <= 'Z') val = (val - 'A') + 0xA;
    else if (val >= 'a' && val <= 'z') val = (val - 'a') + 0xa;
    else assert("invalid id string" && 0);

    if ((i % 2) == 0)
      lshifted = (val << 4) & 0xF0;
    else
      id[(i-1)/2] = lshifted | val;
  }
}

// }}}


Persistent<FunctionTemplate> DHT::constructor_template;
Persistent<String> id_symbol;


DHT::DHT() {
  cage = new libcage::cage;
}

DHT::~DHT() {
  delete cage;
}



Local<String> DHT::getNodeId() {
  return String::New(cage->get_id_str().c_str());
}



Handle<Value> DHT::SetGlobal(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);
  dht->cage->set_global();
  return args.This();;
}



Handle<Value> DHT::GetNatState(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  Local<Value> nat_state = Integer::New(dht->cage->get_nat_state());
  return nat_state;
}



Handle<Value> DHT::Open(const Arguments& args) {
  HandleScope scope;

  if (!(args.Length() != 2 || args.Length() != 3))
    return ThrowTypeError("Requires at least 2 arguments");

  if (!args[0]->IsUint32() || 
      (args[0]->ToInteger()->Value() != PF_INET &&
      args[0]->ToInteger()->Value() != PF_INET6))
    return ThrowTypeError(
      "First argument must be either binding.PF_INET or binding.PF_INET6");

  if (!args[1]->IsUint32())
    return ThrowTypeError("Second argument must be a valid port");

  int domain = args[0]->ToInteger()->Value();
  int port = args[1]->ToInteger()->Value();
  bool dtun = (args.Length() >= 3 && args[3]->IsBoolean()) ? 
              args[3]->ToBoolean()->Value() : true;

  DHT* dht = UnwrapThis<DHT>(args);

  if (! dht->cage->open(domain, port, dtun)) {
    return ThrowError("Could not open dht connection on specific port");
  } else {
    dht->setNodeIdProperty();
  }

  return Undefined();
}



Handle<Value> DHT::Join(const Arguments& args) {
  HandleScope scope;
  bool hasCallback = false;

  if (args.Length() != 3) 
    return ThrowTypeError(
      "Requires at least 2 arguments (host, port, [function])");

  if (!args[0]->IsString()) 
    return ThrowTypeError("First argument must be a string (hostname)");

  if (!args[1]->IsUint32()) 
    return ThrowTypeError("Second argument must be an int (port)");

  if (args.Length() > 2 && !args[2]->IsFunction()) 
    return ThrowTypeError("Third argument must be a function");
  else
    hasCallback = false;

  DHT* dht = UnwrapThis<DHT>(args);

  int port = args[1]->ToInteger()->Value();
  std::string host = StringToStdString(args[0]);

  Persistent<Function> cb = PersistFromLocal<Function>(args[2]);

  dht->join_fn.dht = dht;
  dht->join_fn.cb = cb;

  dht->Ref();

  // -> into libcage ->
  dht->cage->join(host, port, dht->join_fn);

  return args.This();
}


void DHT::setNodeIdProperty() {
  handle_->Set(id_symbol, getNodeId());
}


// Put data into the network (key, value, 
Handle<Value> DHT::Put(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  if (!(args.Length() == 3 || args.Length() == 4))
    return ThrowTypeError(
      "Requires at least 3 arguments (Buffer, Buffer, int, [bool])");
  
  if (!Buffer::HasInstance(args[0]))
    return ThrowTypeError("First argument must be a Buffer (key)");

  if (!Buffer::HasInstance(args[1]))
    return ThrowTypeError("Seconds argument must be a Buffer (value)");

  if (!args[2]->IsUint32())
    return ThrowTypeError("Third argument must be an int (ttl)");
  
  bool hasOptional = args.Length() > 3;
  bool optCorrectType = hasOptional && args[3]->IsBoolean();

  if (hasOptional && !optCorrectType)
    return ThrowTypeError("Fourth argument must be a bool (isUnique)");

  Buffer * key = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
  Buffer * value = ObjectWrap::Unwrap<Buffer>(args[1]->ToObject());
  uint32_t ttl = args[2]->ToInteger()->Value();

  bool hasUnique = hasOptional && optCorrectType;
  bool isUnique = hasUnique ? args[3]->ToBoolean()->Value() : false;

  dht->cage->put(key->data(), key->length(), 
                 value->data(), value->length(), ttl, isUnique);

  return args.This();
}



Handle<Value> DHT::Get(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  if (args.Length() != 2)
    return ThrowTypeError("Requires 2 arguments (Buffer, function)");
  
  if (!Buffer::HasInstance(args[0]))
    return ThrowTypeError("First argument must be a Buffer");

  if (!args[1]->IsFunction())
    return ThrowTypeError("Second argument must be a function");

  Buffer * buffer = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
  Persistent<Function> cb = PersistFromLocal<Function>(args[1]);

  dht->get_fn.dht = dht;
  dht->get_fn.cb = cb;

  // -> into libcage ->
  dht->Ref();
  dht->cage->get(buffer->data(), buffer->length(), dht->get_fn);

  return args.This();
}



// TODO: Remove this, build buffers in c++
Handle<Value> DHT::FillGetBuffers(const Arguments& args) {
  HandleScope scope;

  DHT * dht = UnwrapThis<DHT>(args);

  Local<Array> ar = args[0].As<Array>();
  int n = ar->Length();
  
  // Unwrap each Buffer contained in the array and fill in data
  // from the ready and willing storedBuffers.
  if (n) {
    libcage::dht::value_set::iterator it = dht->storedBuffers->begin();
    for (int i = 0; i < n && it != dht->storedBuffers->end(); ++it, i++) {
      Buffer * buf = ObjectWrap::Unwrap<Buffer>(ar->Get(i).As<Object>());
      memcpy(buf->data(), it->value.get(), buf->length());
    }
  }

  dht->storedBuffers.reset();

  return args.This();
}



Handle<Value> DHT::PrintState(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  dht->cage->print_state();

  return args.This();
}



// TODO: Should look into using EventEmitter instead of this
Handle<Value> DHT::SetDgramCallback(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  assert(args[0]->IsFunction());

  Persistent<Function> cb = PersistFromLocal<Function>(args[0]);
  dgram_func& f = dht->dgram_fn;

  // Dispose any previous persistent callback handles
  // This is safe, Dispose checks if it's empty and does nothing if it is
  f.cb.Dispose();

  f.dht = dht;
  f.cb = cb;

  // -> into libcage ->
  dht->cage->set_dgram_callback(f);

  return args.This();
}



Handle<Value> DHT::SendDgram(const Arguments& args) {
  HandleScope scope;
  DHT* dht = UnwrapThis<DHT>(args);

  assert(args[0]->IsString());
  assert(Buffer::HasInstance(args[1]));

  Buffer * data = UnwrapObject<Buffer>(args[1]);

  uint8_t id[CAGE_ID_LEN];
  StringToId(args[0].As<String>(), id);

  dht->cage->send_dgram(data->data(), data->length(), id);

  return args.This();
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
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_fillGetBuffers", 
                            DHT::FillGetBuffers);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_setDgramCallback", 
                            DHT::SetDgramCallback);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_sendDgram", 
                            DHT::SendDgram);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_get", DHT::Get);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "put", DHT::Put);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "join", DHT::Join);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_setGlobal", DHT::SetGlobal);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "_natState", 
                            DHT::GetNatState);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "printState", 
                            DHT::PrintState);

  #define INT_SYMBOL(x, v) do { \
    target->Set(String::NewSymbol(x), Integer::New(v)); } while (0)

  id_symbol = NODE_PSYMBOL("id");

  target->Set(id_symbol, String::Empty());

  INT_SYMBOL("NODE_UNDEFINED", libcage::node_undefined);
  INT_SYMBOL("NODE_NAT", libcage::node_nat);
  INT_SYMBOL("NODE_CONE", libcage::node_cone);
  INT_SYMBOL("NODE_SYMMETRIC", libcage::node_symmetric);
  INT_SYMBOL("NODE_GLOBAL", libcage::node_global);

  target->Set(String::NewSymbol("PF_INET"), Integer::New(PF_INET));
  target->Set(String::NewSymbol("PF_INET6"), Integer::New(PF_INET6));
  target->Set(String::NewSymbol("DHT"), constructor_template->GetFunction());
}


// <- out of libcage <-
void DHT::dgram_func::operator() (void* buf, size_t len, uint8_t* id) {
  HandleScope scope;

  Buffer * buffer = Buffer::New(len);

  memcpy(buffer->data(), buf, len);
  
  Handle<Value> argv[2];
  argv[0] = buffer->handle_;
  argv[1] = IdToString(id);

  cb->Call(dht->handle_, 2, argv);
}


// <- out of libcage <-
void DHT::get_func::operator() (bool success, 
                                libcage::dht::value_set_ptr buffers) {
  HandleScope scope;

  dht->storedBuffers = buffers;

  Local<Value> argv[2];
  argv[0] = LocalPrimitive<Boolean>(success);

  // Get an array of integers containing the lengths of each buffer 
  Local<Array> ar = success ? Array::New(buffers->size()) : Array::New();
  if (success) {
    libcage::dht::value_set::iterator it;

    int i = 0;
    for (it = buffers->begin(); it != buffers->end(); ++it, i++) {
      ar->Set(i, Integer::New(it->len));
    }
  }

  argv[1] = ar;

  cb->Call(dht->handle_, 2, argv);
  
  cb.Dispose();
  dht->Unref();
}



// <- out of libcage <-
void DHT::join_func::operator() (bool success) {
  HandleScope scope;

  Local<Value> argv[1];
  argv[0] = LocalPrimitive<Boolean>(success);

  cb->Call(dht->handle_, 1, argv);

  cb.Dispose();
  dht->Unref();
}


extern "C" void 
init (Handle<Object> target) {
  HandleScope scope;
  DHT::Initialize(target);
}

} /* namespace node_dht */

