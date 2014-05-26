/* 
 * File:   main.cpp
 * Author: victor
 *
 * Created on 10 de octubre de 2013, 16:27
 */
#include "rofl_config.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/platform/unix/cdaemon.h>
#include <rofl/common/cparams.h>
#include "rofl/common/crofbase.h"
#include "src/proxy/ALHINP.h"


#define ETHSWCTLD_LOG_FILE "/var/log/ALHINP.log"
#define ETHSWCTLD_PID_FILE "/var/run/ALHINP.pid"

using namespace rofl;

int
main(int argc, char** argv)
{
	rofl::cunixenv env_parser(argc, argv);

	/* update defaults */
	env_parser.add_option(coption(true,REQUIRED_ARGUMENT,'l',"logfile","Log file used when daemonization", ETHSWCTLD_LOG_FILE));
	env_parser.add_option(coption(true, REQUIRED_ARGUMENT, 'p', "pidfile", "set pid-file", std::string(ETHSWCTLD_PID_FILE)));
        
#ifdef ROFL_HAVE_OPENSSL
	env_parser.add_option(coption(true, REQUIRED_ARGUMENT, 't', "cert-and-key-file", "Certificate and key to encrypt control traffic (PEM format)", std::string("")));
#endif
	//Parse
	env_parser.parse_args();

	if (not env_parser.is_arg_set("daemonize")) {
		// only do this in non
		std::string ident(env_parser.get_arg("logfile"));

		logging::init();
		rofl::logging::set_debug_level(atoi(env_parser.get_arg("debug").c_str()));
	} else {

		rofl::cdaemon::daemonize(env_parser.get_arg("pidfile"), env_parser.get_arg("logfile"));
		rofl::logging::set_debug_level(atoi(env_parser.get_arg("debug").c_str()));
		rofl::logging::notice << "[ethswctld][main] daemonizing successful" << std::endl;
	}
        ALHINP proxy;
	cioloop::run();
        cioloop::shutdown();

	return 0;
}