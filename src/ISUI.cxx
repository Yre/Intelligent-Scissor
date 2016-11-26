#include "ISUI.h"
#include "ISDoc.h"
#include <iostream>
using namespace std;
ISUI::ISUI(){
    Fl::scheme("gtk+");
    // fl_register_images();   
    menuitems[12].setonly(); 
    menuitems[27].setonly(); 
    mainWindow = new Fl_Double_Window(600,360);
    mainWindow->user_data((void*)(this));
        menuBar = new Fl_Menu_Bar(0,0,600,20);
        menuBar->copy(menuitems);
        pic= new PicView(0,20,600,320,"This is the Picture");
        pic->box(FL_DOWN_FRAME);
        text=new Fl_Output(40,340,560,20,"Pixel");
        text->value("start");
    mainWindow->resizable(pic);
    mainWindow->end();

    numWindow=new Fl_Double_Window(600,30);
    numWindow->user_data((void*)(this));
        numSlider=new Fl_Value_Slider(0,0,540,30,"Expanded");
        numSlider->user_data((void*)(this));
        numSlider->type(FL_HOR_NICE_SLIDER);
        numSlider->labelfont(FL_COURIER);
        numSlider->labelsize(12);
        numSlider->minimum(1);
        numSlider->step(1);
        numSlider->align(FL_ALIGN_RIGHT);
        numSlider->callback(cb_numChange);
    numWindow->end();

}

Fl_Menu_Item ISUI::menuitems[]=
{
        {"File",0,0,0,FL_SUBMENU|FL_MENU_DIVIDER},
            {"Open",        FL_CTRL+'o',(Fl_Callback *)ISUI::cb_open,0,0},
            {"Save Contour",FL_CTRL+'c',(Fl_Callback *)ISUI::cb_save_contour,0,0},
            {"Save Mask",   FL_CTRL+'m',(Fl_Callback *)ISUI::cb_save_mask,0,FL_MENU_DIVIDER},
            {"Quit",        FL_CTRL+'q',(Fl_Callback *)ISUI::cb_quit,0,0},
            {0},
        {"Edit",0,0,0,FL_MENU_DIVIDER|FL_SUBMENU},
            {"Brush",       FL_CTRL+'b',(Fl_Callback *)ISUI::cb_brush,0,0},
            {"Scissor",     FL_CTRL+'s',(Fl_Callback *)ISUI::cb_scissor,0,0},
            {0},
        {"View",FL_CTRL+'i',0,0,FL_MENU_DIVIDER|FL_SUBMENU},
            {"Blur",        FL_CTRL+'r',0,0,FL_SUBMENU},
                {"No blurring",     0,(Fl_Callback *)ISUI::cb_NoBlur,0,FL_MENU_RADIO},
                {"3x3",     0,(Fl_Callback *)ISUI::cb_3x3,0,FL_MENU_RADIO},
                {"5x5",     0,(Fl_Callback *)ISUI::cb_5x5,0,FL_MENU_RADIO},
                {"7x7",     0,(Fl_Callback *)ISUI::cb_7x7,0,FL_MENU_RADIO},
                {0},
            {"Zoom in",     FL_CTRL+'[',(Fl_Callback *)ISUI::cb_zoom_in,0,0},
            {"Zoom out",    FL_CTRL+']',(Fl_Callback *)ISUI::cb_zoom_out,0,0},
            {0},
        {"Action",0,0,0,FL_MENU_DIVIDER|FL_SUBMENU},
            {"Finish Contour",'.',(Fl_Callback *)ISUI::cb_finish,0,0},
            {"Finish closed Contour",FL_CTRL+'.',(Fl_Callback *)ISUI::cb_finishClosed,0,0},
            {"Undo",FL_CTRL+'z',(Fl_Callback *)ISUI::cb_undo,0,0},
            {0},
        {"Work Mode",FL_CTRL+'d',0,0,FL_MENU_DIVIDER|FL_SUBMENU},
            {"Image Only",  0,(Fl_Callback *)ISUI::cb_Image_Only,0,FL_MENU_RADIO},
            {"Image with Contour",  0,(Fl_Callback *)ISUI::cb_Image_Contour,0,FL_MENU_RADIO},
            {0},
        {"Debug Mode",FL_CTRL+'d',0,0,FL_MENU_DIVIDER|FL_SUBMENU},
            {"Pixel Node",  0,(Fl_Callback *)ISUI::cb_Pixel_Node,0,FL_MENU_RADIO},
            {"Cost Graph",  0,(Fl_Callback *)ISUI::cb_Cost_Graph,0,FL_MENU_RADIO},
            {"Path Tree",   0,(Fl_Callback *)ISUI::cb_Path_Tree,0,FL_MENU_RADIO},
            {"Min Path",    0,(Fl_Callback *)ISUI::cb_Min_Path,0,FL_MENU_RADIO},
            {0},
        {0}
};

void ISUI::setDocument(ISDoc* doc){
    myDoc = doc;
    pic->myDoc = doc;
}
ISDoc* ISUI::getDocument(){
    return myDoc;
}

void ISUI::show(){
    mainWindow->show();
}

ISUI* ISUI::whoami(Fl_Widget* o)   
{
    return ( (ISUI*)(o->parent()->user_data()) );//userdata is initialized to store the UI object address
}

/*=======CALL BACK==========*/
void ISUI::cb_open(Fl_Menu_ *w, void *)
{
    ISUI *myUI=whoami(w);
    ISDoc *myDoc;
    if(myUI!=NULL)
    {
        myDoc=myUI->getDocument();
    }
    Fl_File_Chooser chooser(".","*",0,"Choose File");
    chooser.show();
    while(chooser.shown())
    {Fl::wait();}
    if(chooser.value()!=NULL)
    {
       myDoc->loadImage(chooser.value());
    }
}

void ISUI::cb_save_contour(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    Fl_File_Chooser chooser("save.bmp","*",2,"Save File?");//2 means create a new file
    chooser.show();
    while(chooser.shown()){Fl::wait();}
    if (chooser.value() != NULL)
    {
        myDoc->saveImageWithContour((char*)chooser.value());
    }
}
void ISUI::cb_save_mask(Fl_Widget *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    if (whoami(w)->pic->contour!=2) return;

    Fl_File_Chooser chooser("savemask.bmp","*",2,"Save File?");//2 means create a new file
    chooser.show();
    while(chooser.shown()){Fl::wait();}
    if (chooser.value() != NULL)
    {
        myDoc->saveImageWithMask((char*)chooser.value());
    }

}
void ISUI::cb_quit(Fl_Widget *w, void *)
{
    exit(0);
}

void ISUI::cb_brush(Fl_Widget *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->mode = WORK_MODE;
    myDoc->scissorStatus = false;
    myDoc->brushStatus = true;
}
void ISUI::cb_scissor(Fl_Widget *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->mode = WORK_MODE;
    myDoc->scissorStatus = true;
    myDoc->brushStatus = false;
}
void ISUI::cb_NoBlur(Fl_Widget *w,void *){
    if(whoami(w)->getDocument()->mode==DEBUG_MODE)return;
    whoami(w)->getDocument()->initializeMatrix(-1);
    whoami(w)->getDocument()->refreshCurmap();
}
void ISUI::cb_3x3(Fl_Widget *w, void *){
    if(whoami(w)->getDocument()->mode==DEBUG_MODE)return;
    whoami(w)->getDocument()->initializeMatrix(3);
    whoami(w)->getDocument()->refreshCurmap();
}
void ISUI::cb_5x5(Fl_Widget *w, void *){
    if(whoami(w)->getDocument()->mode==DEBUG_MODE)return;
    whoami(w)->getDocument()->initializeMatrix(5);
    whoami(w)->getDocument()->refreshCurmap();
}
void ISUI::cb_7x7(Fl_Widget *w, void *){
    if(whoami(w)->getDocument()->mode==DEBUG_MODE)return;
    whoami(w)->getDocument()->initializeMatrix(7);
    whoami(w)->getDocument()->refreshCurmap();
}
    
void ISUI::cb_zoom_in(Fl_Menu_ *w, void *)
{
    ISDoc *myDoc=whoami(w)->getDocument();//why not use static ISDoc?
    if (myDoc==NULL) return;
    if (myDoc->mode == DEBUG_MODE) return;
    myDoc->zoom('+');
}
void ISUI::cb_zoom_out(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();//why not use static ISDoc?
    if (myDoc==NULL) return;
    if (myDoc->mode == DEBUG_MODE) return;
    myDoc->zoom('-');
}

void ISUI::cb_Image_Only(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    myDoc->toImageOnlyMode();
}

void ISUI::cb_Image_Contour(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    myDoc->toImageWithContourMode();
}

void ISUI::cb_finish(Fl_Menu_ *w, void *){
    whoami(w)->getDocument()->stopContour();
}

void ISUI::cb_finishClosed(Fl_Menu_ *w, void *){
     ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->closeContour();
}

void ISUI::cb_undo(Fl_Menu_ *w,void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->undo();

}
//=============Debug mode==================
void ISUI::cb_Pixel_Node(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->pixelNode();
}
void ISUI::cb_Cost_Graph(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    myDoc->costGraph();
}
void ISUI::cb_Path_Tree(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    if (myDoc->seed==NULL) return;

    whoami(w)->numSlider->maximum((myDoc->height-2)*(myDoc->width-2));
    whoami(w)->numSlider->value((myDoc->height-2)*(myDoc->width-2)/4);
    myDoc->pathTree(myDoc->seed->row,myDoc->seed->col,whoami(w)->numSlider->value());

    whoami(w)->numWindow->show();
}

void ISUI::cb_numChange(Fl_Widget *w,void *){
    ISDoc* myDoc=whoami(w)->getDocument();
    myDoc->pathTree(myDoc->seed->row,myDoc->seed->col,whoami(w)->numSlider->value());
}

void ISUI::cb_Min_Path(Fl_Menu_ *w, void *){
    ISDoc *myDoc=whoami(w)->getDocument();
    if (myDoc==NULL) return;
    if (myDoc->seed==NULL) return;

    myDoc->minPath(myDoc->seed->row,myDoc->seed->col);
}




