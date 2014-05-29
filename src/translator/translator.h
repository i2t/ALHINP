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
     
    std::map<uint32_t , struct real_port >              v_port_to_real;
    std::map<uint64_t , std::map<uint32_t ,uint32_t > > real_to_virtual;
    std::map<uint64_t , std::map<uint64_t, uint32_t>  > route_actions;
    std::map<uint64_t , uint16_t>                       dpid_to_net_dev_id;
    
    std::map<uint64_t , uint16_t>                       mac_to_vlan;
    std::map<uint32_t , uint64_t>                       ip_to_mac;
    
    int find_vlan_tag(uint64_t dpid);
    
public:

};

#endif	/* TRANSLATOR_H */


