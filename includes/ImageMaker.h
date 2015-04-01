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
	 * @param fn file name
	 * @param dr dyanamic range
	 * @param tg ten giga enable
	 */
	ImageMaker(string fn, int dr, int tg);

	/**
	 * Destructor
	 */
	~ImageMaker();

	/**
	 * Opens file and process its
	 */
	void processFile();


private:
	string fname;
	int dynamicrange;
	int tenGiga;

	slsReceiverData <uint32_t> *receiverData;

};



#endif
