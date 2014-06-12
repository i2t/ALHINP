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
#include <rofl/common/openflow/cofmatch.h>


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
        fe_NO_PACKET_IN.set_command(OFPFC_DELETE);
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
   //OK

}

void orchestrator::AGS_connected(cofdpt* dpt){
    
    AGS_reset_flows(dpt);
    AGS_set_port_behavior(dpt);
    proxy->discover->detect_CM(dpt);
    proxy->discover->detect_DPS_traffic(dpt);
    proxy->discover->detect_OUI_control_traffic(dpt);
    std::cout<<"AGS succesfully configured!\n";
    fflush(stdout);
    //flow_test(dpt);
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

void orchestrator::dispath_PACKET_IN(cofdpt *dpt, cofmsg_packet_in *msg){
    
    if(msg->get_reason()==OFPR_NO_MATCH){
        if(proxy->discover->is_hidden_port(dpt->get_dpid(),msg->get_match().get_in_port())){
            delete msg;
            return;
        }else{ //not hidden!
            uint32_t translated_port;
            translated_port=proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),msg->get_match().get_in_port());
            msg->get_match().set_in_port(translated_port);
            try{
                proxy->send_packet_in_message(proxy->controller, 
                        msg->get_buffer_id(), 
                        msg->get_total_len(), 
                        msg->get_reason(), 
                        0, /*table_id =0*/
                        msg->get_cookie(), 
                        (uint16_t)translated_port,
                        msg->get_match(),
                        msg->get_packet().soframe(),
                        msg->get_packet().framelen()); 
                //printf("packet_in received from %" PRIu64 " and in_port %" PRIu16 " (sending to controller)\n",dpt->get_dpid(),msg->get_match().get_in_port());
                //fflush(stdout);
            }catch (eRofBaseNotConnected e){
                   std::cout<<"WARNING: No Controller connected (discarding PACKEt_IN message)\n";
            }
            delete msg;
            return;
        }
    }else if(msg->get_reason()==OFPR_ACTION){
        if(msg->get_table_id()==1 || dpt->get_dpid()!=AGG_DPID){
            uint32_t translated_port;
            translated_port=proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),msg->get_match().get_in_port());
            msg->get_match().set_in_port(translated_port);
            try{
                proxy->send_packet_in_message(proxy->controller, 
                        msg->get_buffer_id(), 
                        msg->get_total_len(), 
                        msg->get_reason(), 
                        0, /*table_id =0*/
                        msg->get_cookie(), 
                        (uint16_t)translated_port,
                        msg->get_match(),
                        msg->get_packet().soframe(),
                        msg->get_packet().framelen()); 
                //printf("packet_in received from %" PRIu64 " and in_port %" PRIu16 " (sending to controller)\n",dpt->get_dpid(),msg->get_match().get_in_port());
                //fflush(stdout);
            }catch (eRofBaseNotConnected e){
                   std::cout<<"WARNING: No Controller connected (discarding PACKEt_IN message)\n";
            }
            delete msg;
            return;
        }else{//FOR INTERNAL USAGE
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
            }catch(...){
                delete msg; 
                return; 
            }
            cmacaddr OUI(OUI_MAC);
            if(msg->get_match().get_in_port()==CMTS_PORT && msg->get_match().get_eth_src_addr().get_mac()-OUI.get_mac()<OUI.get_mac()){
                std::cout << "OUI detected: "<< msg->get_match().get_eth_src_addr().c_str() <<" with VLAN "<< msg->get_match().get_vlan_vid() <<"\n";
                fflush(stdout);
                proxy->discover->AGS_enable_OUI_traffic(dpt , msg->get_match().get_eth_src_addr().get_mac(), msg->get_match().get_vlan_vid());
                delete msg; 
                return;
            }
            //PACKET_OUT temp
            std::cout << "PACKET_OUT_TEMP\n";
            std::map<uint32_t,cofaclist*>::iterator it;
            it=packoutcache.find(msg->get_buffer_id());
            if(it!=packoutcache.end()){
                
                //process aclist
                //proxy->send_packet_out_message(dpt,it->first,)
            }
            packoutcache.erase(msg->get_buffer_id());
            fflush(stdout);
            
            delete msg; 
            return;


        }
        
    }


    delete msg; 
    return;
}//ready for 1.0

void orchestrator::handle_features_request (cofctl *ctl, cofmsg_features_request *msg){
    std::cout<<"features_request received\n";
    fflush(stdout);
    uint32_t xid = msg->get_xid();
    std::set<cofdpt*>::iterator dpt_it2;
    for (dpt_it2 = proxy->ofdpt_set.begin(); dpt_it2 != proxy->ofdpt_set.end(); ++dpt_it2) {
        std::cout<<"sending FEATURES REQUEST to"<< (*dpt_it2)->get_dpid_s()<<"\n";
        proxy->send_features_request((*dpt_it2));
        ++feat_req_sent;
    }
    feat_req_last_xid=xid;
    std::cout<<"reg timer\n";
    proxy->register_timer(FEATURES_REQ,FEATURES_REQ_TIMER);

    delete msg;
    return;
}
void orchestrator::handle_features_reply(cofdpt* dpt, cofmsg_features_reply* msg){

    if(feat_req_sent==0&&feat_req_last_xid==0){ //discovering new ports

        proxy->virtualizer.enable_device(dpt->get_dpid()%0x0001000000000000,dpt->get_dpid(),dpt->get_dpid_s()); 
        std::vector<cofport>::iterator port_it;
        for (port_it = msg->get_ports().begin(); port_it != msg->get_ports().end(); ++port_it) {
            //std::cout<<(*port_it).get_port_no()<<"paso por aqui\n";
            cofport port = *(port_it);
            if(!proxy->discover->is_hidden_port(dpt->get_dpid(),port.get_port_no())){ //No announce Emulator ports
                proxy->virtualizer.enable_port(dpt->get_dpid(),port.get_port_no());
                uint32_t portnum= proxy->virtualizer.get_virtual_port_id(dpt->get_dpid(),port.get_port_no());
                port.set_port_no(portnum);

                try{
                    if(proxy->ctl_find(proxy->controller)->get_version()==OFP12_VERSION){
                            proxy->send_port_status_message(proxy->controller,OFPPR_MODIFY,port) ; 
                    }
                    if(proxy->ctl_find(proxy->controller)->get_version()==OFP10_VERSION){
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
            uint64_t dpid = ALHINP_DPID;
            uint32_t n_buffers = 2048;
            uint8_t n_tables =1;
            uint32_t capabilities = OFPC_PORT_STATS;
            uint8_t of13_auxiliary_id=0;
            uint32_t of10_actions_bitmap =0;
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
                            }
                    
                        }
                    } 
            
                }    
            try{
            proxy->send_features_reply(proxy->controller, feat_req_last_xid, dpid, n_buffers, n_tables, capabilities, of13_auxiliary_id, of10_actions_bitmap, global_portlist);
            feat_req_last_xid=0;
            }catch(...){
                std::cout<<"UOPS\n";
            }
            
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
                try{
                    proxy->send_port_status_message(proxy->controller,msg->get_reason(),port10) ;
                }catch(...){
                    std::cout<<"[WARNING:] Port Status not sent to controller";
                }
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
void orchestrator::handle_set_config (cofctl *ctl, cofmsg_set_config *msg){
    config_flags=msg->get_flags();
    miss_send_len=msg->get_miss_send_len();
    
    std::set<cofdpt*>::iterator dpt_it;
    for (dpt_it = proxy->ofdpt_set.begin(); dpt_it != proxy->ofdpt_set.end(); ++dpt_it) {
        proxy->send_set_config_message(*dpt_it,config_flags,miss_send_len);
    }
    delete msg;
    return;
}

void orchestrator::handle_timeout(int opaque){
    switch(opaque){
        case FEATURES_REQ:
            proxy->cancel_timer(FEATURES_REQ);
            uint64_t dpid = ALHINP_DPID;
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
                    } 
            
                }           
            proxy->send_features_reply(proxy->controller, feat_req_last_xid, dpid, n_buffers, n_tables, capabilities, of13_auxiliary_id, of12_actions_bitmap, global_portlist);
            feat_req_last_xid=0;
            break;
    }
}

//void orchestrator::flow_mod_generator(cofmatch ofmatch,cofinlist instrlist, flow_mod_constants *constants, uint32_t inport, uint32_t outport){
//    
//    if(outport==OFPP10_FLOOD ||outport==OFPP10_NORMAL ||outport==OFPP10_LOCAL){ //NOT SUPPORTED
//        return;
//    }
//    
//    bool inport_wildcarded=false;
//    if(inport==0){
//        inport_wildcarded==true;
//    }
//    if(inport_wildcarded==false){//IN_PORT_FIXED
//        uint64_t src_dpid;
//        src_dpid = proxy->virtualizer.get_own_dpid(inport);
//        uint64_t dst_dpid;
//        if(outport==OFPP10_IN_PORT || outport==OFPP10_CONTROLLER ||outport==OFPP10_NONE){
//            dst_dpid = src_dpid;
//        }else{
//            dst_dpid = proxy->virtualizer.get_own_dpid(outport);
//        }
//
//        if(outport==0){ //security check
//            std::cout<<"FLOW_MOD_ERROR: outport 0 ¿?\n";
//            return;
//        }
//        
//        uint8_t flow_type;
//        if(src_dpid==AGG_DPID){
//            //DownStream Flow
//            flow_type=DOWNSTREAM;
//
//        }else{
//            if(dst_dpid==AGG_DPID){
//               //UPSTREAM Flow
//                flow_type=UPSTREAM;
//            }else{
//                if(dst_dpid==src_dpid){
//                    //local Flow
//                    flow_type=LOCAL;
//                }else{
//                    //Flow between clients
//                    flow_type=BCLIENTS;
//                }
//            }
//        }
//        cflowentry flow(OFP12_VERSION);
//        
//        switch(flow_type){
//            case DOWNSTREAM://DOWNSTREAM FLOW PROCESSING
//
//                //RULE FOR AGS
//                std::cout<<"setting DOWNSTREAM flow\n";
//                flow.set_cookie(constants->cookie&flow_mask);
//                flow.set_cookie_mask(flow_mask);
//                flow.set_command(OFPFC_ADD);
//                flow.set_hard_timeout(constants->hard_timeout);
//                flow.set_idle_timeout(constants->idle_timeout);
//                flow.set_flags(constants->flags);
//                flow.set_priority(constants->priority);
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                
//                flow.set_table_id(1);
//                
//                flow.match=ofmatch;
//                flow.match.set_in_port(proxy->virtualizer.get_real_port_id(inport));
//                
//                flow.instructions=instrlist;
//                //flow.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
//                flow.instructions.back().actions.next() = cofaction_push_vlan(OFP12_VERSION,C_TAG); 
//                flow.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (OFPVID_PRESENT|proxy->virtualizer.get_vlan_tag(AGG_DPID,dst_dpid)));                                                         
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,CMTS_PORT); 
// 
//                proxy->send_flow_mod_message(proxy->dpt_find(src_dpid),flow); 
//
//                    
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                flow.set_table_id(0);
//                flow.match=ofmatch;
//                flow.match.set_in_port(OUI_NETPORT);
//                flow.instructions=instrlist;
//                flow.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION, proxy->virtualizer.get_real_port_id(outport)); 
//                    proxy->send_flow_mod_message(proxy->dpt_find(dst_dpid),flow);   
//
//            
//                break;
//            case UPSTREAM: //UPSTREAM FLOW PROCESSING
//
//                std::cout<<"setting UPSTREAM flow\n";
//                flow.set_cookie(constants->cookie&flow_mask);
//                flow.set_cookie_mask(flow_mask);
//                flow.set_command(OFPFC_ADD);
//                flow.set_hard_timeout(constants->hard_timeout);
//                flow.set_idle_timeout(constants->idle_timeout);
//                flow.set_flags(constants->flags);
//                flow.set_priority(constants->priority);            
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                
//                flow.set_table_id(0);
//           
//                flow.match=ofmatch;
//                flow.match.set_in_port(proxy->virtualizer.get_real_port_id(inport));
//                
//                flow.instructions=instrlist;
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,OUI_NETPORT); 
//
//                proxy->send_flow_mod_message(proxy->dpt_find(src_dpid),flow);
//
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                flow.set_table_id(1);                
//                flow.match=ofmatch;
//                flow.match.set_in_port(CMTS_PORT);
//                flow.match.set_metadata(proxy->virtualizer.get_vlan_tag(src_dpid,dst_dpid));
//                
//                flow.instructions=instrlist;
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport)); 
//
//                proxy->send_flow_mod_message(proxy->dpt_find(dst_dpid),flow);  
//                break;
//                
//            case LOCAL: //LOCAL FLOW
//                  
//                flow.set_cookie(constants->cookie&flow_mask);
//                flow.set_cookie_mask(flow_mask);
//                flow.set_command(OFPFC_ADD);
//                flow.set_hard_timeout(constants->hard_timeout);
//                flow.set_idle_timeout(constants->idle_timeout);
//                flow.set_flags(constants->flags);
//                flow.set_priority(constants->priority);
//                flow.set_buffer_id(constants->buffer_id);
//                flow.set_table_id(0);
//                flow.match=ofmatch;
//                flow.match.set_in_port(proxy->virtualizer.get_real_port_id(inport));
//                
//                if(outport>=OFPP10_MAX)
//                    instrlist.back().actions.next()=cofaction_output(OFP12_VERSION,outport);//SPECIAL PORTS
//                else
//                    instrlist.back().actions.next()=cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport));//IF MORE THAN 1 PORT PER USER
//                flow.instructions=instrlist;
//                proxy->send_flow_mod_message(proxy->dpt_find(src_dpid),flow);
//                break;
//            case BCLIENTS: //CLIENT TO CLIENT FLOW
//
//                std::cout<<"setting CLIENT-toCLIENT FLOW\n";
//                flow.set_cookie(constants->cookie&flow_mask);
//                flow.set_cookie_mask(flow_mask);
//                flow.set_command(OFPFC_ADD);
//                flow.set_hard_timeout(constants->hard_timeout);
//                flow.set_idle_timeout(constants->idle_timeout);
//                flow.set_flags(constants->flags);
//                flow.set_priority(constants->priority);
//                
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                flow.set_table_id(0);
//                flow.match=ofmatch;
//                flow.match.set_in_port(proxy->virtualizer.get_real_port_id(inport));
//                flow.instructions=instrlist;
//                //flow.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,OUI_NETPORT); 
//                    proxy->send_flow_mod_message(proxy->dpt_find(src_dpid),flow); 
//                
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                flow.set_table_id(1);                
//                flow.match=ofmatch;
//                flow.match.set_in_port(CMTS_PORT);
//                //flow.match.set_vlan_vid(OFPVID_PRESENT|proxy->virtualizer.get_vlan_tag(src_dpid,dst_dpid));
//                flow.match.set_metadata(proxy->virtualizer.get_vlan_tag(src_dpid,dst_dpid));
//                flow.instructions=instrlist;
//                //flow.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
//                flow.instructions.back().actions.next() = cofaction_push_vlan(OFP12_VERSION,C_TAG); 
//                flow.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (OFPVID_PRESENT|proxy->virtualizer.get_vlan_tag(AGG_DPID,dst_dpid)));                
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,OFPP12_IN_PORT); 
//                proxy->send_flow_mod_message(proxy->dpt_find(AGG_DPID),flow);  
//                
//                flow.set_buffer_id(OFP_NO_BUFFER);
//                flow.set_table_id(0);
//                flow.match=ofmatch;
//                flow.match.set_in_port(OUI_NETPORT);
//                flow.instructions=instrlist;
//                //flow.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
//                flow.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport)); 
//                proxy->send_flow_mod_message(proxy->dpt_find(dst_dpid),flow);  
//                break;
//            default:
//                break;
//        }    
//    
//    }
//    if(inport_wildcarded==true){//DROPPING RULE MULTIPORT DEPLOYMENT
//        
//    }
//    std::cout<<"exiting\n";
//    sleep(2);
//}

void      orchestrator::flow_test(cofdpt* dpt){

    cflowentry test(OFP12_VERSION);
    cmacaddr test_mac("03:52:20:23:22:11");
    caddress test_ip(AF_INET,"192.168.5.1");

    test.set_command(OFPFC_ADD);
    test.set_table_id(1);
    test.set_priority(32768);
    test.match.set_eth_src(test_mac);
    test.match.set_eth_dst(test_mac);
    test.match.set_eth_type(0x0800);
    test.match.set_vlan_vid(5);
    test.match.set_vlan_pcp(1);
    test.match.set_ipv4_src(test_ip);
    test.match.set_ipv4_dst(test_ip);
    test.match.set_ip_proto(0x06);
    test.match.set_tcp_src(5555);
    test.match.set_tcp_dst(6666);
    test.match.set_metadata(0x65214);
    
    test.instructions.next()=cofinst_apply_actions(OFP12_VERSION);
    test.instructions.back().actions.next() = cofaction_push_vlan(OFP12_VERSION,C_TAG); 
    test.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (66));   
    test.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_eth_dst (test_mac));
    test.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_eth_src (test_mac));
    test.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ipv4_src (test_ip));
    test.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ipv4_src (test_ip));   
    test.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,1);
    proxy->send_flow_mod_message(dpt,test);
    std::cout<<"testing2\n";
}

void      orchestrator::handle_packet_out (cofctl *ctl, cofmsg_packet_out *msg){
    if(msg->get_buffer_id()==OFP_NO_BUFFER){//packet attached to message
        uint8_t *data;
        data=msg->get_packet().soframe();
        size_t datalen = msg->get_packet().framelen();
        
    }else{
        cofaclist* temp = new cofaclist (OFP12_VERSION);
        (*temp)=msg->get_actions();
        packoutcache.insert(std::make_pair(msg->get_in_port(),temp));
        cofaclist list (OFP12_VERSION);
        list.next() = cofaction_output(OFP12_VERSION,OFPP12_CONTROLLER);
        uint32_t realport = proxy->virtualizer.get_real_port_id(msg->get_in_port());
        uint64_t dpid = proxy->virtualizer.get_own_dpid(msg->get_in_port());
        proxy->send_packet_out_message(proxy->dpt_find(dpid),msg->get_buffer_id(),realport,list);
        
        
    }
    delete msg;
    return;
}
void      orchestrator::process_packet_out(cofdpt* dpt,cofaclist list, uint8_t *data,size_t datalen){
    proxy->send_packet_out_message(dpt,OFP_NO_BUFFER,OFPP10_NONE,list,data,datalen);
}

void      orchestrator::flow_mod_add(cofctl *ctl, cofmsg_flow_mod *msg){
    //uint8_t ofversion=ctl->get_version();
    uint8_t ofversion=OFP10_VERSION;
    std::cout<<"processing Flow Mod version: "<< ofversion <<" \n";
    fflush(stdout);
    //getting IN_PORT
    flow_mod_constants* constants;
    constants =new flow_mod_constants;
    constants->buffer_id=OFP_NO_BUFFER;//careful with this
    constants->cookie=msg->get_cookie()&flow_mask;
    constants->idle_timeout=msg->get_idle_timeout();
    constants->hard_timeout=msg->get_hard_timeout();
    constants->flags=msg->get_flags();
    constants->priority=msg->get_priority();
    
    uint32_t virtual_inport = msg->get_match().get_in_port();

    cofmatch* common_match;
    common_match = process_matching(msg);

    flowpath flows;
    flows.longest=0;
    process_action_list(flows,common_match, msg->get_actions(), OFP12_VERSION, virtual_inport,common_match->get_ip_proto());
    std::map<uint64_t , flowmod*>::iterator it;
    
    bool input_wildcarded = false;
    uint16_t flow_id = proxy->flowcache->store_flow(msg);
    for(it=flows.flowmodlist.begin();it!=flows.flowmodlist.end();++it){
        cofinlist list;
        list.next()=cofinst_apply_actions(OFP12_VERSION);
        list.back().actions=(*it->second->actions);
        if(input_wildcarded==false){
            proxy->flowcache->reg_partial_flow(flow_id,it->first);
        }
        proxy->send_flow_mod_message(
		proxy->dpt_find(it->first),
		(*it->second->match),
		it->second->constants->cookie,
		flow_mask,
		it->second->constants->table_id,
		OFPFC_ADD,
		it->second->constants->idle_timeout,
		it->second->constants->hard_timeout,
		it->second->constants->priority,
		OFP_NO_BUFFER,
		0,
		0,
		it->second->constants->flags,
		list);
        
    }
    if(input_wildcarded==true){
        proxy->flowcache->reg_partial_flow(flow_id,ALL_DPID);
    }
    delete constants;
    delete msg;
}
cofmatch* orchestrator::process_matching(cofmsg_flow_mod *msg, uint8_t ofversion){
    cofmatch* common_match = new cofmatch(OFP12_VERSION);
    
    if(ofversion==OFP12_VERSION){
        (*common_match)=msg->get_match();
        //virtual_inport=msg->get_match().get_in_port();
    }
    if(ofversion==OFP10_VERSION){
        //Openflow 1.0 Match parsing
        try{
            //=(uint32_t)msg->get_match().get_in_port();
            //std::cout<<"in_port_not_wildcarded: " << msg->get_match().get_in_port()<<"\n";
        }catch(eOFmatchNotFound& e){}   
        
        try{ //OF 1.0
            (*common_match).set_eth_type(msg->get_match().get_eth_type());
            //std::cout<<"Ethtype_not_wildcarded: "<<msg->get_match().get_eth_type()<<"\n";
        }catch(eOFmatchNotFound& e){}

        try{ //OF 1.0
        if(msg->get_match().get_eth_src_addr().get_mac()!=0){ //ETH_SRC
            (*common_match).set_eth_src(msg->get_match().get_eth_src_addr());
            //std::cout<<"dl_src_not_wildcarded: "<<msg->get_match().get_eth_src_addr().c_str()<<"\n";
        }
        }catch(eOFmatchNotFound& e){}

        try{ //OF 1.0
        if(msg->get_match().get_eth_dst_addr().get_mac()!=0){ //ETH_SRC
            (*common_match).set_eth_dst(msg->get_match().get_eth_dst_addr());
            //std::cout<<"dl_dst_not_wildcarded: "<<msg->get_match().get_eth_dst_addr().c_str()<<"\n";
        }            
        }catch(eOFmatchNotFound& e){}
        
        try{ //OF 1.0
            (*common_match).set_vlan_vid(msg->get_match().get_vlan_vid());
            //std::cout<<"vid_not_wildcarded: "<<msg->get_match().get_vlan_vid()<<"\n";

        }catch(eOFmatchNotFound& e){}

        try{ //OF 1.0
            (*common_match).set_vlan_pcp(msg->get_match().get_vlan_pcp());
            //std::cout<<"pcp_not_wildcarded: "<<(uint32_t)msg->get_match().get_vlan_pcp()<<"\n";
        }catch(eOFmatchNotFound& e){}

        try{ //OF 1.0
            if(msg->get_match().get_nw_dst_value().get_ipv4_addr()!=0){
            (*common_match).set_ipv4_dst(msg->get_match().get_nw_dst_value());
            //std::cout<<"nw_dst_not_wildcarded: "<<msg->get_match().get_nw_dst_value().addr_c_str()<<"\n";
            }

        }catch(eOFmatchNotFound& e){}
        
        try{//check !=0
            if(msg->get_match().get_nw_dst_value().get_ipv4_addr()!=0){
                //caddr ip_src();
            (*common_match).set_ipv4_src(msg->get_match().get_nw_src_value());
            //std::cout<<"nw_src_not_wildcarded: "<<msg->get_match().get_nw_src_value().addr_c_str()<<"\n";
            }
        }catch(eOFmatchNotFound& e){}
        
        try{
            //common_match.set_ip_proto(msg->get_match().get_nw_proto());
            std::cout<<"ip_proto_not_wildcarded: "<<msg->get_match().get_ip_proto()<<"\n";
            if(msg->get_match().get_nw_proto()==6){
            try{
                    try{
                        //std::cout<<"TCP_SRC_not_wildcarded: "<<msg->get_match().get_tp_src()<<"\n";
                        (*common_match).set_tcp_src(msg->get_match().get_tp_src());


                    }catch(eOFmatchNotFound& e){}
                    try{
                        (*common_match).set_tcp_dst(msg->get_match().get_tp_dst());
                        //std::cout<<"TCP_DST_not_wildcarded: "<<msg->get_match().get_tp_dst()<<"\n";

                    }catch(eOFmatchNotFound& e){}
            }catch(eOFmatchNotFound& e){}
            }
            if(msg->get_match().get_nw_proto()==17){
            try{
                try{
                    (*common_match).set_udp_src(msg->get_match().get_tp_src());
                    //std::cout<<"UDP_SRC_not_wildcarded: "<<msg->get_match().get_tp_src()<<"\n";

                }catch(eOFmatchNotFound& e){}
                try{
                    (*common_match).set_udp_dst(msg->get_match().get_tp_dst());
                    //std::cout<<"UDP_src_not_wildcarded: "<<msg->get_match().get_tp_dst()<<"\n";

                }catch(eOFmatchNotFound& e){}
            }catch(eOFmatchNotFound& e){} 
            }
        }catch(eOFmatchNotFound& e){} 
    } 
    return common_match;
}
bool      orchestrator::process_action_list(flowpath flows,cofmatch* common_match,cofaclist aclist, uint8_t ofversion, uint32_t inport,uint8_t nw_proto){
    cofinlist instrlist;
    cofaclist::iterator it;
    bool anyoutput=false;
    uint64_t in_dpid = proxy->virtualizer.get_own_dpid(inport);

    if (ofversion==OFP10_VERSION){            
        instrlist.next()=cofinst_apply_actions(OFP12_VERSION);
        for (it = aclist.begin(); it != aclist.end(); ++it){
            try{
                switch (htobe16((*it).oac_oacu.oacu_header->type)) {
                    
                    case OFP10AT_OUTPUT:{ //overwrite corresponding virtual port w/ real port
                        anyoutput=true;
                        //std::cout<<"SET_OUTPORT: "<< (uint16_t)htobe16((*it).oac_oacu.oacu_10output->port) <<"\n";
                        uint8_t flowtype = typeflow(in_dpid,proxy->virtualizer.get_own_dpid(htobe16((*it).oac_oacu.oacu_10output->port)));
                        fill_flowpath(flows,common_match,instrlist.back().actions, inport,htobe16((*it).oac_oacu.oacu_10output->port),flowtype);
                        instrlist.clear();
                       // flow_mod_generator(common_match, instrlist, constants, virtual_inport, (uint32_t)htobe16((*it).oac_oacu.oacu_10output->port));
                        
                    }break;
                    case OFP10AT_SET_VLAN_VID:{
                        //std::cout<<"SET_VLAN_VID: "<< htobe16((*it).oac_oacu.oacu_10vlanvid->vlan_vid) <<"\n";
                        instrlist.back().actions.next() = cofaction_push_vlan(OFP12_VERSION,C_TAG); 
                        instrlist.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (OFPVID_PRESENT|htobe16((*it).oac_oacu.oacu_10vlanvid->vlan_vid)));
                        break;
                    }
                    case OFP10AT_SET_VLAN_PCP:{
                        //std::cout<<"SET_VLAN_PCP: "<< (*it).oac_oacu.oacu_10vlanpcp->vlan_pcp <<"\n";
                        instrlist.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_pcp ((*it).oac_oacu.oacu_10vlanpcp->vlan_pcp));
                        break;
                    }
                    case OFP10AT_STRIP_VLAN:{
                        instrlist.back().actions.next()= cofaction_pop_vlan(OFP12_VERSION);
                        break;
                    }
                    case OFP10AT_SET_DL_SRC:{
                        cmacaddr mac_dl_src((*it).oac_oacu.oacu_10dladdr->dl_addr,6);
                        //std::cout<<"SET_MAC_SRC: "<< mac_dl_src.c_str()<<"\n";
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_eth_src(mac_dl_src));
                        break;
                    }
                    case OFP10AT_SET_DL_DST:{
                        cmacaddr mac_dl_dst((*it).oac_oacu.oacu_10dladdr->dl_addr,6);
                        //std::cout<<"SET_MAC_DST: "<< mac_dl_dst.c_str()<<"\n";
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_eth_dst(mac_dl_dst));
                        break;
                    }
                    case OFP10AT_SET_NW_SRC:{
                        caddress ip_src(AF_INET);
                        ip_src.set_ipv4_addr(htobe32((*it).oac_oacu.oacu_10nwaddr->nw_addr));
                        //std::cout<<"SET_IP_SRC "<<ip_src.addr_c_str() <<"\n";
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ipv4_src(ip_src));                        
                        break;
                    }
                    case OFP10AT_SET_NW_DST:{
                        caddress ip_dst(AF_INET);
                        ip_dst.set_ipv4_addr(htobe32((*it).oac_oacu.oacu_10nwaddr->nw_addr));
                        //std::cout<<"SET_IP_DST "<< ip_dst.addr_c_str() <<"\n";
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ipv4_dst(ip_dst));
                        break;
                    }                                         
                    case OFP10AT_SET_NW_TOS:{
                        //std::cout<< "SET_NW_TOS: "<<(uint32_t)(*it).oac_oacu.oacu_10nwtos->nw_tos<<"\n";
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ip_ecn((*it).oac_oacu.oacu_10nwtos->nw_tos));
                        instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_ip_dscp((*it).oac_oacu.oacu_10nwtos->nw_tos));
                        break;
                    }
                    case OFP10AT_SET_TP_SRC:{
                        //std::cout<< "SET_TP_SRC "<<htobe16((*it).oac_oacu.oacu_10tpport->tp_port)<<"\n";
                        if(nw_proto==0x06)//TCP
                            instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_tcp_src(htobe16((*it).oac_oacu.oacu_10tpport->tp_port)));
                        else
                            instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_udp_src(htobe16((*it).oac_oacu.oacu_10tpport->tp_port)));
                        break;
                    }
                    case OFP10AT_SET_TP_DST:{
                        //std::cout<< "SET_TP_DST "<<htobe16((*it).oac_oacu.oacu_10tpport->tp_port)<<"\n";
                        if(nw_proto==0x06)//TCP
                            instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_tcp_dst(htobe16((*it).oac_oacu.oacu_10tpport->tp_port)));
                        else
                            instrlist.back().actions.next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_udp_dst(htobe16((*it).oac_oacu.oacu_10tpport->tp_port)));
                        break;
                    }
                    case OFP10AT_SET_BANDWIDTH:{
                        //std::cout<< "ACTIONS: SET_BANDWIDTH\n"<<(*it).oac_oacu.oacu_10setBW->bandwidth <<"\n";
                        break;
                    }
                    default:{
                       break;
                    }
                }
            }catch(...){
                return false; //ACTION LIST CANNOT BE PROCESSED
            }
        }//END FOR
        if(anyoutput==false){
            //OUTPORT=NONE (DROP ACTION) send drop
            fill_flowpath(flows,common_match,instrlist.back().actions, inport,0,LOCAL);
            
        }
    }
    return true;
}
void      orchestrator::fill_flowpath(flowpath flows,cofmatch* common_match,cofaclist aclist,uint32_t inport, uint32_t outport, uint8_t flowtype){
    
    uint64_t outdpid =proxy->virtualizer.get_own_dpid(outport);  
    uint64_t indpid =proxy->virtualizer.get_own_dpid(inport); 
    switch(flowtype){
        case DOWNSTREAM:{
            if(flows.longest>1){
                std::cout<<"Flowmod_error\n";                
            break;
            }else{//AGS-to-Client

                if(flows.longest==0){
                std::cout<<"setting DOWNSTREAM flow\n";
                    flows.flowmodlist.insert(std::make_pair(AGG_DPID,new flowmod)); //INGRESS DPID
                    flows.flowmodlist[AGG_DPID]->dpid=proxy->virtualizer.get_own_dpid(inport);
                    flows.flowmodlist[AGG_DPID]->actions= new cofaclist (OFP12_VERSION);
                    flows.flowmodlist[AGG_DPID]->match = new cofmatch (OFP12_VERSION); 
                    flows.flowmodlist[AGG_DPID]->constants = new flow_mod_constants; 
                    
                    flows.flowmodlist[AGG_DPID]->match = common_match;
                    flows.flowmodlist[AGG_DPID]->constants->table_id=1; //table AGS
                    flows.flowmodlist[AGG_DPID]->match->set_in_port(proxy->virtualizer.get_real_port_id(inport));

                }
                flows.flowmodlist[AGG_DPID]->constants->table_id=1; //table AGS

                flows.flowmodlist[AGG_DPID]->actions->next()= cofaction_push_vlan(OFP12_VERSION,C_TAG); 
                flows.flowmodlist[AGG_DPID]->actions->next()= cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (OFPVID_PRESENT|proxy->virtualizer.get_vlan_tag(AGG_DPID,proxy->virtualizer.get_own_dpid(outport))));                                                         
                flows.flowmodlist[AGG_DPID]->actions->next()= cofaction_output(OFP12_VERSION,CMTS_PORT); 


                    std::cout<<"setting DOWNSTREAM flow CLIENT\n";
                flows.flowmodlist.insert(std::make_pair(proxy->virtualizer.get_own_dpid(outport),new flowmod)); //INGRESS DPID
                flows.flowmodlist[outdpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                flows.flowmodlist[outdpid]->actions= new cofaclist (OFP12_VERSION);
                flows.flowmodlist[outdpid]->match = new cofmatch (OFP12_VERSION); 
                flows.flowmodlist[outdpid]->constants = new flow_mod_constants; 

                flows.flowmodlist[outdpid]->constants->table_id=1; //table AGS
              
//              flows.flowmodlist[outdpid]->match = common_match;
                flows.flowmodlist[outdpid]->match->set_in_port(OUI_NETPORT);
                flows.flowmodlist[outdpid]->actions=&aclist;              
                flows.flowmodlist[outdpid]->actions->next()= cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport));                 
                
                
                
                flows.longest=2;
                break;
            }
                break;            
        }
        case UPSTREAM:{
            if(flows.longest>=2){
            break;
            }else{
                if(flows.longest==0){
                    std::cout<<"setting UPSTREAM flow (1/2)\n";
                    flows.flowmodlist.insert(std::make_pair(AGG_DPID,new flowmod)); //INGRESS DPID
                    flows.flowmodlist[indpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                    flows.flowmodlist[indpid]->actions= new cofaclist (OFP12_VERSION);
                    flows.flowmodlist[indpid]->match = new cofmatch (OFP12_VERSION); 
                    flows.flowmodlist[indpid]->constants = new flow_mod_constants; 
                    
                    flows.flowmodlist[indpid]->match = common_match;
                    flows.flowmodlist[indpid]->constants->table_id=0; //table AGS
                    flows.flowmodlist[indpid]->match->set_in_port(proxy->virtualizer.get_real_port_id(inport));
                    flows.flowmodlist[indpid]->actions->next()= cofaction_output(OFP12_VERSION,OUI_NETPORT); 
                }
                flows.flowmodlist[indpid]->actions->next()= cofaction_output(OFP12_VERSION,OUI_NETPORT); 
                
                    std::cout<<"setting UPSTREAM flow AGG (2/2) \n";
                flows.flowmodlist.insert(std::make_pair(outdpid,new flowmod)); //INGRESS DPID
                flows.flowmodlist[outdpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                flows.flowmodlist[outdpid]->actions= new cofaclist (OFP12_VERSION);
                flows.flowmodlist[outdpid]->match = new cofmatch (OFP12_VERSION); 
                flows.flowmodlist[outdpid]->constants = new flow_mod_constants; 
                
                flows.flowmodlist[outdpid]->constants->table_id=1; //table AGS

                flows.flowmodlist[outdpid]->match = common_match;
                flows.flowmodlist[outdpid]->match->set_in_port(CMTS_PORT);
                flows.flowmodlist[outdpid]->match->set_metadata(proxy->virtualizer.get_vlan_tag(indpid,outdpid));
                
                flows.flowmodlist[outdpid]->actions=&aclist;              
                flows.flowmodlist[outdpid]->actions->next()= cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport));                    
                
                flows.longest=2;
                break;
            }
        }
        case BCLIENTS:{
            if(flows.longest==3){
                break;
            }else{
                if(flows.longest==0){
                    std::cout<<"setting UPSTREAM flow\n";
                    flows.flowmodlist.insert(std::make_pair(indpid,new flowmod)); //INGRESS DPID
                    flows.flowmodlist[indpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                    flows.flowmodlist[indpid]->actions= new cofaclist (OFP12_VERSION);
                    flows.flowmodlist[indpid]->match = new cofmatch (OFP12_VERSION); 
                    flows.flowmodlist[indpid]->constants = new flow_mod_constants; 
                    
                    flows.flowmodlist[indpid]->match = common_match;
                    flows.flowmodlist[indpid]->constants->table_id=0; //table AGS
                    flows.flowmodlist[indpid]->match->set_in_port(proxy->virtualizer.get_real_port_id(inport));
                }
                flows.flowmodlist[indpid]->actions->next()= cofaction_output(OFP12_VERSION,OUI_NETPORT); 
                if(flows.longest==2){
                    std::cout<<"setting UPSTREAM flow\n";
                    flows.flowmodlist.insert(std::make_pair(AGG_DPID,new flowmod)); //INGRESS DPID
                    flows.flowmodlist[AGG_DPID]->dpid=proxy->virtualizer.get_own_dpid(inport);
                    flows.flowmodlist[AGG_DPID]->actions= new cofaclist (OFP12_VERSION);
                    flows.flowmodlist[AGG_DPID]->match = new cofmatch (OFP12_VERSION); 
                    flows.flowmodlist[AGG_DPID]->constants = new flow_mod_constants; 
                    
                    flows.flowmodlist[AGG_DPID]->constants->table_id=1; //table AGS
                    
                    flows.flowmodlist[AGG_DPID]->match = common_match;
                    flows.flowmodlist[AGG_DPID]->match->set_in_port(CMTS_PORT);
                    flows.flowmodlist[AGG_DPID]->match->set_metadata(proxy->virtualizer.get_vlan_tag(indpid,outdpid));
                }        
                flows.flowmodlist[AGG_DPID]->actions->next() = cofaction_push_vlan(OFP12_VERSION,C_TAG); 
                flows.flowmodlist[AGG_DPID]->actions->next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid (OFPVID_PRESENT|proxy->virtualizer.get_vlan_tag(AGG_DPID,outdpid)));                
                flows.flowmodlist[AGG_DPID]->actions->next() = cofaction_output(OFP12_VERSION,OFPP12_IN_PORT); 
            
                     std::cout<<"setting DOWNSTREAM flow CLIENT\n";
                flows.flowmodlist.insert(std::make_pair(proxy->virtualizer.get_own_dpid(outport),new flowmod)); //INGRESS DPID
                flows.flowmodlist[outdpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                flows.flowmodlist[outdpid]->actions= new cofaclist (OFP12_VERSION);
                flows.flowmodlist[outdpid]->match = new cofmatch (OFP12_VERSION); 
                flows.flowmodlist[outdpid]->constants = new flow_mod_constants; 

                flows.flowmodlist[outdpid]->constants->table_id=1; //table AGS
              
                flows.flowmodlist[outdpid]->match = common_match;
                flows.flowmodlist[outdpid]->match->set_in_port(OUI_NETPORT);
                flows.flowmodlist[outdpid]->actions=&aclist;              
                flows.flowmodlist[outdpid]->actions->next()= cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport));              
            
            
            flows.longest=3;
            break;
            }
        }
        case LOCAL:{
            if(flows.longest>1){
            break;
            }else{
                if(flows.longest==0){
                    std::cout<<"setting UPSTREAM flow\n";
                    flows.flowmodlist.insert(std::make_pair(AGG_DPID,new flowmod)); //INGRESS DPID
                    flows.flowmodlist[indpid]->dpid=proxy->virtualizer.get_own_dpid(inport);
                    flows.flowmodlist[indpid]->actions= new cofaclist (OFP12_VERSION);
                    flows.flowmodlist[indpid]->match = new cofmatch (OFP12_VERSION); 
                    flows.flowmodlist[indpid]->constants = new flow_mod_constants; 

                    flows.flowmodlist[indpid]->match = common_match;
                    if(indpid==AGG_DPID)
                        flows.flowmodlist[indpid]->constants->table_id=1; //table AGS
                    else
                        flows.flowmodlist[indpid]->constants->table_id=0; //table A                        
                    flows.flowmodlist[indpid]->match->set_in_port(proxy->virtualizer.get_real_port_id(inport));
                }
                if (outport==0){
                    
                }else{
                flows.flowmodlist[indpid]->actions=&aclist;  
                flows.flowmodlist[outdpid]->actions->next()= cofaction_output(OFP12_VERSION,proxy->virtualizer.get_real_port_id(outport));  
                }
              
              
                
                
                flows.longest=1;
                break;
            }
        }
        
    }
    
}
uint8_t   orchestrator::typeflow(uint64_t src_dpid,uint64_t dst_dpid){
    if(src_dpid==AGG_DPID){
            //DownStream Flow
        return DOWNSTREAM;

    }else{
            if(dst_dpid==AGG_DPID){
               //UPSTREAM Flow
                return UPSTREAM;
            }else{
                if(dst_dpid==src_dpid){
                    //local Flow
                    return LOCAL;
                }else{
                    //Flow between clients
                    return BCLIENTS;
                }
            }
        }
}

void orchestrator::flowstats_request(uint64_t flow_cookie){
    
}