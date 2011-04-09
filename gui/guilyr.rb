require 'rubygems'
require 'gtk2'
require './glubyr.rb'

class ItemView < Gtk::EventBox
	def initialize(path, data, artist, album, title)
		super()
		hbox = Gtk::HBox.new(false,2)
		vbox = Gtk::VBox.new(false,2)
		@internLayout = Gtk::Layout.new.set_size_request(400,170)
		self.add(hbox)

		hbox.pack_start(vbox,true,true,0)
		vbox.pack_start(@internLayout,true,false,0)
		hbox.pack_start(Gtk::Button.new("Save").set_size_request(60,60),false,false,2)
		vbox.pack_start(Gtk::HSeparator.new,false,false,2)
			

		if data.is_image
		    init_as_image(path, data, artist, album, title)
		else
		    init_as_text(path, data, artist, album, title) 
		end
	
		hbox.show_all
		return self
	end

	def init_as_image(path, data, artist, album, title)
		pixbuf = nil 
		begin
		    pixbuf = Gdk::Pixbuf.new(path,150,150)
		    @image = Gtk::Image.new(pixbuf) unless pixbuf == nil
		rescue 
		    puts "Cannot load image: '#{path}'"
		end

		@internLayout.put(@image,10,10) unless @image  == nil
	        @internLayout.put(Gtk::Label.new("Size: #{pixbuf.width}x#{pixbuf.height} (#{data.size} Bytes)"),185,70)
		@internLayout.put(Gtk::Label.new("From: #{data.dsrc}"),      185,50)
	end

	def init_as_text(path, data, artist, album, title)
		
	end
end

class VR_gui

  @@getterHash = 
  {
	:Cover     => { :name => "Cover Art",       :type => Glyr::GET_COVER     },
	:Lyrics	   => { :name => "Lyrics",          :type => Glyr::GET_LYRIC     }, 
	:Photos    => { :name => "Bandphotos",      :type => Glyr::GET_PHOTO     },
	:Ainfo     => { :name => "Artist Biography",:type => Glyr::GET_AINFO     },
	:Similiar  => { :name => "Similiar artists",:type => Glyr::GET_SIMILIAR  },
	:Review    => { :name => "Albumreview",     :type => Glyr::GET_REVIEW    },
	:Albumlist => { :name => "List of Albums",  :type => Glyr::GET_ALBUMLIST },
	:Tracklist => { :name => "List of Tracks",  :type => Glyr::GET_TRACKLIST },
	:Tags	   => { :name => "Random Tags",     :type => Glyr::GET_TAGS      },
	:Relations => { :name => "Random Relations",:type => Glyr::GET_RELATIONS }
  }

  def fill_combobox( box )
     @@getterHash.values.each { |subhash| box.append_text(subhash[:name]) }
     return box
  end

  def initialize
    if __FILE__ == $0
      # This is just dum init code for GTK+ 
      # This script depends on XML file called main.glade where 
      # all GUI description is stored. It's expected to be in the same
      # dir as the script

      # Init Builder
      builder = Gtk::Builder.new
      builder.add_from_file("./main.glade")
      builder.connect_signals{ | handler |  method(handler) }

      # Add the actual working area 
      @innerVBox = Gtk::VBox.new(false,2)
      @sWin = builder.get_object("sWin")
      @sWin.add_with_viewport(@innerVBox)

      # Some highly useful statusbar...
      @statusbar = builder.get_object("statusbar")
      @statusbar_info = @statusbar.get_context_id("Info")
      @statusbar.push(  @statusbar_info, "This is not supposed to be used yet." )

      # Fill the comboxbox with all chocies
      @combo_box = fill_combobox(Gtk::ComboBox.new(true))

      # Pack the rest and make sure everything is shown
      @mVBox = builder.get_object("hbox4")
      @mVBox.pack_start(@combo_box,true,true,2)
      @mVBox.show_all

      # Fill the entry variables
      @artistEntry = builder.get_object("artistEntry") 
      @albumEntry  = builder.get_object("albumEntry")      
      @titleEntry  = builder.get_object("titleEntry")      
    end
  end

  def add_item( path, data ,a, b, t)
	it = ItemView.new(path, data, a, b, t)
	@innerVBox.pack_start(it,true,false,0)
	@innerVBox.show_all
  end

  def main
       puts "-- Starting mainloop --"
       Gtk.main()
  end

  def on_searchButton_clicked 
	puts "Wait a second.."
	m = Glubyr.new
	m.number = 1
	m.verbosity = 2

	results = nil
	boxText = @combo_box.active_text()
	data_type = nil
	unless boxText == nil 
		@@getterHash.values.each do |v|
			if v[:name] == boxText
				data_type = v[:type]
				break
			end
		end
		
		i = 0
		@statusbar.push(@statusbar_info,"Searching..")
		results = m.getByType(data_type,@artistEntry.text,@albumEntry.text,@titleEntry.text)
		results.each do |c|
			path = "/tmp/guilyr_item_#{i}.img" 
			m.writeFile(path,c) if c.is_image
			self.add_item(path,c, @artistEntry.text, @albumEntry.text, @titleEntry.text)
		end
	end
  end

  def on_exitButton_clicked
        puts "Gtk.main_quit"
	cleanup
  end

  def cleanup
    Gtk.main_quit()
  end

end

# Start
VR_gui.new.main()
