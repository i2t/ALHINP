/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* 
 * File:   translator.h
 * Author: victor Fuentes
 * University of the Basque country / Universidad del Pais Vasco (UPV/EHU)
 * I2T Research Group
 */

#ifndef TRANSLATOR_H
#define	TRANSLATOR_H

#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <map>
#include "../discovery/discovery.h"
//#include "../proxy/ALHINP.h"



enum flowtype {
    /* Maximum number of physical switch ports. */
    DOWNSTREAM = 1,
    UPSTREAM = 2, 
    LOCAL = 3,
    BCLIENTS = 4,
    ALL =5
};

enum flowmodtype{
    WCtoALL = 1,
    WCtoSPECIAL = 2,
    WCtoAGS = 3,
    error = 4,
    AGStoCLI = 5,
    CLItoAGS = 6,
    CLItoCLI = 7,
    AGStoALL = 8,
    CLItoALL = 9,
    VirtPorttoSPECIAL = 10,
    DROP = 11,
    WDROP = 12,
    WCtoCLIENT = 13
};


struct real_port {
    uint64_t                                        datap;
    uint32_t                                        real_port;
        
};

class translator {
    
public:
    translator(ALHINP *proxye);
    translator(const translator& orig);
    virtual ~translator();
private:
    //port translation 
    ALHINP* proxy;
    std::map<uint32_t , struct real_port >              portmap; //virtual identifier to real id
    std::map<uint64_t , std::map<uint32_t ,uint32_t > > inv_portmap; //real to virtual mapping 

    std::map<uint64_t , uint16_t>                       dpid_vlan;
    std::map<uint64_t , uint16_t>                       mac_vlan;
    //std::map<uint32_t , uint64_t>                       ip_mac;
    
    uint16_t find_vlan_tag(uint64_t dpid);
    uint16_t get_vlan_from_mac(uint64_t mac);
    uint32_t translating_algorithm(uint64_t dpid, uint32_t realport);
public:
    uint16_t get_vlan_tag(uint64_t dpid_src,uint64_t dpid_dst);
    void insert_mac_vlan(uint64_t mac, uint16_t vlan);
    void enable_device(uint64_t mac, uint64_t dpid,std::string dpid_s);
    uint32_t get_real_port_id(uint32_t virtualport);
    uint32_t get_virtual_port_id(uint64_t dpid, uint32_t realport);
    void     enable_port (uint64_t dpid, uint32_t realport);
    uint64_t get_own_dpid(uint32_t virtualport );
};

#endif	/* TRANSLATOR_H */


