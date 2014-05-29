/* 
 * File:   discovery.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:19
 */

#ifndef DISCOVERY_H
#define	DISCOVERY_H
#define __STDC_FORMAT_MACROS

using namespace rofl;

#include <stdint.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofdpt.h>

class ALHINP;
class discovery {
    ALHINP *proxy;
public:
    discovery(ALHINP *proxy);
    discovery(const discovery& orig);
    virtual ~discovery();
    bool is_aggregator(uint64_t dpid);
    bool is_hidden_port(uint32_t portid);
    bool OUI_is_hidden_port(uint32_t portid);
    
    void detect_CM();

private:
    

};

#endif	/* DISCOVERY_H */

