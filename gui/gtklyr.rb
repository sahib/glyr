require 'gtk2'

class GlyrCoverControl
	def initialize( parent )
		@artist_entry = Gtk::Entry.new
		@artist_entry.text = "[artist]"
		@artist_entry.signal_connect("key-release-event") { |w,e| entry_callback(w,e) }

		@albums_entry = Gtk::Entry.new
		@albums_entry.text = "[album]"
		@albums_entry.signal_connect("key-release-event") { |w,e| entry_callback(w,e) }

		@s_button = Gtk::Button.new("Search!")
		@s_button.signal_connect("clicked") { |w| search_callback(w) }

		@separator = Gtk::HSeparator.new
		@separator.set_size_request(450,5)

		@hbox = Gtk::HBox.new(false,0)
		@hbox.pack_start(@artist_entry, true, true, 0)
		@hbox.pack_start(@albums_entry, true, true, 0)
		@hbox.pack_start(@s_button,     true,  true, 0)

		parent.pack_start(@hbox,     false, false, 2)
		parent.pack_start(@separator,false,false,  0)
	end

	def entry_callback(widget, event)
		puts widget.text
	end

	def search_callback(widget)
	    puts "Hello again - ",widget.label,widget
	end
end

class GlyrColor
	attr_accessor :r,:g,:b
	def initialize(r,g,b) 
		@r=r
		@g=g
		@b=b
	end
end

class GlyrCoverView

	attr_accessor :color
	
	def initialize( parent )
		@border = 5
		@islots = 3
		@imgctr = 0

		# Create a new allocation, as size_request is 
		# first issued after this 
		@SC_ALLOC = Gtk::Allocation.new(0,0,0,0)

		@parent = parent
		create_scrolledwindow( @islots, @islots)		

		@image_queue = []

		artist = "equilibrium"
		album  = "sagas"

		request_sz = ((150+@border+1) * @islots)
		@sc_window.set_size_request(request_sz, request_sz )

		n = 10
		n.times do |i|
			add_image( "./nocover.png", @border, 150, 150 )
		end
	end

	def create_scrolledwindow( rows, cols )
		@sc_window = Gtk::ScrolledWindow.new(nil,nil).set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC)
		@sc_window.signal_connect("size_allocate") { |w,a| on_render_area_resize w,a }
		@table = Gtk::Table.new(rows, cols, true)
		@sc_window.add_with_viewport(@table)
		@parent.pack_start(@sc_window, true, true, 0)
	end

	def update_thumbs
		x = @imgctr % @islots
		y = @imgctr / @islots
	
		opt = Gtk::EXPAND | Gtk::FILL
		@table.attach(@image_queue[@imgctr].event_b, x, x+1, y, y+1, opt, opt, 0,0)
	end

	def add_image( path, borderwidth, width, height )
		@image_queue.push( GlyrCoverViewImage.new(path, borderwidth, width, height))
		update_thumbs
		@imgctr += 1
		nil
	end

	def on_render_area_resize(w,a)
		@SC_ALLOC = a
	end


end

class GlyrCoverViewImage < GlyrCoverView
	
	attr_accessor :event_b,:width,:height 

	def init_draw_area
	        @da = Gtk::DrawingArea.new
                @da.signal_connect("expose-event")       { |w, e| self.draw_on_expose(w, e) }

		# Encapsule da into a event box
		@event_b = Gtk::EventBox.new
		@event_b.signal_connect("button_press_event") { self.image_clicked }
		@event_b.signal_connect("enter-notify-event") { |w, e| self.on_hover(w,e) }
		@event_b.add(@da)
	end

	def on_hover(w, e)
		puts("Hovered")
	end

	
	def image_clicked 
		win = Gtk::Window.new
		win.border_width = 10
		win.title = "Guilyr"

		win.signal_connect("destroy") { cleanup }
		
		vbox = Gtk::VBox.new(false,0)

		img = Gtk::Image.new("/tmp/cover.png")
		vbox.pack_start(img,true,true,0)

		win.add(vbox)
        	win.set_window_position Gtk::Window::POS_CENTER

		win.show_all
	end

	def initialize( path, borderwidth, width, height )
		@width  = width
		@height = height
		@border_width  = borderwidth

		begin 
		    @pixbuf = Gdk::Pixbuf.new(path,width,height)
		rescue GLib::FileError 
		    puts "Unable to load image ${path}!"
		    raise
		    return
		end	

		init_draw_area	
	end

	def draw_on_expose(w,e)
		cr = @da.window.create_cairo_context 

=begin 
		@color.r = 0.0919
		@color.g = 0.5764
		@color.b = 0.8196
=end

		cr.set_source_rgba(0,0,0,1) 
		cr.rectangle(0, 0, @width + @border_width, @height + @border_width)
		cr.fill

		qsz =  5
		num = ((150) / qsz) 

		cr.set_source_pixbuf(@pixbuf,@border_width/2,@border_width/2)
		cr.paint

		cr.set_line_width(10.0)

		@da.set_size_request(@width + @border_width, @height + @border_width)
	end

end

class GlyrMainWindow
	
	attr_accessor :vbox, :window

	def initialize 
		@window = Gtk::Window.new
		@window.border_width = 10
		@window.title = "Guilyr"

		@window.signal_connect("destroy") { cleanup }
		
		@vbox = Gtk::VBox.new(false,0)

		@window.add(vbox)
		@window.set_default_size(500,550)
        	@window.set_window_position Gtk::Window::POS_CENTER

	end

	def cleanup
		Gtk::main_quit
		false
	end
end

glyrW = GlyrMainWindow.new
GlyrCoverControl.new(glyrW.vbox)
GlyrCoverView.new(glyrW.vbox)
glyrW.window.show_all

# exec
Gtk.main
