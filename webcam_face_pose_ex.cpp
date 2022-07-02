// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt

/*



    This example program shows how to find frontal human faces in an image and

    estimate their pose.  The pose takes the form of 68 landmarks.  These are

    points on the face such as the corners of the mouth, along the eyebrows, on

    the eyes, and so forth.  

    



    This example is essentially just a version of the face_landmark_detection_ex.cpp

    example modified to use OpenCV's VideoCapture object to read from a camera instead 

    of files.





    Finally, note that the face detector is fastest when compiled with at least

    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD

    chip then you should enable at least SSE2 instructions.  If you are using

    cmake to compile this program you can enable them by using one of the

    following commands when you create the build project:

        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON

        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON

        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON

    This will set the appropriate compiler options for GCC, clang, Visual

    Studio, or the Intel compiler.  If you are using another compiler then you

    need to consult your compiler's manual to determine how to enable these

    instructions.  Note that AVX is the fastest but requires a CPU from at least

    2011.  SSE4 is the next fastest and is supported by most current machines.  

*/



#include <dlib/opencv.h>

#include <opencv2/highgui/highgui.hpp>

#include <dlib/image_processing/frontal_face_detector.h>

#include <dlib/image_processing/render_face_detections.h>

#include <dlib/image_processing.h>

#include <dlib/gui_widgets.h>

#include <iostream>

#include <string>

#include <opencv2/opencv.hpp>

#include <opencv2/videoio.hpp>

#include <opencv2/highgui.hpp>

#include <opencv2/imgproc.hpp>

#include <opencv2/core/matx.hpp>

#include <dlib/optimization.h>



using namespace dlib;

using namespace std;



//berechnet die gr��te Kontur eines eingegebenen Konturenvektors

int getmax(std::vector<std::vector<cv::Point>> contours) {

	double maxArea = 0;

	int maxAreaID = -1;

	for (int i = 0; i < contours.size(); i++) {

		double newArea = cv::contourArea(contours.at(i));

		if (newArea > maxArea) {

			maxArea = newArea;

			maxAreaID = i;

		}

	}

	return maxAreaID;

}



class color_box : public draggable

{

	/*

		Here I am defining a custom drawable widget that is a colored box that

		you can drag around on the screen.  draggable is a special kind of drawable

		object that, as the name implies, is draggable by the user via the mouse.

		To make my color_box draggable all I need to do is inherit from draggable.

	*/

	unsigned char red, green, blue;



public:

	color_box(

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



	~color_box(

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



	void draw(const canvas& c) const

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

		fill_rect(c, rect, rgb_pixel(red, green, blue));

	}

};



//screen = calibration

class screen : public drawable_window

{

	/*

		Here I am going to define our window.  In general, you can define as

		many window types as you like and make as many instances of them as you want.

		In this example I am only making one though.

	*/

public:

	screen() : // All widgets take their parent window as an argument to their constructor.

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

		b1.set_pos(50, 70);

		b1.set_name("Click Here!");



		// tell our button to put itself at the position (top right).

		b2.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, 70);

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

		c1.set_pos(50, 70 + 20);

		c2.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, 70 + 20);

		c3.set_pos(50, GetSystemMetrics(SM_CYFULLSCREEN) - 70 + 20);

		c4.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) - 120, GetSystemMetrics(SM_CYFULLSCREEN) - 70 + 20);

		c5.set_pos(GetSystemMetrics(SM_CXFULLSCREEN) / 2 + 20, GetSystemMetrics(SM_CYFULLSCREEN) / 2 + 20);





		// set which function should get called when the button gets clicked.  In this case we want

		// the on_button_clicked member to be called on *this.

		b1.set_click_handler(*this, &screen::on_button_clicked1);

		b2.set_click_handler(*this, &screen::on_button_clicked2);

		b3.set_click_handler(*this, &screen::on_button_clicked3);

		b4.set_click_handler(*this, &screen::on_button_clicked4);

		b5.set_click_handler(*this, &screen::on_button_clicked5);

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

		mbar.set_menu_name(0, "Menu", 'M');



		// Now we add some items to the menu.  Note that items in a menu are listed in the

		// order in which they were added.



		// First let's make a menu item that does the same thing as our button does when it is clicked.

		// Again, the 'C' means the C in Click is underlined in the menu. 

		mbar.menu(0).add_menu_item(menu_item_text("Click Button!", *this, &screen::on_button_clicked1, 'C'));

		// let's add a separator (i.e. a horizontal separating line) to the menu

		mbar.menu(0).add_menu_item(menu_item_separator());

		// Now let's make a menu item that calls show_about when the user selects it.  

		mbar.menu(0).add_menu_item(menu_item_text("About", *this, &screen::show_about, 'A'));





		// set the size of this window

		set_size(GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));



		counter = 0;

		buttonsize = 70;



		set_title("dlib gui example");

		show();

	}



	

	~screen()

	{

		// You should always call close_window() in the destructor of window

		// objects to ensure that no events will be sent to this window while 

		// it is being destructed.  

		close_window();

	}

	



//private:
	public:



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



	private:

	void show_about()

	{

		message_box("About", "This is a dlib gui example program");

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







	

	void terminate_screen()

	{

		if (counter >= 5 && buttonIsPressed1 && buttonIsPressed2 && buttonIsPressed3 && buttonIsPressed4 && buttonIsPressed5)

		{

			close_window();

		}

	}

	

};









//berechnung der Distanzen

float iris_distance;

float right_eye_nose_distance;

float left_eye_nose_distance;
// eine Funktion calculation die die Distanzen von Linkes_Auge - Rechtes_Auge, Rechtes_Auge - Nase und Linkes_Auge - Nase ermittelt
void calculation(float p_r_edge_right_x, float p_r_edge_left_x, float p_l_edge_right_x, float p_l_edge_left_x, float p_r_edge_right_y,
	float p_r_edge_left_y, float p_l_edge_right_y, float p_l_edge_left_y, float nose_x, float nose_y) {
	// jeweils für Auge Links und rechts das zentrum (nicht die die werte der Pupillen detection!!)
	float p_r_edge_right_left_x = (p_r_edge_right_x + p_r_edge_left_x) / 2;
	float p_r_edge_right_left_y = (p_r_edge_right_y + p_r_edge_left_y) / 2;

	float p_l_edge_right_left_x = (p_l_edge_right_x + p_l_edge_left_x) / 2;
	float p_l_edge_right_left_y = (p_l_edge_right_y + p_l_edge_left_y) / 2;

	// Berechnung der Distanz Linkes_Auge - Rechtes_Auge 

	iris_distance = hypot(p_r_edge_right_left_x - p_l_edge_right_left_x, p_r_edge_right_left_y - p_l_edge_right_left_y);

	// Berechnung der Distanz von (Rechtes)Auge zu Nase

	right_eye_nose_distance = hypot(p_r_edge_right_left_x - nose_x, p_r_edge_right_left_y - nose_y);

	// Berechnung der Distanz von (Linkes)Auge zu Nase

	left_eye_nose_distance = hypot(p_l_edge_right_left_x - nose_x, p_l_edge_right_left_y - nose_y);

}

// Eine Funktion um Array's mit den Distanzen und Pupillen zu füllen
void fill_Array(float Array[], float pr_x, float pr_y, float pl_x, float pl_y) {

	Array[0] = iris_distance;

	Array[1] = right_eye_nose_distance;

	Array[2] = left_eye_nose_distance;

	Array[3] = pr_x;

	Array[4] = pr_y;

	Array[5] = pl_x;

	Array[6] = pl_y;

}

// Deklaration der punkte für die Pupille Rechts und Links

cv::Point p_l;

cv::Point p_r;



//Matritzen für die Least squares fit funktion 

typedef matrix<double, 3, 1> input_vector;

typedef matrix<double, 4, 1> parameter_vector;







//Auskommentieren, da aktuell nicht benutzt



//------------------------------------------------------------

//einf�gen der Funktionen der Least_quares_ex





//Angepasst an 3 Input werte und 4 Parameter !!!

/*

// We will use this function to generate data.  It represents a function of 3 variables

// and 4 parameters.   The least squares procedure will be used to infer the values of 

// the 3 parameters based on a set of input/output pairs.

double model(

	const input_vector& input,

	const parameter_vector& params

)

{

	const double p0 = params(0);

	const double p1 = params(1);

	const double p2 = params(2);

	const double p3 = params(3);





	const double i0 = input(0);

	const double i1 = input(1);

	const double i2 = input(2);





	const double temp = p0 * i0 + p1 * i1 + p2 * i2 + p3;



	return temp * temp;

}

*/

/*

//Ebenfalls angepasst an mehr werte

// This function is the "residual" for a least squares problem.   It takes an input/output

// pair and compares it to the output of our model and returns the amount of error.  The idea

// is to find the set of parameters which makes the residual small on all the data pairs.

double residual(

	const std::pair<input_vector, double>& data,

	const parameter_vector& params

)

{

	return model(data.first, params) - data.second;

}





//Ebenfalls angepasst an mehr werte



// This function is the derivative of the residual() function with respect to the parameters.

parameter_vector residual_derivative(

	const std::pair<input_vector, double>& data,

	const parameter_vector& params

)

{

	parameter_vector der;



	const double p0 = params(0);

	const double p1 = params(1);

	const double p2 = params(2);

	const double p3 = params(3);



	const double i0 = data.first(0);

	const double i1 = data.first(1);

	const double i2 = data.first(2);





	const double temp = p0 * i0 + p1 * i1 + p2 * i2 + p3;



	der(0) = i0 * 2 * temp;

	der(1) = i1 * 2 * temp;

	der(2) = i2 * 2 * temp;

	der(3) = 2 * temp;



	return der;

}



//------------------------------------------------------------------

*/





int main()

{

    try

    {

        cv::VideoCapture cap(0);

        if (!cap.isOpened())

        {

            cerr << "Unable to connect to camera" << endl;

            return 1;

        }


		// erstelle ein Fenster für die Abbildung der Kamera und den Landmarks bzw. Pupillendetektion
        image_window win;



        // Load face detection and pose estimation models.
		// Laden des Gesichts Detektion und der Richtung

        frontal_face_detector detector = get_frontal_face_detector();

        shape_predictor pose_model;

        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;



		

		//Ein Fenster für die anzeige der Buttons, um die Kalibration durchzuführen 

		screen My_screen;



		//Declarieren von Arrays um die die berechneten werte zu speichern (Distanzen und Pupillen Detektion) 

		float calib_array_1[7];

		float calib_array_2[7];

		float calib_array_3[7];

		float calib_array_4[7];

		float calib_array_5[7];



		// Declarieren von Button Arrays (speichern Positionen der einzelnen Buttons)

		float button_array1[2];

		float button_array2[2];

		float button_array3[2];

		float button_array4[2];

		float button_array5[2];

		



		//calib counter um zu testen das 5 Buttons bet�tigt werden

		int calib = 0;





		//F�r die Least_Squares Fkt.

		//std::vector<std::pair<input_vector, double> > data_samples;

		//std::vector<std::pair<input_vector, double> > data_samples2;



		//zwischenspeichern der Werte beim dr�cken des Buttons

		input_vector input;

		input_vector input2;

		input_vector input3;

		input_vector input4;

		input_vector input5;

		







		//Vektoren mit alpha und beta Werten

		parameter_vector x;

		x = 1;

		parameter_vector y;

		y = 1;

		





        // Grab and process frames until the main window is closed by the user.

        while(!win.is_closed())

        {

            // Grab a frame

            cv::Mat temp;

			

			

            if (!cap.read(temp))

            {

                break;

            }

            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just

            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as

            // long as temp is valid.  Also don't do anything to temp that would cause it

            // to reallocate the memory which stores the image as that will make cimg

            // contain dangling pointers.  This basically means you shouldn't modify temp

            // while using cimg.

            cv_image<bgr_pixel> cimg(temp);



			//kopie des Bildes anlegen, da man temp nicht ver�ndern sollte (siehe Kommentar dar�ber)

			cv::Mat img_copy = temp;



            // Detect faces 

            std::vector<rectangle> faces = detector(cimg);

            // Find the pose of each face.

            std::vector<full_object_detection> shapes;

			for (unsigned long i = 0; i < faces.size(); ++i) {

				shapes.push_back(pose_model(cimg, faces[i]));



				//Auf Landmark zugreifen

				full_object_detection shape = pose_model(cimg, faces[i]);

				//cout << "pixel position of first part:  " << shape.part(37) << endl;

				

				//Auf Farbwert zugreifen

				//cv::Vec3b pixel = temp.at<cv::Vec3b>(cv::Point(shape.part(37).x(), shape.part(37).y()));

				//int b = pixel[0];

				

				//Schwarz Wei� Bild anlegen

				cv::Mat img_copy_sw;

				cv::cvtColor(img_copy, img_copy_sw, CV_BGR2GRAY);





				



				//Augen werden getrennt bearbeitet



				//linkes Auge



				//Hier testen, ob shape-part noch im Bild ist (kann �ber Bildrand ragen und ergibt dadurch dann Fehlermeldung)

				if (shape.part(36).x() > 0 && shape.part(37).y() > 0) {

					//test ob linkes Auge offen ist, wenn ja, dann Pupille berechnen, sonst nicht

					if ((((shape.part(36).y() - shape.part(37).y()) > 3) && ((shape.part(41).y() - shape.part(36).y()) > 3)) || (((shape.part(39).y() - shape.part(38).y()) > 7))) {

						

						//Einen Kasten um das Auge rum bilden anhand der Landmarks

						//Eckpunkte des linken Auge

						int left_corner_left_eye_x  = shape.part(36).x();

						int left_corner_left_eye_y  = shape.part(37).y();

						int right_corner_left_eye_x = shape.part(39).x();

						int right_corner_left_eye_y = shape.part(40).y();



						//Breite und H�he des Kastens des linken Auges

						int width_left_eye = right_corner_left_eye_x - left_corner_left_eye_x;

						int height_left_eye = right_corner_left_eye_y - left_corner_left_eye_y;



						//Den linken Kasten als Rechteck definieren

						cv::Rect roi_left(left_corner_left_eye_x, left_corner_left_eye_y, width_left_eye, height_left_eye);



						//Kasten als eigenes Bilder abspeichern

						cv::Mat img_eye_left = img_copy_sw(roi_left);

						

						/*Histogrammequalisation

						Hier k�nnte man auch noch die Helligkeit ver�ndern

						und nochmal eine �qualisation, macht bei meinen Augen

						aber keinen Unterschied, bei euren??*/



						// linkes Auge

						//Histogrammequalisation

						cv::equalizeHist(img_eye_left, img_eye_left);



						/*Helligkeit runterschrauben

						img_eye_left.convertTo(img_eye_left, -1, 1, -70);

						Nochmal Hist.Equ.

						cv::equalizeHist(img_eye_left, img_eye_left);*/



						//Diesen ausschnitt nochmals als extrabild speichern

						cv::Mat img_eye_left_copy = img_eye_left;



						// Durschnittswert(Grauwert) des Kastens berechnen f�r den Threshold

						cv::Scalar mean_left = cv::mean(img_eye_left);

						float threshhold_left = mean_left.val[0];



						// inversen threshhold anwenden, also hier die Pixel auf dem Bild wei� machen, welche 

						//�ber dem Durschnittswert liegen, die restlichen Pixel schwarz (Binary Threshhold)

						threshold(img_eye_left_copy, img_eye_left_copy, threshhold_left, threshhold_left, cv::THRESH_BINARY_INV);



						//linkes Auge morphologische Operatoren (Dienen dazu, den erkannten bereich besser auf die Iris einzuschr�nken,

						//Pixel welche nicht zu der Iris geh�ren werden hier teilweise entfernt durch die Operatoren (Typischer Bildver-

						//arbeitungs operator))

						//morp. close

						cv::morphologyEx(img_eye_left_copy, img_eye_left_copy, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

						//morp. open

						cv::morphologyEx(img_eye_left_copy, img_eye_left_copy, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

						//morp. close

						cv::morphologyEx(img_eye_left_copy, img_eye_left_copy, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));



						//rechteck ausschneiden wobei jeweils ein pixel am rand entfernt wird

						cv::Rect roi_l(1, 1, width_left_eye - 2, height_left_eye - 2);



						//Diesen Ausschnitt als eigenes Bild abspeichern:

						cv::Mat img_left;

						img_left = img_eye_left_copy(roi_l);



						//Vektor um Konturen zu speichern

						std::vector<std::vector<cv::Point>> contours_l;



						//Konturen finden mit eingebauter Funktion

						cv::findContours(img_left, contours_l, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);



						//Moments anwenden, dadurch kann man center extrahieren

						cv::Moments M_l = cv::moments(contours_l[getmax(contours_l)]);



						//Mittelpunkt der gr��ten Kontur (Iris) mithilfe von Moments herausfinden

						cv::Point p_l2(M_l.m10 / M_l.m00, M_l.m01 / M_l.m00);

						p_l = p_l2;

						

						//Auf Bild die beiden Punkte abbilden

						img_copy.at<cv::Vec3b>(left_corner_left_eye_y + 2 + p_l.y, left_corner_left_eye_x + 2 + p_l.x) = (0, 0, 255);

						cv::Point p_l3(left_corner_left_eye_y + 2 + p_l.y, left_corner_left_eye_x + 2 + p_l.x);

						p_l = p_l3;

					}

				}



				

				//rechtes Auge



				//Hier testen, ob shape-part noch im Bild ist (kann �ber Bildrand ragen und ergibt dadurch dann Fehlermeldung)

				if (shape.part(45).x() < 640 && shape.part(44).y() > 0) {

					//test ob rechte Auge offen ist, wenn ja, dann Pupille berechnen, sonst nicht

					if ((((shape.part(45).y() - shape.part(44).y()) > 3) && ((shape.part(46).y() - shape.part(45).y()) > 3)) || (((shape.part(42).y() - shape.part(43).y()) > 7))) {	

						

						//Eckpunkte des rechten Auges

						int left_corner_right_eye_x  = shape.part(42).x();

						int left_corner_right_eye_y  = shape.part(43).y();

						int right_corner_right_eye_x = shape.part(45).x();

						int right_corner_right_eye_y = shape.part(46).y();



						//Breite und H�he des Kastens des rechten Auges

						int width_right_eye = right_corner_right_eye_x - left_corner_right_eye_x;

						int height_right_eye = right_corner_right_eye_y - left_corner_right_eye_y;



						//Den rechten Kasten als Rechteck definieren

						cv::Rect roi_right(left_corner_right_eye_x, left_corner_right_eye_y, width_right_eye, height_right_eye);



						//Kasten als eigenes Bilder abspeichern

						cv::Mat img_eye_right = img_copy_sw(roi_right);



						/*Histogrammequalisation

						Hier k�nnte man auch noch die Helligkeit ver�ndern

						und nochmal eine �qualisation, macht bei meinen Augen

						aber keinen Unterschied, bei euren??*/



						//Histogrammequalisation

						cv::equalizeHist(img_eye_right, img_eye_right);

						//img_roi_right.convertTo(img_roi_right, -1, 1, -70);

						//cv::equalizeHist(img_roi_right, img_roi_right);



						//Diesen ausschnitt nochmals als extrabild speichern

						cv::Mat img_eye_right_copy = img_eye_right;



						// Durschnittswert(Grauwert) des Kastens berechnen f�r den Threshold

						cv::Scalar mean_right = cv::mean(img_eye_right);

						float threshhold_right = mean_right.val[0];



						// inversen threshhold anwenden

						threshold(img_eye_right_copy, img_eye_right_copy, threshhold_right, threshhold_right, cv::THRESH_BINARY_INV);



						//rechtes Auge Morphologische Operatoren

						//morp. close

						cv::morphologyEx(img_eye_right_copy, img_eye_right_copy, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

						//morp.open

						cv::morphologyEx(img_eye_right_copy, img_eye_right_copy, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

						//morp.close

						cv::morphologyEx(img_eye_right_copy, img_eye_right_copy, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));



						//rechteck ausschneiden wobei jeweils ein pixel am rand entfernt wird

						cv::Rect roi_r(1, 1, width_right_eye - 2, height_right_eye - 2);



						//Diesen Ausschnitt als eigenes Bild abspeichern:

						cv::Mat img_right;

						img_right = img_eye_right_copy(roi_r);



						//Vektor um Konturen zu speichern

						std::vector<std::vector<cv::Point>> contours_r;



						//Konturen finden mit eingebauter Funktion

						cv::findContours(img_right, contours_r, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);



						//Moments anwenden, dadurch kann man center extrahieren

						cv::Moments M_r = cv::moments(contours_r[getmax(contours_r)]);



						//Mittelpunkt der gr��ten Kontur (Iris) mithilfe von Moments herausfinden

						cv::Point p_r2(M_r.m10 / M_r.m00, M_r.m01 / M_r.m00);

						p_r = p_r2;



						//Auf Bild die beiden Punkte abbilden

						img_copy.at<cv::Vec3b>(left_corner_right_eye_y + 2 + p_r.y, left_corner_right_eye_x + 2 + p_r.x) = (0, 0, 255);

						cv::Point p_r3(left_corner_right_eye_y + 2 + p_r.y, left_corner_right_eye_x + 2 + p_r.x);

						p_r = p_r3;



					}

				}



				//Kalibration



				// Landmark der Nase

				float nose_x = shape.part(34).x();

				float nose_y = shape.part(34).y();

				// Landmark des rechten Auge -> rechte Ecke

				float p_r_edge_right_x = shape.part(46).x();

				float p_r_edge_right_y = shape.part(46).y();



				// Landmark des rechten Auge -> linke Ecke

				float p_r_edge_left_x = shape.part(43).x();

				float p_r_edge_left_y = shape.part(43).y();

				// Landmark des linkes Auge -> rechte Ecke

				float p_l_edge_right_x = shape.part(40).x();

				float p_l_edge_right_y = shape.part(40).y();

				// Landmark des linkes Auge -> linke Ecke

				float p_l_edge_left_x = shape.part(37).x();

				float p_l_edge_left_y = shape.part(37).y();



				

				// Array f�r Button 1

				

				button_array1[0] = 50;

				button_array1[1] = 70;

				// Array f�r Button 2

				

				button_array2[0] = GetSystemMetrics(SM_CXFULLSCREEN) - 120;

				button_array2[1] = 70;

				// Array f�r Button 3

				

				button_array3[0] = 50;

				button_array3[1] = GetSystemMetrics(SM_CYFULLSCREEN) - 70;

				// Array f�r Button 4

				

				button_array4[0] = GetSystemMetrics(SM_CXFULLSCREEN) - 120;

				button_array4[1] = GetSystemMetrics(SM_CYFULLSCREEN) - 70;

				// Array f�r Button 5

				

				button_array5[0] = GetSystemMetrics(SM_CXFULLSCREEN) / 2;

				button_array5[1] = GetSystemMetrics(SM_CYFULLSCREEN) / 2;



				

					

					

				//Wenn Button gedr�ckt wird, dann Distanzen berechnen und speichern

					

					if (My_screen.buttonIsPressed1 == true) {

								

						calculation(p_r_edge_right_x, p_r_edge_left_x, p_l_edge_right_x, p_l_edge_left_x, p_r_edge_right_y, p_r_edge_left_y, p_l_edge_right_y, p_l_edge_left_y, nose_x, nose_y);

						fill_Array(calib_array_1, p_r.x, p_r.y, p_l.x, p_l.y);


						//float calib_array_1[7]{iris_distance , right_eye_nose_distance, left_eye_nose_distance, p_r.x, p_r.y, p_l.x , p_l.y };

						My_screen.buttonIsPressed1 = false;

						

						

						input = { calib_array_1[0],calib_array_1[1],calib_array_1[2] };

							



						//datasamples erstellen mit den drei Distanzen

						//input = { calib_array_1[0],calib_array_1[1],calib_array_1[2] };

						//data_samples.push_back(make_pair(input, button_array1[0]));

						///data_samples.push_back(make_pair(input, target_array1[1]));

						cout << calib_array_1[1] << endl;

						calib++;

		

					}

					if (My_screen.buttonIsPressed2 == true) {

						calculation(p_r_edge_right_x, p_r_edge_left_x, p_l_edge_right_x, p_l_edge_left_x, p_r_edge_right_y, p_r_edge_left_y, p_l_edge_right_y, p_l_edge_left_y, nose_x, nose_y);

						fill_Array(calib_array_2, p_r.x, p_r.y, p_l.x, p_l.y);

						My_screen.buttonIsPressed2 = false;



						//input = { calib_array_2[0],calib_array_2[1],calib_array_2[2] };

						//data_samples.push_back(make_pair(input, button_array2[0]));

						//data_samples.push_back(make_pair(input, target_array2[1]));

						calib++;

						input2 = { calib_array_2[0],calib_array_2[1],calib_array_2[2] }; 

						cout << calib_array_2[1] << endl;



						

								

					}

					if (My_screen.buttonIsPressed3 == true) {

						calculation(p_r_edge_right_x, p_r_edge_left_x, p_l_edge_right_x, p_l_edge_left_x, p_r_edge_right_y, p_r_edge_left_y, p_l_edge_right_y, p_l_edge_left_y, nose_x, nose_y);

						fill_Array(calib_array_3, p_r.x, p_r.y, p_l.x, p_l.y);

						My_screen.buttonIsPressed3 = false;



						//input = { calib_array_3[0],calib_array_3[1],calib_array_3[2] };

						//data_samples.push_back(make_pair(input, button_array3[0]));

						//data_samples.push_back(make_pair(input, target_array3[1]));

						calib++;

						input3 = { calib_array_3[0],calib_array_3[1],calib_array_3[2] };

						cout << calib_array_3[1] << endl;

						

							

								

					}

					if (My_screen.buttonIsPressed4 == true) {

						calculation(p_r_edge_right_x, p_r_edge_left_x, p_l_edge_right_x, p_l_edge_left_x, p_r_edge_right_y, p_r_edge_left_y, p_l_edge_right_y, p_l_edge_left_y, nose_x, nose_y);

						fill_Array(calib_array_4, p_r.x, p_r.y, p_l.x, p_l.y);

						My_screen.buttonIsPressed4 = false;



						//input = { calib_array_4[0],calib_array_4[1],calib_array_4[2] };

						//data_samples.push_back(make_pair(input, button_array4[0]));

						//data_samples.push_back(make_pair(input, target_array4[1]));

						calib++;

						input4 = { calib_array_4[0],calib_array_4[1],calib_array_4[2] };

						cout << calib_array_4[1] << endl;

						

								

					}

					if (My_screen.buttonIsPressed5 == true) {

						calculation(p_r_edge_right_x, p_r_edge_left_x, p_l_edge_right_x, p_l_edge_left_x, p_r_edge_right_y, p_r_edge_left_y, p_l_edge_right_y, p_l_edge_left_y, nose_x, nose_y);

						fill_Array(calib_array_5, p_r.x, p_r.y, p_l.x, p_l.y);

						My_screen.buttonIsPressed5 = false;



						//input = { calib_array_5[0],calib_array_5[1],calib_array_5[2] };

						//data_samples.push_back(make_pair(input, button_array5[0]));

						//data_samples.push_back(make_pair(input, target_array5[1]));

						calib++;

						input5 = { calib_array_5[0],calib_array_5[1],calib_array_5[2] };

						cout << calib_array_5[1] << endl;

								

					}



					//Wenn 5 Buttons gedr�ckt wurden dann hier die least_squares Fkt starten (Zu testzwecken, man kann aktuell theoretisch auch

					//5 mal einen Button dr�cken, das �ndern wir sp�ter noch)

					if (calib == 5) {



						//Matrix berechnet aus allen Distanzen (f�r jeden Button eine Zeile)

						matrix<double, 5, 4> m;

						m = 1, calib_array_1[0], calib_array_1[1], calib_array_1[2],

							1, calib_array_2[0], calib_array_2[1], calib_array_2[2],

							1, calib_array_3[0], calib_array_3[1], calib_array_3[2],

							1, calib_array_4[0], calib_array_4[1], calib_array_4[2],

							1, calib_array_5[0], calib_array_5[1], calib_array_5[2];



						

						//buttonpositionen aufgeteilt in x und y Koordinaten

						matrix<double, 5, 1> butx;

						matrix<double, 5, 1> buty;

						butx = { button_array1[0],button_array2[0],button_array3[0],button_array4[0],button_array5[0] };

						buty = { button_array1[1],button_array2[1],button_array3[1],button_array4[1],button_array5[1] };

						

						matrix<double, 4, 5> mtr;

						//berechnung von der Inversen der Matrix

						mtr = inv(trans(m) * m)*trans(m);

						//Inverse Matrix * die Buttons

						x = mtr * butx;

						y = mtr * buty;

						

						/*

						//least squares funktion anwenden

						solve_least_squares_lm(objective_delta_stop_strategy(1e-7).be_verbose(),

							residual,

							derivative(residual),

							data_samples,

							x1);

						//cout << "inferred parameters: " << trans(x1) << endl;



						//um mit residual zu testen erstellt

						

							*/	



						//Testen ob die berechneten Werte wieder zum Button f�hren

						float button = 0;

						//als wir es mit der residual probiert haben, hat das leider nicht geklappt, deswegen haben wir es erst mal so

						//berechnete Alpha werte verrechen mit den gespeicherten Distanzwerten, welche beim dr�cken des Buttons links oben erfasst wurden.

						//Testen ob es wieder zur x-Buttonkoordinate f�hrt

						button = x(0) + x(1)* input(0) + x(2) * input(1) + x(3) * input(2)  ;// residual(make_pair(input2, target_array1[0]), x1);//// 

						cout << "x Koordinate" << button << endl;

						cout << "button" << button_array1[0] << endl;



						float button2 = 0;

						//als wir es mit der residual probiert haben, hat das leider nicht geklappt, deswegen haben wir es erst mal so

						//dasselbe wie oben nur mit y

						button2 = y(0) + y(1)* input(0) + y(2) * input(1) + y(3) * input(2);// residual(make_pair(input2, target_array1[0]), x1);//// 

						cout << "y Koordinate" << button2 << endl;

						cout << "button" << button_array1[1] << endl;



						

						calib = 0;

					}



				



			}

			





			//img_kopie zum reference Bild nehmen

			//hier Mat zu dlib img umwandeln

			cv_image<bgr_pixel> dl(img_copy);



            // Display it all on the screen

            win.clear_overlay();

            win.set_image(dl);

            win.add_overlay(render_face_detections(shapes));

			

		

        }

    }

    catch(serialization_error& e)

    {

        cout << "You need dlib's default face landmarking model file to run this example." << endl;

        cout << "You can get it from the following URL: " << endl;

        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;

        cout << endl << e.what() << endl;

    }

    catch(exception& e)

    {

        cout << e.what() << endl;

    }

}





