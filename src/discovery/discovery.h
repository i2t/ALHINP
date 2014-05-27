/* 
 * File:   discovery.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:19
 */

#ifndef DISCOVERY_H
#define	DISCOVERY_H
#define __STDC_FORMAT_MACROS

#include <stdint.h>

class discovery {
public:
    discovery();
    discovery(const discovery& orig);
    virtual ~discovery();
    bool is_aggregator(uint64_t dpid);
private:
    

};

#endif	/* DISCOVERY_H */

