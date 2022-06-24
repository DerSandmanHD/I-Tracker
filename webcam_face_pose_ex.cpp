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

#include "gui_api_ex.cpp"

using namespace dlib;
using namespace std;


//berechnet die größte Kontur eines eingegebenen Konturenvektors
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

        image_window win;

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

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

			//kopie des Bildes anlegen, da man temp nicht verändern sollte (siehe Kommentar darüber)
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
				
				//Schwarz Weiß Bild anlegen
				cv::Mat img_copy_sw;
				cv::cvtColor(img_copy, img_copy_sw, CV_BGR2GRAY);

				//Augen werden getrennt bearbeitet

				//linkes Auge

				//Hier testen, ob shape-part noch im Bild ist (kann über Bildrand ragen und ergibt dadurch dann Fehlermeldung)
				if (shape.part(36).x() > 0 && shape.part(37).y() > 0) {
					//test ob linkes Auge offen ist, wenn ja, dann Pupille berechnen, sonst nicht
					if ((((shape.part(36).y() - shape.part(37).y()) > 3) && ((shape.part(41).y() - shape.part(36).y()) > 3)) || (((shape.part(39).y() - shape.part(38).y()) > 7))) {
						
						//Einen Kasten um das Auge rum bilden anhand der Landmarks
						//Eckpunkte des linken Auge
						int left_corner_left_eye_x  = shape.part(36).x();
						int left_corner_left_eye_y  = shape.part(37).y();
						int right_corner_left_eye_x = shape.part(39).x();
						int right_corner_left_eye_y = shape.part(40).y();

						//Breite und Höhe des Kastens des linken Auges
						int width_left_eye = right_corner_left_eye_x - left_corner_left_eye_x;
						int height_left_eye = right_corner_left_eye_y - left_corner_left_eye_y;

						//Den linken Kasten als Rechteck definieren
						cv::Rect roi_left(left_corner_left_eye_x, left_corner_left_eye_y, width_left_eye, height_left_eye);

						//Kasten als eigenes Bilder abspeichern
						cv::Mat img_eye_left = img_copy_sw(roi_left);

						/*Histogrammequalisation
						Hier könnte man auch noch die Helligkeit verändern
						und nochmal eine Äqualisation, macht bei meinen Augen
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

						// Durschnittswert(Grauwert) des Kastens berechnen für den Threshold
						cv::Scalar mean_left = cv::mean(img_eye_left);
						float threshhold_left = mean_left.val[0];

						// inversen threshhold anwenden, also hier die Pixel auf dem Bild weiß machen, welche 
						//über dem Durschnittswert liegen, die restlichen Pixel schwarz (Binary Threshhold)
						threshold(img_eye_left_copy, img_eye_left_copy, threshhold_left, threshhold_left, cv::THRESH_BINARY_INV);

						//linkes Auge morphologische Operatoren (Dienen dazu, den erkannten bereich besser auf die Iris einzuschränken,
						//Pixel welche nicht zu der Iris gehören werden hier teilweise entfernt durch die Operatoren (Typischer Bildver-
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

						//Mittelpunkt der größten Kontur (Iris) mithilfe von Moments herausfinden
						cv::Point p_l(M_l.m10 / M_l.m00, M_l.m01 / M_l.m00);

						//Auf Bild die beiden Punkte abbilden
						img_copy.at<cv::Vec3b>(left_corner_left_eye_y + 2 + p_l.y, left_corner_left_eye_x + 2 + p_l.x) = (0, 0, 255);
					}
				}

				
				//rechtes Auge

				//Hier testen, ob shape-part noch im Bild ist (kann über Bildrand ragen und ergibt dadurch dann Fehlermeldung)
				if (shape.part(45).x() < 640 && shape.part(44).y() > 0) {
					//test ob rechte Auge offen ist, wenn ja, dann Pupille berechnen, sonst nicht
					if ((((shape.part(45).y() - shape.part(44).y()) > 3) && ((shape.part(46).y() - shape.part(45).y()) > 3)) || (((shape.part(42).y() - shape.part(43).y()) > 7))) {	
						
						//Eckpunkte des rechten Auges
						int left_corner_right_eye_x  = shape.part(42).x();
						int left_corner_right_eye_y  = shape.part(43).y();
						int right_corner_right_eye_x = shape.part(45).x();
						int right_corner_right_eye_y = shape.part(46).y();

						//Breite und Höhe des Kastens des rechten Auges
						int width_right_eye = right_corner_right_eye_x - left_corner_right_eye_x;
						int height_right_eye = right_corner_right_eye_y - left_corner_right_eye_y;

						//Den rechten Kasten als Rechteck definieren
						cv::Rect roi_right(left_corner_right_eye_x, left_corner_right_eye_y, width_right_eye, height_right_eye);

						//Kasten als eigenes Bilder abspeichern
						cv::Mat img_eye_right = img_copy_sw(roi_right);

						/*Histogrammequalisation
						Hier könnte man auch noch die Helligkeit verändern
						und nochmal eine Äqualisation, macht bei meinen Augen
						aber keinen Unterschied, bei euren??*/

						//Histogrammequalisation
						cv::equalizeHist(img_eye_right, img_eye_right);
						//img_roi_right.convertTo(img_roi_right, -1, 1, -70);
						//cv::equalizeHist(img_roi_right, img_roi_right);

						//Diesen ausschnitt nochmals als extrabild speichern
						cv::Mat img_eye_right_copy = img_eye_right;

						// Durschnittswert(Grauwert) des Kastens berechnen für den Threshold
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

						//Mittelpunkt der größten Kontur (Iris) mithilfe von Moments herausfinden
						cv::Point p_r(M_r.m10 / M_r.m00, M_r.m01 / M_r.m00);

						//Auf Bild die beiden Punkte abbilden
						img_copy.at<cv::Vec3b>(left_corner_right_eye_y + 2 + p_r.y, left_corner_right_eye_x + 2 + p_r.x) = (0, 0, 255);
					}
				}
				
				

			}


            // Landmark der Nase
                 int nose = shape.part(34);
				// Landmark des rechten Auge -> rechte Ecke
				 int p_r_edge_right=shape.part(46);
				// Landmark des rechten Auge -> linke Ecke
				 int p_r_edge_left=shape.part(43);
				// Landmark des linkes Auge -> rechte Ecke
				 int p_l_edge_right=shape.part(40);
				// Landmark des linkes Auge -> linke Ecke
				 int p_l_edge_left=shape.part(37);


				// Berechnung der Distanz von (Linkes)Auge zu  (Rechtes)Auge
                // float iris_distance = hypot(p_r.x - p_l.x, p_r.y -p_l.y);
				 float iris_distance = hypot(((p_r_edge_right.x + p_r_edge_left.x)/2  - (p_l_edge_right.x + p_l_edge_left.x)/2) , ((p_r_edge_right.y + p_r_edge_left.y)/2  - (p_l_edge_right.y + p_l_edge_left.y)/2));
				// Berechnung der Distanz von (Rechtes)Auge zu Nase
                 float right_eye_nose_distance = hypot((p_r_edge_right.x + p_r_edge_left.x)/2 - nose.x, (p_r_edge_right.y + p_r_edge_left.y)/2 - nose.y);
				// Berechnung der Distanz von (Linkes)Auge zu Nase
                 float left_eye_nose_distance = hypot((p_r_edge_right.y + p_r_edge_left.y)/2 - nose.x, (p_l_edge_right.y + p_l_edge_left.y)/2 - nose.y);

				//Speichern der Werte in einem Array
				 float ldmarks_pupil_arr[7];

				 ldmarks_pupil_arr [0] = iris_distance;
				 ldmarks_pupil_arr [1] = right_eye_nose_distance;
				 ldmarks_pupil_arr [2] = left_eye_nose_distance;
				 ldmarks_pupil_arr [3] = p_r.x; 
				 ldmarks_pupil_arr [4] = p_r.y;
				 ldmarks_pupil_arr [5] = p_l.x;  
				 ldmarks_pupil_arr [6] = p_l.y;
			   //  ldmarks_pupil_arr [7] = face_centrumX;
			   //  ldmarks_pupil_arr [8] = face_centrumY;

				// Array für Button 1
				 float target_array1[2];
				 target_array[0]=b1.get_pos.x;
				 target_array[1]=b1.get_pos.y;
				// Array für Button 2
				 float target_array2[2];
				 target_array[0]=b2.get_pos.x;
				 target_array[1]=b3.get_pos.y;
				// Array für Button 3
				 float target_array3[2];
				 target_array[0]=b3.get_pos.x;
				 target_array[1]=b3.get_pos.y;
				// Array für Button 4
				 float target_array4[2];
				 target_array[0]=b4.get_pos.x;
				 target_array[1]=b4.get_pos.y;
				// Array für Button 5
				 float target_array5[2];
				 target_array[0]=b5.get_pos.x;
				 target_array[1]=b5.get_pos.y;
			


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