#ifndef _NODE_UTIL_H_
#define _NODE_UTIL_H_
#pragma once

#include <string>
#include <node.h>

namespace node_util {

template <typename T>
inline T* UnwrapThis(const v8::Arguments& args) {
  return node::ObjectWrap::Unwrap<T>(args.This());
}

template <typename T>
inline T* UnwrapObject(v8::Handle<v8::Value> arg) {
  return node::ObjectWrap::Unwrap<T>(arg.As<v8::Object>());
}

inline std::string StringToStdString(v8::Handle<v8::Value> v8str) {
  v8::String::AsciiValue ascii(v8str);
  return std::string(*ascii);
}

template <typename T, typename T2>
inline v8::Local<T> LocalPrimitive(T2 val) {
  return v8::Local<T>::New(T::New(val));
}

template <typename T>
inline v8::Persistent<T> PersistFromLocal(v8::Local<v8::Value> val) {
  return v8::Persistent<T>::New(val.As<T>());
}

} /* namespace node_util */

#endif /* _NODE_UTIL_H_ */
