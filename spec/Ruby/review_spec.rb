require_relative 'spec_helper.rb'

describe "review" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_ALBUM_REVIEW
	end

	describe "allmusic_com" do
		it "should get me a review of 36 Crazyfists" do
			@spit.artist = "36 Crazyfists"
			@spit.album  = "Rest Inside the Flames"
			@spit.from   = "allmusic"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.size.should be > 200

			puts list.first.data
		end
	end

	describe "amazon" do
		it "should get a review of Adele-21" do
			@spit.artist = "Adele"
			@spit.album  = "21"
			@spit.from   = "amazon"
			list = @spit.get
			
			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.size.should be > 200

			puts list.first.data
		end
	end

	describe "metallum" do
		it "should get me a review of Riders on the Storm" do
			@spit.artist = "Die Apokalyptischen Reiter"
			@spit.album  = "Riders on the Storm"
			@spit.from   = "metallum"
			list = @spit.get
		
			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.size.should be > 200

			puts list.first.data
		end
	end
end
