/* 
 * File:   discovery.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:19
 */

#include <rofl/platform/unix/csyslog.h>
#include "../proxy/ALHINP.h"
#include <rofl/common/openflow/openflow.h>
#include <rofl/common/crofbase.h>
#include "discovery.h"


#define __STDC_FORMAT_MACROS

#define C_TAG 0x8100
#define S_TAG 0x88A8

discovery::discovery(ALHINP *proxye) {
    proxy=proxye;
    VLAN_index=1;
}

discovery::discovery(const discovery& orig) {
}

discovery::~discovery() {
}
bool 
discovery::is_aggregator(uint64_t dpid){
    if(dpid==AGG_DPID){
        return true;
    }else{
        return false;
    }
}
void 
discovery::detect_CM(cofdpt* dpt){
    cflowentry dhcp_req(OFP12_VERSION); 
        dhcp_req.set_command(OFPFC_ADD);
        dhcp_req.set_idle_timeout(0);
        dhcp_req.set_hard_timeout(0);
        dhcp_req.set_table_id(0);
        dhcp_req.set_cookie(cookie_mask);
        dhcp_req.set_cookie_mask(cookie_mask);
        dhcp_req.set_flags(0);
        dhcp_req.set_priority(3);
        dhcp_req.match.set_in_port(CMTS_PORT);
        dhcp_req.match.set_ipv4_dst(caddress(AF_INET, DPS_IP));
        dhcp_req.match.set_udp_dst(67);
        dhcp_req.match.insert(coxmatch_ofb_vlan_vid((uint16_t)OFPVID_NONE));
        dhcp_req.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        dhcp_req.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,OFPP12_CONTROLLER);
        //dhcp_req.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,DPS_PORT);
    proxy->send_flow_mod_message(dpt,dhcp_req); 
}
void 
discovery::detect_DPS_traffic(cofdpt* dpt){
    //enable back path
    cflowentry fe_provisioning(0x03); 
        fe_provisioning.set_command(OFPFC_ADD);
        fe_provisioning.set_idle_timeout(0);
        fe_provisioning.set_hard_timeout(0);
        fe_provisioning.set_table_id(0);
        fe_provisioning.set_cookie(cookie_mask);
        fe_provisioning.set_cookie_mask(cookie_mask);
        fe_provisioning.set_flags(0);
        fe_provisioning.set_priority(2);
        fe_provisioning.match.set_in_port(CMTS_PORT);
        fe_provisioning.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_provisioning.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,DPS_PORT);
    proxy->send_flow_mod_message(dpt,fe_provisioning); 
    
    cflowentry fe_provisioning2(0x03); 
        fe_provisioning2.set_command(OFPFC_ADD);
        fe_provisioning2.set_idle_timeout(0);
        fe_provisioning2.set_hard_timeout(0);
        fe_provisioning2.set_table_id(0);
        fe_provisioning2.set_cookie(cookie_mask);
        fe_provisioning2.set_cookie_mask(cookie_mask);
        fe_provisioning2.set_flags(0);
        fe_provisioning2.set_priority(2);
        fe_provisioning2.match.set_in_port(DPS_PORT);
        fe_provisioning2.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_provisioning2.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,CMTS_PORT);
    proxy->send_flow_mod_message(dpt,fe_provisioning2); 
        //enable ARP form CMTS to DPS
    cflowentry fe_provisioning3(0x03); 
        fe_provisioning3.set_command(OFPFC_ADD);
        fe_provisioning3.set_idle_timeout(0);
        fe_provisioning3.set_hard_timeout(0);
        fe_provisioning3.set_table_id(0);
        fe_provisioning3.set_cookie(cookie_mask);
        fe_provisioning3.set_cookie_mask(cookie_mask);
        fe_provisioning3.set_flags(0);
        fe_provisioning3.set_priority(5);
        fe_provisioning3.match.set_in_port(CMTS_PORT);
        fe_provisioning3.match.set_arp_spa(caddress(AF_INET, CMTS_IP));
        fe_provisioning3.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_provisioning3.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,DPS_PORT);
    proxy->send_flow_mod_message(dpt,fe_provisioning3); 
}
void 
discovery::detect_OUI_control_traffic(cofdpt* dpt){
    cflowentry fe(0x03); 
        fe.set_command(OFPFC_ADD);
        fe.set_idle_timeout(0);
        fe.set_hard_timeout(0);
        fe.set_table_id(0);
        fe.set_cookie(cookie_mask);
        fe.set_cookie_mask(cookie_mask);
        fe.set_flags(0);
        fe.set_priority(2);
        fe.match.set_in_port(CMTS_PORT);
        fe.match.set_eth_src(cmacaddr(OUI_MAC),cmacaddr("FF:FF:FF:00:00:00"));
        fe.match.insert(coxmatch_ofb_vlan_vid(OFPVID_PRESENT,OFPVID_PRESENT));
        fe.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe.instructions[0].actions.next() = cofaction_output(OFP12_VERSION,OFPP12_CONTROLLER);
    proxy->send_flow_mod_message(dpt,fe); 
}
bool
discovery::is_hidden_port(uint32_t portid){
    if(portid==CMTS_PORT ||portid==DPS_PORT || portid==PROXY_PORT)
        return true;
    else
        return false;
}
bool
discovery::OUI_is_hidden_port(uint32_t portid){
    if(portid==OUI_NETPORT)
        return true;
    else
        return false;
}
uint16_t
discovery::register_CM(uint64_t mac){
    
    std::map<uint64_t,uint16_t>::iterator it;
    it=CM_reg.find(mac);
    if(it!=CM_reg.end()){
        return 0;
    }else{
        std::cout<<"Registering...\n";
        CM_reg.insert(std::make_pair(mac,VLAN_index));
        proxy->docsis.enable_L2VPN((char*)cmacaddr(mac).c_str(),VLAN_index);
        VLAN_index++;  
        return (uint16_t)(VLAN_index-1);
    }
    
}
void 
discovery::AGS_enable_OUI_traffic(cofdpt* dpt,uint64_t mac, uint16_t vlan){
        //proxy.translator.insert_mac_vlan(mac.get_mac(),vlan);
        
    cflowentry fe_up(0x03); 
        fe_up.set_command(OFPFC_ADD);
        fe_up.set_buffer_id(OFP_NO_BUFFER);
        fe_up.set_idle_timeout(0);
        fe_up.set_hard_timeout(0);
        fe_up.set_table_id(0);
        fe_up.set_cookie(cookie_mask);
        fe_up.set_cookie_mask(cookie_mask);
        fe_up.set_flags(0);
        fe_up.set_priority(5);

        fe_up.match.set_in_port(CMTS_PORT);
        fe_up.match.set_eth_src(cmacaddr(mac));
        fe_up.match.set_vlan_vid(OFPVID_PRESENT|vlan);
        fe_up.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_up.instructions.back().actions.next() = cofaction_pop_vlan(OFP12_VERSION);
        fe_up.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,PROXY_PORT);
    proxy->send_flow_mod_message(dpt,fe_up); 
        
    cflowentry fe_down(0x03); 
        fe_down.set_command(OFPFC_ADD);
        fe_down.set_buffer_id(OFP_NO_BUFFER);
        fe_down.set_idle_timeout(0);
        fe_down.set_hard_timeout(0);
        fe_down.set_table_id(0);
        fe_down.set_cookie(cookie_mask);
        fe_down.set_cookie_mask(cookie_mask);
        fe_down.set_flags(0);
        fe_down.set_priority(5);
        fe_down.match.set_in_port(PROXY_PORT);
        fe_down.match.set_eth_dst(mac);
        fe_down.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_down.instructions.back().actions.next() = cofaction_push_vlan(OFP12_VERSION,C_TAG);
        fe_down.instructions.back().actions.next() = cofaction_set_field(OFP12_VERSION,coxmatch_ofb_vlan_vid(vlan));//reg from MAC
        fe_down.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,CMTS_PORT);
    proxy->send_flow_mod_message(dpt,fe_down);  

    cflowentry fe_data(0x03); 
        fe_data.set_command(OFPFC_ADD);
        fe_data.set_buffer_id(OFP_NO_BUFFER);
        fe_data.set_idle_timeout(0);
        fe_data.set_hard_timeout(0);
        fe_data.set_table_id(0);
        fe_data.set_cookie(cookie_mask);
        fe_data.set_cookie_mask(cookie_mask);
        fe_data.set_flags(0);
        fe_data.set_priority(4);
        fe_data.match.set_in_port(CMTS_PORT);
        fe_data.instructions.next() = cofinst_write_metadata(OFP12_VERSION,(uint64_t)vlan,0);
        fe_data.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        fe_data.instructions.back().actions.next() = cofaction_pop_vlan(OFP12_VERSION);
        fe_data.instructions.next() = cofinst_goto_table(OFP12_VERSION,1);
        
    proxy->send_flow_mod_message(dpt,fe_data);    
    
}
