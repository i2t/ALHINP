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
#include <rofl/datapath/pipeline/openflow/openflow1x/pipeline/of1x_pipeline.h>

#define __STDC_FORMAT_MACROS

#define C_TAG 0x8100
#define S_TAG 0x88A8



//#define cookie_mask 0x8000000000000000ULL
//#define flow_mask   0x7FFFFFFFFFFFFFFFULL

orchestrator::orchestrator(ALHINP *ofproxy) {
    proxy=ofproxy;
    config_flags=0;
    miss_send_len=OF1X_DEFAULT_MISS_SEND_LEN;
    feat_req_sent=0;
    feat_req_last_xid=0;
    stats_xid=0;
}

orchestrator::orchestrator(const orchestrator& orig) {
}

orchestrator::~orchestrator() {
}

void orchestrator::OUI_connected(cofdpt* dpt){
    OUI_reset_flows(dpt);
    OUI_set_port_behavior(dpt);
}
void orchestrator::OUI_disconnected(cofdpt* dpt){
    
}
void orchestrator::OUI_reset_flows(cofdpt* dpt){
    //RESET any existing flow
    cflowentry fe_NO_PACKET_IN(0x03); 
        fe_NO_PACKET_IN.set_command(OFPFC_ADD);
        fe_NO_PACKET_IN.set_idle_timeout(0);
        fe_NO_PACKET_IN.set_hard_timeout(0);
        fe_NO_PACKET_IN.set_table_id(255);
        fe_NO_PACKET_IN.set_cookie(cookiemask+1);
        fe_NO_PACKET_IN.set_cookie_mask(cookiemask);
        fe_NO_PACKET_IN.set_flags(0);
        fe_NO_PACKET_IN.set_priority(1);
        fe_NO_PACKET_IN.match.set_in_port(OUI_NETPORT);

    proxy->send_flow_mod_message(dpt,fe_NO_PACKET_IN);
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
        reset.set_table_id(1);
        reset.set_cookie(cookiemask);
        reset.set_cookie_mask(cookiemask);
    proxy->send_flow_mod_message(dpt,reset);
    std::cout<<"Flow RESET over AGS\n";
    fflush(stdout);
}
void orchestrator::AGS_set_port_behavior(cofdpt* dpt){
    std::map<uint32_t, cofport*>::iterator port_it;
    std::map<uint32_t, cofport*> portlist= dpt->get_ports();
    for (port_it = portlist.begin(); port_it != portlist.end(); ++port_it) {
        
        if(proxy->discover->is_hidden_port(dpt->get_dpid(),port_it->first)){
            cflowentry fe_NO_PACKET_IN(OFP12_VERSION); 
                fe_NO_PACKET_IN.set_command(OFPFC_ADD);
                fe_NO_PACKET_IN.set_idle_timeout(0);
                fe_NO_PACKET_IN.set_hard_timeout(0);
                fe_NO_PACKET_IN.set_table_id(0);
                fe_NO_PACKET_IN.set_cookie(cookiemask+1);
                fe_NO_PACKET_IN.set_cookie_mask(cookiemask);
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
                default1.set_cookie(cookiemask);
                default1.set_cookie_mask(cookiemask);
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
                fe_NO_PACKET_IN.set_cookie(cookiemask+1);
                fe_NO_PACKET_IN.set_cookie_mask(cookiemask);
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
          
    if(dpt->get_dpid()!=AGG_DPID){ //PACKET_INs generated at CLIENT side
        if(msg->get_match().get_in_port()==OUI_NETPORT){ //Discard PACKET_IN generated at NETPORT
            //printf("PACKET_IN received from %" PRIu64 " and IN PORT %" PRIu32 "(Hidden) (discarded)\n",dpt->get_dpid(),msg->get_match().get_in_port()); 
            std::cout<<"PACKET_IN received from "<< dpt->get_dpid_s() << " @ Netport (discarded)\n";
            fflush(stdout);
            delete msg; 
            return;
        }else{
            //CLIENT TRAFFIC -> transmit to Controller if connected
            //uint8_t *data = msg->get_packet().soframe();
            //size_t datalen =msg->get_packet().framelen();
            uint16_t translated_port=proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),msg->get_in_port());
            msg->get_match().set_in_port(translated_port);
            uint8_t table_id;
            if(dpt->get_dpid()==AGG_DPID){
                table_id=1;
            }else{
                table_id=0;
            }
            try{
                cofmatch newmatch=msg->get_match();
                newmatch.set_in_port(translated_port);
                proxy->send_packet_in_message(proxy->controller, 
                        msg->get_buffer_id(), 
                        msg->get_total_len(), 
                        msg->get_reason(), 
                        table_id, 
                        msg->get_cookie(), 
                        (uint16_t)translated_port,
                        msg->get_match(),
                        msg->get_packet().soframe(), 
                        msg->get_packet().framelen()); 
                std::cout<<"PACKET_IN received from "<< dpt->get_dpid_s() << " @ " << translated_port <<" (sending to controller\n";
                fflush(stdout);
            }catch (eRofBaseNotConnected e){
                   std::cout<<"[WARNING] No Controller connected (discarding PACKEt_IN message)\n";
            }
            delete msg; 
            return;
        } 
    }
    delete msg; 
    return;
}//ready for 1.0

void orchestrator::handle_features_reply(cofdpt* dpt, cofmsg_features_reply* msg){

    if(feat_req_sent==0&&feat_req_last_xid==0){ //discovering new ports

        proxy->virtualizer.enable_device(dpt->get_dpid()%0x0001000000000000,dpt->get_dpid(),dpt->get_dpid_s()); 
        std::vector<cofport>::iterator port_it;
        for (port_it = msg->get_ports().begin(); port_it != msg->get_ports().end(); ++port_it) {
            //std::cout<<(*port_it).get_port_no()<<"paso por aqui\n";
            cofport port = *(port_it);
            if(!proxy->discover->is_hidden_port(dpt->get_dpid(),port.get_port_no())){ //No announce Emulator ports
                std::cout<<"testpointA\n";
                proxy->virtualizer.enable_port(dpt->get_dpid(),port.get_port_no());
                uint32_t portnum= proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),port.get_port_no());
                port.set_port_no(portnum);

                try{
                    if(proxy->ctl_find(proxy->controller)->get_version()==OFP12_VERSION){
                       std::cout<< "Processing reply\n";
                       fflush(stdout);
                            proxy->send_port_status_message(proxy->controller,OFPPR_MODIFY,port) ; 
                    }
                    if(proxy->ctl_find(proxy->controller)->get_version()==OFP10_VERSION){
                       std::cout<< "Processing reply\n";
                       fflush(stdout);
                       cofport port10 (OFP10_VERSION);
                       port10.set_port_no((uint16_t)portnum);
                       port10.set_hwaddr((*port_it).get_hwaddr());
                       port10.set_name((*port_it).get_name());
                       port10.set_config((*port_it).get_config());
                       port10.set_state((*port_it).get_state());
                       port10.set_curr((*port_it).get_curr());
                       port10.set_advertised((*port_it).get_advertised());
                       port10.set_supported((*port_it).get_supported());
                       port10.set_peer((*port_it).get_peer());
                       proxy->send_port_status_message(proxy->controller,OFPPR_MODIFY,port10) ; 

                    }
                }catch(eRofBaseNotConnected& e){
                    std::cout<<"WARNING: No Controller connected \n";   
                    fflush(stdout);
                }catch (eRofBaseNotFound& t){
                    std::cout<<"WARNING: No Controller connected \n";                        
                    fflush(stdout);
                }
            }
        } 
            
    }else{
        --feat_req_sent;
        if(feat_req_sent==0){//last of the features reply requested
            proxy->cancel_timer(FEATURES_REQ);
            uint64_t dpid = dpt->get_dpid();
            uint32_t n_buffers = 2048;
            uint8_t n_tables =1;
            uint32_t capabilities = OFPC_PORT_STATS;
            uint8_t of13_auxiliary_id=0;
            uint32_t of12_actions_bitmap =0;
            cofportlist global_portlist;
            std::set<cofdpt*>::iterator dpt_it;
                for (dpt_it = proxy->ofdpt_set.begin(); dpt_it != proxy->ofdpt_set.end(); ++dpt_it) {
            
                    std::map<uint32_t, cofport*>::iterator port_it;
                    std::map<uint32_t, cofport*> portlist= (*dpt_it)->get_ports();
                    for (port_it = portlist.begin(); port_it != portlist.end(); ++port_it) {

                        uint32_t portnum = port_it->first;
                        cofport port;
                        if(!proxy->discover->is_hidden_port((*dpt_it)->get_dpid(),portnum)){
                            if(proxy->controller->get_version()==OFP12_VERSION){
                                cofport port;
                                port.set_port_no(proxy->virtualizer.get_virtual_port_id((*dpt_it)->get_dpid(),port_it->first));
                                port.set_hwaddr(port_it->second->get_hwaddr());
                                port.set_config(port_it->second->get_config());
                                port.set_state(port_it->second->get_state()); 
                                port.set_curr(port_it->second->get_curr());
                                port.set_advertised(port_it->second->get_advertised()); 
                                port.set_supported(port_it->second->get_supported());
                                port.set_peer(port_it->second->get_peer());                
                                port.set_curr_speed(port_it->second->get_curr_speed());
                                port.set_max_speed(port_it->second->get_max_speed());               
                                global_portlist.next()=port;
                            }
                            if(proxy->controller->get_version()==OFP10_VERSION){
                                cofport port10 (OFP10_VERSION);
                                port10.set_port_no((uint16_t)proxy->virtualizer.get_virtual_port_id((*dpt_it)->get_dpid(),port_it->first));
                                port10.set_hwaddr(port_it->second->get_hwaddr());
                                port10.set_config(port_it->second->get_config());
                                port10.set_state(port_it->second->get_state()); 
                                port10.set_curr(port_it->second->get_curr());
                                port10.set_advertised(port_it->second->get_advertised()); 
                                port10.set_supported(port_it->second->get_supported());
                                port10.set_peer(port_it->second->get_peer());                
                                //port10.set_curr_speed(port_it->second->get_curr_speed());
                                //port10.set_max_speed(port_it->second->get_max_speed()); 
                                global_portlist.next()=port10;

                                fflush(stdout);
                            }
                    
                        }
                    } 
            
                }           
            proxy->send_features_reply(proxy->controller, feat_req_last_xid, dpid, n_buffers, n_tables, capabilities, of13_auxiliary_id, of12_actions_bitmap, global_portlist);
            feat_req_last_xid=0;
            
        }
        
    }   
    delete msg;
    return; 
}

void orchestrator::handle_port_status(cofdpt* dpt, cofmsg_port_status* msg){

//OF 1.0 compatible
    cofport port=msg->get_port();
    if(!proxy->discover->is_hidden_port(dpt->get_dpid(),port.get_port_no())){ //No announce Emulator ports
        port.set_port_no(proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),msg->get_port().get_port_no()));
        if(proxy->ctl_find(proxy->controller)->get_version()==OFP12_VERSION)
                proxy->send_port_status_message(proxy->controller,msg->get_reason(),port) ; 
        if(proxy->ctl_find(proxy->controller)->get_version()==OFP12_VERSION){
            cofport port10(OFP10_VERSION);
                port10.set_port_no((uint16_t)proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),msg->get_port().get_port_no()));
                port10.set_hwaddr(port.get_hwaddr());
                port10.set_config(port.get_config());
                port10.set_state(port.get_state()); 
                port10.set_curr(port.get_curr());
                port10.set_advertised(port.get_advertised()); 
                port10.set_supported(port.get_supported());
                port10.set_peer(port.get_peer()); 
                proxy->send_port_status_message(proxy->controller,msg->get_reason(),port10) ;
        }
    }
    delete msg;
    return;
}

void orchestrator::handle_port_mod (cofctl *ctl, cofmsg_port_mod *msg)  {
        proxy->send_port_mod_message(proxy->dpt_find(proxy->virtualizer.get_own_dpid(msg->get_port_no())),proxy->virtualizer.get_real_port_id(msg->get_port_no()),msg->get_hwaddr(),msg->get_config(),msg->get_mask(),msg->get_advertise());
	delete msg;
        return;
}//ready for 1.0

void orchestrator::handle_get_config_request(cofctl* ctl, cofmsg_get_config_request* msg){
    proxy->send_get_config_reply(proxy->controller,msg->get_xid(),config_flags,miss_send_len);
    delete msg;
}