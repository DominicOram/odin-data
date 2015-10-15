/**
framenotifier_app.cpp

  Created on: 13 Oct 2015
      Author: up45
*/
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;


#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "zmq/zmq.hpp"



void parse_arguments(int argc, char** argv, po::variables_map& vm, LoggerPtr& logger)
{
    try
    {
        std::string config_file;

        // Declare a group of options that will allowed only on the command line
        po::options_description generic("Generic options");
        generic.add_options()
                ("help,h",
                    "Print this help message")
                ("config,c",    po::value<string>(&config_file),
                    "Specify program configuration file")
                ;

        // Declare a group of options that will be allowed both on the command line
        // and in the configuration file
        po::options_description config("Configuration options");
        config.add_options()
                ("logconfig,l",  po::value<string>(),
                    "Set the log4cxx logging configuration file")
                ("ready,r",       po::value<std::string>()->default_value("tcp://127.0.0.1:5001"),
                    "Ready ZMQ endpoint from frameReceiver")
                ("frames,f",     po::value<unsigned int>()->default_value(1),
                    "Set the number of frames to be notified about before terminating")
                ;

        // Group the variables for parsing at the command line and/or from the configuration file
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config);
        po::options_description config_file_options;
        config_file_options.add(config);

        // Parse the command line options
        po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
        po::notify(vm);

        // If the command-line help option was given, print help and exit
        if (vm.count("help"))
        {
            std::cout << "usage: frameReceiver [options]" << std::endl << std::endl;
            std::cout << cmdline_options << std::endl;
            exit(1);
        }

        // If the command line config option was given, parse the specified configuration
        // file for additional options. Note that boost::program_options gives precedence
        // to the first instance occurring. In this case, that implies that command-line
        // options have precedence over equivalent configuration file entries.
        if (vm.count("config"))
        {
            std::ifstream config_ifs(config_file.c_str());
            if (config_ifs)
            {
                LOG4CXX_DEBUG(logger, "Parsing configuration file " << config_file);
                po::store(po::parse_config_file(config_ifs, config_file_options, true), vm);
                po::notify(vm);
            }
            else
            {
                LOG4CXX_ERROR(logger, "Unable to open configuration file " << config_file << " for parsing");
                exit(1);
            }
        }

        if (vm.count("logconfig"))
        {
            PropertyConfigurator::configure(vm["logconfig"].as<string>());
            LOG4CXX_DEBUG(logger, "log4cxx config file is set to " << vm["logconfig"].as<string>());
        }

        if (vm.count("ready"))
        {
            LOG4CXX_DEBUG(logger, "Setting frame notification ZMQ address to " << vm["ready"].as<string>());
        }

        if (vm.count("frames"))
        {
            LOG4CXX_DEBUG(logger, "Setting number of frames to receive to " << vm["frames"].as<unsigned int>());
        }

    }
    catch (po::unknown_option &e)
    {
        LOG4CXX_WARN(logger, "CLI parsing error: " << e.what() << ". Will carry on...");
    }
    catch (Exception &e)
    {
        LOG4CXX_FATAL(logger, "Got Log4CXX exception: " << e.what());
        throw;
    }
    catch (exception &e)
    {
        LOG4CXX_ERROR(logger, "Got exception:" << e.what());
        throw;
    }
    catch (...)
    {
        LOG4CXX_FATAL(logger, "Exception of unknown type!");
        throw;
    }
}

int main(int argc, char** argv)
{
    int rc = 0;

    // Create a default basic logger configuration, which can be overridden by command-line option later
    BasicConfigurator::configure();

    LoggerPtr logger(Logger::getLogger("FrameNotifier"));
    po::variables_map vm;
    parse_arguments(argc, argv, vm, logger);

    zmq::context_t zmq_context;
    zmq::socket_t zsocket(zmq_context, ZMQ_SUB);
    //zsocket.bind(vm["ready"].as<string>().c_str());
    zsocket.connect(vm["ready"].as<string>().c_str());
    zsocket.setsockopt(ZMQ_SUBSCRIBE, "", strlen(""));

    zmq::pollitem_t poll_item;
    poll_item.socket = zsocket;
    poll_item.events = ZMQ_POLLIN;
    poll_item.fd = 0;
    poll_item.revents = 0;

    unsigned long notification_count = 0;
    bool keep_running = true;
    LOG4CXX_DEBUG(logger, "Entering ZMQ polling loop (" << vm["ready"].as<string>().c_str() << ")");
    while (keep_running)
    {
        poll_item.revents = 0;
        zmq::poll(&poll_item, 1, 100);
        if (poll_item.revents & ZMQ_POLLIN)
        {
            LOG4CXX_DEBUG(logger, "Reading data from ZMQ socket");
            notification_count++;
            zmq::message_t msg;
            zsocket.recv(&msg);
            string msg_str(reinterpret_cast<char*>(msg.data()), msg.size()-1);
            LOG4CXX_DEBUG(logger, "Got msg: " << msg_str);
        } else
        {
            // No new data
        }
        if (notification_count >= vm["frames"].as<unsigned int>()) keep_running=false;
    }
    return rc;
}


