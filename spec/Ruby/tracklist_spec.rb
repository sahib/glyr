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
			list.size.should == 13 

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
			list.size.should == 0
		end
	end	
end
