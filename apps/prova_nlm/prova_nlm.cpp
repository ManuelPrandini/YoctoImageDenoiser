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


img::image<vec3f> add_padding( const img::image<vec3f> &input_image,int pad)
{
  auto new_width = input_image.size().x + (2 * pad);
  auto new_height = input_image.size().y + (2 * pad);
  auto padded_dimension = vec2i{new_width,new_height};
  auto padded_image = img::image<vec3f>(padded_dimension,zero3f);
  
  //fill the image
  for(int j = 0;j < input_image.size().y ;j++)
    for(int i = 0; i < input_image.size().x; i++)
      padded_image[{i + pad,j + pad}] = input_image[{i,j}];
  
  return padded_image;
}

vec3f get_mean_pixel( const img::image<vec3f> & input_image)
{
  auto sum = zero3f;
  for(auto j = 0; j < input_image.size().y; j++)
    for(auto i = 0; i < input_image.size().x ; i++)
    {
      sum += input_image[{i,j}];
    }
  sum /= (input_image.size().x * input_image.size().y);
  return sum;
}

//mapping color from (0,1) to (0,255) or viceversa based on boolean parameter
void mapping_colors(img::image<vec3f> &input_image, bool to_255 = false)
{
    for(auto j = 0; j < input_image.size().y ; j++)
      for(auto i = 0 ; i < input_image.size().x ; i++)
      {
        if (to_255) input_image[{i,j}] *= 255;
        else input_image[{i,j}] /= 255; 
      }
}

float get_variance(const vec3f &pixel_color,const vec3f &mean_pixel)
{
  return pow(pixel_color.x - mean_pixel.x, 2) + pow(pixel_color.y - mean_pixel.y, 2) + pow(pixel_color.z - mean_pixel.z, 2);
}



int main(int argc, char *argv[])
{
  //define images
  auto input_filename = ""s;
  auto output_filename = "out.jpg"s;
  auto albedo_filename = ""s;

  auto half_patch_window = 1;
  auto half_search_window = 2;
  auto sigma = 0.0f;
  auto hf = 0.0f;
  auto color_parameter = 0.0f;
  auto distance_parameter = 0.0f;
  auto b_variance = false;

  
  auto cli = cli::make_cli("prova_nlm", " prova");
  add_option(cli, "-p", half_patch_window, "Half size of the patch window.");
  add_option(cli,"-s",half_search_window,"Half size of search window.");
  add_option(cli,"-h",hf,"Filtering parameter.");
  add_option(cli,"-a",sigma,"sigma parameter on the image.");
  add_option(cli,"-v",b_variance, "apply per-pixel color variance of Rousselle at al.");
  add_option(cli,"--alb",albedo_filename, "Albedo image.");
  //add_option(cli,"-c",color_parameter,"color parameter.");
  //add_option(cli,"-d",distance_parameter,"distance parameter.");

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

  // load input image
  if(!img::load_image(input_filename,input_image,ioerror))
    cli::print_fatal(ioerror);
  cli::print_info("Input image loaded!");

  // load albedo image
  if(!albedo_filename.empty())
  {
    if(!img::load_image(albedo_filename,albedo_image,ioerror))
      cli::print_fatal(ioerror);
    cli::print_info("Albedo image loaded!");
  }

  

  auto width = input_image.size().x;
  auto height = input_image.size().y;
  auto pad = half_patch_window + half_search_window;
  hf =(float) (hf * sigma);
  auto epsilon = 1e-7f;
  auto patch_window_dimension = (half_patch_window * 2) + 1;
  auto search_window_dimension = (half_search_window * 2) + 1;
  auto mean_pixel = zero3f;

  //define denoised output image and weights
  auto output_image = img::image<vec3f>(input_image.size(),zero3f);

  //map color from [0,1] to [0,255]
  mapping_colors(input_image,true);
  if(!albedo_image.empty()) mapping_colors(albedo_image,true);


  //get mean pixel
  if(b_variance) mean_pixel = get_mean_pixel(input_image);

  //add pad to  input image (patch_window + search_window)
  auto symmetrized_image = add_padding(input_image,pad);
  cli::print_info("Input image padded!");

  auto symmetrized_albedo_image = img::image<vec3f>({width + (2 * pad),height + (2 * pad)},zero3f);
  //add pad to albedo image if exists (patch_window + search_window)
  if(!albedo_image.empty())
  {
    symmetrized_albedo_image = add_padding(albedo_image,pad);
    cli::print_info("Albedo image padded!");
  }


  //main loop -- iterate on input image (x1,x2) is the center of first patch
  cli::print_progress("denoising...", 0, width * height);
  for( auto x2 = 0; x2 <= height -1 ; x2++) {
    for( auto x1 = 0; x1 <= width - 1 ; x1++)
    {
      //compute NLM weights (y1,y2) is the center of the second patch
      auto sum_weights = 0.0f;
      auto denoised_color = zero3f;

      //for each pixel (y1,y2) in research window 
      for ( auto y2 = x2 - half_search_window; y2 <= x2 + half_search_window;y2++){
        for( auto y1 = x1 - half_search_window; y1 <= x1 + half_search_window ;y1++)
        {
          //compute distance between two patches
          auto dist2 = 0.0f;
          
          for(auto z2 = -half_patch_window; z2<= half_patch_window; z2++){
            for(auto z1 = -half_patch_window; z1<= half_patch_window; z1++)
            {
                auto &p = symmetrized_image[{x1+pad+z1 , x2 + pad + z2}];
                auto &q = symmetrized_image[{y1+pad+z1 , y2 + pad + z2}];

                if(b_variance)
                {
                  //get variance of both pixels
                  auto var_p = get_variance(p, mean_pixel);
                  auto var_q = get_variance(q , mean_pixel);

                  //apply per-pixel color variance distance by Rousselle et al. formula
                  dist2 += ((pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2)) - (var_p + min(var_p,var_q))) / ( (epsilon + var_q + var_p) / (2 * pow(sigma,2)) );
                } 
                else dist2 += (pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2)) - (2 * pow(sigma,2));
                //dist2 += (pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2));
            }
          }
          //dist2 -= (2 * pow(sigma,2));
          //dist2 = max(0.0f,dist2 - 2 * (sigma * sigma));
          auto den_dist =  3 * pow(patch_window_dimension,2);
          dist2 = max(0.0f, dist2/den_dist);

          auto dist_albedo = 0.0f;
          auto w_albedo = 0.0f;
          if(!albedo_image.empty())
          {
            auto &p = symmetrized_albedo_image[{x1 + pad, x2 + pad}] ;
            auto &q = symmetrized_albedo_image[{y1 + pad, y2 + pad}] ;
            dist_albedo = pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2);
            w_albedo = exp(-(dist_albedo / (2 * pow(sigma,2) ) ) );
            //printf("dist albedo %f \n",dist_albedo);
            //printf("w albedo %f \n",w_albedo);
          }


          //calculate weight of (x,y)
          auto &p = symmetrized_image[{x1 + pad, x2 + pad}] ;
          auto &q = symmetrized_image[{y1 + pad, y2 + pad}] ;
          auto dist = pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2) ;//- (2 * (sigma * sigma)); 
          auto w = (exp(-dist / (2 * pow(sigma,2))) * exp(-dist2 / ( pow(hf,2) * (2*pow(sigma,2)) ) )) * (!albedo_image.empty() ? w_albedo : 1); 

          //printf("w tot %f \n",w);
          //compute denoised pixel (x1,x2)
          denoised_color += w * symmetrized_image[{y1+pad,y2+pad}];

          //sum weights
          sum_weights += w;
        }
      }

      //final estimate based on the assumption that original planes take values in [0,255]
      output_image[{x1,x2}] = denoised_color / sum_weights; //min(math::max(r/sum_weights,0),255);
    }
    cli::print_progress("denoising...", x2, height);
  }
  cli::print_progress("denoising...Done!", height, height);

  //remap color from [0,255] to [0,1] on output image
  mapping_colors(output_image,false);

  
  //save image
  if(!img::save_image(output_filename,output_image,ioerror))
    cli::print_fatal(ioerror);
  cli::print_info("Output saved correctly! \n");

  return 0;
}






