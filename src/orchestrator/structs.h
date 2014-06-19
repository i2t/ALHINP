/* 
 * File:   structs.h
 * Author: victor
 *
 * Created on 13 de junio de 2014, 10:17
 */

#ifndef STRUCTS_H
#define	STRUCTS_H

using namespace std;

struct flow_mod_constants {
    uint64_t                cookie;
    uint16_t                idle_timeout;
    uint16_t                hard_timeout;
    uint16_t                priority;
    uint32_t                buffer_id;
    uint16_t                flags;
    uint8_t                 table_id;
    
};

struct flowmod{
    uint64_t                    dpid;
    cflowentry                  partial_fe;

};

struct flowpath{
    uint8_t longest;
    std::map<uint64_t , cflowentry*> flowmodlist;
};

struct ALHINPconfig {
    string      controller_ip;
    string      of_version;
    uint16_t    controller_port;
    
    string      listening_IP_ags;
    uint16_t    listening_ags_port;

    string      listening_IP_oui;
    uint16_t    listening_oui_port;

    string      DPS_ip;
    string      CMTS_ip;
    
    string      oui_mac;
    string      cm_mac;
    uint16_t    VLANstart;
    
    uint64_t    AGS_dpid;
    uint64_t    ALHINP_dpid;
    
};

struct ALHINPportconfig {
    uint16_t    cmts_port;
    uint16_t    data_port;
    uint16_t    dps_port;
    uint16_t    proxy_port;
    
    uint16_t    oui_netport;
    uint16_t    oui_userport;
};

#endif	/* STRUCTS_H */

