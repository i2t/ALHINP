/* 
 * File:   translator.h
 * Author: victor
 *
 * Created on 12 de julio de 2013, 21:00
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

struct real_port {
    uint64_t                                        datap;
    uint32_t                                        real_port;
        
};

class translator {
    
public:
    translator();
    translator(const translator& orig);
    virtual ~translator();
private:
    //port translation 
    std::map<uint32_t , struct real_port >              portmap; //virtual identifier to real id
    std::map<uint64_t , std::map<uint32_t ,uint32_t > > inv_portmap; //real to virtual mapping 

    std::map<uint64_t , uint16_t>                       dpid_vlan;
    std::map<uint64_t , uint16_t>                       mac_vlan;
    std::map<uint32_t , uint64_t>                       ip_mac;
    
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
    uint64_t get_own_dpid(uint32_t virtualport ){return(portmap [virtualport].datap);}
};

#endif	/* TRANSLATOR_H */


