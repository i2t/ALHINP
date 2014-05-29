/* 
 * File:   CMTS_handler.cpp
 * Author: victor
 * 
 * Created on 28 de abril de 2014, 17:37
 */

#include "CMTS_handler.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>

#define ScriptPath
#define AsignVlanScript

//using namespace std;

CMTS_handler::CMTS_handler() {
}

CMTS_handler::CMTS_handler(const CMTS_handler& orig) {
}

CMTS_handler::~CMTS_handler() {
}

void CMTS_handler::L2VPN_enable(char* mac,uint16_t vlan_vid,uint8_t interface){

    std::string string2;
    string2 = mac;
    std::string::iterator end_pos = std::remove(string2.begin(), string2.end(),':');
    string2.erase(end_pos, string2.end());
    string2.insert(4,".");
    string2.insert(9,".");

    
    
    //std::cout << pid << std::endl;
    
    std::string command;
    command = "tclsh assign_vlan.tcl";
    command.append(" ");
    command.append(string2);
    command.append(" ");
    std::ostringstream convert;
    convert << vlan_vid;
    command.append(convert.str());
    command.append(" &");

    std::cout <<"Command over CMTS: "<<command << "\n";
    
    
    //int result =system(command.c_str());

}