
.dht.configured: wscript
	node-waf configure && touch .dht.configured

dht.node: dht.cc node_util.cc dht.h node_util.h .dht.configured
	node-waf

clean:
	node-waf clean
