/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <regex>

EFUArgs::EFUArgs() {
  CLIParser.add_option("-a,--logip", GraylogConfig.address, "Graylog server IP address");
  CLIParser.add_option("-b,--broker_addr", EFUSettings.KafkaBrokerAddress, "Kafka broker address");
  CLIParser.add_option("-k,--broker_port", EFUSettings.KafkaBrokerPort, "Kafka broker port");
  CLIParser.add_option("-t,--broker_topic", EFUSettings.KafkaTopic, "Kafka broker topic");
  CLIParser.add_option("-c,--core_affinity", [this](std::vector<std::string> Input){return parseAffinityStrings(Input);}, "Thread to core affinity. Ex: \"-c input_t:4\"");
  detectorOption = CLIParser.add_option("-d,--det", det, "Detector name")->required();
  CLIParser.add_option("-f,--file", config_file, "Pipeline-specific config file");
  CLIParser.add_option("-g,--graphite", graphite_ip_addr, "IP address of graphite metrics server");
  CLIParser.add_option("-i,--dip", EFUSettings.DetectorAddress, "IP address of receive interface");
  CLIParser.add_option("-p,--port", EFUSettings.DetectorPort, "TCP/UDP receive port");
  CLIParser.add_option("-m,--cmdport", cmdserver_port, "Command parser tcp port");
  CLIParser.add_option("-o,--gport", graphite_port, "Graphite tcp port");
  CLIParser.add_option("-s,--stopafter", stopafter, "Terminate after timeout seconds");
}

bool EFUArgs::parseAffinityStrings(std::vector<std::string> ThreadAffinityStrings) {
  bool CoreIntegerCorrect = false;
  int CoreNumber = 0;
  try {
    CoreNumber = std::stoi(ThreadAffinityStrings.at(0));
    CoreIntegerCorrect = true;
  } catch (std::invalid_argument &e) {
    //No nothing
  }
  if (ThreadAffinityStrings.size() == 1 and CoreIntegerCorrect) {
    ThreadAffinity.emplace_back(ThreadCoreAffinity{"implicit_affinity", static_cast<std::uint16_t>(CoreNumber)});
  } else {
    std::string REPattern = "([^:]+):(\\d{1,2})";
    std::regex AffinityRE(REPattern);
    std::smatch AffinityRERes;
    for (auto &AffinityStr : ThreadAffinityStrings) {
      if (not std::regex_match(AffinityStr, AffinityRERes, AffinityRE)) {
        return false;
      }
      ThreadAffinity.emplace_back(ThreadCoreAffinity{AffinityRERes[0], static_cast<std::uint16_t>(std::stoi(AffinityRERes[1]))});
    }
  }
  return true;
}

void EFUArgs::printSettings() {
    XTRACE(INIT, ALW, "Starting event processing pipeline2\n");
    XTRACE(INIT, ALW, "  Log IP:        %s\n", GraylogConfig.address.c_str());
    XTRACE(INIT, ALW, "  Detector:      %s\n", det.c_str());
//    XTRACE(INIT, ALW, "  CPU Offset:    %d\n", cpustart);
    XTRACE(INIT, ALW, "  Config file:   %s\n", config_file.c_str());
    XTRACE(INIT, ALW, "  IP addr:       %s\n", EFUSettings.DetectorAddress.c_str());
    XTRACE(INIT, ALW, "  UDP Port:      %d\n", EFUSettings.DetectorPort);
    XTRACE(INIT, ALW, "  Kafka broker:  %s\n", EFUSettings.KafkaBrokerAddress.c_str());
    XTRACE(INIT, ALW, "  Graphite:      %s\n", graphite_ip_addr.c_str());
    XTRACE(INIT, ALW, "  Graphite port: %d\n", graphite_port);
    XTRACE(INIT, ALW, "  Command port:  %d\n", cmdserver_port);
    XTRACE(INIT, ALW, "  Stopafter:     %u\n", stopafter);
}

void EFUArgs::printHelp() {
  std::cout << CLIParser.help();
}

bool EFUArgs::parseAndProceed(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    if (0 == detectorOption->count()) {
      CLIParser.exit(e);
      return false;
    } else {
      det = detectorOption->results()[0];
    }
  }
  CLIParser.reset();
  return true;
}

bool EFUArgs::parseAgain(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return false;
  }
  return true;
}
