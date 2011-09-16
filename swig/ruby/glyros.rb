#!/usr/bin/ruby
require 'rubygems'

begin
  # Load C library, You __have__ to append the '.so',
  # Will be confused with the wrapper (this file) otherwise
  require_relative "./glyros.so"
rescue LoadError => e
  puts "-- Error while loading Glyr's ruby module"
  puts "-- It is supposed to be in the bin/ruby directory."
  puts "-- ..did you build it? (cmake . -DSWIG_RUBY)"
  exit(-1)
end

class Glyros::GlyrMemCache
	# You don't need to do this yourself
	def register_free
	    ObjectSpace.define_finalizer(self, 
	      proc do 
		  		self.finalize
	      end
	    )
	    return self
	end

	# Use these instead of new
	def self.instance
	    copy = Glyros::glyr_new_cache()
	    return copy.register_free
	end

	def self.instance_by_copy(copy_me)
	    copy = (copy_me.nil?) ? nil : Glyros::glyr_copy_cache(copy_me)
	    return copy.register_free
	end

	def write_to_file(path) 
		return Glyros::glyr_cache_write(self,path)
	end

	def update_md5sum
		Glyros::glyr_cache_update_md5sum(self)
	end

	def print
		q = Glyros::GlyrQuery.instance
		Glyros::glyr_opt_verbosity(q,2)
		Glyros::glyr_cache_print(q,self)
	end

	# disable ctor
	private_class_method :new    

	private
	def finalize
	    Glyros::glyr_free_cache(self)	
	end
end

class Glyros::GlyrQuery
	def register_free
	    ObjectSpace.define_finalizer(self,
	    proc do
		self.finalize
	    end)
	    return self
	end

	def self.instance
	    new_query = Glyros::GlyrQuery.new
	    Glyros::glyr_query_init(new_query)
	    return new_query.register_free
	end

	def download_url(url) 
 		return Glyros::glyr_download(url,self)
	end	

	private
	def finalize
	    Glyros::glyr_query_destroy(self)
	end
end


class GlyrosSpit
	# members
	@query = nil
	@providers = nil

	def initialize
		@query = Glyros::GlyrQuery.instance()
	end

	def reset 
		@query = nil
		@providers = nil
		@query = Glyros::GlyrQuery.new
	end

   	def get(type_enum)
		if type_enum.is_a? Integer
		  Glyros::glyr_opt_type(@query,type_enum)
		  return call_get
		end
      		nil
	end	

	def get
		unless @query.type == Glyros::GLYR_GET_UNSURE
			return call_get
		end
		nil
	end
	
	def version
		return Glyros::glyr_version()
	end

	def verbosity=(level) 
 	 	@query.verbosity = level
		self
  	end

	def artist 
		return @query.artist
	end

	def artist=(artist)
		Glyros::glyr_opt_artist(@query,artist)
		self
	end

	def album 
		return @query.album
	end

	def album=(album)
		Glyros::glyr_opt_album(@query,album)
		self
	end

	def title
		return @query.title
	end

	def title=(title)
		Glyros::glyr_opt_title(@query,title)
		self
	end

	def type=(type_enum) 
		 Glyros::glyr_opt_type(@query,type_enum)
	end

	def type
		return @query.type
	end
	
	def number=(num)
		Glyros::glyr_opt_number(@query,num)
		self
	end

	def number
		return @query.number
	end

	def plugmax=(num)
		Glyros::glyr_opt_plugmax(@query,num)
		self
	end

	def plugmax
		return @query.plugmax
	end

	def lang=(code)
		Glyros::glyr_opt_lang(@query,code)
		self
	end

	def lang 
		return @query.lang
	end

	def timeout=(timeout)
		Glyros::glyr_opt_timeout(@query,timeout)
		self
	end

	def timeout
		return @query.timeout
	end

	def redirects=(redirects) 
		Glyros::glyr_opt_redirects(@query,redirects)
		self
	end

	def redirects 
		return @query.redirects
	end

	def parallel=(parallel)
		Glyros::glyr_opt_parallel(@query,parallel)
		self
	end

	def parallel
		return @query.parallel
	end

	def proxy=(proxystring)
		Glyros::glyr_opt_proxy(@query,proxystring)
		self
	end
	
	def proxy
		return @query.proxy
	end

	def image_size=(min, max) 
		Glyros::glyr_opt_cminsize(@query,min);
		Glyros::glyr_opt_cmaxsize(@query,max);
		self
	end

	def image_size
		return @query.img_min_size, @query.img_max_size
	end

	def from=(argstring )
		@providers = argstring
		self
	end

	def from
		return @providers
	end

	def download=(boolean)
		Glyros::glyr_opt_download(@query,boolean)
		self
	end

	def download?
		return @query.download
	end

	def fuzzyness=(fuzzval) 
		Glyros::glyr_opt_fuzzyness(@query,fuzzval)
		self
	end
 
	def fuzzyness 
		return @query.fuzzyness
	end

	def formats
		return @query.formats
	end

	def formats=(allowed_formats )
		Glyros::glyr_opt_formats(@query, allowed_formats)
		self
	end

	def qsratio
		return @query.qsratio
	end

	def qsratio=(ratio)
		Glyros::glyr_opt_qsratio(@query,qsratio)
	end

	def force_utf8?
		@query.force_utf8
	end

	def force_utf8=(will_force) 
		Glyros::glyr_opt_force_utf8(@query,will_force)
	end

	def useragent=(useragent)
		Glyros::glyr_opt_useragent(@query,useragent)
	end

	def useragent
		@query.useragent
	end

	def self.type_to_s(mytype)
		Glyros::glyr_type_to_string(mytype)
	end	

	def self.get_plugin_info
		return Glyros::GlyrFetcherInfo.new
	end

	private

	def call_get
		length = 0

		Glyros::glyr_opt_from(@query,@providers) unless @providers == nil
		cache_head = Glyros::glyr_get(@query, nil, nil)

		convert = []
		unless cache_head == nil 
				# convert to a normal Ruby Array
				# cachelist is of no use in such a powerful language
				until cache_head == nil
					convert.insert(0,cache_head)
					cache_head.register_free
					cache_head = cache_head.next
				end
		end
		return convert
	end
end

def test_me
	# Note the dots at the end.
	
	q = GlyrosSpit.new
	q.verbosity = 0
	q.artist = "Die Apokalyptischen Reiter"
	q.album  = "Moral & Wahnsinn"
	q.title  = "Die Boten"

	puts("---------------------------")
	q.get(Glyros::GLYR_GET_LYRICS).each do |item|
		puts "Recieved a #{GlyrosSpit.type_to_s(item.type)}:"
		item.print
	end
	
end

def use_strange_functions 
	info = GlyrosSpit.get_plugin_info()
	
	until info.nil?
		puts "\n-----------"
		puts info.name
		source = info.head
		until source.nil?
			puts "  [#{source.key}] #{source.name}"
			source = source.next
		end		
		info = info.next
	end
end
