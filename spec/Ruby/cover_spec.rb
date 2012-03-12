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
		system("sxiv #{path} &")
	end
end

describe "cover" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_COVERART
	end	

	describe "lyricswiki" do
		before :each do
			@spit.from = "lyricswiki"
		end
		
		it "should get the cover of Axxis - Access All Areas " do
			@spit.artist = "Axxis"
			@spit.album  = "Access All Areas"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "What an cool band"
			@spit.album  = "Cool album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "albumart" do
		before :each do
			@spit.from = "albumart"
		end
		
		it "should test a pagehit " do
			@spit.artist = "Pussycat Dolls"   # Don't ask, yeah?
			@spit.album  = "Dolls Domination"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Bideldibam"
			@spit.album  = "Cool album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "allmusic" do
		before :each do
			@spit.from = "allmusic"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Adele"
			@spit.album  = "19"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "The band they called horst"
			@spit.album  = "Album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

=begin
	describe "amazon" do
		before :each do
			@spit.from = "amazon"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Adele"
			@spit.album  = "21"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "The band they called horst"
			@spit.album  = "Album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end
=end

	# coverhnt seems to be down.

	describe "discogs" do
		before :each do
			@spit.from = "discogs"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Nirvana"
			@spit.album  = "In Utero"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "The band they called horst"
			@spit.album  = "Album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "lastfm" do
		before :each do
			@spit.from = "lastfm"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Alestorm"
			@spit.album  = "Black Sails at Midnight"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Alestorm"
			@spit.album  = "Unknown Album"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "google" do
		before :each do
			@spit.from = "google"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Amon Amarth"
			@spit.album  = "Fate of Norns"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Unknown artists of unknown lands" 

			# Yes, Im cheating here, but did you tried to 
			# get 0 results with an valid album name? no?
			@spit.album  = "Bloghregnfehegle"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "picsearch" do
		before :each do
			@spit.from = "picsearch"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Amon Amarth"
			@spit.album  = "Fate of Norns"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Unknown artists of unknown lands" 

			# Yes, Im cheating here, but did you tried to 
			# get 0 results with an valid album name? no?
			@spit.album  = "Bloghregnfehegle"
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
			@spit.artist = "knorkator"
			@spit.album  = "hasenchartbreaker"
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
	
    describe "jamendo" do
		before :each do
			@spit.from = "jamendo"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Diablo Swing Orchestra"
			@spit.album  = "The Butcher's Ballroom"
            @spit.max_size = 599
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
	
    describe "musicbrainz" do
		before :each do
			@spit.from = "musicbrainz"
		end
		
		it "should test a pagehit" do
			@spit.artist = "Akrea"
			@spit.album  = "Lebenslinie"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1

			show_in_sxiv(list.first)
		end

		it "Don't know this cover" do
			@spit.artist = "Unknown artists of unknown lands" 

			# Yes, Im cheating here, but did you tried to 
			# get 0 results with an valid album name? no?
			@spit.album  = "Bloghregnfehegle"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end
end
