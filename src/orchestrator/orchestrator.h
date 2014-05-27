/* 
 * File:   orchestrator.h
 * Author: victor
 *
 * Created on 26 de mayo de 2014, 11:06
 */

#ifndef ORCHESTRATOR_H
#define	ORCHESTRATOR_H

class orchestrator {

private:
    
public:
    orchestrator();
    orchestrator(const orchestrator& orig);
    virtual ~orchestrator();
    void AGS_connected();
    void AGS_disconnected();
    void OUI_connected();
    void OUI_disconnected();
    void CTRL_disconnected();
    void CTRL_connected();


};

#endif	/* ORCHESTRATOR_H */

