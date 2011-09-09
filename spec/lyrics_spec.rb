# vi: set fileencoding=utf-8 :
require_relative 'spec_helper.rb'

describe "lyrics" do
	before :each do
		@spit = GlyrosSpit.new
		@spit.type = Glyros::GLYR_GET_LYRICS
	end	

	describe "elyrics" do
		before :each do
			@spit.from = "elyrics"
		end

		it "should test normal page hit" do
			@spit.artist = "In Flames"
			@spit.title  = "Deliver us"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "Wacka Wacka Ding dong"
			@spit.title  = "Bimmel bammel"
			list = @spit.get
			
			list.should be_an_instance_of Array
			list.first.should be_an_instance_of NilClass
			list.size.should equal 0
		end
	end

	describe "lipwalk" do
		before :each do
			@spit.from = "lipwalk"
		end

		it "should test search-results parser" do
			@spit.artist = "In Flames"
			@spit.title  = "Trigger"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should test normal page hit" do
			@spit.artist = "Knorkator"
			@spit.title  = "Absolution"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "Wacka Wacka Ding dong"
			@spit.title  = "Bimmel bammel"
			list = @spit.get
			
			list.should be_an_instance_of Array
			list.first.should be_an_instance_of NilClass
			list.size.should equal 0
		end
	end

	describe "lyrdb" do
		before :each do
			@spit.from = "lyrdb"
		end

		it "should test page hit" do
			@spit.artist = "Knorkator"
			@spit.title  = "Ma baker"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "lyrdbs happy meal"
			@spit.title  = "wont arrive you"
			list = @spit.get
		
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "lyricsreg" do
		before :each do 
			@spit.from = "lyricsreg"
		end

		it "should test page hit" do
			@spit.artist = "Annihilator"
			@spit.title  = "Coward"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be > 100
		end

		it "should find nothing at all" do
			@spit.artist = "The band they called Horse"
			@spit.title  = "It's not there."
			list = @spit.get

			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "lyricstime" do
		before :each do
			@spit.from = "lyricstime"
		end

		it "should test page hit" do
			@spit.artist = "Knorkator"
			@spit.title  = "Absolution"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be >= 100
		end

		it "should find nothing at all" do
			@spit.artist = "The band they called Horse"
			@spit.title  = "It's not there."	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "lyricsvip" do
		before :each do 
			@spit.from = "lyricsvip"
		end

		it "should test page hit" do
			@spit.artist = "Excrementory Grindfuckers"
			@spit.title  = "Das Kuh"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be >= 60
		end

		it "should find nothing at all" do
			@spit.artist = "The band they called Horse"
			@spit.title  = "It's not there."	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end
		
	describe "lyricswiki" do
		before :each do 
			@spit.from = "lyricswiki"
		end

		it "should test page hit" do
			@spit.artist = "Excrementory Grindfuckers"
			@spit.title  = "Das Kuh"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 1
		end

		it "should find nothing at all" do
			@spit.artist = "The band they called Horse"
			@spit.title  = "It's not there."	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should == 0
		end
	end

	describe "lyrix" do
		before :each do 
			@spit.from = "lyrix"
		end

		it "should test page hit" do
			@spit.artist = "Amy Winehouse"
			@spit.title  = "Back to Black"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
		end

		it "should find nothing at all" do
			@spit.artist = "The band they called Horse"
			@spit.title  = "It's not there."	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "magistrix" do
		before :each do
			@spit.from = "magistrix"
		end
		
		it "should test a single page hit" do
			@spit.artist = "Knorkator"
			@spit.title  = "Absolution"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be >= 150
		end

		it "should find nothing" do
			@spit.artist = "Some Unknown Artist"
			@spit.title  = "Unknown Song"
			list = @spit.get

			list.each {|song| puts song.data }
			list.should be_an_instance_of Array
			list.empty?.should equal true
		end

		it "should test the searchresults parser" do
			@spit.artist = "Lady Gaga"
			@spit.title  = "Pokerface"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
			list.first.size.should be >= 550
		end
	end

	describe "metallum" do
		before :each do 
			@spit.from = "metallum"
		end

		it "should test page hit" do
			@spit.artist = "Equilibrium"
			@spit.title  = "Wurzelbert"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should equal 1
		end

		it "should find nothing at all" do
			# This is a joke, but let's hope it won't find anything
			@spit.artist = "Lady Gaga"
			@spit.title  = "Pokerface"	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should equal 0
		end
	end

	describe "chartlyrics" do
		before :each do 
			@spit.from = "chartlyrics"
		end
			
		it "should test page hit" do 
			@spit.artist = "Die Apokalyptischen Reiter"
			@spit.title  = "Friede sei mit dir"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 1
		end

		it "should find nothing at all" do
			@spit.artist = "Some band"
			@spit.title  = "Some album"	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should == 0
		end
	end

	describe "metrolyrics" do
		before :each do 
			@spit.from = "metrolyrics"
		end

		it "should test page hit" do
			@spit.artist = "Die Apokalyptischen Reiter"
			@spit.title  = "Friede sei mit dir"
			list = @spit.get

			list.should be_an_instance_of Array
			list.first.should be_an_instance_of Glyros::GlyrMemCache
			list.size.should == 1
		end

		it "should find nothing at all" do
			@spit.artist = "Some band not existing"
			@spit.title  = "Some album not existing"	

			list = @spit.get
			list.should be_an_instance_of Array
			list.size.should == 0
		end
	end
end
