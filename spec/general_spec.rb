require_relative 'spec_helper.rb'

describe "libglyr [general fetchers]" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.artist = "Equilibrium"
		@spit.album  = "Sagas"
		@spit.title  = "Wurzelbert"
		@spit.verbosity = 0
	end
	
	describe "lyrics" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_LYRICS
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end

	describe "cover" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_COVERART
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end
	
	describe "biography" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_ARTISTBIO
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end
end


