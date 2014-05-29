/* 
 * File:   orchestrator.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:06
 */

#include <rofl/common/crofbase.h>
#include "orchestrator.h"
#include "../proxy/ALHINP.h"
#include "../discovery/discovery.h"

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

//void
//orchestrator::OUI_connected(cofpdt* dpt){
//    
//}
void orchestrator::OUI_disconnected(){
    
}
void orchestrator::AGS_connected(cofdpt* dpt){
    
    AGS_reset_flows(dpt);
    AGS_set_port_behavior(dpt);
    proxy->discover->detect_CM(dpt);
    proxy->discover->detect_DPS_traffic(dpt);
    proxy->discover->detect_OUI_control_traffic(dpt);
    std::cout<<"AGS succesfully configured!\n";
    fflush(stdout);
}
void orchestrator::AGS_reset_flows(cofdpt* dpt){
    //RESET any existing flow
    cflowentry reset(OFP12_VERSION);
        reset.set_command(OFPFC_DELETE);
        reset.set_table_id(255);
        reset.set_cookie(cookie_mask);
        reset.set_cookie_mask(cookie_mask);
    proxy->send_flow_mod_message(dpt,reset);
    std::cout<<"Flow RESET over AGS\n";
    fflush(stdout);
}
void orchestrator::AGS_set_port_behavior(cofdpt* dpt){
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
                default1.set_table_id(0);
                default1.set_cookie(cookie_mask);
                default1.set_cookie_mask(cookie_mask);
                default1.match.set_in_port(port_it->first);
                default1.instructions.next() = cofinst_goto_table(OFP12_VERSION,1);
            proxy->send_flow_mod_message(dpt,default1);
        }
     
    }

}
void orchestrator::AGS_disconnected(cofdpt* dpt){
    
}


void orchestrator::CTRL_connected(){
    
}
void orchestrator::CTRL_disconnected(){
    
}

void orchestrator::OUI_set_port_behavior(cofdpt* dpt){
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

void orchestrator::dispath_PACKET_IN(cofdpt *dpt, cofmsg_packet_in *msg){

    if(dpt->get_dpid()==AGG_DPID) { //PACKET_INs generated at AGGREGATION
        if(msg->get_table_id()==1&&msg->get_match().get_in_port()!=CMTS_PORT){
            //CLIENT TRAFFIC -> transmit to Controller if connected
            
            cofmatch m_match(OFP12_VERSION);
            m_match=msg->get_match();
            uint32_t translated_port;
            //translated_port=docsis.translator.translate_real_port(dpt->get_dpid(),in_port);
            try{
                m_match.set_in_port(translated_port);
                proxy->send_packet_in_message(proxy->controller, msg->get_buffer_id(), msg->get_total_len(), msg->get_reason(), 0, msg->get_cookie(), (uint16_t)translated_port,m_match, msg->get_packet().soframe(), msg->get_packet().framelen()); 
                //printf("packet_in received from %" PRIu64 " and in_port %" PRIu16 " (sending to controller)\n",in_dpid,in_port);
                fflush(stdout);
            }catch (eRofBaseNotConnected e){
                   std::cout<<"WARNING: No Controller connected (discarding PACKEt_IN message)\n";
            }
            delete msg; 
            return;
        }
        
        fflush(stdout);
        try{
        if(msg->get_match().get_udp_dst()==67 && msg->get_match().get_ipv4_src_value().get_ipv4_addr()==caddress(AF_INET,"10.10.20.1").get_ipv4_addr()){ // CM_traffic BOOTP
            std::string string;
            string = msg->get_packet().payload()->c_str();
            std::string::iterator end_pos = std::remove(string.begin(), string.end(), ' ');
            string.erase(end_pos, string.end());
            unsigned pos = string.find("a4a24a");
            std::string bootp_mac;
            
            bootp_mac = string.substr(pos,12);
            std::string bootp_mac2 =bootp_mac;
            bootp_mac.insert(2,":");
            bootp_mac.insert(5,":");
            bootp_mac.insert(8,":");
            bootp_mac.insert(11,":");
            bootp_mac.insert(14,":");
            
            cmacaddr mac(bootp_mac.c_str());

            fflush(stdout);
            uint16_t vlan=proxy->discover->register_CM(mac.get_mac());
            
            if(vlan!=0){
                std::cout << "New CableModem registered with MAC: " <<mac.c_str() <<"\n";
                std::cout << "VLAN assigned:"<< vlan<< "\n";
            fflush(stdout);
            //docsis.CMTS.assign_vlan((char*)mac.c_str(),assigned_vlan,1);
            }else{
                std::cout << "CableModem RETRY with MAC: " <<mac.c_str() <<"\n";
            }
            cofaclist aclist(OFP12_VERSION);
            aclist.next() = cofaction_output(OFP12_VERSION,DPS_PORT);
            proxy->send_packet_out_message(dpt,msg->get_buffer_id(),CMTS_PORT,aclist);
            delete msg; 
            return;            
        }
        }catch(...){}
        cmacaddr OUI(OUI_MAC);
        if(msg->get_match().get_in_port()==CMTS_PORT && msg->get_match().get_eth_src_addr().get_mac()-OUI.get_mac()<OUI.get_mac()){
            std::cout << "OUI detected: "<< msg->get_match().get_eth_src_addr().c_str() <<" with VLAN "<< msg->get_match().get_vlan_vid() <<"\n";
            fflush(stdout);
            proxy->discover->AGS_enable_OUI_traffic(dpt , msg->get_match().get_eth_src_addr().get_mac(), msg->get_match().get_vlan_vid());
            delete msg; 
            return;
        }
        
    }
    delete msg; 
    return;
}//ready for 1.0
