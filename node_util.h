#ifndef _NODE_UTIL_H_
#define _NODE_UTIL_H_
#pragma once

#include <node.h>

namespace node_util {

template <typename T>
T* UnwrapThis(const v8::Arguments& args) {
  return node::ObjectWrap::Unwrap<T>(args.This());
}

template <typename T>
T* UnwrapThis(const v8::AccessorInfo& args) {
  return node::ObjectWrap::Unwrap<T>(args.This());
}

} /* namespace node_util */

#endif /* _NODE_UTIL_H_ */
