/* 
 * File:   main.cpp
 * Author: victor
 *
 * Created on 10 de octubre de 2013, 16:27
 */

#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/ciosrv.h>
#include "src/proxy/ALHINP.h"

using namespace rofl;

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

void TCL_finish(int dummy=0) {
	//Only stop ciosrv 
    wait(NULL);
    
}

int main(int argc, char** argv){


	//Capture control+C
	signal(SIGINT, interrupt_handler);
        signal(SIGCLD, TCL_finish);

	/* update defaults */
	rofl::csyslog::initlog(
			rofl::csyslog::LOGTYPE_FILE,//STDERR
			rofl::csyslog::DBG,//EMERGENCY
			std::string("./controlador.log"),
			"an example: ");

	rofl::csyslog::set_debug_level("ciosrv", "emergency");
	rofl::csyslog::set_debug_level("cthread", "emergency");

        rofl::ciosrv::init();
        
        
        ALHINP ofdocsis (argv[1]);
        

        //ciosrv run. Only will stop in Ctrl+C
	rofl::ciosrv::run();

	//Printing nice trace
	printf("\nCleaning the house...\n");
        fflush(stdout);
        
	//ciosrv destroy
	rofl::ciosrv::destroy();
	printf("House cleaned!\nGoodbye\n");
        fflush(stdout);
	
	exit(EXIT_SUCCESS);
}