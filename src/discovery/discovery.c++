/* 
 * File:   discovery.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:19
 */

#include <rofl/platform/unix/csyslog.h>
#include <rofl/common/crofbase.h>
#include "../proxy/ALHINP.h"

#include "discovery.h"

#define __STDC_FORMAT_MACROS



#define FEATURES_REQ 0x45678
#define FEATURES_REQ_TIMER 3

#define AGG_DPID 0x01
#define OUI_DPID    "02:00:0C:00:00:00"
#define CM_MAC      "A4:A2:4A:00:00:00"

#define CONTROLLER_IP "158.227.98.5"
#define CONTROLLER_OF "1.0"
#define CONTROLLER_PORT "6633"
#define OF_LISTEN_PORT "6633"
#define AGGR_LISTEN_IP "158.227.98.21"
#define OUI_LISTEN_IP "158.227.98.6"

discovery::discovery(ALHINP *proxye) {
    proxy=proxye;
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
void 
discovery::detect_CM(){

}
bool
discovery::is_hidden_port(uint32_t portid){
    return true;
}
bool
discovery::OUI_is_hidden_port(uint32_t portid){
    return true;
}