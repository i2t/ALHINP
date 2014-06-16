/* 
 * File:   ALHINP.c++
 * Author: victor
 * 
 * Created on 26 de mayo de 2014, 11:07
 */

#include "ALHINP.h"
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/openflow.h>
#include <rofl/common/openflow/coxmatch.h>
#include <list>





using namespace rofl;
using namespace std;
using namespace libconfig;

ALHINP::ALHINP(): crofbase::crofbase((uint32_t)(1 << OFP10_VERSION) | (1 << OFP12_VERSION)){
    //listen for AGS
    discover= new discovery(this);
    manager= new orchestrator(this);
    flowcache= new Flowcache(this);
    rpc_listen_for_dpts(caddress(AF_INET, "158.227.98.21", 6633));
    //listen for OUI
    rpc_listen_for_dpts(caddress(AF_INET, "192.168.10.1", 6633));

}
ALHINP::ALHINP(const ALHINP& orig) {
}
ALHINP::~ALHINP() {
    
}

int ALHINP::parse_config_file(char* file, ALHINPconfig config, ALHINPportconfig portconfig){
    Config cfg;
    try{
        cfg.readFile(file);
    }catch(const FileIOException &fioex){
        std::cerr << "I/O error while reading file." << std::endl;
          return(EXIT_FAILURE);
    }catch(const ParseException &pex){
            std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError() << std::endl;
        return(EXIT_FAILURE);
    }
    // Get the store name.
    try{
        string name = cfg.lookup("name");
        cout << "Store name: " << name << endl << endl;
        string desc =cfg.lookup("desc");
        uint64_t ALHINPdpid = cfg.lookup("dpid");
    }catch(const SettingNotFoundException &nfex){
        cerr << "Uops! something went wrong :S" << endl;
    }
    //GET ALHINP config
    Setting &root = cfg.getRoot();
    uint32_t controllerport;
    uint32_t ouiport;
    uint32_t agsport;
    uint32_t vlanstart;
    try{
        const Setting &AHLPconfig = root["ALHINP-config"];
        if(!(AHLPconfig.lookupValue("CONTROLLER_IP",config.controller_ip) 
                &&AHLPconfig.lookupValue("CONTROLLER_OF_VERSION",config.of_version)
                &&AHLPconfig.lookupValue("CONTROLLER_PORT",controllerport)
                &&AHLPconfig.lookupValue("LISTENING_IP_AGS",config.listening_IP_ags)
                &&AHLPconfig.lookupValue("LISTENING_PORT_AGS",agsport)
                &&AHLPconfig.lookupValue("LISTENING_IP_OUIS",config.listening_IP_oui)
                &&AHLPconfig.lookupValue("VLANstart",vlanstart)
                &&AHLPconfig.lookupValue("LISTENING_PORT_OUIS",ouiport) ));
    }catch(...){
        return(EXIT_FAILURE);
    }
    config.controller_port=(uint16_t)controllerport;
    config.listening_oui_port=(uint16_t)ouiport;
    config.listening_ags_port=(uint16_t)agsport;
    config.VLANstart=(uint16_t)vlanstart;
    
    uint32_t cmtsport;
    uint32_t dataport;
    uint32_t dpsport;
    uint32_t ouinetport;    
    uint32_t ouiuserport;
    try{
        const Setting &AHLPortConfig = root["Port-config"];
        if(!(AHLPortConfig.lookupValue("CONTROLLER_IP",cmtsport) 
                &&AHLPortConfig.lookupValue("CONTROLLER_OF_VERSION",dataport)
                &&AHLPortConfig.lookupValue("CONTROLLER_PORT",dpsport)
                &&AHLPortConfig.lookupValue("LISTENING_IP_AGS",ouinetport)
                &&AHLPortConfig.lookupValue("LISTENING_PORT_AGS",ouiuserport) ));
    }catch(...){
        return(EXIT_FAILURE);
    }
    portconfig.cmts_port=(uint16_t)cmtsport;
    portconfig.data_port=(uint16_t)dataport;
    portconfig.dps_port=(uint16_t)dpsport;
    portconfig.oui_netport=(uint16_t)ouinetport;
    portconfig.oui_userport=(uint16_t)ouiuserport;
    
    
    try{
        const Setting &AGSconfig = root["AGS-config"];
        if(!AGSconfig.lookupValue("AGSDPID",config.AGS_dpid));
    }catch(...){
        return(EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}

void ALHINP::handle_dpath_open(cofdpt* dpt){
    if(discover->is_aggregator(dpt->get_dpid())){
        manager->AGS_connected(dpt);
        //manager->flow_test(dpt);
    }else{
        manager->OUI_connected(dpt);
        rpc_connect_to_ctl(OFP10_VERSION,1,caddress(AF_INET,"127.0.0.1",6633));
        sleep(2);
    }

}
void ALHINP::handle_dpath_close(cofdpt* dpt){
    if(discover->is_aggregator(dpt->get_dpid())){
        //AGS connected
        manager->AGS_disconnected(dpt);
        
        
    }else{
        //An OUI is connected
        manager->OUI_disconnected(dpt);
    }    
}

void ALHINP::handle_ctrl_open(cofctl *ctl){
    controller=ctl;
    std::cout<<"Controller connected\n";
    fflush(stdout);
}
void ALHINP::handle_ctrl_close(cofctl *ctl){
    controller=0;
    std::cout<<"[WARNING]: Controller disconnected\n";
}

void ALHINP::handle_timeout(int opaque){
    manager->handle_timeout(opaque);
}

void ALHINP::handle_packet_in(cofdpt *dpt, cofmsg_packet_in *msg){
    
    manager->dispath_PACKET_IN(dpt,msg);
}

void ALHINP::handle_port_status(cofdpt* dpt, cofmsg_port_status* msg){
    manager->handle_port_status(dpt,msg);
}
void ALHINP::handle_port_mod (cofctl *ctl, cofmsg_port_mod *msg){
    manager->handle_port_mod (ctl, msg);
} 

void ALHINP::handle_get_config_request(cofctl* ctl, cofmsg_get_config_request* msg){
    manager->handle_get_config_request (ctl,msg);
}
void ALHINP::handle_get_config_reply (cofdpt *dpt, cofmsg_get_config_reply *msg){
        //send_set_config_message(dpt,config_flags,miss_send_len);
        delete msg;
        return;
}
void ALHINP::handle_set_config (cofctl *ctl, cofmsg_set_config *msg){
    manager->handle_set_config (ctl, msg);
}

void ALHINP::handle_features_request (cofctl *ctl, cofmsg_features_request *msg){
    std::cout<<"features request received 1\n";
    fflush(stdout);
    manager->handle_features_request (ctl,msg);
}
void ALHINP::handle_features_reply(cofdpt *dpt, cofmsg_features_reply *msg){
    manager->handle_features_reply(dpt,msg);
}

void ALHINP::handle_flow_mod (cofctl *ctl, cofmsg_flow_mod *msg){
    switch (msg->get_command()) {
        case OFPFC_ADD: {
                manager->flow_mod_add(ctl, msg);
                } break;

        case OFPFC_MODIFY: {
                manager->flow_mod_modify(ctl, msg, false);
                } break;

        case OFPFC_MODIFY_STRICT: {
                manager->flow_mod_modify(ctl, msg, true);
                } break;

        case OFPFC_DELETE: {
                manager->flow_mod_delete(ctl, msg, true);
                } break;

        case OFPFC_DELETE_STRICT: {
                manager->flow_mod_delete(ctl, msg, true);
                } break;

        default:
                throw eFlowModBadCommand();
    }
    //delete msg;
    return;
}  
void ALHINP::handle_flow_removed (cofdpt *dpt, cofmsg_flow_removed *msg){
    //firstly exists??
    manager->handle_flow_removed(dpt,msg);
   
    
}
void ALHINP::handle_packet_out (cofctl *ctl, cofmsg_packet_out *msg){
    manager->handle_packet_out(ctl,msg);
}

void ALHINP::handle_barrier_request(cofctl *ctl, cofmsg_barrier_request *msg){
    send_barrier_reply(ctl,msg->get_xid());
}

void ALHINP::handle_flow_stats_request (cofctl *ctl, cofmsg_flow_stats_request *msg){
    manager->handle_flow_stats_request(ctl,msg);
    
}
void ALHINP::handle_flow_stats_reply (cofdpt *dpt, cofmsg_flow_stats_reply *msg){
    manager->handle_flow_stats_reply (dpt, msg);
}

void ALHINP::handle_desc_stats_request (cofctl *ctl, cofmsg_desc_stats_request *msg){
    
    cofdesc_stats_reply stats(
                    OFP10_VERSION,
                    "mfr desc",
                    "DOCSIS ALHINP",
                    "alpha 0.1",
                    "1234567890",
                    "DOCSIS ALHINP");
    send_desc_stats_reply(controller,msg->get_xid(),stats);
    cofmsg_desc_stats_request request;
    delete msg;
    return;
}

void ALHINP::handle_table_stats_request (cofctl *ctl, cofmsg_table_stats_request *msg){
    manager->handle_table_stats_request(ctl,msg);
    
}

void ALHINP::handle_port_stats_request (cofctl *ctl, cofmsg_port_stats_request *msg){
    manager->handle_port_stats_request (ctl, msg);
}
void ALHINP::handle_port_stats_reply   (cofdpt *dpt, cofmsg_port_stats_reply *msg){
    manager->handle_port_stats_reply (dpt, msg);
}