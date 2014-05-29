/* 
 * File:   DOCSISdriver.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:08
 */

#ifndef DOCSISDRIVER_H
#define	DOCSISDRIVER_H

#include "CMTS/CMTS_handler.h"

class DOCSISdriver {
    CMTS_handler cisco;
public:
    DOCSISdriver();
    DOCSISdriver(const DOCSISdriver& orig);
    virtual ~DOCSISdriver();
    int enable_L2VPN(char* mac,uint16_t vlan);

};

#endif	/* DOCSISDRIVER_H */

