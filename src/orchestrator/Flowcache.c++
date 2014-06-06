/* 
 * File:   Flowcache.c++
 * Author: victor
 * 
 * Created on 6 de junio de 2014, 9:41
 */

#include <rofl/common/openflow/messages/cofmsg_flow_mod.h>
#include <rofl/common/crofbase.h>
#include "../proxy/ALHINP.h"
#include "Flowcache.h"

Flowcache::Flowcache(ALHINP *ofproxy){ 
    proxy=ofproxy;
    cookie_counter=1;
}

Flowcache::Flowcache(const Flowcache& orig) {
}

Flowcache::~Flowcache() {
}
uint64_t
Flowcache::store_flow(cofmsg_flow_mod *msg){
    cflow* temp = new cflow;
    temp->constants.buffer_id=OFP_NO_BUFFER;
    temp->constants.cookie=msg->get_cookie();
    temp->constants.flags=msg->get_flags();
    temp->constants.hard_timeout=msg->get_hard_timeout();
    temp->constants.idle_timeout=msg->get_idle_timeout();
    temp->constants.priority=msg->get_priority();
    temp->match=msg->get_match();
    temp->actions=msg->get_actions();
    flowcache.insert(std::make_pair(cookie_counter,temp));
    ++cookie_counter;
    return (cookie_counter-1);
}
void 
Flowcache::reg_partial_flow(uint16_t virtualcookie, uint64_t dpid){
    dpidcache [dpid].insert(virtualcookie);
}
void 
Flowcache::deleteflow(uint64_t realcookie){
    std::map < uint16_t /*virtual cookie*/, cflow* > ::iterator it;
    std::set<uint16_t>::iterator it3;
    for(it=flowcache.begin();it!=flowcache.end();++it){   
        if(it->second->cookie==realcookie){ //FOUND!
                std::map < uint64_t /*dpid*/ , std::set <uint16_t> >::iterator it2;    
                for(it2=dpidcache.begin();it2!=dpidcache.end();++it2){
                    it3=it2->second.find(it->first);
                    if(it3!=it2->second.end()){
                        cflowentry flowrem(OFP12_VERSION);
                        proxy->send_flow_mod_message(proxy->dpt_find(it2->first),flowrem);
                    }
                }
            break;
        }
        
    }
}

void 
Flowcache::deleteflow(uint32_t outport,cofmatch match){
    
    
} 

void
Flowcache::resetcache(){
    flowcache.erase(flowcache.begin(),flowcache.end());
    std::map < uint64_t /*dpid*/ , std::set <uint16_t> >::iterator it;
    
    for(it=dpidcache.begin();it!=dpidcache.end();++it){
        (it->second.clear());
    }
    cookie_counter=1;
    
}