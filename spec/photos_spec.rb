# vi: set fileencoding=utf-8 :
require_relative 'spec_helper.rb'

SHOW_COVER = true
def show_in_sxiv(cache)
	if cache.respond_to? :write_to_file and SHOW_COVER then
		path = "/tmp/rspec_cover.jpg"
		cache.write_to_file path
		system("sxiv #{path}")
	end
end

describe "artistphotos" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_ARTIST_PHOTOS
	end	

	describe "discogs" do
		before :each do
			@spit.from = "discogs"
		end
		
		it "should get the image of the bad band Onkelz" do
			@spit.artist = "Böhse Onkelz"
			@spit.download = false
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
		end

		it "Don't know this band" do
			@spit.artist = "What an cool band"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "flickr" do
		before :each do
			@spit.from = "flickr"
		end
		
		it "should test pagehit" do
			@spit.artist = "Die Ärzte"
			@spit.download = false
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 1
		end

		it "Don't know this band" do
			@spit.artist = "Beeblebrooks"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "google" do
		before :each do
			@spit.from = "google"
		end
		
		it "should test pagehit" do
			@spit.artist = "DeViLdRiVeR"
			@spit.download = false
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 1
		end
		# Senseless to have a no-fit
	end

	describe "lastfm" do
		before :each do
			@spit.from = "lastfm"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Alestorm"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Unknown Album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "singerpictures" do
		before :each do
			@spit.from = "singerpictures"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Equilibrium"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Gamma Gandalf"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "rhapsody" do
		before :each do
			@spit.from = "rhapsody"
		end
		
		it "should test a pagehit" do
			@spit.artist = "In Flämes"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 1
			list.first.should be_an_instance_of Glyros::GlyrMemCache

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Unknownartist" 
			@spit.album  = "Myalbum"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

end
