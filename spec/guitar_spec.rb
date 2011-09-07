# vi: set fileencoding=utf-8 :
require_relative 'spec_helper.rb'

describe 'guitartabs' do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_GUITARTABS
	end
	
	describe "guitartabs" do
		before :each do
			@spit.from = "guitaretab"
		end

		it "should get me two van halen tabs" do
			@spit.artist = "Van Halen"
			@spit.title = "Cathedral"
			@spit.number = 2

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should == 2
			list.first.should be_an_instance_of Glyros::GlyrMemCache
		end
		
		it "should find nothing, because of silly name" do
			@spit.artist = "Dj. Bobo"
			@spit.album  = "Chichiwawa"
			list = @spit.get
		
			list.should be_an_instance_of Array
			list.size.should == 0
		end
	end

	describe "chordie" do
		before :each do
			@spit.from = "chordie"
		end

		it "should get me two van halen tabs" do
			@spit.artist = "Van Halen"
			@spit.title = "Cathedral"
			@spit.number = 2

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should == 2
			list.first.should be_an_instance_of Glyros::GlyrMemCache
		end
		
		it "should find nothing, because of silly name" do
			@spit.artist = "Dj. Bobo"
			@spit.album  = "Chichiwawa"
			list = @spit.get
		
			list.should be_an_instance_of Array
			list.size.should == 0
		end
	end
end
