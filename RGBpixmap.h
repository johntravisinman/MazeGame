#ifndef _RGBPIXMAP
#define _RGBPIXMAP

#include <string>
#include <iostream>
#include <fstream>
#include <strstream>
#include <gl/glut.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;

using namespace std;

class IntPoint {
public:
	int x, y;
	IntPoint() { x = y = 0; }
	IntPoint(int xx, int yy) { x = xx; y = yy; } // constructor
	void set(int xx, int yy) { x = xx; y = yy; }
	void set(IntPoint p) { x = p.x; y = p.y; }
};

class IntRect {
public:
	int left, top, right, bott;
	void set(int ll, int tt, int rr, int bb) { left = ll; top = tt; right = rr; bott = bb; }
	void set(IntRect r) { left = r.left; top = r.top; right = r.right; bott = r.bott; }
	void print()
	{
		cout << "IntRect: " << left << " " << top << " " << right << " " << bott << endl;
		int nCols = right - left;
		int nBytes = 3 * nCols;
		int nBytesInRow = ((nBytes + 3) / 4) * 4;
		int numPadBytes = nBytesInRow - nBytes;
		cout << " nCols, # pad bytes = " << nCols << "," << numPadBytes << endl;
	}
	void fix()
	{ // reestablish order of left, right, etc.
		if (left > right) { int tmp = left; left = right; right = tmp; } //swap
		if (bott > top) { int temp = top; top = bott; bott = temp; }
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<< drawRubber >>>>>>>>>>>
	void draw() // draw rectangle
	{
		glBegin(GL_LINE_LOOP);
		glVertex2i(left, top);  glVertex2i(right, top);
		glVertex2i(right, bott);	glVertex2i(left, bott);
		glEnd();
		glFlush();
	}
	void drawDiag() // draw diagonal of rect (for rubber line)
	{
		glBegin(GL_LINES);
		glVertex2i(left, top);
		glVertex2i(right, bott);
		glEnd();
	}
}; // end of IntRect
   
class mRGB { // the name RGB is reserved in windows
public: uchar r, g, b;
		mRGB() { r = g = b = 0; }
		mRGB(mRGB& p) { r = p.r; g = p.g; b = p.b; }
		mRGB(uchar rr, uchar gg, uchar bb) { r = rr; g = gg; b = bb; }
		void set(uchar rr, uchar gg, uchar bb) { r = rr; g = gg; b = bb; }
};

class RGBpixmap {
private:
	mRGB* pixel; // array of pixels

public:
	int nRows, nCols; // dimensions of the pixmap
	RGBpixmap() { nRows = nCols = 0; pixel = 0; }
	RGBpixmap(int rows, int cols) //constructor
	{
		nRows = rows;
		nCols = cols;
		pixel = new mRGB[rows*cols];
	}
	int readBMPFile(string fname); // read BMP file into this pixmap

	void setTexture(GLuint textureName);
	void makeCheckerboard();

	int writeBMPFile(string fname); // write this pixmap to a BMP file
	void freeIt() // give back memory for this pixmap
	{
		delete[]pixel;
		nRows = nCols = 0;
	}
	//<<<<<<<<<<<<<<<<<< copy >>>>>>>>>>>>>>>>>>>
	void copy(IntPoint from, IntPoint to, int x, int y, int width, int height)
	{ // copy a region of the display back onto the display
		if (nRows == 0 || nCols == 0) return;
		glCopyPixels(x, y, width, height, GL_COLOR);
	}
	//<<<<<<<<<<<<<<<<<<< draw >>>>>>>>>>>>>>>>>
	void draw()
	{ // draw this pixmap at current raster position
		if (nRows == 0 || nCols == 0) return;
		//tell OpenGL NOT to try to align pixels to 4 byte boundaries in memory
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glDrawPixels(nCols, nRows, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	}
	//<<<<<<<<<<<<<<<<< read >>>>>>>>>>>>>>>>
	int read(int x, int y, int wid, int ht)
	{ // read a rectangle of pixels into this pixmap
		nRows = ht;
		nCols = wid;
		pixel = new mRGB[nRows *nCols]; if (!pixel) return -1;
		//tell OpenGL NOT to try to align pixels to 4 byte boundaries in memory
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(x, y, nCols, nRows, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		return 0;
	}
	//<<<<<<<<<<<<<<<<< read from IntRect >>>>>>>>>>>>>>>>
	int read(IntRect r)
	{ // read a rectangle of pixels into this pixmap
		nRows = r.top - r.bott;
		nCols = r.right - r.left;
		pixel = new mRGB[nRows *nCols]; if (!pixel) return -1;
		//tell OpenGL NOT to try to align pixels to 4 byte boundaries in memory
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(r.left, r.bott, nCols, nRows, GL_RGB, GL_UNSIGNED_BYTE, pixel);
		return 0;
	}
	//<<<<<<<<<<<<<< setPixel >>>>>>>>>>>>>
	void setPixel(int x, int y, mRGB color)
	{
		if (x >= 0 && x <nCols && y >= 0 && y < nRows)
			pixel[nCols * y + x] = color;
	}
	//<<<<<<<<<<<<<<<< getPixel >>>>>>>>>>>
	mRGB getPixel(int x, int y)
	{
		mRGB bad(255, 255, 255);
		if (x < 0 || x >= nCols || y < 0 || y >= nRows)
		{
			cout << "\nx,y = " << x << "," << y << " bad in getPixel()";
			return bad;
		}
		return pixel[nCols * y + x];
	}
};

fstream inf;  // global in this file for convenience
fstream outf; // ditto

// write short in little-endian form
void putShort(ushort i)
{ 
	outf.put((char)(i & 0xff));
	outf.put((char)((i >> 8) & 0xff));
}

// write long in little-endian form
void putLong(ulong i)
{ 
	outf.put((char)(i & 0xff));
	outf.put((char)((i >> 8) & 0xff));
	outf.put((char)((i >> 16) & 0xff));
	outf.put((char)((i >> 24) & 0xff));
}

// read a short in little-endian form
ushort getShort()
{	
	char ic;
	ushort ip;
	inf.get(ic); ip = ic;  //first byte is little one 
	inf.get(ic);  ip |= ((ushort)ic << 8); // or in high order byte
	return ip;
}

// get little-endian 4-byte value from file, compose along portably
ulong getLong()
{	
	ulong ip = 0;
	char ic = 0;
	unsigned char uc = ic;
	inf.get(ic); uc = ic; ip = uc;
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 8);
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 16);
	inf.get(ic); uc = ic; ip |= ((ulong)uc << 24);
	return ip;
}

void RGBpixmap::setTexture(GLuint textureName)
{
	glBindTexture(GL_TEXTURE_2D, textureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nCols, nRows, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
}

void RGBpixmap::makeCheckerboard()
{
	nRows = nCols = 64;
	pixel = new mRGB[3 * nRows * nCols];
	if (!pixel) { cout << "out of memory!"; return; }
	long count = 0;
	for (int i = 0; i < nRows; i++)
		for (int j = 0; j < nCols; j++)
		{
			if (((i / 8) + (j / 8)) % 2 == 0)
			{
				pixel[count].r = 0; //red
				pixel[count].g = 255; //green
				pixel[count++].b = 127; //blue
			}
			else {
				pixel[count].r = 255; //red
				pixel[count].g = 69; //green
				pixel[count++].b = 0; //blue
			}
		}
}

// Read into memory an mRGB image from an uncompressed BMP file.
// return 0 on failure, 1 on success
int RGBpixmap::readBMPFile(string fname)
{  
	inf.open(fname.c_str(), ios::in | ios::binary); //must read raw binary char's.
	if (!inf) { cout << " TEST: can't open file: " << fname << endl; return 0; }
	int k, row, col, numPadBytes, nBytesInRow;
	// read header information
	char ch1, ch2;
	inf.get(ch1); inf.get(ch2); // type is always 'BM'
								//cout << "file type = " << ch1 << ch2 << endl;
	ulong fileSize = getLong();
	ushort reserved1 = getShort();     // always 0
	ushort reserved2 = getShort();     // always 0 
	ulong offBits = getLong();	    // offset to image - unreliable
	ulong headerSize = getLong();      // always 40
	ulong numCols = getLong();	    // number of columns in image
	ulong numRows = getLong();	    // number of rows in image
	ushort planes = getShort();     // always 1 
	ushort bitsPerPix = getShort();    // 8 or 24;only 24 bit case done 
	ulong compression = getLong();     // must be 0 for umcompressed 
	ulong imageSize = getLong();      // total bytes in image 
	ulong xPels = getLong();      // always 0 
	ulong yPels = getLong();      // always 0 
	ulong numLUTentries = getLong();   // 256 for 8 bit, otherwise 0 
	ulong impColors = getLong();      // always 0 
	if (bitsPerPix != 24) { cout << "not a 24 bit/pixelimage!\n"; inf.close(); return 0; }; // error!
	// in BMP file, pad bytes inserted at end of each row so total number is a mult. of 4
	nBytesInRow = ((3 * numCols + 3) / 4) * 4; // round up 3*numCols to next mult. of 4
	numPadBytes = nBytesInRow - 3 * numCols; // need this many
	nRows = numRows; // set class's data members
	nCols = numCols;
	cout << "numRows,numCols = " << numRows << "," << numCols << endl;
	cout.flush();
	pixel = new mRGB[nRows * nCols]; //space for array in memory
	if (!pixel) return 0; // out of memory!
	long count = 0;
	char dum, r, g, b;

	for (row = 0; row < nRows; row++) // read pixel values
	{
		for (col = 0; col < nCols; col++)
		{
			inf.get(b); inf.get(g); inf.get(r); // funny color order in BMP file
			pixel[count].r = r; pixel[count].g = g; pixel[count++].b = b;
		}
		for (k = 0; k < numPadBytes; k++) //skip over padding bytes at row's end
			inf >> dum;
	}
	inf.close();
	return 1; // success
}

int RGBpixmap::writeBMPFile(string fname)
{
	outf.open(fname.c_str(), ios::out | ios::binary);
	if (!outf) { cout << " can't open file!\n"; return 0; }
	if ((nRows <= 0) || (nCols <= 0)) { cout << "\n degenerate image!\n"; return 0; }

	// Must write a multiple of four bytes in each row of the image
	ushort nBytesInRow = ((3 * nCols + 3) / 4) * 4; // round up 3 * nCols to next mult. of 4
	int numPadBytes = nBytesInRow - 3 * nCols; // num of pad bytes at end of each row
	ulong biSizeImage = nBytesInRow * (ulong)nRows; // size of image
													//write pixmap FileHeader
	ushort bfType = 0x4d42;	// 'BM';'B'=0x42, 'M'=0x4d
	putShort(bfType); // write it to file
	ulong bfSize = 54 + biSizeImage; //total size of image
	putLong(bfSize);
	putShort(0); putShort(0);// reserved 1 & 2
	putLong(ulong(54)); //bfOffBits: 54 - but not used in readers
	putLong(ulong(40)); // biSize - bytes in info header
	putLong(ulong(nCols)); putLong(ulong(nRows));
	putShort(ushort(1));   putShort(ushort(24)); // bit planes & bit count
	putLong(0L); // compression
	putLong(biSizeImage);
	putLong(0L); putLong(0L);//pelsPerMeterX and Y
	putLong(0L); putLong(0);//colors used & important colors
	
	//##############write bytes  ###############################
	long count = 0;
	for (int row = 0; row < nRows; row++)
	{
		for (int col = 0; col < nCols; col++)
		{
			outf.put(pixel[count].b);
			outf.put(pixel[count].g);
			outf.put(pixel[count++].r);
		}
		//now pad this row so num bytes is a mult of 4
		for (int k = 0; k < numPadBytes; k++) //write dummy bytes to pad out row
			outf.put(char(0)); //padding bytes of 0
	}
	outf.close();
	return 1;  //success
}

#endif
