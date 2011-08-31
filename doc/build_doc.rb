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
]

DOC_DIR ="#{Dir.getwd}/doc"
if Dir.exists? DOC_DIR
	puts "-- Cleaning up.."
   system("rm -rf DOC_DIR")
end

puts "-- Processing file #{DIR}"

def build_ignore_files
	ignored = ""
	`find #{DIR} -name '*\.h'`.each_line do |line|
		line.chomp!
		unless BUILD_HEADERS.include? line
			ignored << ',' unless ignored.empty?
			ignored << line
		end
	end
	ignored
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

create_dir_and_cd "doc"

puts "-- Scanning directory"
do_or_die("gtkdoc-scan --module='#{MODULE}' --source-dir '#{DIR}' --ignore-headers '#{build_ignore_files}'")
puts " - done"

puts "-- Setting up DB"
do_or_die("gtkdoc-mkdb --module=#{MODULE} --main-sgml-file=#{MODULE}.sgml --source-dir=#{DIR} --xml-mode --output-format=xml --ignore-files '#{build_ignore_files}'")
puts " - done"

create_dir_and_cd "html"

puts "-- Generating HTML"
do_or_die("gtkdoc-mkhtml --path=#{DIR} libglyr ../#{MODULE}.sgml");
puts " - done"

puts "-- Fixing crosslinks"
Dir.chdir("..")
do_or_die("gtkdoc-fixxref --module=#{MODULE} --module-dir=html --html-dir=html")
puts " - done"
