
#!/usr/bin/ruby
# This is a very simple script to test if providers are working as expected
# It may need some work still, as tags relations are a little harder to test,
# Three queries are created, and filled with popular acts (or known to be found)
# Those are tested for every provider and marked with DONE if found at least 2/3
# If 0 were found you should have a look there..
#
# P.S: Sorry, the providernames are not working yet

require 'rubygems'
require 'thread'

begin
  require_relative '../swig/ruby/glyros.rb'
rescue LoadError => e
  puts "-- Cannot test, you monster"
  puts "-- Reason: #{e}"
end

begin 
	   require 'rainbow'
rescue LoadError => e
	   puts "-- disabling colors"
	   class String 
		   def color(sym)
		     #no color here
				 self
		   end
	   end
end

# Random acts filled as query
# You can add more, this test will handle them automagically
def get_test_subjects( num_of_filled )
		queries = []		

		verb = 1
		timeout = 100

	  adele = GlyrosSpit.new
	  rider = GlyrosSpit.new
	  equilibrium = GlyrosSpit.new

    adele.artist = "Adele"  if num_of_filled > 0
    adele.album  = "21"   if num_of_filled > 1
	  adele.title  = "Rolling In The Deep"  if num_of_filled > 2
		adele.fuzzyness = 6
		adele.verbosity = verb
		adele.timeout = timeout
		
    rider.artist = "Die Apokalyptischen Reiter"  if num_of_filled > 0
    rider.album  = "Moral & Wahnsinn"  if num_of_filled > 1
	  rider.title  = "Wir reiten"  if num_of_filled > 2
		rider.fuzzyness = 6
		rider.verbosity = verb
		rider.timeout = timeout

    equilibrium.artist = "Equilibrium"  if num_of_filled > 0
    equilibrium.album  = "Rekreatur"  if num_of_filled > 1
	  equilibrium.title  = "Verbrannte Erde" if num_of_filled > 2
		equilibrium.fuzzyness = 6
		equilibrium.verbosity = verb
		equilibrium.timeout = timeout

		queries.push adele
		queries.push rider
		queries.push equilibrium
		return queries	
end

def print_visual(arr)
		first_col = false
		cols = ['A','R','E']
		arr.each do |sub|
			sub.each do |elm|
				if first_col
					print cols.first + ": "
					cols.shift
					first_col = false
				end
				  print "#{elm} "
			end
			puts ""
			first_col = true
		end
end


  # Actual testing here
begin
		trap("INT") do
			puts
			puts "## Interrupted Test, you monster!! ".color(:red)
			puts "## Exiting therefore. Good testing.".color(:red)
			exit(-1)
		end

		# Names and keys
		names = GlyrosSpit.name_by_id Glyros::GET_UNSURE
		gKeys = GlyrosSpit.key_by_id  Glyros::GET_UNSURE
		gGIDs = GlyrosSpit.gid_by_id  Glyros::GET_UNSURE
		iName = 0

		output_list = []
		output_threads = []
		output_mutex   = Mutex.new	

		# Make it though all types glyr is offering
		gGIDs.each_byte { |get_type|
			pKeys = GlyrosSpit.key_by_id get_type
			field_counter = 3
			stop_after_fl = (get_type == Glyros::GET_RELATIONS or 
											 get_type == Glyros::GET_TAGS
											) ? 0 : 2

			output_threads.push(Thread.new do
					until field_counter <= stop_after_fl
							test_subjects = get_test_subjects(field_counter)
							output_mutex.synchronize {	
									puts "-- Testing getter [#{field_counter}]'#{names.first}'"
									names.shift
									field_counter -= 1
							}				

							# An array containing the results
							visual = []

							# Add a neat header to it
							header = ["   "]
							pKeys.each_char {|sub| header.push (" " + sub + " ")}
							visual.push header
		
							test_subjects.each { |test|
									test_result = []
									sub_threads = []

										pKeys.each_char { |key|
											visual_mutex = Mutex.new
											sub_threads.push Thread.new {
												test.from = key
				#puts "\nget(#{key}): #{test.artist} - #{test.album}  => #{get_type}\n"
												list = test.get(get_type)

												visual_mutex.synchronize {
														index = pKeys.index(key)
														test_result[index] = ((list.empty?) ? "  x".color(:red) : "  v".color(:green))
												}
										 }
									}
									sub_threads.each {|thread| thread.join() }
									visual.push test_result
							}

							output_mutex.synchronize {
									output_list.push visual
							}
					end
				end)
		}
		output_threads.each {|thread| thread.join() }
		puts "-- Joining threads."

		output_list.each do |out|
			print_visual out
			puts "---------"
		end
end
