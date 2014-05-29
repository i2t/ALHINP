/* 
 * File:   orchestrator.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:06
 */

#include <rofl/common/crofbase.h>
#include "orchestrator.h"
#include "../proxy/ALHINP.h"

#define __STDC_FORMAT_MACROS

#define C_TAG 0x8100
#define S_TAG 0x88A8



#define cookie_mask 0x8000000000000000ULL
#define flow_mask   0x7FFFFFFFFFFFFFFFULL

orchestrator::orchestrator(ALHINP *ofproxy) {
    proxy=ofproxy;
}

orchestrator::orchestrator(const orchestrator& orig) {
}

orchestrator::~orchestrator() {
}

void
orchestrator::OUI_connected(){
    
}
void
orchestrator::OUI_disconnected(){
    
}
void
orchestrator::AGS_connected(cofdpt* dpt){
    AGS_reset_flows(dpt);
    AGS_set_port_behavior(dpt);
    std::cout<<"AGS succesfully inited";
}
void
orchestrator::AGS_disconnected(cofdpt* dpt){
    
}
void
orchestrator::CTRL_connected(){
    
}
void
orchestrator::CTRL_disconnected(){
    
}
void
orchestrator::AGS_reset_flows(cofdpt* dpt){
    //RESET any existing flow
    cflowentry reset(OFP12_VERSION);
        reset.set_command(OFPFC_DELETE);
        reset.set_table_id(255);
        reset.set_cookie(cookie_mask);
        reset.set_cookie_mask(cookie_mask);
    proxy->send_flow_mod_message(dpt,reset);
    std::cout<<"Flow RESET over AGS";
}
void 
orchestrator::AGS_set_port_behavior(cofdpt* dpt){
    std::map<uint32_t, cofport*>::iterator port_it;
    std::map<uint32_t, cofport*> portlist= dpt->get_ports();
    for (port_it = portlist.begin(); port_it != portlist.end(); ++port_it) {
        
        if(proxy->discover->is_hidden_port(port_it->first)){
            cflowentry fe_NO_PACKET_IN(OFP12_VERSION); 
                fe_NO_PACKET_IN.set_command(OFPFC_ADD);
                fe_NO_PACKET_IN.set_idle_timeout(0);
                fe_NO_PACKET_IN.set_hard_timeout(0);
                fe_NO_PACKET_IN.set_table_id(0);
                fe_NO_PACKET_IN.set_cookie(cookie_mask+1);
                fe_NO_PACKET_IN.set_cookie_mask(cookie_mask);
                fe_NO_PACKET_IN.set_flags(0);
                fe_NO_PACKET_IN.set_priority(1);
                fe_NO_PACKET_IN.match.set_in_port(port_it->first);
            proxy->send_flow_mod_message(dpt,fe_NO_PACKET_IN);
        }else{
            cflowentry default1(OFP12_VERSION);
                default1.set_command(OFPFC_ADD);
                default1.set_idle_timeout(0);
                default1.set_hard_timeout(0);
                default1.set_table_id(1);
                default1.set_cookie(cookie_mask);
                default1.set_cookie_mask(cookie_mask);
                default1.match.set_in_port(port_it->first);
                default1.instructions.next() = cofinst_goto_table(OFP12_VERSION,1);
            proxy->send_flow_mod_message(dpt,default1);
        }
     
    }

}
void 
orchestrator::OUI_set_port_behavior(cofdpt* dpt){
    std::map<uint32_t, cofport*>::iterator port_it;
    std::map<uint32_t, cofport*> portlist= dpt->get_ports();
    for (port_it = portlist.begin(); port_it != portlist.end(); ++port_it) {
        
        if(proxy->discover->OUI_is_hidden_port(port_it->first)){
            cflowentry fe_NO_PACKET_IN(OFP12_VERSION); 
                fe_NO_PACKET_IN.set_command(OFPFC_ADD);
                fe_NO_PACKET_IN.set_idle_timeout(0);
                fe_NO_PACKET_IN.set_hard_timeout(0);
                fe_NO_PACKET_IN.set_table_id(0);
                fe_NO_PACKET_IN.set_cookie(cookie_mask+1);
                fe_NO_PACKET_IN.set_cookie_mask(cookie_mask);
                fe_NO_PACKET_IN.set_flags(0);
                fe_NO_PACKET_IN.set_priority(1);
                fe_NO_PACKET_IN.match.set_in_port(port_it->first);
            proxy->send_flow_mod_message(dpt,fe_NO_PACKET_IN);
        }
     
    }

}