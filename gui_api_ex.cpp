// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*
    This is an example illustrating the use of the gui api from the dlib C++ Library.
    This is a pretty simple example.  It makes a window with a user
    defined widget (a draggable colored box) and a button.  You can drag the
    box around or click the button which increments a counter. 
*/




#include <dlib/gui_widgets.h>
#include <sstream>
#include <string>


using namespace std;
using namespace dlib;

//  ----------------------------------------------------------------------------




class color_box : public draggable 
{
    /*
        Here I am defining a custom drawable widget that is a colored box that
        you can drag around on the screen.  draggable is a special kind of drawable
        object that, as the name implies, is draggable by the user via the mouse.
        To make my color_box draggable all I need to do is inherit from draggable.
    */
    unsigned char red, green,blue;

public:
    color_box (
        drawable_window& w,
        rectangle area,
        unsigned char red_,
        unsigned char green_,
        unsigned char blue_
    ) :
        draggable(w),
        red(red_),
        green(green_),
        blue(blue_)
    {
        rect = area;
        set_draggable_area(rectangle(10, 10, GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN)));
        
        // Whenever you make your own drawable widget (or inherit from any drawable widget 
        // or interface such as draggable) you have to remember to call this function to 
        // enable the events.  The idea here is that you can perform whatever setup you 
        // need to do to get your object into a valid state without needing to worry about 
        // event handlers triggering before you are ready.
        enable_events();
    }

    ~color_box (
    )
    {
        // Disable all further events for this drawable object.  We have to do this 
        // because we don't want any events (like draw()) coming to this object while or 
        // after it has been destructed.
        disable_events();
        
        // Tell the parent window to redraw its area that previously contained this
        // drawable object.
        parent.invalidate_rectangle(rect);
    }

private:

    void draw (const canvas& c) const
    {
        // The canvas is an object that represents a part of the parent window
        // that needs to be redrawn.  

        // The first thing I usually do is check if the draw call is for part
        // of the window that overlaps with my widget.  We don't have to do this 
        // but it is usually good to do as a speed hack.  Also, the reason
        // I don't have it set to only give you draw calls when it does indeed
        // overlap is because you might want to do some drawing outside of your
        // widget's rectangle.  But usually you don't want to do that :)
        rectangle area = c.intersect(rect);
        if (area.is_empty() == true)
            return;

        // This simple widget is just going to draw a box on the screen.   
        fill_rect(c,rect,rgb_pixel(red,green,blue));
    }
};

//  ----------------------------------------------------------------------------

class win : public drawable_window
{
	/*
		Here I am going to define our window.  In general, you can define as
		many window types as you like and make as many instances of them as you want.
		In this example I am only making one though.
	*/
public:
	win() : // All widgets take their parent window as an argument to their constructor.
		c1(*this),
		c2(*this),
		c3(*this),
		c4(*this),
		c5(*this),
		b1(*this),
		b2(*this),
		b3(*this),
		b4(*this),
		b5(*this),
        mbar(*this)
    {
        // tell our button to put itself at the position (top left). 
		b1.set_pos(50 , 70);
        b1.set_name("Click Here!");

		// tell our button to put itself at the position (top right).
		b2.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120 , 70);
		b2.set_name("Click Here!");

		// tell our button to put itself at the position (bottom left). 
		b3.set_pos(50, GetSystemMetrics(SM_CYFULLSCREEN) - 70);
		b3.set_name("Click Here!");

		// tell our button to put itself at the position (bottom right). 
		b4.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, GetSystemMetrics(SM_CYFULLSCREEN) - 70);
		b4.set_name("Click Here!");

		// tell our button to put itself at the position (bottom right). 
		b5.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) / 2, GetSystemMetrics(SM_CYFULLSCREEN) / 2);
		b5.set_name("Click Here!");


        // let's put the label 5 pixels below the button
		c1.set_pos(50, 70+20);
		c2.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, 70+20);
		c3.set_pos(50, GetSystemMetrics(SM_CYFULLSCREEN) - 70+20);
		c4.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, GetSystemMetrics(SM_CYFULLSCREEN) - 70+20);
		c5.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) / 2 + 20, GetSystemMetrics(SM_CYFULLSCREEN) / 2 + 20);


        // set which function should get called when the button gets clicked.  In this case we want
        // the on_button_clicked member to be called on *this.
		b1.set_click_handler(*this, &win::on_button_clicked1);
		b2.set_click_handler(*this, &win::on_button_clicked2);
		b3.set_click_handler(*this, &win::on_button_clicked3);
		b4.set_click_handler(*this, &win::on_button_clicked4);
		b5.set_click_handler(*this, &win::on_button_clicked5);
        // Alternatively, if you have a compiler which supports the lambda functions from the
        // new C++ standard then you can use a lambda function instead of telling the click
        // handler to call one of the member functions.  So for example, you could do this
        // instead (uncomment the code if you have C++0x support):
        /*
        b.set_click_handler([&](){
                ++counter;
                ostringstream sout;
                sout << "Counter: " << counter;
                c.set_text(sout.str());
                });
        */
        // In general, all the functions which register events can take either member 
        // functions or lambda functions.

        
        // Let's also make a simple menu bar.  
        // First we say how many menus we want in our menu bar.  In this example we only want 1.
        mbar.set_number_of_menus(1);
        // Now we set the name of our menu.  The 'M' means that the M in Menu will be underlined
        // and the user will be able to select it by hitting alt+M
        mbar.set_menu_name(0,"Menu",'M');

        // Now we add some items to the menu.  Note that items in a menu are listed in the
        // order in which they were added.

        // First let's make a menu item that does the same thing as our button does when it is clicked.
        // Again, the 'C' means the C in Click is underlined in the menu. 
        mbar.menu(0).add_menu_item(menu_item_text("Click Button!",*this,&win::on_button_clicked1,'C'));
        // let's add a separator (i.e. a horizontal separating line) to the menu
        mbar.menu(0).add_menu_item(menu_item_separator());
        // Now let's make a menu item that calls show_about when the user selects it.  
        mbar.menu(0).add_menu_item(menu_item_text("About",*this,&win::show_about,'A'));


        // set the size of this window
		set_size(GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));

        counter = 0;
		buttonsize = 70;

        set_title("dlib gui example");
        show();
    } 

    ~win()
    {
        // You should always call close_window() in the destructor of window
        // objects to ensure that no events will be sent to this window while 
        // it is being destructed.  
        close_window();
    }

private:

	bool buttonIsPressed1 = false;
	bool buttonIsPressed2 = false;
	bool buttonIsPressed3 = false;
	bool buttonIsPressed4 = false;
	bool buttonIsPressed5 = false;
	

	void on_button_clicked1(
	)
	{
		// when someone clicks our button it will increment the counter and 
		// display it in our label c.
		++counter;
		ostringstream sout;
		sout << "counter: " << counter;
		c1.set_text(sout.str());
		// also when the button is pressed, the flag of flase will be 
		// changed to true
		buttonIsPressed1 = true;
    }

	void on_button_clicked2(
	)
	{
		// when someone clicks our button it will increment the counter and 
		// display it in our label c.
		++counter;
		ostringstream sout;
		sout << "counter: " << counter;
		c2.set_text(sout.str());
		// also when the button is pressed, the flag of flase will be 
		// changed to true
		buttonIsPressed2 = true;
	}

	void on_button_clicked3(
	)
	{
		// when someone clicks our button it will increment the counter and 
		// display it in our label c.
		++counter;
		ostringstream sout;
		sout << "counter: " << counter;
		c3.set_text(sout.str());
		// also when the button is pressed, the flag of flase will be 
		// changed to true
		buttonIsPressed3 = true;
	}

	void on_button_clicked4(
	)
	{
		// when someone clicks our button it will increment the counter and 
		// display it in our label c.
		++counter;
		ostringstream sout;
		sout << "counter: " << counter;
		c4.set_text(sout.str());
		// also when the button is pressed, the flag of flase will be 
		// changed to true
		buttonIsPressed4 = true;
	}

	void on_button_clicked5(
	)
	{
		// when someone clicks our button it will increment the counter and 
		// display it in our label c.
		++counter;
		ostringstream sout;
		sout << "counter: " << counter;
		c5.set_text(sout.str());
		// also when the button is pressed, the flag of flase will be 
		// changed to true
		buttonIsPressed5 = true;
	}

    void show_about()
    {
        message_box("About","This is a dlib gui example program");
    }

    unsigned long counter;
	int buttonsize;

    label c1;
	label c2;
	label c3;
	label c4;
	label c5;
    button b1;
	button b2;
	button b3;
	button b4;
	button b5;
    menu_bar mbar;




	void terminate_win()
	{
		if (counter >= 5 && buttonIsPressed1 && buttonIsPressed2 && buttonIsPressed3 && buttonIsPressed4 && buttonIsPressed5)
		{
			close_window();
		}
	}
};


//  ----------------------------------------------------------------------------

int main()
{
	// create our window
	win my_window;


    // wait until the user closes this window before we let the program 
    // terminate.
    my_window.wait_until_closed();




    return 0;
}

//  ----------------------------------------------------------------------------

// Normally, if you built this application on MS Windows in Visual Studio you
// would see a black console window pop up when you ran it.  The following
// #pragma directives tell Visual Studio to not include a console window along
// with your application.  However, if you prefer to have the console pop up as
// well then simply remove these #pragma statements.
#ifdef _MSC_VER
#   pragma comment( linker, "/entry:mainCRTStartup" )
#   pragma comment( linker, "/SUBSYSTEM:WINDOWS" )
#endif
