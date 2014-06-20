/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* 
 * File:   Flowcache.h
 * Author: victor Fuentes
 * University of the Basque country / Universidad del Pais Vasco (UPV/EHU)
 * I2T Research Group
 */

#ifndef FLOWCACHE_H
#define	FLOWCACHE_H

#include <rofl/common/openflow/cofmatch.h>
#include <rofl/common/openflow/cofaclist.h>
//#include "orchestrator.h"

#define ALL_DPID 0xFFFFFFFF
using namespace rofl;

struct cflow{

    uint64_t ask_dpid;
    cflowentry* fe;
};

class ALHINP;


class Flowcache {
public:
    Flowcache(ALHINP *ofproxy);
    Flowcache(const Flowcache& orig);
    virtual ~Flowcache();
    
    uint16_t store_flow(cofmsg_flow_mod *msg, uint64_t whereask);
    void reg_partial_flow(uint16_t virtualcookie,uint64_t dpid);
    void ask_for_flows();
    void deleteflow(uint64_t realcookie);
    void deleteflow(uint32_t outport,cofmatch match);    
    void resetcache();
    bool flow_exists(uint16_t virtualcookie);
    cflow get_flow(uint16_t virtualcookie);
    uint64_t get_dpid_for(uint64_t realcookie);
    uint16_t  get_virtual_cookie(uint64_t realcookie);
    
    
private:
    ALHINP *proxy;
    uint16_t                    cookie_counter;
    std::map < uint64_t /*dpid*/ , std::set <uint16_t> /*virtual cookie*/ > dpidcache;
    std::map < uint16_t /*virtual cookie*/, cflow > flowcache;
};

#endif	/* FLOWCACHE_H */

