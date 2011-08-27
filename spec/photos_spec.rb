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

describe "cover" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GET_ARTIST_PHOTOS
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
			list.first.data.should == "http://s.dsimg.com/image/A-262898-1106221539.jpg"
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


end
