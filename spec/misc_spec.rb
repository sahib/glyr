require_relative 'spec_helper.rb'

describe GlyrosSpit do
	before :each do
		@spit = GlyrosSpit.new
	end
	
	describe "#new" do
   	it "takes no parameter and returns a new spit" do
   		@spit.should be_an_instance_of GlyrosSpit
   	end
	end

	describe "#get_plugin_info" do
		before :each do
			@info = GlyrosSpit.get_plugin_info
		end
		it "should get information about fetchers" do
			@info.should be_an_instance_of Glyros::GlyrFetcherInfo
		end
		
		it "should be able to iterate over" do
		 	it =  @info
			puts "A list of fetchers:"
			until it.nil? 
				puts "   #{it.name}"
				it = it.next
			end
			it.should be_an_instance_of NilClass
		end

		it "should show you a tree of providers" do
			it = @info
			puts ("  Tree:")
			until it.nil?
				puts "  #{it.name} => #{it.type}"
				head = it.head
				until head.nil?
					puts "    # #{head.name} [#{head.key}]"
					puts "       - Quality: #{head.quality}"
					puts "       - Speed:   #{head.speed}"
					puts "       - Type:    #{head.type}"
					head = head.next
				end
				it = it.next
				puts 
			end
		end
	end

	describe "#glyr_init" do
		it "should not crash now" do
			Glyros::glyr_cleanup()
			Glyros::glyr_init()
		end
		
		it "should not crash now" do
			Glyros::glyr_cleanup()
			Glyros::glyr_cleanup()
			Glyros::glyr_cleanup()
		end

		it "should not crash now" do
			Glyros::glyr_init()
			Glyros::glyr_init()
			Glyros::glyr_init()
		end

		it "should not crash now" do
			Glyros::glyr_init()
			Glyros::glyr_cleanup()
			Glyros::glyr_cleanup()
			Glyros::glyr_init()
			Glyros::glyr_cleanup()
			Glyros::glyr_init()
			Glyros::glyr_cleanup()
			Glyros::glyr_init()
		end
	end

	describe "#glyr_init_query" do
		before :each do
			@q = Glyros::GlyrQuery.new
		end

		it "Get me the a default query" do
			Glyros::glyr_query_init @q
		end

		it "Should destroy the query" do
			Glyros::glyr_query_destroy @q
		end

		it "Should destroy the query" do
			Glyros::glyr_query_destroy @q
			Glyros::glyr_query_init @q
		end
	end	

	describe "#glyr_get confusage" do
		before :each do
			@spit = GlyrosSpit.new
			@spit.verbosity = 4
		end
		
		it "should confuse glyr :-)" do
			# Those get "filtered"
			# i.e. replaced by "" internally
			confuser = -> type do 
 				@spit.artist = "feat."
				@spit.album  = "  ()"
 				@spit.title  = "  "
				@spit.useragent = nil
 				@spit.type   = Glyros::GLYR_GET_COVERART
				@spit.plugmax = 0
			   @spit.get()
			end

			info = GlyrosSpit.get_plugin_info
			until info.nil?
				l = confuser[info.type]
				puts l.first.data unless l.nil? or l.first.nil?
				info = info.next
			end
		end
		
		it "should do nothing" do
			@spit.artist = nil
			@spit.album  = nil
			@spit.album  = nil

			info = GlyrosSpit.get_plugin_info
			until info.nil?
				@spit.type = info.type
				@spit.get.first.should == nil
				info = info.next
			end
			
		end
	end
end
