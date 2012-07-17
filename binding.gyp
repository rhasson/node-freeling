{
  'targets': [
    {
      'target_name': 'freeling',
      'type': 'loadable_module',
      'product_extension': 'node',
      'product_prefix': '',
      'include_dirs': ['.','/home/roy/freeling/free3/include', '/home/roy/cvv8/include/cvv8'],
      'conditions': [
         ['OS=="linux"', {
          'link_settings': {
            'ldflags': ['-L/home/roy/freeling/free3/lib', '-lfreeling', '--rpath=/home/roy/freeling/free3/lib']
#            'libraries': ['/home/roy/freeling/free3/lib/libfreeling.so']
            },
         }],
       ],
      'sources': ['freeling.cc', 'freeling_tokenizer.cc', 'freeling_splitter.cc', 'helper.cc'],
    },
  ],
}
