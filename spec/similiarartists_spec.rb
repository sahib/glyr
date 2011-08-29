require_relative 'spec_helper.rb'

describe "similar_artists" do
	before :each do 
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_SIMILIAR_ARTISTS
	end

	describe "last.fm - similar songs" do
		it "Equilibrium - Wurzelbert" do
			@spit.artist = "Equilibrium"
			@spit.get.size.should equal 1
		end

		it "In Flames - Reroute to remain" do
			@spit.artist = "In Flames"
			@spit.number = 5
			list = @spit.get
	
			list.should be_an_instance_of Array
			list.size.should equal 5

			list.each do |artist|
				puts artist.data
				puts "-------"
			end
		end
	end
end
