#ifndef ISUI_H
#define ISUI_H


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
//#include <FL/fl_ask.h>
#include <FL/Fl_File_Chooser.H>
//#include <fstream>
//#include <FL/Fl_Shared_Image.H>
//#include <FL/Fl_BMP_Image.H>
//#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include "PicView.h"
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Output.H>

class ISDoc;


class ISUI{

public:
	ISUI();

	void show();

	void setDocument(ISDoc*);
	ISDoc* getDocument();

	Fl_Double_Window* mainWindow;
	Fl_Menu_Bar* menuBar;
	PicView* pic;
	Fl_Output *text;
	static Fl_Menu_Item menuitems[];
	Fl_Double_Window* numWindow;
	Fl_Value_Slider* numSlider;


private:
	static ISUI* whoami(Fl_Widget* o);

	static void cb_open(Fl_Menu_ *w, void *);
	static void cb_save_contour(Fl_Menu_ *w, void *);
	static void cb_save_mask(Fl_Widget *w, void *);
	static void cb_quit(Fl_Widget *w, void *);

	static void cb_brush(Fl_Widget *w, void *);
	static void cb_scissor(Fl_Widget *w, void *);
	static void cb_NoBlur(Fl_Widget *w, void *);
	static void cb_3x3(Fl_Widget *w, void *);
	static void cb_7x7(Fl_Widget *w, void *);
	static void cb_5x5(Fl_Widget *w, void *);

	static void cb_zoom_in(Fl_Menu_ *w, void *);
	static void cb_zoom_out(Fl_Menu_ *w, void *);

	static void cb_Image_Only(Fl_Menu_ *w, void *);
	static void cb_Image_Contour(Fl_Menu_ *w, void *);

	static void cb_finish(Fl_Menu_ *w, void *);
	static void cb_finishClosed(Fl_Menu_ *w, void *);
	static void cb_undo(Fl_Menu_ *w,void *);

	static void cb_Pixel_Node(Fl_Menu_ *w, void *);
	static void cb_Cost_Graph(Fl_Menu_ *w, void *);
	static void cb_Path_Tree(Fl_Menu_ *w, void *);
	static void cb_Min_Path(Fl_Menu_ *w, void *);
	static void cb_numChange(Fl_Widget *w,void *);
	ISDoc* myDoc;

};

#endif