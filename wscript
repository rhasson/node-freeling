srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  conf.env.append_unique('CXXFLAGS', ['-Wall', '-O3'])
  conf.env['LIB_FREELING'] = 'freeling'

def build(bld):
#  bld.env.append_value('LINKFLAGS', '-L/home/roy/freeling/free3/lib')
#  bld.env.append_value('LINKFLAGS', '-lfreeling')
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
#  obj.add_obj_file("/home/roy/freeling/free3/lib/libfreeling-3.0-alfa1.so")
  obj.cxxflags = ["-I/home/roy/freeling/free3/include", "-L/home/roy/freeling/free3/lib", "-lfreeling", "-g", "-Wall"]
  obj.source = ['freeling.cc', 'freeling_tokenizer.cc']
  obj.target = 'freeling'
