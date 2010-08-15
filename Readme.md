node-dht
========

node-dht is a Kademlia-based Distributed Hash Table library for node.js with 
NAT-traversing super powers! node-dht is a set of bindings to the libcage 
library which does the heavy lifting under the hood.

What can you do with it? node-dht can be used as a decentralized peer-to-peer
key/value store with the ability to traverse NATs if needed. More complicated
protocols can be implemented on top of node-dht as well!

The goal of node-dht is to provide a very flexible, usable, and hackable set of
tools to easily implement DHT services into your application.

node-dht is in an early-alpha stage and is very unstable.

## Quick Start

    var dht = require('../dht')
    var port = 10000;

    // create our initial nodes
    var node = dht.createNode(port).setGlobal();
    var node2 = dht.createNode(port + 1).setGlobal();

    // node2 -> join -> node
    node2.join("localhost", port, function (success) {
      console.log("join", success ? "succeed" : "failed");

      node.printState();
      node2.printState();
    });

## Building

Good luck, the automated build process isn't well fleshed out yet.

## Depedencies

* [libev](http://software.schmorp.de/pkg/libev.html)
* [boost](http://www.boost.org/)
* [libcage](http://github.com/jb55/libcage)

## Step by Step

### boost, libev

1. build and install boost
2. build libev *WITH* -DEV\_MULTIPLICITY=0, install it
3. libcage requires omake, download and install it

### libcage

libcage was originally implemented with libevent. To make it play nice
inside node.js' event loop, we need to link to libev instead. Luckily
libev provides a compatibility header for libevent so all that needed to be
changed was the linker options. You can get this version from the libev
branch on my fork of libcage (link in the dependencies section).

4. git checkout libev
5. sudo omake install

If you have linking errors when building libcage there's a good chance 
that it's trying to link to libevent, make sure it's trying to include
libev's event.h instead of libevent's event.h by removing libevent or by
moving the headers around.

### node-dht

6. make
