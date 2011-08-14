require 'rubygems'
# Load C-Library Wrapper
begin
   require_relative '../swig/ruby/glyros.rb'
rescue LoadError => e
   puts "-- Cannot test, you monster"
   puts "-- Reason: #{e}"
	puts "-- Which most likely means:"
	puts "--- 'The Ruby module was not built'"
	puts "--- Solution: cmake with -DWSIG_RUBY=true"
end
