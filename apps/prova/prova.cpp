//INCLUDES -- vedere alla fine quali include servono
#include <yocto_extension/yocto_extension.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_commonio.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include <signal.h>

//NAMESPACES
namespace ext = yocto::extension;
namespace cmn = yocto::common;
namespace cli = yocto::commonio;

using namespace std::string_literals;
using namespace oidn;

const auto filter_types = std::vector<std::string>{"RT", "RTLightmap"};

int main(int argc, char* argv[])
{
  //declaring of various parameters
  std::string filterType = filter_types.at(0);
  std::string colorFilename, albedoFilename, normalFilename;
  std::string outputFilename, refFilename;
  std::string weightsFilename;
  

  bool hdr = false;
  bool srgb = false;
  int numBenchmarkRuns = 0;
  int numThreads = -1;
  int setAffinity = -1;
  int maxMemoryMB = -1;
  int verbose = -1;
  outputFilename = "out.hdr"s;

  /*
  std::cout << "Intel(R) Open Image Denoise - Example" << std::endl;
  std::cout << "usage: yimagedenoise [-f/--filter RT|RTLightmap]" << std::endl
            << "               [--ldr color.pfm] [--srgb] [--hdr color.pfm]" << std::endl
            << "               [--alb albedo.pfm] [--nrm normal.pfm]" << std::endl
            << "               [-o/--output output.pfm] [-r/--ref reference_output.pfm]" << std::endl
            << "               [-w/--weights weights.tza]" << std::endl
            << "               [--threads n] [--affinity 0|1] [--maxmem MB]" << std::endl
            << "               [--bench ntimes] [-v/--verbose 0-3]" << std::endl;
  
  */
  // parse command line
  auto cli = cli::make_cli("yimgdenoise", " Intel(R) Open Image denoiser");
  add_option(cli, "--filter,-f", filterType, "Filter type.");
  
  add_option(cli, "--hdr", hdr, "Hdr image.");
  add_option(cli, "--srgb", srgb, "Srgb image.");
  add_option(cli, "--alb,", albedoFilename, "Albedo image.");
  add_option(cli, "--nrm", normalFilename, "Normal image.");
  
  add_option(cli, "--ref,-r", refFilename, "Reference output image.");
  add_option(cli, "--weights,-w", weightsFilename, "Weights file.");
  add_option(cli, "--threads,-t", numThreads, "Threads.");
  //da vedere affinity per i valori da scegliere
  add_option(cli,"--affinity",setAffinity,"Affinity.");
  add_option(cli,"--maxmem",maxMemoryMB,"Max memory in MB.");
  add_option(cli,"--bench",numBenchmarkRuns,"Number of benchmark runs.");
  add_option(cli,"--verbose,-v",verbose,"Verbose.");
  add_option(cli, "--output,-o", outputFilename, "Output image.");
  

  //arguments
  add_option(cli,"image",colorFilename,"Input image.",true);
  

  parse_cli(cli, argc, argv);



  return 0;
}
