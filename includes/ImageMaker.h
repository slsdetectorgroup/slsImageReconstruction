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
	 * Opens file and process its
	 * @param fname file name
	 * returns success or fail
	 */
	int processFile(string fname);


private:
	int dynamicrange;
	int tenGiga;
	int dataSize;

	slsReceiverData <uint32_t> *receiverData;
};



#endif
