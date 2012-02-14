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

describe "albumlist" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_ALBUMLIST
	end

	describe "musicbrainz" do
		before :each do
			@spit.from = "musicbrainz"
		end

		it "should get me 4 In Flames albums" do
			@spit.artist = "In Flames"
			@spit.number = 12
			list = @spit.get

            list.each do |item|
                puts item.data
            end

			list.should be_an_instance_of Array
	        list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 4
		end

		it "should find nothing at all" do
			@spit.artist = "A good nonexistent band"
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end
end		
