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
