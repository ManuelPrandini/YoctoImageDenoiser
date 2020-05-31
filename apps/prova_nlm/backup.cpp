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


img::image<vec3f> add_padding( const img::image<vec3f> &input_image,int half_patch_window, int half_search_window)
{
  auto pad = half_patch_window + half_search_window;
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

float get_variance(vec3f pixel_color, vec3f mean_pixel)
{
  return pow(pixel_color.x - mean_pixel.x, 2) + pow(pixel_color.y - mean_pixel.y, 2) + pow(pixel_color.z - mean_pixel.z, 2);
}



int main(int argc, char *argv[])
{
  //define images
  auto input_filename = ""s;
  auto output_filename = "out.jpg"s;

  auto half_patch_window = 1;
  auto half_search_window = 2;
  auto sigma = 0.0f;
  auto hf = 0.0f;
  auto color_parameter = 0.0f;
  auto distance_parameter = 0.0f;

  
  auto cli = cli::make_cli("prova_nlm", " prova");
  add_option(cli, "-p", half_patch_window, "Half size of the patch window.");
  add_option(cli,"-s",half_search_window,"Half size of search window.");
  add_option(cli,"-h",hf,"Filtering parameter.");
  add_option(cli,"-a",sigma,"standard deviation of the gaussian error.");
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


  // load input image
  auto ioerror = ""s;
  auto input_image = img::image<vec3f>{};
  if(!img::load_image(input_filename,input_image,ioerror))
    cli::print_fatal(ioerror);

  auto width = input_image.size().x;
  auto height = input_image.size().y;
  auto pad = half_patch_window + half_search_window;
  hf =(float) (hf * sigma);
  auto epsilon = 1e-7f;
  auto patch_window_dimension = (half_patch_window * 2) + 1;
  auto search_window_dimension = (half_search_window * 2) + 1;

  //define denoised output image and weights
  auto output_image = img::image<vec3f>(input_image.size(),zero3f);
  auto weights = std::vector<float>();

  
  //mapping color from (0,1) to (0,255)
  for(auto j = 0; j < height ; j++)
    for(auto i = 0 ; i < width ; i++)
      input_image[{i,j}] *= 255;
  
  //get mean pixel
  auto mean_pixel = get_mean_pixel(input_image);

  //add pad to image (patch_window + search_window)
  auto symmetrized_image = add_padding(input_image,half_patch_window,half_search_window);

  cli::print_info("image padded!\n");

  //main loop -- iterate on input image (x1,x2) is the center of first patch
  cli::print_progress("compute output", 0, input_image.size().x * input_image.size().y);
  for( auto x2 = 0; x2 <= height -1 ; x2++)
  {
    for( auto x1 = 0; x1 <= width - 1 ; x1++)
    {
      cli::print_progress("compute output", x1 + (input_image.size().y * x2), input_image.size().x * input_image.size().y);
      //compute NLM weights (y1,y2) is the center of the second patch

      //for each pixel (y1,y2) in research window 
      for ( auto y2 = x2 - half_search_window; y2 <= x2 + half_search_window;y2++)
      {
        for( auto y1 = x1 -half_search_window; y1 <= x1 + half_search_window ;y1++)
        {
          //compute distance between two patches
          auto dist2 = 0.0f;
          
          //compute distance of the patches
          for(auto z2 = -half_patch_window; z2<= half_patch_window; z2++)
          {
            for(auto z1 = -half_patch_window; z1<= half_patch_window; z1++)
            {
                auto &p = symmetrized_image[{x1+pad+z1 , x2 + pad + z2}];
                auto &q = symmetrized_image[{y1+pad+z1 , y2 + pad + z2}];
                if( x1 == width / 2 && x2 == height / 2)
                {
                  printf("colore p %f \n",p);
                  printf("colore q %f \n",q);
                }
                //auto var_p = get_variance(p,mean_pixel );
                //auto var_q = get_variance(q , mean_pixel);

                dist2 += (pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2)) - (2 * pow(sigma,2));//(var_p + min(var_p,var_q))) / ( (epsilon + var_q + var_p) / (2 * pow(sigma,2)) );
            }
          }

          //printf("distanza %f \n" , dist2);
          //printf("dist %f, %f, %f \n",dist2.x,dist2.y,dist2.z);
          //dist2 = max(0.0f,dist2 - 2 * (sigma * sigma));
          auto den_dist =  pow(patch_window_dimension,2);
          dist2 = max(0.0f, dist2/den_dist);

          //printf("weight %i, %i, %i \n", dist2.x,dist2.y,dist2.z);
          

          //dist2 /= den_dist;

          //calculate weight of (x,y)
          //auto w =  math::exp(-max(dist2 - 2*pow2(sigma),0.0f)/(3*pow2(hf)));
          auto &p = symmetrized_image[{x1 + pad, x2 + pad}] ;
          auto &q = symmetrized_image[{y1 + pad, y2 + pad}] ;
          auto dist = pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2) - (2 * (sigma * sigma)); 
          auto w = exp(-dist / (2 * pow(sigma,2))) * exp(-dist2 / ( pow(hf,2) * (2*pow(sigma,2)) ) ); 
          //printf("weight %f \n",w);
          //printf("weight %f, %f, %f \n",w);
          // ultimo --> auto w = math::exp(-dist2 / (hf * hf));
          //if(x1 == 15 && x2 == 20)
           // printf("weight %f  dist %f \n",w,dist2);
          //auto w = exp(-(dist2/(h*h)));
          
          weights.push_back(w);
        }
      }

      //compute denoised pixel (x1,x2)
        auto r = zero3f;
        auto s = 0.0f;
        auto cont = 0;
        for ( auto y2 = x2 - half_search_window; y2 <= x2 + half_search_window; y2++)
        {
          for( auto y1 = x1 - half_search_window; y1 <= x1 + half_search_window; y1++)
          {
            r += weights.at(cont) * symmetrized_image[{y1+pad,y2+pad}];
            s += weights.at(cont);
            cont++;
          }
        }
        //final estimate based on the assumption that original planes take values in [0,255]
        output_image[{x1,x2}] = r / s; //min(math::max(r/s,0),255);
          if(x1 == 15 && x2 == 20)
              printf("colore %i %i %i \n",output_image[{x1,x2}].x,output_image[{x1,x2}].y,output_image[{x1,x2}].z);
      
      //clear weights 
      weights.clear();
    }
  }

  
  // rescaling colors
  //mapping color from (0,1) to (0,255)
  for(auto j = 0; j < height ; j++)
    for(auto i = 0 ; i < width ; i++)
      output_image[{i,j}] /= 255;
  

  //auto output_image = add_padding(input_image,half_patch_window,half_search_window);
  //save image
  if(!img::save_image(output_filename,output_image,ioerror))
    cli::print_fatal(ioerror);
  printf("\n Fine\n");

  return 0;
}






