/* 
 * File:   ALHINP.h
 * Author: victor
 *
 * 
 */

#ifndef ALHINP_H
#define	ALHINP_H

#define __STDC_FORMAT_MACROS

#include <map>
#include <rofl/common/cmacaddr.h>
#include <rofl/common/caddress.h>
#include <rofl/common/crofbase.h>
#include "../orchestrator/orchestrator.h"
#include "../discovery/discovery.h"

#include <list>

using namespace rofl;

class ALHINP : public crofbase {
public:
    ALHINP();
    ALHINP(const ALHINP& orig);
    virtual ~ALHINP();
private:
    friend orchestrator;
    virtual void handle_dpath_open(cofdpt* dpt);

};

#endif	/* ALHINP_H */

