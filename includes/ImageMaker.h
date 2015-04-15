/********************************************//**
 * @file ImageMaker.h
 * @short creates image data from file
 ***********************************************/
#ifndef IMAGEMAKER_H
#define IMAGEMAKER_H

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"

#include <string.h>
using namespace std;

/**
 *@short creates image data from file
 */

class ImageMaker {


public:

	/**
	 * Constructor
	 * creates the tcp interface and the udp class
	 * @param dr dyanamic range
	 * @param tg ten giga enable
	 * @param top true if its top, else bottom
	 */
	ImageMaker(int dr, int tg, bool top = true);

	/**
	 * Destructor
	 */
	~ImageMaker();

	/**
	 * Opens file
	 * @param fname file name
	 * @param file stream
	 * returns ok if success or fail
	 */
	int openFile(string fname, ifstream &infile);

	/**
	 * closes file
	 * @param infile file pointer
	 */
	void closeFile(ifstream &infile);

	/**
	 * reads one from from file
	 * @param infile file pointer
	 */
	char* readOneFrame(ifstream &infile);

	/**
	 * Gets value for the particular coordinate
	 * @param ix x coordinate
	 * @param iy y coordinate
	 * @param buff the buffer
	 */
	double getValue(int ix, int iy, char* buff);


private:
	int dynamicrange;
	int tenGiga;
	int dataSize;

	slsReceiverData <uint32_t> *receiverData;
};



#endif
