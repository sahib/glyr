require 'rubygems'
require 'gtk2'

# Use glyr's ruby module
require '../swig/ruby/glyros.rb'

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

	def get_new_layout(x,y)
		layout = Gtk::Layout.new.set_size_request(x,y)
		layout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		return layout
	end

	def initialize(path, data, artist, album, title)
		super()

		# Box containing all other widgets
		hbox = Gtk::HBox.new(false,5)
		vbox = Gtk::VBox.new(true,2)
		hbox.pack_start(vbox,true,true,0)
		self.add(hbox)
	
		# Button to save an item
		@save_button  = Gtk::Button.new("Save").set_size_request(50,30)
		@save_button.signal_connect("clicked") do
		  dialog = Gtk::FileChooserDialog.new("Save File",nil,Gtk::FileChooser::ACTION_SAVE,nil,
						      [Gtk::Stock::CANCEL, Gtk::Dialog::RESPONSE_CANCEL],
						      [Gtk::Stock::SAVE, Gtk::Dialog::RESPONSE_ACCEPT])
		  
		  if dialog.run == Gtk::Dialog::RESPONSE_ACCEPT
		    puts "filename = #{dialog.filename}"
        data.write_file(filename)
		  end
		  dialog.destroy
		end

		# Where the item came from
		unless data.dsrc == nil
		    @source_label = Gtk::Label.new.set_markup("<a href='#{data.dsrc.gsub(/&/,"&amp;")}'><small>(Source:#{data.prov})</small></a>")
		end
	
		# Delegate task to subroutines	
		if data.is_image
		    init_as_image(path, data, artist, album, title, vbox)
		elsif data.type == Glyros::TYPE_SIMILIAR
		    init_as_similiar_artist(path,data,artist,vbox)
		elsif data.type == Glyros::TYPE_TAGS or data.type == Glyros::TYPE_RELATION
		    init_as_oneliner(data,artist,album,vbox)
		elsif data.type == Glyros::TYPE_TRACK or data.type == Glyros::TYPE_ALBUMLIST
		    init_as_oneliner(data,artist,album,vbox)
		else #lyrics,review,bio
		    init_as_text(path, data, artist, album, title, vbox) 
		end
	
		# register some signals on the whole area:
		self.signal_connect("key-press-event") do
			puts "Clicked"
		end

		# Seperate and show
		vbox.pack_start(Gtk::HSeparator.new,false,false,1)
		hbox.show_all
		return self
	end

	def init_as_oneliner(data, artist, album, vbox)
		lo = get_new_layout(150,30)
		label = nil
		if data.type == Glyros::TYPE_TRACK
		    label = Gtk::Label.new.set_markup("#{data.data} <small>#{data.duration / 60}:#{data.duration % 60}</small>")
		elsif data.type == Glyros::TYPE_ALBUMLIST
		    label = Gtk::Label.new.set_markup("#{data.data}")
		elsif data.type == Glyros::TYPE_TAGS
		    label = Gtk::Label.new.set_markup("#{data.data}")
		elsif data.type == Glyros::TYPE_RELATION
		    splitted = data.data.split(":")
		    typename = splitted.shift
		    label = Gtk::Label.new.set_markup("<a href='#{splitted.join.gsub(/&/,"&amp;")}'>#{typename}</a>")
		end

		lo.put(label,0,0)
		lo.put(@source_label,350,0)
		vbox.pack_start(lo,true,false,0)
	end

	def bytes_to_human_readable( bytes = 0)
		quad = (1024 ** 2).to_f
		if bytes >= (quad) / 2
		     return "#{(bytes / (quad)).round 2} MB"
		elsif bytes >= 1024 / 2
		     return "#{(bytes / (1024.0)).round 2} KB"
		else
		     return "#{bytes} Bytes"
		end
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

		if data.type == Glyros::TYPE_PHOTOS
		  internLayout.put(Gtk::Label.new.set_markup("Images related to <b>#{artist}</b>"),185,30)
		else
		  internLayout.put(Gtk::Label.new.set_markup("<b>#{album}</b> <i>by</i> <b>#{artist}</b>"),185,30)
		end

	        internLayout.put(Gtk::Label.new.set_markup("<b>Size:</b> #{pixbuf.width}x#{pixbuf.height} <small>(#{bytes_to_human_readable data.size})</small>"),185,50)
		internLayout.put(@source_label, 185,70)
		internLayout.put(@save_button,180,130)
		vbox.pack_start(internLayout,true,true,0)
	end

	def init_as_text(path, data, artist, album, title, vbox)
		view = Gtk::TextView.new
		view.buffer.text = data.data
		sc_win = Gtk::ScrolledWindow.new.set_size_request(100,300).add(view)
		sc_win.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC)
		text_vbox = Gtk::VBox.new(false,1)
		text_vbox.pack_start(sc_win,true,true,0)
		layout = Gtk::Layout.new.set_size_request(150,30)
		layout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		text_vbox.pack_start(layout,false,false,0)
		layout.put(@save_button,0,0)
		layout.put(@source_label,60,10)

		info_label = nil
		if Glyros::TYPE_LYRICS
		    layout.put(Gtk::Label.new.set_markup("<b>#{title}</b> <i>by</i> <b>#{artist}</b>"),200,10)
		else
		    info_label = Gtk::Label.new.set_markup("<small>Review of album</small><b>#{album}</b><i>by</i><b>#{album}</b>")
		end

		vbox.add(text_vbox)
	end

	def load_image_from_url( url )
		q = Glyros::GlyQuery.new
		cache = Glyros::download(url,q)
		path = "/tmp/guilyr_simphoto_#{@@similiar_photo_count}.img"
    cache.write_file(path)
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
		# Sort by length of string, thus prefer large image(s)
		links.sort! { |a,b| b.size - a.size }

		layout = Gtk::Layout.new.set_size_request(150,170)
		layout.modify_bg(Gtk::STATE_NORMAL,Gdk::Color.parse("white"))
		layout.put(Gtk::Label.new.set_markup("<big><b>#{infos[0]}</b></big>"),185,30)
		vbox.pack_start(layout,false,false,0)

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
		layout.put(Gtk::Label.new.set_markup("<small><a href='#{infos[2]}'>last.fm page</a> | </small>"),185,70)
		layout.put(@source_label,270,70)
		layout.put(@save_button,185,100)
	end
end

class VR_gui
  @@getterHash = 
  {
	:Cover     => { :name => "Cover Art",       :type => Glyros::GET_COVERART, :active => [true, true,false]},
	:Lyrics	   => { :name => "Lyrics",          :type => Glyros::GET_LYRICS,    :active => [true, true, true]}, 
	:Photos    => { :name => "Bandphotos",      :type => Glyros::GET_ARTIST_PHOTOS,    :active => [true,false,false]},
	:Ainfo     => { :name => "Artist Biography",:type => Glyros::GET_ARTISTBIO,    :active => [true,false,false]},
	:Similiar  => { :name => "Similiar artists",:type => Glyros::GET_SIMILIAR_ARTISTS, :active => [true,false,false]},
	:Review    => { :name => "Albumreview",     :type => Glyros::GET_ALBUM_REVIEW,   :active => [true, true,false]},
	:Albumlist => { :name => "List of Albums",  :type => Glyros::GET_ALBUMLIST,:active => [true,false,false]},
	:Tracklist => { :name => "List of Tracks",  :type => Glyros::GET_TRACKLIST,:active => [true, true,false]},
	:Tags	   => { :name => "Random Tags",     :type => Glyros::GET_TAGS,     :active => [true, true, true]},
	:Relations => { :name => "Random Relations",:type => Glyros::GET_RELATIONS,:active => [true, true, true]}
  }

  def fill_combobox( box )
     @@getterHash.values.each { |subhash| box.append_text(subhash[:name]) }

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
     box.active = 0
     return box
  end

  def get_abtname( type )
      case type 
	when 0 
	    @artistEntry.text = (`mpc current -f "%artist%"`).chomp
	when 1 
	    @albumEntry.text = (`mpc current -f "%album%"`).chomp
	when 2 
	    @titleEntry.text = (`mpc current -f "%title%"`).chomp
	else puts "w00t?"
      end
  end
 
  def register_autocomplete_callback( builder )
      builder.get_object("evb_artist").signal_connect("button-press-event") { |w,e| get_abtname 0 }
      builder.get_object("evb_album" ).signal_connect("button-press-event") { |w,e| get_abtname 1 }
      builder.get_object("evb_title" ).signal_connect("button-press-event") { |w,e| get_abtname 2 }
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

      # Pack the rest and make sure everything is shown
      @mVBox = builder.get_object("hbox4")
      # Fill the entry variables
      @artistEntry = builder.get_object("artistEntry") 
      @albumEntry  = builder.get_object("albumEntry")      
      @titleEntry  = builder.get_object("titleEntry")

      # Fill the comboxbox with all chocies
      @combo_box = fill_combobox(Gtk::ComboBox.new(true))
      @mVBox.pack_start(@combo_box,true,true,2)
      @mVBox.show_all

      # get automated mpc
      register_autocomplete_callback(builder)

      # Settingscontrol
      @sbp_number = builder.get_object("sbp_number")
      @sbp_number.adjustment = Gtk::Adjustment.new(50.0, 0.0, 100.0, 1.0, 5.0, 0.0)
      @sbp_number.value = 1
      @sbp_fuzzyness = builder.get_object("sbp_fuzzyness")
      @sbp_fuzzyness.adjustment = Gtk::Adjustment.new(50.0, 0.0, 100.0, 1.0, 5.0, 0.0)
      @sbp_fuzzyness.value = Glyros::DEFAULT_FUZZYNESS
      @provider_entry = builder.get_object("provider_entry")
      @lang_entry = builder.get_object("lang_entry")
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
	m = GlyrosSpit.new
	m.number = @sbp_number.value.to_i
	m.fuzzyness = @sbp_fuzzyness.value.to_i
	m.verbosity = 2
	
	prov_string = @provider_entry.text
	unless prov_string.size == 0
	    puts prov_string
	    m.from = prov_string 
	else
	    m.from = "all"
	end

	lang_string = @lang_entry.text
	unless lang_string.size == 0
	    m.lang = lang_string
	else
	    m.lang = "en"
	end

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

                m.artist = @artistEntry.text
                m.album  = @albumEntry.text
                m.title  = @titleEntry.text
                results  = m.get(data_type)

		unless results.size == 0
			results.each do |c|
				path = "/tmp/guilyr_item_#{i}.img"
        c.write_file(path) if c.is_image 
				self.add_item(path,c, @artistEntry.text, @albumEntry.text, @titleEntry.text)
			end
		        @statusbar.push(@statusbar_info,"Done: found #{results.size} item(s)")
		else
			@statusbar.push(@statusbar_info,"Done: nothing found")
		end
	end
	m = nil
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
    return false
  end

end

# Start
VR_gui.new.main()
