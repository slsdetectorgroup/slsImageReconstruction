/* Reads files to get values for images
 * main_csaxs.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: tinti_g
 */

#include "sls_receiver_defs.h"
#include "slsReceiverData.h"
#include "eigerHalfModuleData.h"
#include "ansi.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
using namespace std;


int main(int argc, char *argv[]) {

	if(argc < 2){
		cprintf(RED, "Error: Not enough arguments: cbfMakerMulti [file_name_with_dir] \nExiting.\n");
		exit(-1);
	}

	string exe = string(argv[0]);
	size_t exePos = exe.rfind("1.5M");
	if( exePos!= string::npos){
	  exe.erase(exePos,4);
	  
	  //1.5M
	  string command = exe + " " + string(argv[1]) + " 3072 512 1";
	  cout<<"command:"<<command<<endl;
	  system(command.c_str());
	}

	exePos = exe.rfind("9M");
	if( exePos!= string::npos){
	  exe.erase(exePos,2);
	  //9M
	  string command = exe + " " + string(argv[1]) + " 3072 3072 0";
	  cout<<"command:"<<command<<endl;
	  system(command.c_str());
	}

	
	return slsReceiverDefs::OK;
}

