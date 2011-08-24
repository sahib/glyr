require_relative 'spec_helper.rb'

describe "lyrics" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GET_LYRICS
	end	

	describe "elyrics" do
		before :each do
			@spit.from = "elyrics"
		end

		it "should test normal page hit" do
			@spit.artist = "In Flames"
			@spit.title  = "Deliver us"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "Wacka Wacka Ding dong"
			@spit.title  = "Bimmel bammel"
			list = @spit.get
			
			list.should be_an_instance_of Array
			list.first.should be_an_instance_of NilClass
			list.size.should equal 0
		end
	end

	describe "lipwalk" do
		before :each do
			@spit.from = "lipwalk"
		end

		it "should test search-results parser" do
			@spit.artist = "In Flames"
			@spit.title  = "Trigger"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should test normal page hit" do
			@spit.artist = "Knorkator"
			@spit.title  = "Absolution"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "Wacka Wacka Ding dong"
			@spit.title  = "Bimmel bammel"
			list = @spit.get
			
			list.should be_an_instance_of Array
			list.first.should be_an_instance_of NilClass
			list.size.should equal 0
		end
	end
end
