/*
 * main.cpp
 *
 *  Created on: Mar 16, 2015
 *      Author: maliakal_d
 */

#include "sls_receiver_defs.h"
#include <iostream>
#include <string.h>
#include <map>
#include <getopt.h>

#include "ImageMaker.h"
using namespace std;

int getParameters(int argc, char *argv[], int &dr, string &fn, int &tg);


int main(int argc, char *argv[]) {

	//get config parameters
	string fname = "";
	int dynamicrange = 16;
	int tenGiga = 0;
	if(getParameters(argc, argv, dynamicrange,fname,tenGiga) == slsReceiverDefs::OK){

		cout << "\n\nDynamic range:"<< dynamicrange << "\nFile name:" << fname << "\nTen giga:"<< tenGiga << endl << endl;;

		//construct image
		ImageMaker *topImageMaker = new ImageMaker(dynamicrange, tenGiga, true);
		topImageMaker->processFile(fname);
		ImageMaker *bottomImageMaker = new ImageMaker(dynamicrange, tenGiga, false);
		bottomImageMaker->processFile(fname2);




	}
	cout << endl << "Goodbye!" << endl;
	return slsReceiverDefs::OK;
}



int getParameters(int argc, char *argv[], int &dr, string &fn, int &tg){

	map<string, string> configuration_map;
	static struct option long_options[] = {
			{"dr",				required_argument,       0, 'd'},
			{"tengiga",    		required_argument,       0, 't'},
			{"fname",     		required_argument,       0, 'f'},
			{"help",  			no_argument,      		 0, 'h'},
			{0, 0, 0, 0}
	};
	int option_index = 0, c;
	while ( c != -1 ){
		c = getopt_long (argc, argv, "dtfh", long_options, &option_index);
		if (c == -1)//end of options
			break;
		switch(c){
		case 'd':
			if(sscanf(optarg, "%d", &dr) <=0){
				cprintf(RED,"ERROR: Cannot parse dynamic range. Options: 4,8,16,32.\n");
				return slsReceiverDefs::FAIL;
			}
			switch(dr){
			case 4:
			case 8:
			case 16:
			case 32: break;
			default:
				cprintf(RED,"ERROR: Invalid dynamic range. Options: 4,8,16,32.\n");
				return slsReceiverDefs::FAIL;
			}
			break;
			case 't':
				if(sscanf(optarg, "%d", &tg) <=0){
					cprintf(RED,"ERROR: Cannot parse ten giga parameter. Options: 0 or 1.\n");
					return slsReceiverDefs::FAIL;
				}
				switch(tg){
				case 0:
				case 1: break;
				default:
					cprintf(RED,"ERROR: Invalid ten giga parameter. Options: 0 or 1.\n");
					return slsReceiverDefs::FAIL;
				}
				break;
		case 'f':
			fn = optarg;
			if(fn.empty()){
				cprintf(RED,"ERROR: Empty file name.\n");
				return slsReceiverDefs::FAIL;
			}
			break;
		case 'h':
			string help_message = """\nSLS Image Reconstruction\n\n""";
			help_message += """usage: image --f config_fname [--dr dynamic_range]\n\n""";
			help_message += """\t--dr:\t dynamic range or bit mode. Default:16. Options: 4,8,16,32.\n""";
			help_message += """\t--tengiga:\t 1 if 10Gbe or 0 for 1Gbe. Default: 0\n""";
			help_message += """\t--fname:\t file name of image\n\n""";
			cout << help_message << endl;
			break;

		}
	}

	//cout << " dynamic range:"<< dr << " file name:" << fn << " ten giga:"<< tg << endl;
	return slsReceiverDefs::OK;
}
