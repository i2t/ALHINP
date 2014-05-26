/* 
 * File:   main.cpp
 * Author: victor
 *
 * Created on 10 de octubre de 2013, 16:27
 */

#include <cstdlib>
#include <signal.h>
#include <rofl/platform/unix/cunixenv.h>
#include "src/proxy/ALHINP.h"
#include <sys/wait.h>

using namespace rofl;
using namespace std;

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

void TCL_finish(int dummy=0) {
	//Only stop ciosrv 
    wait(NULL);
    std::cout<<"Alma recogida\n";
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

	//Orchestrator instance creation
        orchestrator orchestrator;
        //Start listening for AGGR_DPID
	orchestrator.rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));
        //Start listening for CLIENT_DPIDs
        orchestrator.rpc_listen_for_dpts(caddress(AF_INET, "192.168.10.1", 6633));
        //Start connection to the controller
        std::cout<<"Trying to connect controller: 10.10.10.3@6633\n";
        uint8_t retry_count_ctrl=0;
        //orchestrator.rpc_connect_to_ctl(OFP10_VERSION,1,caddress(AF_INET, CONTROLLER_IP, CONTROLLER_PORT));
      //  try{
               orchestrator.rpc_connect_to_ctl(OFP10_VERSION,1,caddress(AF_INET, CONTROLLER_IP, CONTROLLER_PORT));
                
      //  }catch(eSocketError, eSocketConnectFailed){
      //          std::cout<<"Trying to connect controller @ 10.10.10.3@6633\n";
      //          sleep(5);
      //  }  
        
	
        //ciosrv run. Only will stop in Ctrl+C
	ciosrv::run();

	//Printing nice trace
	printf("\nCleaning the house...\n");
        fflush(stdout);
        
	//ciosrv destroy
	ciosrv::destroy();
	printf("House cleaned!\nGoodbye\n");
        fflush(stdout);
	
	exit(EXIT_SUCCESS);
}



