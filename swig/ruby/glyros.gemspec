# gem build glyros.gemspec
# sudo gem install glyros-*.gem 
Gem::Specification.new do |s|
  s.name = %q{glyros}
  s.version = "0.8.5"
  s.date = %q{201-08-29}
  s.authors = ["Christopher Pahl"]
  s.email = %q{sahib@online.de}
  s.summary = %q{Glyros is the ruby wrapper of libglyr, a library implementing a music metadata searchengine.}
  s.homepage = %q{https://github.com/sahib/glyr}
  s.description = %q{Glyros is the ruby wrapper of libglyr, a library implementing a music metadata searchengine.}
  s.files = ["glyros.so", "glyros.rb"]
end
