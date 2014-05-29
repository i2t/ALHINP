/* 
 * File:   orchestrator.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:06
 */

#ifndef ORCHESTRATOR_H
#define	ORCHESTRATOR_H

#include <rofl/common/openflow/cofdpt.h>


using namespace rofl;
class ALHINP;
class orchestrator {
    ALHINP *proxy;
private:

public:
    orchestrator(ALHINP *ofproxy);
    orchestrator(const orchestrator& orig);
    virtual ~orchestrator();

    //void OUI_connected(cofdpt* dpt);
    void OUI_disconnected();
    void AGS_connected(cofdpt* dpt);
    void AGS_disconnected(cofdpt* dpt);    
    void AGS_reset_flows(cofdpt* dpt);
    void AGS_set_port_behavior(cofdpt* dpt);
    void OUI_set_port_behavior(cofdpt* dpt);
    void CTRL_disconnected();
    void CTRL_connected();


};

#endif	/* ORCHESTRATOR_H */

