
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

/**
 * Denoiser that exploits the Intel Open Image Denoiser library
*/

int main(int argc, char* argv[])
{
  //declaring of various parameters
  auto filter_type = filter_types.at(0);
  auto color_filename = ""s;
  auto albedo_filename = ""s;
  auto normal_filename = ""s;
  auto output_filename = "out.hdr"s;


  auto hdr = false;
  auto srgb = false;
  auto num_threads = -1;
  auto set_affinity = 1;


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
  try
  {
    // parse command line
    auto cli = cli::make_cli("yimage_intel_denoiser", " Intel(R) Open Image denoiser");
    add_option(cli, "--filter,-f", filter_type, "Filter type RT|RTLightmap .");
    add_option(cli, "--hdr", hdr, "Hdr image.");
    add_option(cli, "--srgb", srgb, "Srgb image.");
    add_option(cli, "--alb,", albedo_filename, "Albedo image.");
    add_option(cli, "--nrm", normal_filename, "Normal image.");
    add_option(cli, "--threads,-t", num_threads, "Threads.");
    add_option(cli, "--output,-o", output_filename, "Output image.");
    //arguments
    add_option(cli,"image",color_filename,"Input image.",true);
    parse_cli(cli, argc, argv);


    //checks on arguments
    if(filter_type.compare("RT") != 0 && filter_type.compare("RTLightmap") != 0)
      cli::print_fatal("Error: invalid filter type!");

    if(!color_filename.empty() && !normal_filename.empty() && albedo_filename.empty())
      cli::print_fatal("Error: unsupported combination of input features!");

    if(srgb && hdr)
      cli::print_fatal("Error: srgb and hdr modes cannot be enabled at the same time!");

    if(filter_type == "RTLightmap" && (!albedo_filename.empty() || !normal_filename.empty()))
      cli::print_fatal("Error: unsupported combination of input features!");
    
    
    // Enable the FTZ and DAZ flags to maximize performance
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    // Define images 
    auto color_image = img::image<vec3f>{};
    auto albedo_image = img::image<vec3f>{};
    auto normal_image = img::image<vec3f>{};
    auto ioerror = ""s;

    //Color input image
    cli::print_info("Load input image ...");
    if(!img::load_image(color_filename,color_image,ioerror))
      cli::print_fatal("Error : "+ioerror);
      /*
    if(srgb)
    {
      color_image = img::rgb_to_srgb(color_image);
      cli::print_info("Input image converted to srgb.");
      
    }
*/
    //Color albedo image
    if (!albedo_filename.empty())
    {
      cli::print_info("Load albedo image ...");
      if(!img::load_image(albedo_filename,albedo_image, ioerror))
        cli::print_fatal(ioerror);
      if(albedo_image.size() != color_image.size())
        cli::print_fatal("Error: invalid albedo image");
        /*
      if(srgb)
      {
        albedo_image = img::rgb_to_srgb(albedo_image);
        cli::print_info("Albedo image converted to srgb.");
      }
      */
    }

    //Color normal image
    if (!normal_filename.empty())
    {
      cli::print_info("Load normal image ...\n");
      if(!img::load_image(normal_filename,normal_image, ioerror))
        cli::print_fatal(ioerror);
      if(normal_image.size() != color_image.size())
        cli::print_fatal("Error: invalid normal image");
        /*
      if(srgb)
      {
        normal_image = img::rgb_to_srgb(normal_image);
        cli::print_info("Normal image converted to srgb.");
        
      }
      */
    }
    
    //get information by input image
    cli::print_info("Image info:");
    auto width = color_image.size().x;
    auto height = color_image.size().y;
    std::cout << "Resolution: " << width << "x" << height << std::endl;

    // Initialize the output image
    auto output_image = img::image<vec3f>(color_image.size(),zero3f);


    // Initialize the denoising filter
    auto device = ext::create_device(num_threads,set_affinity);
  
    //define filter for device
    auto filter = oidn::FilterRef();
    if(albedo_image.empty() && normal_image.empty())
      filter = ext::set_filter_to_device(device,width,height,color_image,output_image,hdr,srgb,filter_type);
    else if(!albedo_image.empty() && !normal_image.empty())
      filter = ext::set_filter_to_device(device,width,height,color_image,output_image,albedo_image,normal_image,
                                        hdr,srgb,filter_type);
    else
    {
      filter = ext::set_filter_to_device(device,width,height,color_image,output_image,albedo_image,
                                        hdr,srgb,filter_type);
    }
    
    //print info of device
    const int versionMajor = device.get<int>("versionMajor");
    const int versionMinor = device.get<int>("versionMinor");
    const int versionPatch = device.get<int>("versionPatch");

    cli::print_info("Device info: ");
    std::cout << "  version=" << versionMajor << "." << versionMinor << "." << versionPatch
              << ", filter=" << filter_type << std::endl;

    // Denoise the image
    auto timer = cli::print_timed("Start denoising ..\n");
    ext::denoise(filter);

    //print time for denoise process
    printf("finished ");
    cli::print_elapsed(timer);


    // Save output image
    if (!output_filename.empty())
    {
      cli::print_info("Saving output..");
      
      if(img::save_image(output_filename, output_image,ioerror))
        cli::print_fatal(ioerror);
      cli::print_info("Saved in "+output_filename);
    }

  }
  catch (std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

return 0;
}
