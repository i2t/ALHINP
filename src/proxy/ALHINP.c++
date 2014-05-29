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





using namespace rofl;

ALHINP::ALHINP()
{
    //listen for OUI
    discover= new discovery(this);
    manager= new orchestrator(this);
    rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));
    //listen for AGG
    //rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));

}

ALHINP::ALHINP(const ALHINP& orig) {
}

ALHINP::~ALHINP() {
    
}
void
ALHINP::handle_dpath_open(cofdpt* dpt){
    if(discover->is_aggregator(dpt->get_dpid())){
        manager->AGS_connected(dpt);
    }else{
       // manager->OUI_connected(dpt);
    }

}
void
ALHINP::handle_dpath_close(cofdpt* dpt){
    if(discover->is_aggregator(dpt->get_dpid())){
        //AGS connected
        manager->AGS_disconnected(dpt);
        
    }else{
        //An OUI is connected
        manager->OUI_disconnected();
    }    
}
void
ALHINP::handle_ctrl_open(cofctl *ctl){
    controller=ctl;
}
void
ALHINP::handle_ctrl_close(cofctl *ctl){
    controller=0;
    std::cout<<"[WARNING]: Controller disconnected\n";
}
void
ALHINP::handle_packet_in(cofdpt *dpt, cofmsg_packet_in *msg){
    
    manager->dispath_PACKET_IN(dpt,msg);
}

