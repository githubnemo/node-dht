node-dht
========

node-dht is a Kademlia-based Distributed Hash Table library for node.js with 
NAT-traversing super powers! node-dht is a set of bindings to the libcage 
library which does the heavy lifting under the hood.

The goal of node-dht is to provide a very flexible, usable, and hackable set of
tools to easily implement DHT services into your application.

node-dht is in an early-alpha stage and is very unstable.

### Quick Start

    var dht = require('dht')
    var port = 10000;

    // create our initial nodes
    var node = dht.createNode(port).setGlobal();
    var node2 = dht.createNode(port + 1).setGlobal();

    var makeBuffer = function (s) { 
      return new Buffer(s.toString(), encoding="ascii"); 
    }

    // node2 -> join -> node
    node2.join("localhost", port, function (success) {

      var key = makeBuffer("myKey");
      var value = makeBuffer(1234);

      // put data into the network
      node.put(key, value, /* ttl */ 30000);

      // get data from the network
      node.get(key, function (success, results) {
        if (success)
          console.log("got data!", results);
      });
    });

### Building

Good luck, the automated build process isn't well fleshed out yet.

## Depedencies

* [libev][http://software.schmorp.de/pkg/libev.html]
* [boost][http://www.boost.org/]
* [libcage][http://github.com/jb55/libcage]

## Step by Step

# boost, libev

1. build and install boost
2. build libev *WITH* -DEV\_MULTIPLICITY=0, install it
3. libcage requires omake, download and install it

# libcage

libcage was originally implemented with libevent. To make it play nice
inside node.js' event loop, we need to link to libev instead. Luckily
libev provides a compatibility header for libevent so all that needed to be
changed was the linker options. You can get this version from the libev
branch on my fork of libcage (link in the dependencies section).

4. git checkout libev
5. sudo omake install

# node-dht

6. make
