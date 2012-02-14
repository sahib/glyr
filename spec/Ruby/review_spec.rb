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

=begin
# Amazon disabled due to their new API terms.
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
=end

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
