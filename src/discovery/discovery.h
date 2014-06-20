/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* 
 * File:   discovery.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:19
 */

#ifndef DISCOVERY_H
#define	DISCOVERY_H
#define __STDC_FORMAT_MACROS



#include <stdint.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofdpt.h>

#define ALHINP_DPID 0x1000000000000001ULL

#define FEATURES_REQ 0x45678
#define FEATURES_REQ_TIMER 3

//#define AGG_DPID 0x01
//#define OUI_MAC    "02:00:0C:00:00:00"
//#define CM_MAC      "A4:A2:4A:00:00:00"
//
//#define CONTROLLER_IP "158.227.98.5"
//#define CONTROLLER_OF "1.0"
//#define CONTROLLER_PORT "6633"
//
//#define OF_LISTEN_PORT_FOR_DPIDS "6633"
//#define AGGR_LISTEN_IP "158.227.98.21"
//#define OUI_LISTEN_IP "158.227.98.6"
//
//#define DPS_IP "10.10.10.62"
//#define CMTS_IP "10.10.10.61"
////******** AGS ports *******
//#define CMTS_PORT 1
//#define DATA_PORT 2
//#define DPS_PORT 3
//#define PROXY_PORT 4
////******** OUI ports *******
//
//#define OUI_NETPORT 2
//#define OUI_USERPORT 1
//
//#define VLAN_START 2

#define flow_mask   0x7FFFFFFFFFFFFFFFULL
#define cookiemask 0x8000000000000000ULL

using namespace rofl;

class ALHINP;
class discovery {
    ALHINP *proxy;
    //CMTS_handler CMTS;   
    uint16_t VLAN_index;
    std::map < uint64_t, uint16_t > CM_reg;
    
    
    
    
public:
    discovery(ALHINP *proxy);
    discovery(const discovery& orig);
    virtual ~discovery();
    bool is_aggregator(uint64_t dpid);
    bool is_hidden_port(uint64_t dpid, uint32_t portid);
    bool OUI_is_hidden_port(uint32_t portid);
    
    void detect_CM(cofdpt* dpt);
    void detect_DPS_traffic(cofdpt* dpt);
    void detect_OUI_control_traffic(cofdpt* dpt);
    uint16_t register_CM(uint64_t);
    void AGS_enable_OUI_traffic(cofdpt* dpt,uint64_t mac, uint16_t vlan);
private:
    

};

#endif	/* DISCOVERY_H */

