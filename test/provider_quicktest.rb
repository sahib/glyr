#!/usr/bin/ruby
# This is a very simple script to test if providers are working as expected
# It may need some work still, as tags relations are a little harder to test,
# Three queries are created, and filled with popular acts (or known to be found)
# Those are tested for every provider and marked with DONE if found at least 2/3
# If 0 were found you should have a look there..
#
# P.S: Sorry, the providernames are not working yet

require 'rubygems'

begin
   require_relative '../swig/glyros.rb'
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

$FAILSTRING = "[" + "FAIL".color(:red)   + "]"
$DONESTRING = "[" + "DONE".color(:green) + "]"

def fill_fields(num_of_filled)
   adele = GlyrosSpit.new
   rider = GlyrosSpit.new
   equilibrium = GlyrosSpit.new

   adele.artist = "Adele" if num_of_filled > 0
   adele.album  = "21" if num_of_filled > 1
   adele.title  = "Rolling In The Deep" if num_of_filled > 2
   adele.fuzzyness = 6


   rider.artist = "Die Apokalyptischen Reiter" if num_of_filled > 0
   rider.album  = "Moral & Wahnsinn" if num_of_filled > 1
   rider.title  = "Wir reiten" if num_of_filled > 2
   rider.fuzzyness = 6


   equilibrium.artist = "Equilibrium" if num_of_filled > 0
   equilibrium.album  = "Rekreatur" if num_of_filled > 1
   equilibrium.title  = "Verbrannte Erde" if num_of_filled > 2
   equilibrium.fuzzyness = 6

   return adele,rider,equilibrium
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
   names = GlyrosSpit.name_by_id(Glyros::GET_UNSURE).split(/\|/)
   gKeys = GlyrosSpit.key_by_id(Glyros::GET_UNSURE)

   gNums = 0 # num of types
   dTest = 0 # Total tests
   dPass = 0 # Total passed


   GlyrosSpit.gid_by_id(Glyros::GET_UNSURE).each_byte do |type|
      # Handle relation /tags
      filled_fields = 3
      stop_after_fl = 2

      if type == Glyros::GET_RELATIONS or type == Glyros::GET_TAGS
         stop_after_fl = 0
      end

      # To iterate over the iterators
      key_collection = GlyrosSpit.key_by_id(type)

      # The matrix we store the results in
      visual = Matrix.build(names.size,key + 1_collection.size + 1) do |r,c|
         return names[r] if c == 0 and r != 0
      end

      while filled_fields != stop_after_fl
         adele,rider,equilibrium = fill_fields(filled_fields)

         passed = 0
         tested = 0

         puts "  -- Testing getter [#{filled_fields}]'#{names[gNums]}'"
         key_collection.each_char do |key|
            print "    -- Testing provider '#{key}' "

            # Set origin
            adele.from = key
            rider.from = key
            equilibrium.from = key

            threads = [], ad = [], ri = [], eq = []
            threads.push Thread.new { ad = adele.get(type)  		 }
            threads.push Thread.new { ri = rider.get(type)  		 }
            threads.push Thread.new { eq = equilibrium.get(type) }
            threads.each { |t| t.join }

            list = ad + ri + eq
            if list.size < 2
               print $FAILSTRING
            else
               print $DONESTRING
               passed += 1
            end

            print " [" +
            (ad.empty? ? "x".color(:red) : "v".color(:green)) + "|" +
            (ri.empty? ? "x".color(:red) : "v".color(:green)) + "|" +
            (eq.empty? ? "x".color(:red) : "v".color(:green)) +
            "]\n"

            list.each do |item|
               item.write_file(File.dirname(__FILE__) + "/out/" + gKeys[gNums] + "_" + key + (item.is_image ? ".img" : ".txt"))
            end


            tested += 1
         end
         puts "  -- Passed #{passed.to_s.color(:red)} of #{tested.to_s.color(:green)} tests. (#{((passed.to_f / tested.to_f)*100).to_i}%)"


         filled_fields -= 1

         dPass += passed
         dTest += tested
      end
      gNums += 1

   end

   puts "-- In total: #{dPass}/#{dTest} (#{((dPass.to_f / dTest.to_f)*100).to_i})%"
   puts "-- => Battle station fully operational?"
   puts "-- Thanks for testing, you monster."
end
