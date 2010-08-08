#ifndef _NODE_DHT_H_
#define _NODE_DHT_H_
#pragma once

#include <node.h>
#include <node_events.h>

namespace node_dht {
  
class DHT : public node::EventEmitter
{

public:
  DHT();

  static void Initialize(v8::Handle<v8::Object> target);

  static v8::Handle<v8::Value> Join(const v8::Arguments& args);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
};
  
} /* namespace node_dht */

#endif /* _NODE_DHT_H_ */
