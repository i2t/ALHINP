/* 
 * File:   structs.h
 * Author: victor
 *
 * Created on 13 de junio de 2014, 10:17
 */

#ifndef STRUCTS_H
#define	STRUCTS_H

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
    rofl::cofaclist*            actions;
    rofl::cofmatch*             match;
    flow_mod_constants*         constants;

};

struct flowpath{
    uint8_t longest;
    std::map<uint64_t , flowmod*> flowmodlist;
};

#endif	/* STRUCTS_H */

