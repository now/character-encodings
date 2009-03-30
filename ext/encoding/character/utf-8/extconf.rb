require 'mkmf'

def try_compiler_option(opt, &block)
  checking_for "#{opt} option to compiler" do
    $CFLAGS += " #{opt}" if try_compile '', opt, &block
  end
end

try_compiler_option '-std=c99'
try_compiler_option '-finline-functions'
try_compiler_option '-fno-common'
try_compiler_option '-Wall'
try_compiler_option '-Waggregate-return'
try_compiler_option '-Wcast-align'
# NOTE: Generates way too many false positives.
# try_compiler_option '-Wconversion'
try_compiler_option '-Wextra'
try_compiler_option '-Wformat=2'
try_compiler_option '-Winit-self'
try_compiler_option '-Winline'
# NOTE: We need bigger ones for TR_TABLE_SIZE.
#try_compiler_option '-Wlarger-than-65500'
try_compiler_option '-Wmissing-declarations'
try_compiler_option '-Wmissing-format-attribute'
try_compiler_option '-Wmissing-include-dirs'
try_compiler_option '-Wmissing-noreturn'
try_compiler_option '-Wmissing-prototypes'
try_compiler_option '-Wnested-externs'
try_compiler_option '-Wold-style-definition'
try_compiler_option '-Wpacked'
try_compiler_option '-Wp,-D_FORTIFY_SOURCE=2'
try_compiler_option '-Wpointer-arith'
# NOTE: ruby.h and intern.h have too many of these.
# try_compiler_option '-Wredundant-decls'
# NOTE: Complains about index, for example.
# try_compiler_option '-Wshadow'
try_compiler_option '-Wsign-compare'
try_compiler_option '-Wstrict-aliasing=2'
# NOTE: ruby.h and intern.h are messed up and for some reason GCC wants to
# complain about them as well.  System headers?
try_compiler_option '-Wswitch-default'
try_compiler_option '-Wswitch-enum'
try_compiler_option '-Wundef'
# NOTE: Incredible amounts of false positives.
#try_compiler_option '-Wunreachable-code'
try_compiler_option '-Wunsafe-loop-optimizations'
try_compiler_option '-Wwrite-strings'

have_header 'assert.h'
have_header 'limits.h'
have_header 'locale.h'
have_header 'stdbool.h'
have_header 'stddef.h'
have_header 'stdint.h'
have_header 'stdio.h'
have_header 'stdlib.h'
have_header 'string.h'
have_header 'sys/types.h'
have_header 'wchar.h'

$INSTALLFILES ||= []
$INSTALLFILES << ['unicode.h', '$(RUBYARCHDIR)', 'lib']

create_makefile 'encoding/character/utf-8/utf8'
