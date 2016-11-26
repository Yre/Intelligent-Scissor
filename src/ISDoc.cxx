
#include <FL/fl_ask.H>
#include "ISDoc.h"
#include "ISUI.h"
#include <cstring>
#include <iostream>
#include <cmath>
#include <queue>
#include <stack>
#include <string>
using namespace std;

priority_queue <Node> q;
bool operator < (Node x,Node y){
     return x.totalCost > y.totalCost;     // 默认大根堆，这里是小根堆，注意小根堆是大于！与sort的cmp是反的，这里是满足则交换
};

stack <Point> seeds;

// For auxiliary purpose
	//diagonal:1,3,5,7
int dir[8][2]={{1,0},{1,-1},{0,-1},{-1,-1},
				   {-1,0},{-1,1},{0,1},{1,1}};


ISDoc::ISDoc(){
	curmap=bitmap = NULL;
	imageName[0] = '\0';
	z=1.0;
	width=height=0;
	nodeMatrix=NULL;
	debugMatrix=NULL;
	seed=last=NULL;
}

void ISDoc::setUI(ISUI* ui){
	myUI = ui;
}


char* ISDoc::getImageName(){
	return imageName;
}

int ISDoc::loadImage(const char* picName){
	unsigned char* data;
	int	w, h;

	if ( (data=readBMP(picName, w, h))==NULL ) {
		fl_alert("Can't load bitmap file");
		return 0;
	}
	bitmap = data;
	zw=width = w;
	zh=height = h;
	z=1.0;
	mode = WORK_MODE;
	initializeMatrix(-1);
	refreshCurmap();
	myUI->pic->show();
	//calcCostTree(1,1,-1);
	
	// int l=width*height*3;
	// curmap=new unsigned char[l];
	// for(int i=0;i<l;i++)
	// 	curmap[i]=bitmap[i];
	//curmap=bitmap;
	//myUI->mainWindow->resize(myUI->mainWindow->x(),myUI->mainWindow->y(),width,height+20);

	//myUI->pic->resizeView(width,height);

	//myUI->pic->refresh();
	return 1;

}

int ISDoc::saveImageWithContour(char* picName){
	if(bitmap==NULL)
		return 0;
	unsigned char* savemap=new unsigned char[3*width*height];
	for(int i=0;i<height;i++)
			for(int j=0;j<width;j++){
				if(nodeMatrix[i][j].drawed!=0){
					savemap[(i*width+j)*3]=255;
					savemap[(i*width+j)*3+1]=0;
					savemap[(i*width+j)*3+2]=0;
				}
				else{
					savemap[(i*width+j)*3]=nodeMatrix[i][j].c1;
					savemap[(i*width+j)*3+1]=nodeMatrix[i][j].c2;
					savemap[(i*width+j)*3+2]=nodeMatrix[i][j].c3;
				}
			}

	writeBMP(picName, width, height, savemap);
	return 1;
}



int ISDoc::saveImageWithMask(char* picName){
	if(bitmap==NULL)
		return 0;
	queue<Node> q;
	Node cur;
	int xx,yy;
	bool b[width][height];

	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){
			b[i][j]=false;
			nodeMatrix[i][j].outside=false;
		}
	
	nodeMatrix[0][0].outside=true; q.push(nodeMatrix[0][0]);

	
	while (!q.empty()){
		cur = q.front(); q.pop();
		for (int d=0;d<8;d+=2){
			xx=cur.row+dir[d][0]; yy=cur.col+dir[d][1];
			if (xx<0 || xx>height-1 || yy<0 || yy>width-1) continue;
			if (nodeMatrix[xx][yy].outside || nodeMatrix[xx][yy].drawed) continue;
			q.push(nodeMatrix[xx][yy]);
			nodeMatrix[xx][yy].outside=true;
		}
	}


	unsigned char* savemap=new unsigned char[3*width*height];
	for(int i=0;i<height;i++)
			for(int j=0;j<width;j++){
				if(nodeMatrix[i][j].outside==true){
					savemap[(i*width+j)*3]=0;
					savemap[(i*width+j)*3+1]=0;
					savemap[(i*width+j)*3+2]=0;
				}
				else{
					savemap[(i*width+j)*3]=255;
					savemap[(i*width+j)*3+1]=255;
					savemap[(i*width+j)*3+2]=255;
				}
			}

	writeBMP(picName, width, height, savemap);
	return 1;
}


// ===================== WORK MODE ====================

void ISDoc::zoom(char inq){
	if(inq=='+')
		z*=2;
	else if(inq=='-')
		z*=0.5;
	zw=z*width;
	zh=z*height;
	refreshCurmap();
}

void ISDoc::toImageWithContourMode(){
	zw=width;
	zh=height;
	z=1.0;
	mode = WORK_MODE;
	refreshCurmap();
	myUI->pic->show();
}

void ISDoc::toImageOnlyMode(){
	zw=width;
	zh=height;
	z=1.0;
	mode = IMAGE_ONLY;
	refreshCurmap();
	myUI->pic->show();
}

void ISDoc::drawContour(int row,int col){
	if(last!=NULL){
		Node* p=&nodeMatrix[last->row][last->col];
		while( p->row!=seed->row || p->col!=seed->col){
			if(p->drawed==curlayer)
				p->drawed=0;
			p=p->preNode;
		}
		last=NULL;//////
	}
	// if(nodeMatrix[row][col].drawed==false)
	{
		last=new Point;
		last->row=row;
		last->col=col;
		Node *p=&nodeMatrix[last->row][last->col];
		while( p->row!=seed->row || p->col!=seed->col ){
			if(p->drawed==0)
				p->drawed=curlayer;
			p=p->preNode;
		}
	}
	refreshCurmap();
}

void ISDoc::drawBrush(int row,int col){
	haveBrushed = true;
	int xx,yy;
	for (xx=row-12;xx<=row+12;xx++)
		for (yy=col-12;yy<=col+12;yy++){
			if (xx<0 || yy<0 || xx>=height || yy>=width) continue;
			nodeMatrix[xx][yy].brushed = 1;
		}
	refreshCurmap();
}


// ===================== End of WORK MODE ====================

// ===================== DEBUG MODE ====================

void ISDoc::pixelNode(){
	mode = DEBUG_MODE;
	debugMatrix = new Color*[3*height];
	for (int i=0;i<3*height;i++)
		debugMatrix[i] = new Color[3*width];
	// initial the 3*h * 3*w matrix(h&w in original size)
	Color Black; Black.c1 = Black.c2 = Black.c3 = 0;
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){

			for (int l1=0;l1<=2;l1++)
				for (int l2=0;l2<=2;l2++)
					debugMatrix[i*3+l1][j*3+l2] = Black;

			debugMatrix[i*3+1][j*3+1].c1 = nodeMatrix[i][j].c1;
			debugMatrix[i*3+1][j*3+1].c2 = nodeMatrix[i][j].c2;
			debugMatrix[i*3+1][j*3+1].c3 = nodeMatrix[i][j].c3;

		}
	// z*=3.0; zw*=3.0; zh*=3.0;
	z=1; zw=width*3; zh=height*3;
	refreshCurmap();
	for (int i=0;i<3*height;i++)
		delete debugMatrix[i];
	delete debugMatrix;
}

void ISDoc::costGraph(){
	mode = DEBUG_MODE;
	debugMatrix = new Color*[3*height];
	for (int i=0;i<3*height;i++)
		debugMatrix[i] = new Color[3*width];
	// initial the 3*h * 3*w matrix(h&w in original size)
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){

			for (int d=0;d<8;d++){
				int grey = 255-(int)nodeMatrix[i][j].linkCost[d]*255/(maxD*sqrt(2));
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c1=grey;
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c2=grey;
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c3=grey;
			}

			debugMatrix[i*3+1][j*3+1].c1 = nodeMatrix[i][j].c1;
			debugMatrix[i*3+1][j*3+1].c2 = nodeMatrix[i][j].c2;
			debugMatrix[i*3+1][j*3+1].c3 = nodeMatrix[i][j].c3;

		}
	z=1; zw=width*3; zh=height*3;
	refreshCurmap();

	for (int i=0;i<3*height;i++)
		delete debugMatrix[i];
	delete debugMatrix;
}

void ISDoc::pathTree(int seedr, int seedc, int expand){
	mode = DEBUG_MODE;
	debugMatrix = new Color*[3*height];
	for (int i=0;i<3*height;i++)
		debugMatrix[i] = new Color[3*width];

	int maxCost = calcCostTree(seedr, seedc, expand);

	Color Black; Black.c1 = Black.c2 = Black.c3 = 0;
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){

			for (int l1=0;l1<=2;l1++)
				for (int l2=0;l2<=2;l2++)
					debugMatrix[i*3+l1][j*3+l2] = Black;
		
			for (int d=0;d<8;d++){
				if (nodeMatrix[i][j].totalCost==-1 || nodeMatrix[i][j].preNode==NULL) break;
				//if (nodeMatrix[i][j].preNode != &nodeMatrix[i+dir[d][0]][j+dir[d][1]]) break;
				int yellow = 255-(int)nodeMatrix[i][j].totalCost*255/(maxCost*sqrt(2));
				
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c1=255;
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c2=255;
				debugMatrix[i*3+dir[d][0]+1][j*3+dir[d][1]+1].c3=yellow;
			}

			debugMatrix[i*3+1][j*3+1].c1 = nodeMatrix[i][j].c1;
			debugMatrix[i*3+1][j*3+1].c2 = nodeMatrix[i][j].c2;
			debugMatrix[i*3+1][j*3+1].c3 = nodeMatrix[i][j].c3;

		}
	z=1; zw=width*3; zh=height*3;
	refreshCurmap();
	for (int i=0;i<3*height;i++)
		delete debugMatrix[i];
	delete debugMatrix;
}

void ISDoc::minPath(int seedr, int seedc){
	mode = DEBUG_MODE;
	debugMatrix = new Color*[3*height];
	for (int i=0;i<3*height;i++)
		debugMatrix[i] = new Color[3*width];

	calcCostTree(seedr,seedc,-1);


	Color Black; Black.c1 = Black.c2 = Black.c3 = 0;
	Color Blue;  Blue.c2 = 255; Blue.c1 = Blue.c3 = 0;
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){
			for (int l1=0;l1<=2;l1++)
				for (int l2=0;l2<=2;l2++)
					debugMatrix[i*3+l1][j*3+l2] = Black;
			debugMatrix[i*3+1][j*3+1].c1 = nodeMatrix[i][j].c1;
			debugMatrix[i*3+1][j*3+1].c2 = nodeMatrix[i][j].c2;
			debugMatrix[i*3+1][j*3+1].c3 = nodeMatrix[i][j].c3;
		}
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){
			if (nodeMatrix[i][j].totalCost==-1 || nodeMatrix[i][j].preNode==NULL) continue;

			int d = nodeMatrix[i][j].preNodeDir;
			int preNodex = i+dir[(d+4)%8][0];
			int preNodey = j+dir[(d+4)%8][1];
			debugMatrix[i*3 + dir[d][0]+1][j*3 + dir[d][1]+1] = Blue;
			debugMatrix[preNodex*3 + dir[(d+4)%8][0]+1][preNodey*3 + dir[(d+4)%8][1]+1] = Blue;	
		}
	z=1; zw=width*3; zh=height*3;
	refreshCurmap();
	for (int i=0;i<3*height;i++)
		delete debugMatrix[i];
	delete debugMatrix;

}
// ===================== End of DEBUG MODE ====================
void ISDoc::initializeMatrix(int blur=-1){
	seed=last=NULL;
	while(!seeds.empty())
		seeds.pop();
	curlayer=0;
	if(seed!=NULL){
		delete seed;
		seed=NULL;
	}
	if(last!=NULL){
		delete last;
		last=NULL;
	}
	myUI->pic->contour=0;
	//myUI->pic->compContour=false;
	scissorStatus = false;
    brushStatus = false;
    haveBrushed=false;

	nodeMatrix = new Node*[height];
	for (int i=0;i<height;i++)
		nodeMatrix[i] = new Node[width];

	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){
			nodeMatrix[i][j].c1=bitmap[(i*width+j)*3];
			nodeMatrix[i][j].c2=bitmap[(i*width+j)*3+1];
			nodeMatrix[i][j].c3=bitmap[(i*width+j)*3+2];
			nodeMatrix[i][j].row=i;
			nodeMatrix[i][j].col=j;
			nodeMatrix[i][j].state=INITIAL;
			nodeMatrix[i][j].preNode=NULL;
			nodeMatrix[i][j].totalCost=-1;
			nodeMatrix[i][j].drawed=0;
			nodeMatrix[i][j].brushed=false;
		}
	Node ** origin;
	switch(blur){
		case -1:
			break;
		case 3:
			origin=nodeMatrix;
			nodeMatrix = new Node*[height];
			for (int i=0;i<height;i++)
				nodeMatrix[i] = new Node[width];
			for (int i=0;i<height;i++)
				for (int j=0;j<width;j++)
				{
					if(i==0||i==height-1||j==0||j==width-1)
					{
						nodeMatrix[i][j]=origin[i][j];
						continue;
					}
					nodeMatrix[i][j].c1=(origin[i][j].c1*2+origin[i-1][j-1].c1+origin[i-1][j].c1+origin[i-1][j+1].c1+origin[i][j-1].c1+origin[i][j+1].c1+origin[i+1][j-1].c1+origin[i+1][j].c1+origin[i+1][j+1].c1)/10;
					nodeMatrix[i][j].c2=(origin[i][j].c2*2+origin[i-1][j-1].c2+origin[i-1][j].c2+origin[i-1][j+1].c2+origin[i][j-1].c2+origin[i][j+1].c2+origin[i+1][j-1].c2+origin[i+1][j].c2+origin[i+1][j+1].c2)/10;
					nodeMatrix[i][j].c3=(origin[i][j].c3*2+origin[i-1][j-1].c3+origin[i-1][j].c3+origin[i-1][j+1].c3+origin[i][j-1].c3+origin[i][j+1].c3+origin[i+1][j-1].c3+origin[i+1][j].c3+origin[i+1][j+1].c3)/10;
					nodeMatrix[i][j].row=i;
					nodeMatrix[i][j].col=j;
					nodeMatrix[i][j].state=INITIAL;
					nodeMatrix[i][j].preNode=NULL;
					nodeMatrix[i][j].totalCost=-1;
					nodeMatrix[i][j].drawed=0;
					nodeMatrix[i][j].brushed=0;
				}
			break;
		case 5:
			origin=nodeMatrix;
			nodeMatrix = new Node*[height];
			for (int i=0;i<height;i++)
				nodeMatrix[i] = new Node[width];
			for (int i=0;i<height;i++)
				for (int j=0;j<width;j++)
				{
					if(i==0||i==height-1||j==0||j==width-1)
					{
						nodeMatrix[i][j]=origin[i][j];
						continue;
					}
					if(i<=1||i>=height-2||j<=1||j>=width-2)
					{
						nodeMatrix[i][j].c1=(origin[i][j].c1*2+origin[i-1][j-1].c1+origin[i-1][j].c1+origin[i-1][j+1].c1+origin[i][j-1].c1+origin[i][j+1].c1+origin[i+1][j-1].c1+origin[i+1][j].c1+origin[i+1][j+1].c1)/10;
						nodeMatrix[i][j].c2=(origin[i][j].c2*2+origin[i-1][j-1].c2+origin[i-1][j].c2+origin[i-1][j+1].c2+origin[i][j-1].c2+origin[i][j+1].c2+origin[i+1][j-1].c2+origin[i+1][j].c2+origin[i+1][j+1].c2)/10;
						nodeMatrix[i][j].c3=(origin[i][j].c3*2+origin[i-1][j-1].c3+origin[i-1][j].c3+origin[i-1][j+1].c3+origin[i][j-1].c3+origin[i][j+1].c3+origin[i+1][j-1].c3+origin[i+1][j].c3+origin[i+1][j+1].c3)/10;
						nodeMatrix[i][j].row=i;
						nodeMatrix[i][j].col=j;
						nodeMatrix[i][j].state=INITIAL;
						nodeMatrix[i][j].preNode=NULL;
						nodeMatrix[i][j].totalCost=-1;
						nodeMatrix[i][j].drawed=0;
						nodeMatrix[i][j].brushed=0;
						continue;
					}
					nodeMatrix[i][j].row=i;
					nodeMatrix[i][j].col=j;
					nodeMatrix[i][j].state=INITIAL;
					nodeMatrix[i][j].preNode=NULL;
					nodeMatrix[i][j].totalCost=-1;
					nodeMatrix[i][j].drawed=0;
					nodeMatrix[i][j].brushed=0;
					int c1=0,c2=0,c3=0;
					for(int k=-2;k<=2;k++)
						for(int l=-2;l<=2;l++)
						{
							c1+=origin[i+k][j+l].c1;
							c2+=origin[i+k][j+l].c2;
							c3+=origin[i+k][j+l].c3;
						}
					for(int k=-1;k<=1;k++)
						for(int l=-1;l<=1;l++)
						{
							c1+=origin[i+k][j+l].c1;
							c2+=origin[i+k][j+l].c2;
							c3+=origin[i+k][j+l].c3;
						}
					nodeMatrix[i][j].c1=(origin[i][j].c1+c1)/35;
					nodeMatrix[i][j].c2=(origin[i][j].c2+c2)/35;
					nodeMatrix[i][j].c3=(origin[i][j].c3+c3)/35;
				}
			break;
		case 7:
			origin=nodeMatrix;
			nodeMatrix = new Node*[height];
			for (int i=0;i<height;i++)
				nodeMatrix[i] = new Node[width];
			for (int i=0;i<height;i++)
				for (int j=0;j<width;j++)
				{
					if(i==0||i==height-1||j==0||j==width-1)
					{
						nodeMatrix[i][j]=origin[i][j];
						continue;
					}
					if(i<=2||i>=height-3||j<=2||j>=width-3)
					{
						nodeMatrix[i][j].c1=(origin[i][j].c1*2+origin[i-1][j-1].c1+origin[i-1][j].c1+origin[i-1][j+1].c1+origin[i][j-1].c1+origin[i][j+1].c1+origin[i+1][j-1].c1+origin[i+1][j].c1+origin[i+1][j+1].c1)/10;
						nodeMatrix[i][j].c2=(origin[i][j].c2*2+origin[i-1][j-1].c2+origin[i-1][j].c2+origin[i-1][j+1].c2+origin[i][j-1].c2+origin[i][j+1].c2+origin[i+1][j-1].c2+origin[i+1][j].c2+origin[i+1][j+1].c2)/10;
						nodeMatrix[i][j].c3=(origin[i][j].c3*2+origin[i-1][j-1].c3+origin[i-1][j].c3+origin[i-1][j+1].c3+origin[i][j-1].c3+origin[i][j+1].c3+origin[i+1][j-1].c3+origin[i+1][j].c3+origin[i+1][j+1].c3)/10;
						nodeMatrix[i][j].row=i;
						nodeMatrix[i][j].col=j;
						nodeMatrix[i][j].state=INITIAL;
						nodeMatrix[i][j].preNode=NULL;
						nodeMatrix[i][j].totalCost=-1;
						nodeMatrix[i][j].drawed=0;
						nodeMatrix[i][j].brushed=0;
						continue;
					}
					nodeMatrix[i][j].row=i;
					nodeMatrix[i][j].col=j;
					nodeMatrix[i][j].state=INITIAL;
					nodeMatrix[i][j].preNode=NULL;
					nodeMatrix[i][j].totalCost=-1;
					nodeMatrix[i][j].drawed=0;
					nodeMatrix[i][j].brushed=0;
					int c1=0,c2=0,c3=0;
					for(int k=-3;k<=3;k++)
						for(int l=-3;l<=3;l++)
						{
							c1+=origin[i+k][j+l].c1;
							c2+=origin[i+k][j+l].c2;
							c3+=origin[i+k][j+l].c3;
						}
					for(int k=-2;k<=2;k++)
						for(int l=-2;l<=2;l++)
						{
							c1+=origin[i+k][j+l].c1;
							c2+=origin[i+k][j+l].c2;
							c3+=origin[i+k][j+l].c3;
						}
					for(int k=-1;k<=1;k++)
						for(int l=-1;l<=1;l++)
						{
							c1+=origin[i+k][j+l].c1;
							c2+=origin[i+k][j+l].c2;
							c3+=origin[i+k][j+l].c3;
						}
					nodeMatrix[i][j].c1=(origin[i][j].c1+c1)/84;
					nodeMatrix[i][j].c2=(origin[i][j].c2+c2)/84;
					nodeMatrix[i][j].c3=(origin[i][j].c3+c3)/84;
				}
			break;
		default:
			break;
	}
	calcLinkCost();
}

void ISDoc::refreshCurmap(){
	curmap=new unsigned char[zw*zh*3];
	if(mode==IMAGE_ONLY)
	{
		for(int i=0;i<zh;i++)
			for(int j=0;j<zw;j++)
			{
				curmap[(i*zw+j)*3]=nodeMatrix[int(i/z)][int(j/z)].c1;
				curmap[(i*zw+j)*3+1]=nodeMatrix[int(i/z)][int(j/z)].c2;
				curmap[(i*zw+j)*3+2]=nodeMatrix[int(i/z)][int(j/z)].c3;
			}
	}
	else if (mode == WORK_MODE){
		for(int i=0;i<zh;i++)
			for(int j=0;j<zw;j++){
				if(nodeMatrix[int(i/z)][int(j/z)].brushed!=0){
					curmap[(i*zw+j)*3]=(int)nodeMatrix[int(i/z)][int(j/z)].c1*0.5;
					curmap[(i*zw+j)*3+1]=(int)nodeMatrix[int(i/z)][int(j/z)].c2*0.5;
					curmap[(i*zw+j)*3+2]=(int)nodeMatrix[int(i/z)][int(j/z)].c3*0.5;
				}
				if(nodeMatrix[int(i/z)][int(j/z)].drawed!=0){
					curmap[(i*zw+j)*3]=255;
					curmap[(i*zw+j)*3+1]=0;
					curmap[(i*zw+j)*3+2]=0;
				}
				else if (nodeMatrix[int(i/z)][int(j/z)].brushed==0){
					curmap[(i*zw+j)*3]=nodeMatrix[int(i/z)][int(j/z)].c1;
					curmap[(i*zw+j)*3+1]=nodeMatrix[int(i/z)][int(j/z)].c2;
					curmap[(i*zw+j)*3+2]=nodeMatrix[int(i/z)][int(j/z)].c3;
				}
			}

	}else{
		for(int i=0;i<zh;i++)
			for(int j=0;j<zw;j++){
				curmap[(i*zw+j)*3]=debugMatrix[int(i/z)][int(j/z)].c1;
				curmap[(i*zw+j)*3+1]=debugMatrix[int(i/z)][int(j/z)].c2;
				curmap[(i*zw+j)*3+2]=debugMatrix[int(i/z)][int(j/z)].c3;
			}
	}
	myUI->pic->refresh();
}

void ISDoc::calcLinkCost(){
	// Not calcute the out round of the pixels. To be handled later.

	maxD = 0.0;
	for (int i=1;i<height-1;i++)
		for (int j=1;j<width-1;j++)
			for (int d=0;d<8;d++){
				double DR,DG,DB;
				if (d%2==0){ //vertical or horizontal lines
					DR= abs( (nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c1 +
							  nodeMatrix[i+dir[(d+2)%8][0]][j+dir[(d+2)%8][1]].c1)/2.0 -
							 (nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c1 +
							  nodeMatrix[i+dir[(d+6)%8][0]][j+dir[(d+6)%8][1]].c1)/2.0 ) /2.0;
					DG= abs( (nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c2 +
							  nodeMatrix[i+dir[(d+2)%8][0]][j+dir[(d+2)%8][1]].c2)/2.0 -
							 (nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c2 +
							  nodeMatrix[i+dir[(d+6)%8][0]][j+dir[(d+6)%8][1]].c2)/2.0 ) /2.0;
					DB= abs( (nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c3 +
							  nodeMatrix[i+dir[(d+2)%8][0]][j+dir[(d+2)%8][1]].c3)/2.0 -
							 (nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c3 +
							  nodeMatrix[i+dir[(d+6)%8][0]][j+dir[(d+6)%8][1]].c3)/2.0 ) /2.0;
					nodeMatrix[i][j].D[d] = sqrt((DR*DR+DG*DG+DB*DB)/3);

					maxD = max(maxD, nodeMatrix[i][j].D[d]);
				}else{ // diagonal lines
					DR= abs( nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c1 - 
						 	 nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c1)/sqrt(2);
					DG= abs( nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c2 - 
						 	 nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c2)/sqrt(2);
					DB= abs( nodeMatrix[i+dir[(d+1)%8][0]][j+dir[(d+1)%8][1]].c3 - 
						 	 nodeMatrix[i+dir[(d+7)%8][0]][j+dir[(d+7)%8][1]].c3)/sqrt(2);
					nodeMatrix[i][j].D[d] = sqrt((DR*DR+DG*DG+DB*DB)/3);

					maxD = max(maxD, nodeMatrix[i][j].D[d]);
				}
			}

	for (int i=1;i<height-1;i++)
		for (int j=1;j<width-1;j++)
			for (int d=0;d<8;d++){
				nodeMatrix[i][j].linkCost[d] = ( maxD - nodeMatrix[i][j].D[d]);
				if (d%2==0)
					nodeMatrix[i][j].linkCost[d] *= 1;
				else
					nodeMatrix[i][j].linkCost[d] *= sqrt(2);
			}
}

void ISDoc::initStates(){
	for (int i=0;i<height;i++)
		for (int j=0;j<width;j++){
			nodeMatrix[i][j].totalCost = -1;
			nodeMatrix[i][j].preNode = NULL;
			nodeMatrix[i][j].state = INITIAL;
			nodeMatrix[i][j].preNodeDir=-1;
		}
}

int ISDoc::calcCostTree(int row,int col,int expand){ //return the max cost within expand number of vertices
	Node x;
	int di,dj;
	int cnt=0,maxCost=0;
	if (expand == -1) 
		expand = width*height;

	initStates();
	cnt++;
	q.push(nodeMatrix[row][col]);
	nodeMatrix[row][col].state = ACTIVE; 
	nodeMatrix[row][col].totalCost = 0;
	while (!q.empty()){
		x=q.top(); q.pop();
		nodeMatrix[x.row][x.col].state = EXPANDED;
		for (int d=0;d<8;d++){
			di = x.row + dir[d][0]; dj = x.col + dir[d][1];
			if (di<=0 || dj<=0 || di>=height-1 || dj>=width-1) continue;
			if (nodeMatrix[di][dj].state ==EXPANDED) continue;
			if (haveBrushed && nodeMatrix[di][dj].brushed==false) continue;
			//if (nodeMatrix[di][dj].drawed ==true) continue;
			if (nodeMatrix[di][dj].state == INITIAL && cnt<expand){
				nodeMatrix[di][dj].state = ACTIVE;
				nodeMatrix[di][dj].preNode = &nodeMatrix[x.row][x.col];
				nodeMatrix[di][dj].preNodeDir = d; //preNode's d direction is new node
				nodeMatrix[di][dj].totalCost = x.totalCost + x.linkCost[d];
				if(nodeMatrix[di][dj].totalCost>maxCost)
						maxCost = nodeMatrix[di][dj].totalCost;
				q.push(nodeMatrix[di][dj]);
				cnt++;
			}else{
				if (nodeMatrix[di][dj].totalCost > x.totalCost + x.linkCost[d]){
					nodeMatrix[di][dj].totalCost = x.totalCost + x.linkCost[d];
					if(nodeMatrix[di][dj].totalCost>maxCost)
						maxCost = nodeMatrix[di][dj].totalCost;
					nodeMatrix[di][dj].preNode = &nodeMatrix[x.row][x.col];
					nodeMatrix[di][dj].preNodeDir = d; //preNode's d direction is new node
				}
			}
		}
	}
	return maxCost;
}


void ISDoc::stopContour(){
	myUI->pic->contour=0;
}

void ISDoc::undo(){
	if(seed==NULL)
		return;

	if(last!=NULL){
		Node* p=&nodeMatrix[last->row][last->col];
		while( p->row!=seed->row || p->col!=seed->col )
		{
			if(p->drawed==curlayer)
				p->drawed=0;
			p=p->preNode;
		}
		last=NULL;
	}
	if(seeds.empty()){
		myUI->pic->contour=0;
		//myUI->pic->compContour=false;
		seed=NULL;
		refreshCurmap();
		return;
	}
	if(myUI->pic->contour==2)
	{
		myUI->pic->contour=1;
		refreshCurmap();
		return;
	}
	if(myUI->pic->contour==0)
	{
		myUI->pic->contour=1;
		refreshCurmap();
		return;
	}
	last=seed;
	seed=new Point;
	(*seed)=seeds.top();
	seeds.pop();
	curlayer--;
	calcCostTree(seed->row,seed->col,-1);
	if(last!=NULL)//????
	{
		Node* p=&nodeMatrix[last->row][last->col];
		while( p->row!=seed->row || p->col!=seed->col )
		{
			if(p->drawed==curlayer)
				p->drawed=0;
			p=p->preNode;
		}
		last=NULL;
	}
	
	// last=temp;
	//drawContour(temp->row,temp->col);
	refreshCurmap();
}

void ISDoc::setSeed(int row,int col){
	if(seed!=NULL)
	{
		seeds.push(*seed);
		delete seed;
	}
	curlayer++;
	seed=new Point;
	seed->row=row;
	seed->col=col;
	calcCostTree(seed->row,seed->col,-1);
}

void ISDoc::closeContour(){
	// Point temp;
	// while(!seeds.empty())
	// {
	// 	temp=seeds.top();
	// 	seeds.pop();
	// }
	//seeds.push(*startSeed);
	drawContour(startSeed->row,startSeed->col);
	myUI->pic->contour=2;
//	myUI->pic->compContour=true;
}

void ISDoc::setStartSeed(int row,int col)
{
	startSeed=new Point;
	startSeed->col=col;
	startSeed->row=row;
}

void ISDoc::setText(int row,int col)
{
	string output="x: ";
	output.append(to_string(col));
	output.append(" y: ");
	output.append(to_string(row));
	output.append(" R: ");
	output.append(to_string(nodeMatrix[row][col].c1));
	output.append(" G: ");
	output.append(to_string(nodeMatrix[row][col].c2));
	output.append(" B: ");
	output.append(to_string(nodeMatrix[row][col].c3));
	myUI->text->value(output.c_str());
}


