#include <stdio.h>

#include "ISUI.h"
#include "ISDoc.h"
#include <iostream>
using namespace std;
ISUI* myUI;
ISDoc* myDoc;


int main(){

	myUI = new ISUI();
	myDoc = new ISDoc();

	myUI->setDocument(myDoc);
	myDoc->setUI(myUI);

	Fl::visual(FL_DOUBLE|FL_INDEX);

	myUI->show();

	return Fl::run();
}