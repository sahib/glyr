require_relative 'spec_helper.rb'

describe GlyrosSpit do
	before :each do
		@spit = GlyrosSpit.new
	end
	
	describe "#new" do
   	    it "takes no parameter and returns a new spit" do
           @spit.should be_an_instance_of GlyrosSpit
   	    end
	end

	describe "#get_plugin_info" do
		it "should get information about fetchers" do
			GlyrosSpit.get_plugin_info.should be_an_instance_of Glyros::GlyrFetcherInfo
		end
	end
end
