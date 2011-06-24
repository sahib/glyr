#!/usr/bin/ruby

# should be there
require 'rubygems'

begin 
  require File.dirname(__FILE__) + "/../../bin/ruby/glyros"
rescue LoadError => e
  puts "-- Error while loading Glyr's ruby module"
  puts "-- It is supposed to be in the same path as this script"
  puts "-- Exact message: #{e}"
  exit(-1)
end

# Mixin free routines
class Glyros::GlyMemCache
	# You don't need to do this yourself
	def register_free
	    ObjectSpace.define_finalizer(self, 
	    proc do 
		self.finalize
	    end)
	    return self
	end

	# Use these instead of new
	def self.instance
	    copy = Glyros::Gly_new_cache()
	    return copy.register_free
	end

	def self.instance_by_copy(copy_me)
	    copy = (copy_me.nil?) ? nil : Glyros::Gly_copy_cache(copy_me)
	    return copy.register_free
	end

	# disable ctor
	private_class_method :new    

	private
	def finalize
	    Glyros::Gly_free_cache(self)	
	end

end

class Glyros::GlyQuery
	def register_free
	    ObjectSpace.define_finalizer(self,
	    proc do
		self.finalize
	    end)
	    return self
	end

	def self.instance
	    new_query = Glyros::GlyQuery.new
	    Glyros::Gly_init_query(new_query)
	    return new_query.register_free
	end

	private
	def finalize
	    Glyros::Gly_destroy_query(self)
	end
end

class GlyrosSpit
	# members
	@query = nil
	@providers = nil

	def initialize
		@query = Glyros::GlyQuery.instance()
	end

	# public
	def reset 
		# finalizer gets called
		@query = nil
		@providers = nil
		@query = Glyros::GlyQuery.new
	end

        def get(type_enum)
		Glyros::GlyOpt_type(@query,type_enum)
		return call_get
	end	
	
	def download_util( url ) 
		return Glyros::Gly_download(URL, @query) unless @query.nil?
	end	
	
	def version
		return Glyros::Gly_version()
	end

	def key_by_id(get_id)
		return Glyros::GlyPlug_get_key_by_id(get_id)
	end

	def gid_by_id(get_id)
		return Glyros::GlyPlug_get_gid_by_id(get_id)
	end

	# not working yet
	def name_by_id(get_id)
		return Glyros::GlyPlug_get_key_by_id(get_id)
	end

	def groupname_by_id(get_id)
		return Glyros::Gly_groupname_by_id(get_id)
	end
=begin 
=| GETTER|SETTER |=
=end
    	def verbosity=(level)
                Glyros::GlyOpt_verbosity(@query,level) 
		self
    	end

	def artist 
		return @query.artist
	end

	def artist=(artist)
		Glyros::GlyOpt_artist(@query,artist)
		self
	end

	def album 
		return @query.artist
	end

	def album=(album)
		Glyros::GlyOpt_album(@query,album)
		self
	end

	def title
		return @query.title
	end

	def title=(title)
		Glyros::GlyOpt_title(@query,title)
		self
	end

	def type
		return @query.type
	end

	def write_file( path, cache ) 
		return Glyros::Gly_write(cache,path)
	end
	
	def number=(num)
		Glyros::GlyOpt_number(@query,num)
		self
	end

	def number
		return @query.number
	end

	def plugmax=(num)
		Glyros::GlyOpt_plugmax(@query,num)
		self
	end

	def plugmax
		return @query.plugmax
	end

	def lang=(code)
		Glyros::GlyOpt_lang(@query,code)
		self
	end

	def lang 
		return @query.lang
	end

	def timeout=(timeout)
		Glyros::GlyOpt_timeout(@query,timeout)
		self
	end

	def timeout
		return @query.timeout
	end

	def redirects=(redirects) 
		Glyros::GlyOpt_redirects(@query,redirects)
		self
	end

	def redirects 
		return @query.redirects
	end

	def parallel=(parallel)
		Glyros::GlyOpt_parallel(@query,parallel)
		self
	end

	def parallel
		return @query.parallel
	end

	def proxy=(proxystring)
		Glyros::GlyOpt_proxy(@query,proxystring)
		self
	end
	
	def proxy
		return @query.proxy
	end

	def coverSize=( min, max ) 
		Glyros::GlyOpt_cminsize(@query,min);
		Glyros::GlyOpt_cmaxsize(@query,max);
		self
	end

	def coverSize
		return @query.cover.min_size, @query.cover.max_size
	end

	def from=(argstring )
		@providers = argstring
		self
	end

	def from
		return @providers
	end

	def download=( boolean )
		Glyros::GlyOpt_download(@query,boolean)
		self
	end

	def download?
		return @query.download
	end

	def color=( boolean )
		Glyros::GlyOpt_color(@query,boolean)
		self
	end
       
	def fuzzyness=(fuzzval) 
		Glyros::GlyOpt_fuzzyness(@query,fuzzval)
		self
	end
 
	def fuzzyness 
		return @query.fuzzyness
	end

        def color?
     	        return @query.color
    	end

	def grouped_download?
		return @query.grouped_download
	end

	def grouped_download=(make_it_grouped)
		Glyros::GlyOpt_groupedDL(@query,make_it_grouped)
	end

	def formats
		return @query.formats
	end

	def formats=(allowed_formats )
		Glyros::GlyOpt_formats(@query, allowed_formats)
		self
	end

	def duplcheck=(check)
		Glyros::GlyOpt_duplcheck(@query, check)
		self
	end

	def duplcheck?
		return @query.duplcheck
	end

=begin
=| PRIVATE HELPERS |=
=end
	private
	#-----#

	def call_get
		length = 0

		Glyros::GlyOpt_from(@query,@providers) unless @providers == nil
		cache_head = Glyros::Gly_get(@query, nil, nil)

		convert = []
		unless cache_head == nil 
				# convert to a normal Ruby Array
				# cachelist is of no use in such a powerful language
				until cache_head == nil
					convert << cache_head
					cache_head.register_free()
					cache_head = cache_head.next
				end
		end
		return convert
	end
end

# A simple test, more tests will follow
def test_me
	# Note the dots at the end.
	
	q = GlyrosSpit.new
	q.artist = "Die Apokalyptischen Reiter"
	q.album  = "Moral & Wahnsinn"
	q.title  = "Die Boten"
	q.get(Glyros::GET_LYRICS).each do |item|
		puts item,item.data
	end
end

def use_strange_functions 
	puts GlyrosSpit.new.key_by_id(Glyros::GET_LYRICS)
	puts GlyrosSpit.new.name_by_id(Glyros::GET_LYRICS)
	GlyrosSpit.new.gid_by_id(Glyros::GET_LYRICS).each_byte { |int| print "#{int} " }
	puts "\n",GlyrosSpit.new.groupname_by_id(Glyros::GRP_FAST)
end

#test_me()
#use_strange_functions()
