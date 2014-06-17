/* 
 * File:   translator.cc
 * Author: victor
 * 
 * Created on 12 de julio de 2013, 21:00
 */

#include "translator.h"
#include <iostream>
#include <string>
#include <sstream>
#include "../proxy/ALHINP.h"
//#include "../proxy/ALHINP.h"



/**
 * @brief: constructor
 */
translator::translator(ALHINP *proxye) {
    proxy=proxye;
}

translator::translator(const translator& orig) {
    
}

translator::~translator() {
}

uint16_t
translator::get_vlan_tag(uint64_t dpid_src,uint64_t dpid_dst){
   // std::cout<< "src_dpid: " << dpid_src;
   // std::cout<< " dst_dpid: " << dpid_dst <<"\n";
   
    
    if(dpid_src!=proxy->config.AGS_dpid){
        return find_vlan_tag(dpid_src); //UPSTREAM
    }else{
        return find_vlan_tag(dpid_dst); //DOWNSTREAM
    }
}
uint16_t translator::find_vlan_tag(uint64_t dpid){ //VLAN assigment algorithm to TBD
    return (uint16_t)dpid_vlan [dpid];
}
void translator::enable_device(uint64_t mac, uint64_t dpid,std::string dpid_s){
    if(dpid==proxy->config.AGS_dpid){
    dpid_vlan.insert(std::make_pair(dpid,1));
    std::cout <<"\n" << dpid_s <<" as Device ID "<< (uint16_t)dpid_vlan [dpid] <<"\n";
    }else{
    dpid_vlan.insert(std::make_pair(dpid,get_vlan_from_mac(mac)));
    std::cout <<"\n" << dpid_s <<" as Device ID "<< (uint16_t)dpid_vlan [dpid] <<"\n";
    }
}
void translator::insert_mac_vlan(uint64_t mac, uint16_t vlan){
    mac_vlan.insert(std::make_pair(mac,vlan));
}
uint16_t translator::get_vlan_from_mac(uint64_t mac){
    std::cout<<mac_vlan [mac];
    return (mac_vlan [mac]);
}
uint32_t translator::get_real_port_id(uint32_t virtualport){
    if(virtualport<OFPP10_MAX)
        return(portmap[virtualport].real_port);
    else
        return virtualport;
}
uint32_t translator::get_virtual_port_id(uint64_t dpid, uint32_t realport){
    uint32_t virtualport = inv_portmap [dpid] [realport];
    return(virtualport);
}
void translator::enable_port (uint64_t dpid, uint32_t realport){
    uint32_t virtualport = translating_algorithm(dpid,realport);
    portmap [virtualport].datap=dpid;
    portmap [virtualport].real_port=realport;
    inv_portmap [dpid] [realport] = virtualport;
    std::cout<< "\ndpid: "<<(uint64_t) dpid << " port " << (int)realport<< " virtualized as: " << (int)inv_portmap [dpid] [realport] << "\n";
  
}
uint32_t translator::translating_algorithm(uint64_t dpid, uint32_t realport ){
    return ((dpid_vlan [dpid])*10 +realport);
}
uint64_t translator::get_own_dpid(uint32_t virtualport ){
    std::cout<<"VirtPort: "<<virtualport<<" belongs to: "<<portmap [virtualport].datap<<"\n";
    return(portmap [virtualport].datap);
}