require_relative 'spec_helper.rb'

describe "similar_songs" do
	before :each do 
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_SIMILIAR_SONGS
	end

	describe "last.fm - similar songs" do
		it "Equilibrium - Wurzelbert" do
			@spit.artist = "Equilibrium"
			@spit.title  = "Wurzelbert"
			@spit.get.size.should equal 1
		end

		it "In Flames - Reroute to remain" do
			@spit.artist = "In Flames"
			@spit.title  = "Reroute to remain"
			@spit.number = 5
			list = @spit.get
	
			list.should be_an_instance_of Array
			list.size.should equal 5

			list.each do |song|
				puts song.data
				puts "-------"
			end
		end
	end
end
