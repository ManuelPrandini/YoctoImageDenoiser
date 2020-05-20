
/*
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
namespace img = yocto::image;

using namespace yocto::math;
using namespace std::string_literals;
using namespace oidn;

const auto filter_types = std::vector<std::string>{"RT", "RTLightmap"};

int main(int argc, char* argv[])
{
  //declaring of various parameters
  auto filter_type = filter_types.at(0);
  auto color_filename = ""s;
  auto albedo_filename = ""s;
  auto normal_filename = ""s;
  auto output_filename = "out.hdr"s;
  auto ref_filename = ""s;
  auto weights_filename = ""s;

  auto hdr = false;
  auto srgb = false;
  auto num_benchmark_runs = 0;
  auto num_threads = -1;
  auto set_affinity = -1;
  auto max_memory_mb = -1;
  auto verbose = -1;

  
  std::cout << "Intel(R) Open Image Denoise - Example" << std::endl;
  std::cout << "usage: yimagedenoise [-f/--filter RT|RTLightmap]" << std::endl
            << "               [--ldr color.pfm] [--srgb] [--hdr color.pfm]" << std::endl
            << "               [--alb albedo.pfm] [--nrm normal.pfm]" << std::endl
            << "               [-o/--output output.pfm] [-r/--ref reference_output.pfm]" << std::endl
            << "               [-w/--weights weights.tza]" << std::endl
            << "               [--threads n] [--affinity 0|1] [--maxmem MB]" << std::endl
            << "               [--bench ntimes] [-v/--verbose 0-3]" << std::endl;
  
  
try
{
 
  // parse command line
  auto cli = cli::make_cli("yimgdenoise", " Intel(R) Open Image denoiser");
  add_option(cli, "--filter,-f", filter_type, "Filter type RT|RTLightmap .");
  add_option(cli, "--hdr", hdr, "Hdr image.");
  add_option(cli, "--srgb", srgb, "Srgb image.");
  add_option(cli, "--alb,", albedo_filename, "Albedo image.");
  add_option(cli, "--nrm", normal_filename, "Normal image.");
  add_option(cli, "--ref,-r", ref_filename, "Reference output image.");
  add_option(cli, "--weights,-w", weights_filename, "Weights file.");
  add_option(cli, "--threads,-t", num_threads, "Threads.");
  //da vedere affinity per i valori da scegliere
  add_option(cli,"--affinity",set_affinity,"Affinity [0|1].");
  add_option(cli,"--maxmem",max_memory_mb,"Max memory in MB.");
  add_option(cli,"--bench",num_benchmark_runs,"Number of benchmark runs.");
  add_option(cli,"--verbose,-v",verbose,"Verbose [0-3].");
  add_option(cli, "--output,-o", output_filename, "Output image.");
  //arguments
  add_option(cli,"image",color_filename,"Input image.",true);
  parse_cli(cli, argc, argv);


  //checks on arguments
  if (!ref_filename.empty() && num_benchmark_runs > 0)
    cli::print_fatal("Error: reference and benchmark modes cannot be enabled at the same time!");

  if(filter_type.compare("RT") != 0 && filter_type.compare("RTLightmap") != 0)
    cli::print_fatal("Error: invalid filter type!");

  if(!color_filename.empty() && !normal_filename.empty() && albedo_filename.empty())
    cli::print_fatal("Error: unsupported combination of input features!");

  if(srgb && hdr)
    cli::print_fatal("Error: srgb and hdr modes cannot be enabled at the same time!");

  if(filter_type == "RTLightmap" && (!albedo_filename.empty() || !normal_filename.empty()))
    cli::print_fatal("Error: unsupported combination of input features!");
  
  
  // Set MXCSR flags
  if (!ref_filename.empty())
  {
    // In reference mode we have to disable the FTZ and DAZ flags to get accurate results
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_OFF);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_OFF);
  }
  else
  {
    // Enable the FTZ and DAZ flags to maximize performance
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
  }

  // Load the input image
  //ImageBuffer ref;
  
  auto color_image = img::image<vec3f>{};
  auto albedo_image = img::image<vec3f>{};
  auto normal_image = img::image<vec3f>{};
  auto ioerror = ""s;

  cli::print_info("Load input image ...");
  //color = loadImage(color_filename, 3, srgb);
  if(!img::load_image(color_filename,color_image,ioerror))
    cli::print_fatal("Error : "+ioerror);
  if(srgb)
  {
    color_image = img::rgb_to_srgb(color_image);
    cli::print_info("Input image converted to srgb.");
  }

  if (!albedo_filename.empty())
  {
    cli::print_info("Load albedo image ...");
    if(!img::load_image(albedo_filename,albedo_image, ioerror))
      cli::print_fatal(ioerror);
    if(albedo_image.size() != color_image.size())
      cli::print_fatal("Error: invalid albedo image");
    if(srgb)
    {
      albedo_image = img::rgb_to_srgb(albedo_image);
      cli::print_info("Albedo image converted to srgb.");
    }
  }

  if (!normal_filename.empty())
  {
    cli::print_info("Load normal image ...\n");
    if(!img::load_image(normal_filename,normal_image, ioerror))
      cli::print_fatal(ioerror);
    if(normal_image.size() != color_image.size())
      cli::print_fatal("Error: invalid normal image");
    if(srgb)
    {
      normal_image = img::rgb_to_srgb(normal_image);
      cli::print_info("Normal image converted to srgb.");
    }

    //map values of normals between -1,1
    

  }

  
  if (!ref_filename.empty())
  {
    ref = loadImage(ref_filename, 3, srgb);
    if (ref.getDims() != color.getDims())
      throw std::runtime_error("invalid reference output image");
  }
  

  auto width = color_image.size().x;
  auto height = color_image.size().y;
  std::cout << "Resolution: " << width << "x" << height << std::endl;

  // Initialize the output image
  auto output_image = img::image<vec3f>(color_image.size(),zero3f);
  //ImageBuffer output(width, height, 3);

  // Load the filter weights if specified
  auto weights = ""s;
  if (!weights_filename.empty())
  {
    std::cout << "Loading filter weights" << std::endl;
    //cli::load_text(weights_filename);
    if(!cli::load_text(weights_filename,weights,ioerror))
      cli::print_fatal(ioerror);
  }

//FINO A QUI CONTROLLATO!!!

// Initialize the denoising filter
  std::cout << "Initializing" << std::endl;

  //create device
  //vedere come inserire all'interno di extension
  oidn::DeviceRef device = oidn::newDevice();

  const char* errorMessage;
  if (device.getError(errorMessage) != oidn::Error::None)
    cli::print_fatal(errorMessage);
  

  //set parameters of device
  if (num_threads > 0)
    device.set("num_threads", num_threads);
  if (set_affinity >= 0)
    device.set("set_affinity", bool(set_affinity));
  if (verbose >= 0)
    device.set("verbose", verbose);
  device.commit();

  //create filter for device
  //forse è possibile utilizzare image di yocto
  //con load image annessa
  oidn::FilterRef filter = device.newFilter(filter_type.c_str());

  filter.setImage("color", color_image.data(), oidn::Format::Float3, width, height);
  if (!albedo_filename.empty())
  {
    cli::print_info("set filter for albedo\n");
    filter.setImage("albedo", albedo_image.data(), oidn::Format::Float3, width, height);
  }
  if (!normal_filename.empty())
  {
    cli::print_info("set filter for normal\n");
    filter.setImage("normal", normal_image.data(), oidn::Format::Float3, width, height);
  }

  filter.setImage("output", output_image.data(), oidn::Format::Float3, width, height);

  if (hdr)
    filter.set("hdr", true);
  if (srgb)
    filter.set("srgb", true);

  if (max_memory_mb >= 0)
    filter.set("max_memory_mb", max_memory_mb);

  if (!weights.empty())
    filter.setData("weights", &weights, weights.size());

  const bool showProgress =  num_benchmark_runs == 0 && verbose <= 2;
  if (showProgress)
  {
    //filter.setProgressMonitorFunction(progressCallback);
    //signal(SIGINT, signalHandler);
  }

  filter.commit();

  auto timer = cli::print_timed("start denoising \n");

  const int versionMajor = device.get<int>("versionMajor");
  const int versionMinor = device.get<int>("versionMinor");
  const int versionPatch = device.get<int>("versionPatch");

  std::cout << "  version=" << versionMajor << "." << versionMinor << "." << versionPatch
            << ", filter=" << filter_type << std::endl;

  // Denoise the image
  if (!showProgress)
    std::cout << "Denoising" << std::endl;

  filter.execute();

  //const double denoiseTime = timer.query();
  if (showProgress)
    std::cout << std::endl;
  if (verbose <= 2)
    //std::cout << "  msec=" << (1000. * denoiseTime) << std::endl;
    cli::print_elapsed(timer);

  if (showProgress)
  {
    filter.setProgressMonitorFunction(nullptr);
    signal(SIGINT, SIG_DFL);
  }

  if (!output_filename.empty())
  {
    // Save output image
    std::cout << "Saving output" << std::endl;
    if(img::save_image(output_filename, output_image,ioerror))
      cli::print_fatal(ioerror);
  }

  /*
  if (ref)
  {
    // Verify the output values
    std::cout << "Verifying output" << std::endl;

    int numErrors;
    float maxError;
    std::tie(numErrors, maxError) = compareImage(output, ref, 1e-4);

    std::cout << "  values=" << output.getSize() << ", errors=" << numErrors << ", maxerror=" << maxError << std::endl;

    if (numErrors > 0)
    {
      // Save debug images
      std::cout << "Saving debug images" << std::endl;
      saveImage("denoise_in.ppm",   color,  srgb);
      saveImage("denoise_out.ppm",  output, srgb);
      saveImage("denoise_ref.ppm",  ref,    srgb);

      throw std::runtime_error("output does not match the reference");
    }
  }
  

  if (num_benchmark_runs > 0)
  {
    // Benchmark loop
  #ifdef VTUNE
    __itt_resume();
  #endif

    std::cout << "Benchmarking: " << "ntimes=" << num_benchmark_runs << std::endl;
    //timer.reset();

    for (int i = 0; i < num_benchmark_runs; ++i)
      filter.execute();

    //const double totalTime = timer.query();
    auto totalTime = cmn::get_time();
    std::cout << "  sec=" << totalTime << ", msec/image=" << (1000.*totalTime / num_benchmark_runs) << std::endl;

  #ifdef VTUNE
    __itt_pause();
  #endif
  }
}
catch (std::exception& e)
{
  std::cerr << "Error: " << e.what() << std::endl;
  return 1;
}

return 0;
}
*/