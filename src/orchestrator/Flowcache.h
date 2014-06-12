/* 
 * File:   Flowcache.h
 * Author: victor
 *
 * Created on 6 de junio de 2014, 9:41
 */

#ifndef FLOWCACHE_H
#define	FLOWCACHE_H

#include <rofl/common/openflow/cofmatch.h>
#include <rofl/common/openflow/cofaclist.h>
//#include "orchestrator.h"

#define ALL_DPID 0xFFFFFFFF
using namespace rofl;
struct params {
    uint64_t                cookie;
    uint16_t                idle_timeout;
    uint16_t                hard_timeout;
    uint16_t                priority;
    uint32_t                buffer_id;
    uint16_t                flags;

};

struct cflow{

    uint64_t cookie;
    cofmatch match;
    cofaclist actions;
    uint64_t ask_dpid;
    params constants;
};

class ALHINP;


class Flowcache {
public:
    Flowcache(ALHINP *ofproxy);
    Flowcache(const Flowcache& orig);
    virtual ~Flowcache();
    
    uint16_t store_flow(cofmsg_flow_mod *msg);
    void reg_partial_flow(uint16_t virtualcookie,uint64_t dpid);
    void ask_for_flows();
    void deleteflow(uint64_t realcookie);
    void deleteflow(uint32_t outport,cofmatch match);    
    void resetcache();
    
    
private:
    ALHINP *proxy;
    uint16_t                    cookie_counter;
    std::map < uint64_t /*dpid*/ , std::set <uint16_t> /*virtual cookie*/ > dpidcache;
    std::map < uint16_t /*virtual cookie*/, cflow* > flowcache;
};

#endif	/* FLOWCACHE_H */

