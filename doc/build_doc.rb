#################################################################
# This file is part of glyr
# + a commnandline tool and library to download various sort of musicrelated metadata.
# + Copyright (C) [2011-2012]  [Christopher Pahl]
# + Hosted at: https://github.com/sahib/glyr
#
# glyr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# glyr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with glyr. If not, see <http://www.gnu.org/licenses/>.
#################################################################
#!/usr/bin/ruby

# A simple ruby script wrapping around gtk-doc;
# Normally gtk-doc requires autoconf, which I dont want to use.
# It's a pain in the ass.

MODULE="libglyr"

DIR =File.expand_path("../lib")
BUILD_HEADERS = 
[
  "#{DIR}/glyr.h",
  "#{DIR}/types.h",
  "#{DIR}/cache.h",
  "#{DIR}/config.h",
]

DOC_DIR = "#{File.expand_path(Dir.getwd)}/doc"
BLD_DIR = "#{File.expand_path(Dir.getwd)}/build"
INC_DIR = "#{File.expand_path(Dir.getwd)}/include"

if Dir.exists? DOC_DIR
   puts "-- Cleaning up.."
   system("rm -rf DOC_DIR")
   system("rm -rf BLD_DIR")
end


def copy_doc_files
   begin
		Dir.mkdir "build"
	rescue;end
	
	BUILD_HEADERS.each do |file|
		unless system("cp #{file} build/") then
			puts "Unable to copy #{file}!"
			exit -1
		end
	end
end 

def copy_includes
	if Dir.exists? INC_DIR
		Dir.open(INC_DIR).each do |filename|
			inc_filename = File.expand_path("include/%s" % filename)
			if File.file? inc_filename then
				system("cp #{inc_filename} doc/html")
				puts " - copy: #{inc_filename}"
			end
		end
	end
end

def create_dir_and_cd dirname
   begin
		Dir.mkdir dirname
	rescue;end

  	Dir.chdir dirname
end

def do_or_die is_cool
	exit unless system is_cool	
end

############################

copy_doc_files()

puts "-- Processing file #{BLD_DIR}"

create_dir_and_cd "doc"
puts "-- Scanning directory"
do_or_die("gtkdoc-scan --module='#{MODULE}' --source-dir '../build'")
puts " - done"

puts "-- Setting up DB"
do_or_die("gtkdoc-mkdb --module=#{MODULE} --main-sgml-file=#{MODULE}.sgml --source-dir='../build' --xml-mode --output-format=xml")
puts " - done"

create_dir_and_cd "html"

puts "-- Generating HTML"
do_or_die("gtkdoc-mkhtml --path='../../build' libglyr ../#{MODULE}.sgml 2> /dev/null");
puts " - done"

puts "-- Fixing crosslinks"
Dir.chdir("..")
do_or_die("gtkdoc-fixxref --module=#{MODULE} --module-dir=html --html-dir=html")
puts " - done"

Dir.chdir("..")
puts "-- Copying include files."
copy_includes()

if Dir.exists? "../child/glyr/doc"
	puts "-- Copying to repo"
	system("cp -r doc/html ../child/glyr/doc")
end
