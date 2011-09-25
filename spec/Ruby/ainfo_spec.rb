# vi: set fileencoding=utf-8 :
require_relative 'spec_helper.rb'

describe 'artistbio' do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_ARTISTBIO
	end

	describe "lastfm" do
		before :each do
			@spit.from = "lastfm"
		end

		it "should test the language support" do
			@spit.artist = "Die Ärzte"
			list = @spit.get
			@spit.lang = "de"
			lsde = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.first.size.should be > (lsde.first.size - 15000)
			lsde.first.size.should be > 35000 # wow, that was surprisingly long
		end

		it "should return nothing" do
			@spit.artist = "The Band they called horse"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "lyricsreg" do
		before :each do 
			@spit.from = "lyricsreg"
		end

		it "should get me a pagehit" do
			@spit.artist = "Adele"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.first.size.should be > 1000
		end

		it "should get me nothing" do
			@spit.artist = "A good band"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal  0
		end
	end

	describe "allmusic" do
		before :each do 
			@spit.from = "allmusic"
		end

		it "should get me a pagehit" do
			@spit.artist = "Die Ärzte"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.first.size.should be > 1000
		end

		it "should not parse the searchpage, but hop to the long version instead" do
			@spit.artist = "In Flames"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.first.size.should be > 512
		end

		it "should get a pagehit but return nothing (empty bio)" do
			@spit.artist = "Hämatom"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal  0
		end

		it "should get me nothing" do
			@spit.artist = "A good band"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal  0
		end
	end
end
