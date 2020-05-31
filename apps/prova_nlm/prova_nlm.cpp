#include <yocto_extension/yocto_extension.h>
#include <yocto/yocto_common.h>
#include <yocto/yocto_commonio.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <signal.h>
#include <yocto_pathtrace/yocto_pathtrace.h>

#define PARAM_NOT_SET -1
//NAMESPACES
namespace ext = yocto::extension;
namespace cmn = yocto::common;
namespace cli = yocto::commonio;
namespace img = yocto::image;
namespace math = yocto::math;

using namespace yocto::math;
using namespace std::string_literals;
using namespace oidn;

int main(int argc, char *argv[])
{
  //define some parameters
  auto input_filename = ""s;
  auto output_filename = "out.jpg"s;
  auto albedo_filename = ""s;
  auto normal_filename = ""s;

  auto half_patch_window = 1;
  auto half_search_window = 2;
  auto sigma = 0.0f;
  auto hf = 0.0f;
  auto b_variance = false;

  //parse input
  auto cli = cli::make_cli("prova_nlm", " prova");
  add_option(cli, "-p", half_patch_window, "Half size of the patch window.");
  add_option(cli,"-s",half_search_window,"Half size of search window.");
  add_option(cli,"-h",hf,"Filtering parameter.");
  add_option(cli,"-a",sigma,"sigma parameter on the image.");
  add_option(cli,"-v",b_variance, "apply per-pixel color variance of Rousselle at al.");
  add_option(cli,"--alb",albedo_filename, "Albedo image.");
  add_option(cli,"--nrm",normal_filename, "Normal image.");


  //arguments
  add_option(cli, "image", input_filename, "Input image.");
  add_option(cli, "output",output_filename,"Output denoised image.");

  /* Parse options */
  parse_cli(cli, argc, argv);

  //check on arguments
  if(half_search_window <= half_patch_window)
    cli::print_fatal("Error: half_search_window should be greather than half_patch_window!");

  auto ioerror = ""s;
  auto input_image = img::image<vec3f>{};
  auto albedo_image = img::image<vec3f>{};
  auto normal_image = img::image<vec3f>{};

  // load input image
  if(!img::load_image(input_filename,input_image,ioerror))
    cli::print_fatal(ioerror);
  cli::print_info("Input image loaded!");

  auto width = input_image.size().x;
  auto height = input_image.size().y;

  // load albedo image
  if(!albedo_filename.empty())
  {
    if(!img::load_image(albedo_filename,albedo_image,ioerror))
      cli::print_fatal(ioerror);
    cli::print_info("Albedo image loaded!");

    //check dimensions
    if(albedo_image.size().x != width && albedo_image.size().y != height)
      cli::print_fatal("Error: albedo image size and input image size are different!");
  }

  // load normal image
  if(!normal_filename.empty())
  {
    if(!img::load_image(normal_filename,normal_image,ioerror))
      cli::print_fatal(ioerror);
    cli::print_info("Normal image loaded!");

    //check dimensions
    if(normal_image.size().x != width && normal_image.size().y != height)
      cli::print_fatal("Error: normal image size and input image size are different!");
  }

  auto pad = half_patch_window + half_search_window;
  hf =(float) (hf * sigma);
  auto epsilon = 1e-7f;
  auto patch_window_dimension = (half_patch_window * 2) + 1;
  auto search_window_dimension = (half_search_window * 2) + 1;
  auto mean_pixel = zero3f;

  //map color from [0,1] to [0,255]
  ext::mapping_colors(input_image,true);
  if(!albedo_image.empty()) ext::mapping_colors(albedo_image,true);
  if(!normal_image.empty()) ext::mapping_colors(normal_image,true);

  //get mean pixel
  if(b_variance) mean_pixel = ext::get_mean_pixel(input_image);

  //add pad to  input image (patch_window + search_window)
  auto symmetrized_image = ext::add_padding(input_image,pad);
  cli::print_info("Input image padded!");

  auto symmetrized_albedo_image = img::image<vec3f>({width + (2 * pad),height + (2 * pad)},zero3f);
  //add pad to albedo image if exists (patch_window + search_window)
  if(!albedo_image.empty())
  {
    symmetrized_albedo_image = ext::add_padding(albedo_image,pad);
    cli::print_info("Albedo image padded!");
  }

  auto symmetrized_normal_image = img::image<vec3f>({width + (2 * pad),height + (2 * pad)},zero3f);
  //add pad to normal image if exists (patch_window + search_window)
  if(!normal_image.empty())
  {
    symmetrized_normal_image = ext::add_padding(normal_image,pad);
    cli::print_info("Normal image padded!");
  }

  // compute denoising with non local means on input images
  auto aux_images = std::vector<img::image<vec3f>>();
  if(!symmetrized_normal_image.empty()) aux_images.push_back(symmetrized_normal_image);
  if(!symmetrized_albedo_image.empty()) aux_images.push_back(symmetrized_albedo_image);

  auto output_image = ext::non_local_means_denoiser(symmetrized_image, aux_images,
  height ,width,half_patch_window,half_search_window,hf,sigma,b_variance,mean_pixel);

  //remap color from [0,255] to [0,1] on output image
  ext::mapping_colors(output_image,false);

  
  //save image
  if(!img::save_image(output_filename,output_image,ioerror))
    cli::print_fatal(ioerror);
  cli::print_info("Output saved correctly! \n");

  return 0;
}






