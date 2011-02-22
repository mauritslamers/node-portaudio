import Options, Utils
from os import unlink, symlink, popen
from os.path import exists 

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')
  opt.add_option('--pacpp', action='store', default='/Users/maurits/Development/node/pa/portaudio/bindings/cpp/include', help='Path to pacpp, e.g. /usr/portaudio')
  opt.add_option('--pa', action='store', default='portaudio', help='path to portaudio')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  
  #conf.env.append_unique('CXXFLAGS', Utils.cmd_output('--include ' + Options.options.pacpp))
  conf.env.append_unique('CXXFLAGS', '-I ' + Options.options.pacpp)
  #conf.env.append_unique('CXXFLAGS', Utils.cmd_output('--include ' + Options.options.pa))
  conf.env.append_unique('CXXFLAGS', '-I ' + Options.options.pa)
  #conf.env.append_unique('LINKFLAGS', Utils.cmd_output(Options.options.pacpp + ' --libs_r').split())
  conf.env.append_unique('LINKFLAGS', '-L' + Options.options.pacpp)
  conf.env.append_unique('LINKFLAGS', '-L' + Options.options.pa)
  #conf.env.append_unique('LINKFLAGS', Utils.cmd_output(Options.options.pa + ' --libs_r').split())
  
  if not conf.check_cxx(header_name='portaudio.h'):
    conf.fatal("Missing portaudio.h header from portaudio")
    
  #if not conf.check_cxx(header_name='portaudiocpp/PortAudioCpp.hxx'):
  #  conf.fatal("Missing PortAudioCpp.hxx header from portaudio")

  
def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'binding'
  obj.source = "binding.cc"
  #obj.uselib = "PA"

