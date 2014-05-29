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



#define FEATURES_REQ 0x45678
#define FEATURES_REQ_TIMER 3

#define AGG_DPID 0x01
#define OUI_DPID    "02:00:0C:00:00:00"
#define CM_MAC      "A4:A2:4A:00:00:00"

#define CONTROLLER_IP "158.227.98.5"
#define CONTROLLER_OF "1.0"
#define CONTROLLER_PORT "6633"

#define OF_LISTEN_PORT_FOR_DPIDS "6633"
#define AGGR_LISTEN_IP "158.227.98.21"
#define OUI_LISTEN_IP "158.227.98.6"

#define DPS_IP "10.10.10.62"
//******** AGS ports *******
#define CMTS_PORT 1
#define DATA_PORT 2
#define DPS_PORT 3
#define PROXY_PORT 4
//******** OUI ports *******

#define OUI_NETPORT 2
#define OUI_USERPORT 1
//
#define cookie_mask 0x8000000000000000ULL
#define flow_mask   0x7FFFFFFFFFFFFFFFULL

discovery::discovery(ALHINP *proxye) {
    proxy=proxye;
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
    cflowentry cablemodems(OFP12_VERSION); 
        cablemodems.set_command(OFPFC_ADD);
        cablemodems.set_idle_timeout(0);
        cablemodems.set_hard_timeout(0);
        cablemodems.set_table_id(0);
        cablemodems.set_cookie(cookie_mask);
        cablemodems.set_cookie_mask(cookie_mask);
        cablemodems.set_flags(0);
        cablemodems.set_priority(3);
        cablemodems.match.set_in_port(CMTS_PORT);
        cablemodems.match.set_ipv4_dst(caddress(AF_INET, DPS_IP));
        //cablemodems.match.set_udp_dst(67);
        //cablemodems.match.insert(coxmatch_ofb_vlan_vid((uint16_t)0x1000));
        //cablemodems.match.insert(coxmatch_ofb_vlan_vid((uint16_t)0x1000,(uint16_t)0x1000));
        cablemodems.instructions.next() = cofinst_apply_actions(OFP12_VERSION);
        cablemodems.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,OFPP12_CONTROLLER);
        cablemodems.instructions.back().actions.next() = cofaction_output(OFP12_VERSION,DPS_PORT);
    proxy->send_flow_mod_message(dpt,cablemodems); 
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