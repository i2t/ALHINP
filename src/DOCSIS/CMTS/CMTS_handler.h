/* 
 * File:   CMTS_handler.h
 * Author: victor
 *
 * Created on 28 de abril de 2014, 17:37
 */

#ifndef CMTS_HANDLER_H
#define	CMTS_HANDLER_H

#include <inttypes.h>
class CMTS_handler {
public:
    CMTS_handler();
    CMTS_handler(const CMTS_handler& orig);
    void assign_vlan(char* mac,uint16_t vlan_vid,uint8_t interface);
    virtual ~CMTS_handler();
private:

};

#endif	/* CMTS_HANDLER_H */

