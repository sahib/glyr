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

describe "libglyr [general fetchers]" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.artist = "Equilibrium"
		@spit.album  = "Sagas"
		@spit.title  = "Wurzelbert"
		@spit.verbosity = 0
	end
	
	describe "lyrics" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_LYRICS
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end

	describe "cover" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_COVERART
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end
	
	describe "biography" do
		it "should get me a non empty array with exactly one element " do
			@spit.type = Glyros::GLYR_GET_ARTISTBIO
			done = @spit.get
			done.should be_an_instance_of Array
			done.should satisfy {|arr| arr.size == 1}
		end
	end
end


