import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "dht"
  obj.source = "dht.cc"
  obj.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", 
    "-DEV_MULTIPLICITY=0"]
  obj.lib = "cage"

def shutdown():
  if Options.commands['clean']:
    if exists('dht.node'): unlink('dht.node')
  else:
    if exists('build/default/dht.node') and not exists('dht.node'):
      symlink('build/default/dht.node', 'dht.node')

