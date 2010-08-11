#ifndef _NODE_DHT_H_
#define _NODE_DHT_H_
#pragma once

#include <node.h>
#include <libcage/cage.hpp>
#include <node_events.h>

namespace node_dht {

void JoinCallback(bool);

class DHT : public node::ObjectWrap
{
  libcage::cage* cage;

public:

  static v8::Persistent<v8::FunctionTemplate> constructor_template;

  DHT();
  ~DHT();

  static void Initialize(v8::Handle<v8::Object> target);


  static v8::Handle<v8::Value> Open(const v8::Arguments& args);
  static v8::Handle<v8::Value> Join(const v8::Arguments& args);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
};
  
} /* namespace node_dht */

#endif /* _NODE_DHT_H_ */
