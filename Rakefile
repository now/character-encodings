# contents: Rakefile for the unicode library.
#
# Copyright © 2006 Nikolai Weibull <now@bitwi.se>

require 'rake'
require 'rake/rdoctask'
require 'rake/gempackagetask'
require 'spec/rake/spectask'

PackageName = 'character-encodings'
PackageVersion = '0.1.0'

desc 'Default task'
task :default => [:extensions]

ExtensionDirectory = "ext/encoding/character"

extensions = [
  'utf-8'
].map{ |extension| File.join(ExtensionDirectory, extension) }

Make = 'make'
Makefile = 'Makefile'
ExtConf = 'extconf.rb'
Depend = 'depend'
TAGS = 'TAGS'

desc 'Build all C-based extensions'
task :extensions

extensions.each do |extension|
  makefile = File.join(extension, Makefile)
  so = File.join(extension, File.basename(extension).delete('-') + '.' + Config::CONFIG['DLEXT'])
  tags = File.join(extension, TAGS)

  task :extensions => [makefile, so, tags]

  sources = IO.read(makefile).grep(/^\s*SRCS/).first.sub(/^\s*SRCS\s*=\s*/, "").split(' ')

  file makefile => [ExtConf, Depend].map{ |tail| File.join(extension, tail) } do
    Dir.chdir(extension) do
      ruby ExtConf
      File.open(Makefile, 'a') do |f|
        f.puts <<EOF
#{`#{Config.expand("$(CC) -c -I#{extension} -I#{Config::CONFIG['archdir']} -M -MT TAGS #{sources.join(' ')}")}`}

TAGS:
	@echo Running ‘ctags’ on source files…
	@exuberant-ctags -f $@ -I UNUSED,HIDDEN $^

tags: TAGS

all: tags

.PHONY: tags
EOF
      end
    end
  end

  extension_sources = sources.map{ |source| File.join(extension, source) }

  file so => extension_sources do
    sh %{#{Make} -C #{extension}}
    # TODO: Perhaps copying the ‘so’ to “lib” could save us some trouble with
    # how libraries are loaded.
  end

  file tags => extension_sources do
    sh %{#{Make} -C #{extension} tags}
  end
end

desc 'Extract embedded documentation and build HTML documentation'
task :doc => [:rdoc]
task :rdoc => FileList['**/*.c', '**/*.rb']

desc 'Clean up by removing all generated files, e.g., documentation'
task :clean => [:clobber_rdoc]

Spec::Rake::SpecTask.new do |t|
  t.warning = true
  t.libs = ['lib', 'ext']
  t.spec_files = FileList['specifications/*.rb']
end

RDocDir = 'api'

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_dir = RDocDir
  rdoc.title = 'Unicode'
  rdoc.options = ['--charset UTF-8']
  rdoc.rdoc_files.include('**/*.c')
  rdoc.rdoc_files.include('**/*.rb')
end

PackageFiles = %w(README Rakefile) +
  Dir.glob("{lib,specification}/**/*") +
  Dir.glob("ext/**/{*.{c,h,rb},depend}")

spec =
  Gem::Specification.new do |s|
    s.name = PackageName
    s.version = PackageVersion
    s.platform = Gem::Platform::RUBY
    s.has_rdoc = false
    s.extra_rdoc_files = []
    s.summary = 'A pluggable character-encoding library'
    s.description = s.summary
    s.author = 'Nikolai Weibull'
    s.email = 'now@bitwi.se'
    s.homepage = 'http://git.bitwi.se/?p=ruby-character-encodings.git;a=summary'
    s.files = PackageFiles
    s.require_path = "lib"
    s.extensions = FileList["ext/**/extconf.rb"].to_a
  end

Rake::GemPackageTask.new(spec) do |p|
  p.need_tar_gz = true
  p.gem_spec = spec
end

desc 'Install the gem for this project'
task :install => [:package] do
  sh %{gem install pkg/#{PackageName}-#{PackageVersion}}
end

desc 'Uninstal the gem for this package'
task :uninstall => [] do
  sh %{gem uninstall #{PackageName}}
end
