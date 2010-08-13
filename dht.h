#ifndef _NODE_DHT_H_
#define _NODE_DHT_H_
#pragma once

#include <node.h>
#include <libcage/cage.hpp>
#include <node_events.h>

namespace node_dht {

#define CB_INTERNAL \
  DHT* dht; \
  v8::Persistent<v8::Function> cb;

class DHT : public node::ObjectWrap
{
public:
  static v8::Persistent<v8::FunctionTemplate> constructor_template;

  // callback classes {{{
  class join_func {
  public:
    CB_INTERNAL
    void operator() (bool success);
  };

  class get_func {
  public:
    CB_INTERNAL
    void operator() (bool success, libcage::dht::value_set_ptr data);
  };
  // }}}

  libcage::cage* cage;
  join_func join_fn;
  get_func get_fn;

  // pointer to the data we got back from cage->get
  // we need to inform our javascript friends how many
  // value_t's we got back so they can send that many
  // Buffers our way. As far as I know I can't simply
  // make new Buffers from C++ and send them off right away.
  libcage::dht::value_set_ptr storedBuffers;

  DHT();
  ~DHT();

  v8::Local<v8::String> getNodeId();
  void setNodeIdProperty();

  static v8::Local<v8::Array> 
    bufferLengthsFromValueSet(libcage::dht::value_set_ptr buffers);

  static void Initialize(v8::Handle<v8::Object> target);

protected:

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
