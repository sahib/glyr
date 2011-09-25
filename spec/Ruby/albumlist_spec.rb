require_relative 'spec_helper.rb'

describe "albumlist" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_ALBUMLIST
	end

	describe "musicbrainz" do
		before :each do
			@spit.from = "musicbrainz"
		end

		it "should get me 6 In Flames albums" do
			@spit.artist = "In Flames"
			@spit.number = 12
			list = @spit.get

			list.should be_an_instance_of Array
		        list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 6
		end

		it "should find nothing at all" do
			@spit.artist = "A good nonexistent band"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end
end		
