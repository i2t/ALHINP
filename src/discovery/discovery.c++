/* 
 * File:   discovery.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:19
 */

#include "discovery.h"
#define __STDC_FORMAT_MACROS
#define AGG_DPID 0x01

discovery::discovery() {
}

discovery::discovery(const discovery& orig) {
}

discovery::~discovery() {
}
bool 
discovery::is_aggregator(uint64_t dpid){
    if(dpid==AGG_DPID){
        return true;
    }else{
        return false;
    }
}
