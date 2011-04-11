require 'rubygems'
require 'gtk2'
require './glubyr.rb'

# This works for the moment only for Linux 64bit out of the box,
# as glyr.so is compiled like this, you'd have to recompile it.

# Simple imageviewer that's shown when clicking on a image
class ImageViewer < Gtk::Window
     @@imageViewer_active = false
     def initialize(pixbuf)
	unless @@imageViewer_active 
		super()
		begin
		  self.add Gtk::Image.new(pixbuf)
		  self.decorated = false
		  self.resizable = false
		  self.signal_connect("leave-notify-event") { self.cleanup }
		  self.signal_connect("key-press-event")    { self.cleanup }
		  self.show_all
		  @@imageViewer_active = true
		rescue 
		  puts "Unable to show image.."
		end
	else
		puts "ImageView is already active."
	end
     end

     def cleanup
   	@@imageViewer_active = false
	self.destroy
     end
end

class ItemView < Gtk::EventBox
	@@similiar_photo_count = 0

	def initialize(path, data, artist, album, title)
		super()

		# Box containing all other widgets
		hbox = Gtk::HBox.new(false,5)
		vbox = Gtk::VBox.new(false,2)
		hbox.pack_start(vbox,true,true,0)
		self.add(hbox)
	
		# Button to save an item
		@save_button  = Gtk::Button.new("Save").set_size_request(50,30)

		# Where the item came from
		@source_label = Gtk::Label.new.set_markup("<a href='#{data.dsrc}' ><small>(Source:#{data.prov})</small></a>")
	
		# Delegate task to subroutines	
		if data.is_image
		    init_as_image(path, data, artist, album, title, vbox)
		elsif data.type == Glyr::TYPE_SIMILIAR
		    init_as_similiar_artist(path,data,artist,vbox)
		elsif data.type == Glyr::GET_TAGS or data.type == Glyr::GET_RELATIONS

		elsif data.type == Glyr::GET_ALBUMLIST or data.type == Glyr::GET_TRACKLIST

		else #lyrics,review,bio
		    init_as_text(path, data, artist, album, title, vbox) 
		end
	
		# Seperate and show
		vbox.pack_start(Gtk::HSeparator.new,false,false,2)
		hbox.show_all
		return self
	end

	def init_as_image(path, data, artist, album, title, vbox)
		pixbuf = nil
		click_box = nil
		begin
		    pixbuf = Gdk::Pixbuf.new(path)
		    image = Gtk::Image.new(pixbuf.scale(150,150)) unless pixbuf == nil
		    click_box = Gtk::EventBox.new
		    click_box.add(image)
		    click_box.signal_connect("button_press_event") { ImageViewer.new(pixbuf) }
		rescue 
		    puts "Cannot load image: '#{path}'"
		end
		internLayout = Gtk::Layout.new.set_size_request(160,170)
		internLayout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		internLayout.put(click_box,10,10) unless click_box == nil

		if data.type == Glyr::TYPE_PHOTOS
		  internLayout.put(Gtk::Label.new.set_markup("Images related to <b>#{artist}</b>"),185,30)
		else
		  internLayout.put(Gtk::Label.new.set_markup("<b>#{album}</b> <i>by</i> <b>#{artist}</b>"),185,30)
		end

	        internLayout.put(Gtk::Label.new.set_markup("<b>Size:</b> #{pixbuf.width}x#{pixbuf.height} <small>(#{data.size} Bytes)</small>"),185,50)
		internLayout.put(@source_label, 185,70)
		internLayout.put(@save_button,180,130)
		vbox.pack_start(internLayout,true,false,0)
	end

	def init_as_text(path, data, artist, album, title, vbox)
		view = Gtk::TextView.new
		view.buffer.text = data.data
		sc_win = Gtk::ScrolledWindow.new.set_size_request(100,300).add(view)
		sc_win.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC)
		vbox.pack_start(sc_win,true,true,0)
		layout = Gtk::Layout.new.set_size_request(150,30)
		layout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		vbox.pack_start(layout,false,false,0)
		layout.put(@save_button,0,0)
		layout.put(@source_label,60,10)
		layout.put(Gtk::Label.new.set_markup("<b>#{title}</b> <i>by</i> <b>#{artist}</b>"),120,10)
	end

	def load_image_from_url( url )
		q = Glyr::GlyQuery.new
		cache = Glyr::download(url,q)
		path = "/tmp/guilyr_simphoto_#{@@similiar_photo_count}.img"
		Glyr::writeFile(q,cache,path)
		@@similiar_photo_count += 1

		pixbuf = nil
		begin 
		    pixbuf = Gdk::Pixbuf.new(path)
		rescue
		    puts "moop"
		end

		return pixbuf
	end
	
	def init_as_similiar_artist(path, data, artist, vbox)
		i = 0
		infos = []
		links = []
		data.data.each_line do |line|
		    if i < 3
		         infos << line.chomp!
		    else
		         links << line.chomp!
		    end
		    i += 1
		end

		layout = Gtk::Layout.new.set_size_request(150,170)
		layout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		vbox.pack_start(layout,false,false,0)

		layout.put(Gtk::Label.new.set_markup("<big><b>#{infos[0]}</b></big>"),185,30)
		puts infos,"---",links

		# Sort by length of string, thus prefer large image(s)
		links.sort! { |a,b| b.size - a.size }

		# Download this image..
		pixbuf = load_image_from_url(links[0])
		unless pixbuf == nil
		    eb = Gtk::EventBox.new
		    im = Gtk::Image.new(pixbuf.scale(150,150))
		    eb.add(im)
	
		    # show the large image when clicking
		    eb.signal_connect("button_press_event") do
			ImageViewer.new(pixbuf)
		    end
		    layout.put(eb,10,10)
		end
		similiarity = (infos[1].to_f * 100.0).round(2)
		layout.put(Gtk::Label.new.set_markup("Similiarity: <i>#{similiarity}%</i>"),185,50)
		layout.put(@source_label,185,70)
		layout.put(Gtk::Label.new.set_markup("| <a href=\"#{infos[2]}\">last.fm page</a>"),210,70)
		layout.put(@save_button,185,120)
	end
end

class VR_gui
  @@getterHash = 
  {
	:Cover     => { :name => "Cover Art",       :type => Glyr::GET_COVER,    :active => [true, true,false]},
	:Lyrics	   => { :name => "Lyrics",          :type => Glyr::GET_LYRIC,    :active => [true, true, true]}, 
	:Photos    => { :name => "Bandphotos",      :type => Glyr::GET_PHOTO,    :active => [true,false,false]},
	:Ainfo     => { :name => "Artist Biography",:type => Glyr::GET_AINFO,    :active => [true,false,false]},
	:Similiar  => { :name => "Similiar artists",:type => Glyr::GET_SIMILIAR, :active => [true,false,false]},
	:Review    => { :name => "Albumreview",     :type => Glyr::GET_REVIEW,   :active => [true, true,false]},
	:Albumlist => { :name => "List of Albums",  :type => Glyr::GET_ALBUMLIST,:active => [true,false,false]},
	:Tracklist => { :name => "List of Tracks",  :type => Glyr::GET_TRACKLIST,:active => [true, true,false]},
	:Tags	   => { :name => "Random Tags",     :type => Glyr::GET_TAGS,     :active => [true, true, true]},
	:Relations => { :name => "Random Relations",:type => Glyr::GET_RELATIONS,:active => [true, true, true]}
  }

  def fill_combobox( box )
     @@getterHash.values.each { |subhash| box.append_text(subhash[:name]) }
     box.active = 0

     # make fields insensitive
     box.signal_connect("changed") do 
	text = box.active_text()
	@@getterHash.values.each do |sh|
	    if sh[:name] == text
		@artistEntry.sensitive = sh[:active][0]
		@albumEntry.sensitive  = sh[:active][1]
		@titleEntry.sensitive  = sh[:active][2]
        	break
	     end
	end 
     end
     return box
  end

  def initialize
    if __FILE__ == $0
      # This is just dum init code for GTK+ 
      # This script depends on XML file called main.glade where 
      # all GUI description is stored. It's expected to be in the same
      # dir as the script

      # List of items
      @itemList = []
 
      # Init Builder
      builder = Gtk::Builder.new
      builder.add_from_file("./main.glade")
      builder.connect_signals{ | handler |  method(handler) }

      @innerVBox = nil
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
	@itemList << it
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
		unless results == nil
			results.each do |c|
				path = "/tmp/guilyr_item_#{i}.img" 
				m.writeFile(path,c) if c.is_image
				self.add_item(path,c, @artistEntry.text, @albumEntry.text, @titleEntry.text)
			end
		end
	end
  end

  def cleanup_itemlist
	puts "Cleaning up.."
	@itemList.each do |it|
	    @innerVBox.remove(it)
	    it = nil
	end
	@itemList = []
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
