/*
 * xdyn_grpc_airy.cpp
 *
 */
#include <google/protobuf/stubs/common.h>
#include <grpcpp/grpcpp.h>

#include <exception>
#include <iostream>
#include <ssc/text_file_reader.hpp>

#include "ErrorReporter.hpp"
#include "display_command_line_arguments.hpp"
#include "gRPCChecks.hpp"
#include "parse_XdynCommandLineArguments.hpp"
#include "xdyn/grpc/AiryGRPC.hpp"
#include "xdyn/simulator_api/simulator_api.hpp"

constexpr const char* xdyn_grpc_airy_description =
    "This is the Airy wave gRPC server, based on xdyn";

constexpr const char* default_yaml = R"(rotations convention: [psi, theta', phi'']

environmental constants:
    g: {value: 9.81, unit: m/s^2}
    rho: {value: 1026, unit: kg/m^3}
    nu: {value: 1.18e-6, unit: m^2/s}
environment models:
  - model: waves
    discretization:
       n: 128
       omega min: {value: 0.1, unit: rad/s}
       omega max: {value: 6, unit: rad/s}
       energy fraction: 0.999
    spectra:
      - model: airy
        depth: {value: 100, unit: m}
        seed of the random data generator: 0
        stretching:
          delta: 1
          h: {unit: m, value: 0}
        directional spreading:
           type: dirac
           waves propagating to: {value: 90, unit: deg}
        spectral density:
           type: dirac
           omega0: {value: 0.05, unit: rad/s}
           Hs: {value: 15, unit: m}
    output:
        frame of reference: NED
        mesh:
            xmin: {value: -10, unit: m}
            xmax: {value: 10, unit: m}
            nx: 21
            ymin: {value: -20, unit: m}
            ymax: {value: +20, unit: m}
            ny: 41
output:
   - format: hdf5
     filename: simple_waves.h5
     data: ['waves']
)";

struct XdynGrpcAiryCommandLineArguments {
    short unsigned int port{50051};
    std::vector<std::string> yaml_filenames;
    bool catch_exceptions{false};
};

po::options_description attach_command_line_arguments_to_options_description(
    XdynGrpcAiryCommandLineArguments& input_data);
po::options_description attach_command_line_arguments_to_options_description(
    XdynGrpcAiryCommandLineArguments& input_data)
{
    po::options_description desc("Options");
    desc.add_options()("help,h", "Show this help message")(
        "port,p",
        po::value<short unsigned int>(&input_data.port),
        "Port for the gRPC server. Default is 50051. Available values are 1024-65535 (2^16, but port 0 is reserved and unavailable and ports in range 1-1023 are privileged (application needs to be run as root to have access to those ports)")(
        "yml,y",
        po::value<std::vector<std::string>>(&input_data.yaml_filenames),
        "Name(s) of the YAML file(s)")(
        "debug,d",
        "Used by the application's support team to help error diagnosis. Allows us to pinpoint the exact location in code where the error occurred (do not catch exceptions), eg. for use in a debugger.");
    return desc;
}

bool invalid(const XdynGrpcAiryCommandLineArguments& input);
bool invalid(const XdynGrpcAiryCommandLineArguments& input)
{
    if (input.port == 0) {
        std::cerr
            << "Error: no port number was defined (or port 0 was defined, which is equally invalid)."
            << std::endl
            << "You can define one using the -p flag. Check the available port range using the -h flag."
            << std::endl;
        return true;
    }
    // input.port is always less than 65536 because it is a short unsigned int.
    if (input.port < 1024) {
        std::cerr << "Error: you cannot start this gRPC server on port " << input.port
                  << ": only range 1024-65535 is available." << std::endl;
        return true;
    }
    return false;
}

int parse_command_line_for_xdyn_grpc_airy(
    int argc,
    char** argv,
    XdynGrpcAiryCommandLineArguments& input_data);
int parse_command_line_for_xdyn_grpc_airy(
    int argc,
    char** argv,
    XdynGrpcAiryCommandLineArguments& input_data)
{
    const po::options_description desc =
        attach_command_line_arguments_to_options_description(input_data);
    const BooleanArguments has = parse_input(argc, argv, desc);
    input_data.catch_exceptions = not(has.debug);
    if (has.help) {
        print_usage(std::cout, desc, argv[0], xdyn_grpc_airy_description);
        return EXIT_SUCCESS;
    } else if (invalid(input_data)) {
        print_usage(std::cout, desc, argv[0], xdyn_grpc_airy_description);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run(const XdynGrpcAiryCommandLineArguments& input_data, ErrorReporter& error_outputter);
int run(const XdynGrpcAiryCommandLineArguments& input_data, ErrorReporter& error_outputter)
{
    std::string yaml_input;
    if (input_data.yaml_filenames.empty()) {
        yaml_input = default_yaml;
        std::cout << "Using default input YAML:" << std::endl;
        std::cout << yaml_input << std::endl;
    } else {
        error_outputter.run_and_report_errors_without_yaml_dump([&yaml_input, &input_data]() {
            yaml_input =
                ssc::text_file_reader::TextFileReader(input_data.yaml_filenames).get_contents();
        });
    }
    EnvironmentAndFrames env;
    error_outputter.run_and_report_errors_with_yaml_dump(
        [&env, &yaml_input]() { env = get_environment_for_wave_queries(yaml_input); },
        yaml_input);
    if (error_outputter.contains_errors()) {
        return EXIT_FAILURE;
    }
    error_outputter.run_and_report_errors_with_yaml_dump(
        [&env, &input_data]() { run_xdyn_airy_server(env, input_data.port); },
        yaml_input);
    if (error_outputter.contains_errors()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int display_help(char* argv, XdynGrpcAiryCommandLineArguments& input_data);
int display_help(char* argv, XdynGrpcAiryCommandLineArguments& input_data)
{
    const po::options_description desc =
        attach_command_line_arguments_to_options_description(input_data);
    print_usage(std::cout, desc, argv, xdyn_grpc_airy_description);
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    int error = 0;
    XdynGrpcAiryCommandLineArguments input_data;
    ErrorReporter error_outputter;
    try {
        if (argc == 1)
            return display_help(argv[0], input_data);
        error = parse_command_line_for_xdyn_grpc_airy(argc, argv, input_data);
    } catch (boost::program_options::error& e) {
        error_outputter.invalid_command_line(e.what());
        return EXIT_FAILURE;
    }
    if (error) {
        return error;
    }
    if (input_data.catch_exceptions) {
        try {
            return run(input_data, error_outputter);
        } catch (const std::exception& e) {
            error_outputter.internal_error(e.what());
            if (error_outputter.contains_errors()) {
                std::cerr << error_outputter.get_message() << std::endl;
            }
            return EXIT_FAILURE;
        }
    }
    const auto ret = run(input_data, error_outputter);
    google::protobuf::ShutdownProtobufLibrary();
    return ret;
}
