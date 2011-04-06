require 'rubygems'
require 'gtk2'

class ItemView < Gtk::EventBox
	def initialize(path)
		super()
		@internLayout = Gtk::Layout.new
		self.add(@internLayout)

		begin
		    @image = Gtk::Image.new( Gdk::Pixbuf.new(path,150,150) )
		rescue 
		    puts "Cant load file"
		end
		@internLayout.put(@image,10,10)
		@internLayout.put(Gtk::Label.new(path),160,30)
		@internLayout.set_size_request(400,170)
		return self
	end
end

class VR_gui
  def fill_combobox( box )
     box.append_text("cover")
     box.append_text("lyrics")
     box.append_text("photos")
     box.append_text("ainfo")
     box.append_text("similiar")
     box.append_text("review")
     return box
  end
  def initialize
    if __FILE__ == $0
      builder = Gtk::Builder.new
      builder.add_from_file("swig/ruby/main.glade")
      builder.connect_signals{ | handler |  method(handler) }
      @innerVBox = Gtk::VBox.new(false,2)
      @sWin = builder.get_object("sWin")
      @sWin.add_with_viewport(@innerVBox)

      @statusbar = builder.get_object("statusbar")
      @statusbar.push( @statusbar.get_context_id("Info"), "This is not supposed to be used yet." )

      
      combo_box = fill_combobox(Gtk::ComboBox.new(true))
      @mVBox = builder.get_object("hbox4")
      @mVBox.pack_start(combo_box,true,true,2)
      @mVBox.show_all
    end
  end
  def add_item( path )
	it = ItemView.new(path)
	@innerVBox.pack_start(it,true,false,0)
	@innerVBox.show_all
  end

  def main
       Gtk.main()
  end

  def on_searchButton_clicked 
	puts "Wait a second.."
  end

  def on_exitButton_clicked
        puts "Gtk.main_quit"
	cleanup
  end

  def cleanup
    Gtk.main_quit()
  end

end


x = VR_gui.new
puts "--Init--"
28.times do |i|
   puts c_path = "/tmp/Devildriver_Beast_cover_#{i}.jpg"
   x.add_item(c_path)
end

x.main()
