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

  class join_func {
  public:
    DHT* dht;
    v8::Persistent<v8::Function> cb;

    void operator() (bool success);
  };

  libcage::cage* cage;
  join_func join_fn;

  DHT();
  ~DHT();

  v8::Local<v8::String> getNodeId();
  void setNodeIdProperty();

  static void Initialize(v8::Handle<v8::Object> target);

protected:

  static v8::Handle<v8::Value> Open(const v8::Arguments& args);
  static v8::Handle<v8::Value> SetGlobal(const v8::Arguments& args);
  static v8::Handle<v8::Value> GetNatState(const v8::Arguments& args);
  static v8::Handle<v8::Value> Join(const v8::Arguments& args);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);

private:
  

};
  
} /* namespace node_dht */

#endif /* _NODE_DHT_H_ */
