# vi: set fileencoding=utf-8 :
#################################################################
# This file is part of glyr
# + a commnandline tool and library to download various sort of musicrelated metadata.
# + Copyright (C) [2011-2012]  [Christopher Pahl]
# + Hosted at: https://github.com/sahib/glyr
#
# glyr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# glyr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with glyr. If not, see <http://www.gnu.org/licenses/>.
#################################################################
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
		
		it "should get the image of the Nirvana" do
			@spit.artist = "Nirvana"
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
