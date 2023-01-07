#include <string>
#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>

#include "find_process_windows.h"
#include "simple_windows.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;

bool parse_command_line(int argc, char **argv, po::variables_map &vm) {
    po::options_description opt { "Allowed Options" };
    opt.add_options()
        ("help", "Display help message.")
        ("easy", "Inject by creating a remote thread and loading the library (default).")
        ("manual", "Inject by manually mapping the library contents into the remote processes memory.")
    ;

    po::options_description pos { "Positional Arguments" };
    pos.add_options()
        ("library", "Library .dll file to inject.")
        ("process", "Process to attach to.")
    ;

    po::positional_options_description p;
    p.add("library", 1);
    p.add("process", 1);

    po::options_description desc { "Combined options" };
    desc.add(opt).add(pos);

    po::command_line_parser clp { argc, argv };
    clp.options(desc).positional(p);

    try {
        po::store(clp.run(), vm);
    } catch (po::error &e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return true;
    }

    return true;
}

int main(int argc, char **argv) {
    po::variables_map vm;
    if (!parse_command_line(argc, argv, vm))
        return 1;


    if (vm.count("help")) {
        return 0; // The help message is already displayed in parse_command_line
    }

    if (vm.count("manual") && vm.count("easy")) {
        std::cout << "Use either --easy or --manual!" << std::endl;
        return 1;
    }

    if (vm.count("library") != 1) {
        std::cout << "Please specify exactly one library to inject." << std::endl;
        return 1;
    }

    if (vm.count("process") != 1) {
        std::cout << "Please specify exactly one process to attach to." << std::endl;
        return 1;
    }

    auto library_name = vm["library"].as<std::string>();
    fs::path library_path { library_name };
    if (!fs::exists(library_path)) {
        std::cout << "The specified library doesn't exist." << std::endl;
        return 1;
    }

    if (vm.count("manual")) {
        std::cout << "Manual injecting is currently not supported." << std::endl;
        return 1;
    }

    ProcessInfo pi;
    try {
        pi = find_process(vm["process"].as<std::string>());
        simple_inject(pi.pid, library_path.string());
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
