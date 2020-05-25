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

using namespace yocto::math;
using namespace std::string_literals;
using namespace oidn;

vec3f get_convolution( int i, int j, const img::image<vec3f> &input_image, int window_side);
img::image<vec3f> add_padding( const img::image<vec3f> &input_image,int half_patch_window, int half_search_window);


int main(int argc, char *argv[])
{
  //define images
  auto input_filename = ""s;
  auto output_filename = "out.jpg"s;

  auto half_patch_window = 1;
  auto half_search_window = 2;
  auto sigma = 10;
  auto hf = 4;
  auto b_gaussian_kernel = false;

  
  auto cli = cli::make_cli("prova_nlm", " prova");
  add_option(cli, "-p", half_patch_window, "Half size of the patch window.");
  add_option(cli,"-s",half_search_window,"Half size of search window.");
  add_option(cli,"-h",hf,"Filtering parameter.");
  add_option(cli,"-a",sigma,"standard deviation of the gaussian error.");
  add_option(cli,"-g",b_gaussian_kernel,"use Gaussian kernel, otherwise constant kernel 1/d^2.");

  //arguments
  add_option(cli, "image", input_filename, "Input image.");
  add_option(cli, "output",output_filename,"Output denoised image.");

  /* Parse options */
  parse_cli(cli, argc, argv);

  //check on arguments
  if(half_search_window <= half_patch_window)
    cli::print_fatal("Error: half_search_window should be greather than half_patch_window!");

  auto patch_window_dimension = (half_patch_window * 2) + 1;
  auto search_window_dimension = (half_search_window * 2) + 1;

  // load input image
  auto ioerror = ""s;
  auto input_image = img::image<vec3f>{};
  if(!img::load_image(input_filename,input_image,ioerror))
    cli::print_fatal(ioerror);

  
  //add pad to image (patch_window + search_window)
  auto symmetrized_image = add_padding(input_image,half_patch_window,half_search_window);
  //auto symmetrized_image = img::image<vec3f>{symmetrized_image_temp.size(),zero3f};
  cli::print_info("image padded!\n");

  /*
  for(auto j = 0;j < symmetrized_image.size().y;j++)
  {
    for(auto i = 0; i< symmetrized_image.size().x;i++)
    {
      //get pixel
      //auto& pixel = input_image[{i,j}];
      symmetrized_image[{i,j}] = get_convolution(i,j,symmetrized_image_temp,half_patch_window);
    }
  }
  */

  
  auto output_image = img::image<vec3f>(input_image.size(),zero3f);
  auto weights = std::vector<vec3f>();
  
  auto h =(float) (hf * (sigma/10));
  auto constant_kernel = 1 / (patch_window_dimension * patch_window_dimension);

  auto width = input_image.size().x;
  auto height = input_image.size().y;
  
  //main loop -- iterate on input image (x1,x2) is the center of first patch
  cli::print_progress("compute output", 0, input_image.size().x * input_image.size().y);
  for( auto x2 = 0; x2 <= height -1 ; x2++)
  {
    for( auto x1 = 0; x1 <= width - 1 ; x1++)
    {
      cli::print_progress("compute output", x1 + (input_image.size().y * x2), input_image.size().x * input_image.size().y);
      //compute NLM weights (y1,y2) is the center of the second patch
      auto start_i = max(0, x1 -half_search_window);
      auto end_i = min(x1 + half_search_window ,width-1);
      auto start_j = max(0, x2 - half_search_window);
      auto end_j = min(x2 + half_search_window, height -1);
      
      for ( auto y2 = start_j; y2 <= end_j;y2++)
      {
        for( auto y1 = start_i; y1 <= end_i;y1++)
        {
          //compute distance between two patches
          auto dist2 = zero3f;

          auto den_kernel = 0;
          auto cont_z = 1;
          //calculate Gaussian Kernel
          if(b_gaussian_kernel)
          {
            
            for(auto z2 = -half_patch_window; z2<= half_patch_window; z2++)
            {
              for(auto z1 = -half_patch_window; z1<= half_patch_window; z1++)
              {
                den_kernel += exp(-(pow2() / 2 * pow2(sigma)));
                cont_z++;
              }
            }
            cont_z = 1;
          }

          for(auto z2 = -half_patch_window; z2<= half_patch_window; z2++)
          {
            for(auto z1 = -half_patch_window; z1<= half_patch_window; z1++)
            {
              dist2+= (b_gaussian_kernel ? exp(-(pow2(z1) / 2 * pow2(sigma))) / den_kernel : constant_kernel) * 
              ((symmetrized_image[{x1+z1,x2+z2}] - symmetrized_image[{y1+z1,y2+z2}]) * (symmetrized_image[{x1+z1,x2+z2}] - symmetrized_image[{y1+z1,y2+z2}]));
            
              cont_z++;
            }
          }

          auto w = exp(-(dist2/(3*(h*h))));
          //auto w = exp(-(dist2/(h*h)));
          weights.push_back(w);
        }
      }

      //compute denoised pixel (x1,x2)
      auto r = zero3f;
      auto s = zero3f;
      auto cont = 0;
      for ( auto y2 = x2 - half_search_window; y2 <= x2 + half_search_window; y2++)
      {
        for( auto y1 = x1 - half_search_window; y1 <= x1 + half_search_window; y1++)
        {
          r += weights.at(cont) * symmetrized_image[{y1,y2}];
          s += weights.at(cont);
          cont++;
        }
      }

      //final estimate based on the assumption that original planes take values in [0,255]
      output_image[{x1,x2}] = min(max(r/s,zero3f),{255,255,255});

      //clear weights 
      weights.clear();
    }
  }
  
  
  //auto output_image = add_padding(input_image,half_patch_window,half_search_window);
  //save image
  if(!img::save_image(output_filename,output_image,ioerror))
    cli::print_fatal(ioerror);
  printf("\n Fine\n");

  return 0;
}

img::image<vec3f> add_padding( const img::image<vec3f> &input_image,int half_patch_window, int half_search_window)
{
  auto pad = half_patch_window + half_search_window;
  auto new_width = input_image.size().x + (2 * pad);
  auto new_height = input_image.size().y + (2 * pad);
  auto padded_dimension = vec2i{new_width,new_height};
  auto padded_image = img::image<vec3f>(padded_dimension,zero3f);

  //auto left_border = img::image<vec3f>({pad,input_image.size().y},zero3f);

  /*
  for(auto j = 0;input_image.size().y;j++)
  {
    for(auto i = 0; i < pad; i++)
    {
      padded_image[{(pad-1) - i,j+pad}] = input_image[{i,j}];
    }
  }
  */
  
  //fill the image
  for(int j = 0;j < input_image.size().y ;j++)
    for(int i = 0; i < input_image.size().x; i++)
      padded_image[{i + pad,j +pad}] = input_image[{i,j}];
  

  return padded_image;
}

vec3f get_convolution(int i, int j, const img::image<vec3f> &input_image, int window_half_side)
{
  auto sum_pixels = zero3f;
  auto width = input_image.size().x;
  auto height = input_image.size().y;
  auto start_i = max(0, i-window_half_side);
  auto end_i = min(i+window_half_side ,width-1);
  auto start_j = max(0, j - window_half_side);
  auto end_j = min(j + window_half_side, height -1);
  auto n_pixels = 0;

  for(auto c = start_j; c <= end_j; c++)
    for(auto r = start_i; r <= end_i; r++)
    {
      sum_pixels+= input_image[{r,c}];
      n_pixels++; 
    }
  auto result = sum_pixels/n_pixels;
  if(j==0 && i==0)
  {
    printf("numero pixels %i \n",n_pixels);
    printf("valore originale %f , %f , %f \n",input_image[{i,j}].x,input_image[{i,j}].y, input_image[{i,j}].z);
    printf("result %f , %f , %f \n",result.x,result.y, result.z);
  }
  return sum_pixels/n_pixels;
}