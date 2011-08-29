require_relative 'spec_helper.rb'

describe "tracklist" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_TRACKLIST
	end

    describe "Equilibrium - Sagas" do
		it "should be the tracklist of Sagas" do
			@spit.artist = "Equilibrium"
			@spit.album  = "Sagas"
			@spit.number = 13
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 13 

			x = 0
			list.each do |track|
				x += 1
				puts "##{"%02d" % x}: [#{"%02d" % (track.duration/60)}:#{"%02d" % (track.duration%60)}] #{track.data} "
			end
		end
	end

	describe "Not found case" do 
		it "should return an empy array" do
			@spit.artist = "Long string musicbrainz won't find at all"
			@spit.album  = "Same here, no album like this in DB"
			list = @spit.get
		
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end	
end
