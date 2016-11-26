#include "PicView.h"
#include "ISDoc.h"
#include <iostream>
using namespace std;
#ifndef WIN32
#define min(a, b)	( ( (a)<(b) ) ? (a) : (b) )
#endif

PicView::PicView(int x,int y,int width,int height,const char* message): 
				Fl_Gl_Window(x,y,width,height,message)
{
	this->width=width;
	this->height=height;
	contour=0;
}

void PicView::draw()
{
	
	if(!valid())
	{
		glClearColor(0.7f, 0.7f, 0.7f, 1.0);

		// We're only using 2-D, so turn off depth 
		glDisable( GL_DEPTH_TEST );

		// Tell openGL to read from the front buffer when capturing
		// out paint strokes 
		glReadBuffer( GL_FRONT );
		ortho();
		valid(1);

	}

	glClear( GL_COLOR_BUFFER_BIT );
	if(myDoc->curmap)
	{
		
		int drawWidth, drawHeight;
		GLvoid* bitstart;

		width=w();
		height=h();

		Point scrollpos;//=GetScrollPosition();
		scrollpos.col=scrollpos.row=0;

		drawWidth	= min(width, myDoc->zw);
		drawHeight	= min(height,myDoc->zh);

		int startrow = myDoc->zh - (scrollpos.row+drawHeight);
		if(startrow<0)
			startrow=0;
		bitstart = myDoc->curmap+3*((myDoc->zw*startrow)+scrollpos.col);

		glRasterPos2i( 0, height - drawHeight );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glPixelStorei( GL_UNPACK_ROW_LENGTH, myDoc->zw );
		glDrawBuffer( GL_BACK );
		glDrawPixels( drawWidth, drawHeight, GL_RGB, GL_UNSIGNED_BYTE, bitstart );//*/
		//glScalef(4,4,4);
		
		}

	glFlush();
}

void PicView::refresh()
{
	flush();
}

int PicView::handle(int event)
{
	int rx,ry;
	switch(event){
	case FL_PUSH:
		rx = Fl::event_y()/myDoc->z; ry = Fl::event_x()/myDoc->z;
		if (rx<=0 || rx >= myDoc->height-1 || ry<=0 || ry >= myDoc->width-1) break;
		if (myDoc->mode!=WORK_MODE) break;
		if (myDoc->scissorStatus && contour!=2){
			if (myDoc->haveBrushed && myDoc->nodeMatrix [myDoc->height-rx-1][ry].brushed==false) break;
			if (contour==0)
				myDoc->setStartSeed(myDoc->height -rx-1,ry);
			myDoc->setSeed(myDoc->height-rx-1,ry);
			contour=1;
		}
		flush();
		break;
	case FL_MOVE:
		rx = Fl::event_y()/myDoc->z; ry = Fl::event_x()/myDoc->z;
		if (rx < 0 || rx >= myDoc->height || ry < 0 || ry >= myDoc->width) break;
		myDoc->setText(myDoc->height -rx-1,ry);
		if (rx <= 0 || rx >= myDoc->height-1 || ry <= 0 || ry >= myDoc->width-1) break;
		if (myDoc->haveBrushed && 
			 myDoc->nodeMatrix[myDoc->height-rx-1][ry].brushed==false) break;
		
		if(/*compContour==false && */myDoc->scissorStatus && contour==1 && myDoc->mode==WORK_MODE){
			myDoc->drawContour(myDoc->height-rx-1,ry);
		}
		break;
	case FL_DRAG:
		rx = Fl::event_y()/myDoc->z; ry = Fl::event_x()/myDoc->z;
		if (rx<=0 || rx >= myDoc->height-1 || ry<=0 || ry >= myDoc->width-1) break;
		if (myDoc->brushStatus && myDoc->mode==WORK_MODE){
			myDoc->drawBrush(myDoc->height-rx-1,ry);
		}
		break;
	case FL_ENTER:
		return true;
	default:
		return 0;
		break;

	}
	return true;
}


