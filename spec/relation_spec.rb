require_relative 'spec_helper.rb'

describe "relations" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GET_RELATIONS
	end
		
	describe "Not found case" do 
		it "should return an empy array" do
			@spit.artist = "Gnomoquadrupelupedia"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "Metallica (Artist)" do 
		it "Should return some neat things about metallica" do
			@spit.artist = "Metallica"
			@spit.number = 10
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should_not equal 0
			
			list.each do |url|
				puts url.data
			end
		end
	end

	describe "Metallica - Some kind of monster (album)" do
		it "Should return a link (most likely an asin)" do
			@spit.artist = "Metallica"
			@spit.album  = "Some kind of monster"
			list = @spit.get
	
			list.should be_an_instance_of Array
			list.should have(1).things
			puts list.first.data
		end
	end
end
