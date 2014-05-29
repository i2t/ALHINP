/* 
 * File:   DOCSISdriver.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:08
 */

#include "DOCSISdriver.h"

DOCSISdriver::DOCSISdriver() {
}

DOCSISdriver::DOCSISdriver(const DOCSISdriver& orig) {
}

DOCSISdriver::~DOCSISdriver() {
}
int
DOCSISdriver::enable_L2VPN(char* mac,uint16_t vlan) {
    cisco.L2VPN_enable(mac,vlan,1);
    return 1;
}