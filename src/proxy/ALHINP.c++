/* 
 * File:   ALHINP.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:07
 */

#include "ALHINP.h"
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/openflow.h>
#include <rofl/common/openflow/coxmatch.h>
#include <list>

#define CONTROLLER_IP "158.227.98.5"
#define CONTROLLER_OF "1.0"
#define CONTROLLER_PORT "6633"
#define OF_LISTEN_PORT "6633"
#define AGGR_LISTEN_IP "158.227.98.21"
#define OUI_LISTEN_IP "158.227.98.6"



using namespace rofl;

ALHINP::ALHINP()
{
    //listen for OUI
    discovery network;
    orchestrator orchestrator;
    rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));
    //listen for AGG
    rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));

}

ALHINP::ALHINP(const ALHINP& orig) {
}

ALHINP::~ALHINP() {
    
}
void
ALHINP::handle_dpath_open(cofdpt* dpt){
    if(dpt->get_dpid()==docsis.get_aggr_dpid()){ //Aggr. device init
        init_aggr_device(dpt);
        std::cout<< "Aggregation switch succesfully connected\n";
        fflush(stdout);
        
    }
    if(dpt->get_dpid()!=docsis.get_aggr_dpid()){ //DOCSIS device init
        init_client_device(dpt); 
        std::cout<< "Client switch succesfully connected\n";
        fflush(stdout);
    }
}

