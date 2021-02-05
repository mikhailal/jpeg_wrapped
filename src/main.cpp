#include "jpeg_manipulator.h"
#include <boost/program_options.hpp>
#include <iostream>

void CheckArguments(const boost::program_options::variables_map &vm) {
    if (!(vm.count("inputfile"))) {
        throw std::runtime_error("Input file was not set.");
    } else if (!(vm.count("outputfile"))) {
        throw std::runtime_error("Output file was not set.");
    } else if (!(vm.count("AX"))) {
        throw std::runtime_error("A point, X coordinate was not set.");
    } else if (!(vm.count("AY"))) {
        throw std::runtime_error("A point, Y coordinate was not set.");
    } else if (!(vm.count("BX"))) {
        throw std::runtime_error("B point, X coordinate was not set.");
    } else if (!(vm.count("BY"))) {
        throw std::runtime_error("B point, Y coordinate was not set.");
    } else if (!(vm.count("intensity"))) {
        throw std::runtime_error("Intensity was not set.");
    } else {
        double factor_of_intensity = vm["intensity"].as<double>();
        if ((factor_of_intensity > 1.0) || (factor_of_intensity < 0.0)) {
            throw std::runtime_error("Intensity value should be int [0.0..1.0] range.");
        }
    }
}

int main(int argc, char **argv) {
    // Declare the supported options.
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("inputfile", boost::program_options::value<std::string>(), "input filename")
            ("outputfile", boost::program_options::value<std::string>(), "output filename")
            ("intensity", boost::program_options::value<double>(), "set compression level")
            ("AX", boost::program_options::value<int>(), "X coordinate of point A")
            ("AY", boost::program_options::value<int>(), "Y coordinate of point A")
            ("BX", boost::program_options::value<int>(), "X coordinate of point B")
            ("BY", boost::program_options::value<int>(), "Y coordinate of point B");

    boost::program_options::variables_map vm;

    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);
        CheckArguments(vm);
    } catch (const boost::program_options::error &e) {
        std::cerr << "problem with arguments' parsing: " << e.what() << std::endl;
        return 0;
    } catch (const std::runtime_error &e) {
        std::cerr << "arguments error, missing or invalid: " << e.what() << std::endl;
        return 0;
    } catch (std::exception &e) {
        std::cerr << "unknown exception: " << e.what() << std::endl;
        return -1;
    }

    JpegManipulator jpegManipulator;
    // holds buffer validity variable into metadata struct; always executed successfully
    jpegManipulator.LoadFromJpegFile(vm["inputfile"].as<std::string>().c_str());
    // always executed successfully; initialized, cannot be nullptr; metadata is valid
    auto metadataInfo = jpegManipulator.GetMetadata();
    if (metadataInfo->buffer_is_valid == true) {
        auto jpegBuffer = jpegManipulator.GetDataBuffer();
        for (int index = 0; index < metadataInfo->width * metadataInfo->height * metadataInfo->components; index += 3) {
            int diffAy = index / (metadataInfo->width * metadataInfo->components) - vm["AY"].as<int>();
            int diffBy = index / (metadataInfo->width * metadataInfo->components) - vm["BY"].as<int>();
            if ((diffAy >= 0) && (diffBy < 0)) {
                int diffAx = index % (metadataInfo->width * metadataInfo->components) -
                             metadataInfo->components * vm["AX"].as<int>();
                int diffBx = index % (metadataInfo->width * metadataInfo->components) -
                             metadataInfo->components * vm["BX"].as<int>();
                if ((diffAx > 0) && (diffBx < 0)) {
                    jpegBuffer[index] *= vm["intensity"].as<double>();
                }
            }
        }
        //always executed successfully
        jpegManipulator.SaveToJpegFile(vm["outputfile"].as<std::string>().c_str());
    } else {
        std::cerr << "LibJPEG wrapper: buffer status is illegal" << std::endl;
        return -1;
    }
}
