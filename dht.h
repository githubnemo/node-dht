#ifndef _NODE_DHT_H_
#define _NODE_DHT_H_
#pragma once

#include <node.h>
#include <libcage/cage.hpp>
#include <node_events.h>

namespace node_dht {

class DHT : public node::ObjectWrap
{
public:
  static v8::Persistent<v8::FunctionTemplate> constructor_template;

  // callback classes {{{

  class dht_callback {
  public:
    DHT* dht;
    v8::Persistent<v8::Function> cb;
  };

  class dgram_func : public dht_callback {
  public:
    void operator() (void* buf, size_t len, uint8_t* addr);
  };

  class join_func : public dht_callback {
  public:
    void operator() (bool success);
  };

  class get_func : public dht_callback {
  public:
    void operator() (bool success, libcage::dht::value_set_ptr data);
  };

  // }}}

  libcage::cage* cage;
  dgram_func dgram_fn;
  join_func join_fn;
  get_func get_fn;

  libcage::dht::value_set_ptr storedBuffers;

  DHT();
  ~DHT();

  v8::Local<v8::String> getNodeId();
  void setNodeIdProperty();

  static void Initialize(v8::Handle<v8::Object> target);

protected:

  static v8::Handle<v8::Value> SendDgram(const v8::Arguments& args);
  static v8::Handle<v8::Value> SetDgramCallback(const v8::Arguments& args);
  static v8::Handle<v8::Value> PrintState(const v8::Arguments& args);
  static v8::Handle<v8::Value> FillGetBuffers(const v8::Arguments& args);
  static v8::Handle<v8::Value> Put(const v8::Arguments& args);
  static v8::Handle<v8::Value> Get(const v8::Arguments& args);
  static v8::Handle<v8::Value> Open(const v8::Arguments& args);
  static v8::Handle<v8::Value> SetGlobal(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetNatState(const v8::Arguments& args);
  static v8::Handle<v8::Value> Join(const v8::Arguments& args);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);

private:

  
};

} /* namespace node_dht */

#endif /* _NODE_DHT_H_ */
