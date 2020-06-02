//
// Implementation for Yocto/Extension.
//

//
// LICENSE:
//
// Copyright (c) 2020 -- 2020 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "yocto_extension.h"

#include <atomic>
#include <deque>
#include <future>
#include <memory>
#include <mutex>


using namespace std::string_literals;

// -----------------------------------------------------------------------------
// MATH FUNCTIONS
// -----------------------------------------------------------------------------
namespace yocto::extension {

// import math symbols for use
using math::abs;
using math::acos;
using math::atan2;
using math::clamp;
using math::cos;
using math::exp;
using math::flt_max;
using math::fmod;
using math::fresnel_conductor;
using math::fresnel_dielectric;
using math::identity3x3f;
using math::invalidb3f;
using math::log;
using math::make_rng;
using math::max;
using math::min;
using math::pif;
using math::pow;
using math::sample_discrete_cdf;
using math::sample_discrete_cdf_pdf;
using math::sample_uniform;
using math::sample_uniform_pdf;
using math::sin;
using math::sqrt;
using math::zero2f;
using math::zero2i;
using math::zero3f;
using math::zero3i;
using math::zero4f;
using math::zero4i;

}  // namespace yocto::pathtrace

// -----------------------------------------------------------------------------
// IMPLEMENTATION FOR EXTENSION
// -----------------------------------------------------------------------------
namespace yocto::extension {

/**-------------------------------------
 * START PART OF INTEL OPEN IMAGE DENOISER
 * -------------------------------------
 * */

    oidn::DeviceRef create_device(int num_threads = -1, int set_affinity = -1)
    {
        // Create an Intel Open Image Denoise device
        auto device = oidn::newDevice();
    
        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
            cli::print_fatal(errorMessage);
  

        //set parameters of device
        if (num_threads > 0)
            device.set("num_threads", num_threads);
        if (set_affinity >= 0)
            device.set("set_affinity", bool(set_affinity));

        //commit all setting
        device.commit();

        return device;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    img::image<vec3f>& normal_image, 
    bool hdr = false, bool srgb = false, std::string filter_type = "RT")
    { 
        auto filter = device.newFilter(filter_type.c_str());

        //input
        cli::print_info("set filter for color image");
        filter.setImage("color", color_image.data(), oidn::Format::Float3, width, height);
       
        //albedo
        cli::print_info("set filter for albedo");
        filter.setImage("albedo", albedo_image.data(), oidn::Format::Float3, width, height);
        
        //normal
        cli::print_info("set filter for normal");
        filter.setImage("normal", normal_image.data(), oidn::Format::Float3, width, height);
        
        //output
        filter.setImage("output", output_image.data(), oidn::Format::Float3, width, height);

        if (hdr)
            filter.set("hdr", true);
        if (srgb)
            filter.set("srgb", true);

        filter.commit();

        return filter;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    bool hdr = false, bool srgb = false, std::string filter_type = "RT")
    { 
        auto filter = device.newFilter(filter_type.c_str());

        cli::print_info("set filter for color image");
        filter.setImage("color", color_image.data(), oidn::Format::Float3, width, height);

        filter.setImage("output", output_image.data(), oidn::Format::Float3, width, height);

        if (hdr)
            filter.set("hdr", true);
        if (srgb)
            filter.set("srgb", true);

        filter.commit();

        return filter;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    bool hdr = false, bool srgb = false, std::string filter_type = "RT")
    { 
        auto filter = device.newFilter(filter_type.c_str());

        cli::print_info("set filter for color image");
        filter.setImage("color", color_image.data(), oidn::Format::Float3, width, height);
       
        cli::print_info("set filter for albedo");
        filter.setImage("albedo", albedo_image.data(), oidn::Format::Float3, width, height);

        filter.setImage("output", output_image.data(), oidn::Format::Float3, width, height);

        if (hdr)
            filter.set("hdr", true);
        if (srgb)
            filter.set("srgb", true);


        filter.commit();

        return filter;
    }

    void denoise( oidn::FilterRef& filter)
    {
        filter.execute();
    }

/**-------------------------------------
 * END PART OF INTEL OPEN IMAGE DENOISER
 * -------------------------------------
 * */



/**-------------------------------------
 * START OF NON LOCAL MEANS DENOISED
 * -------------------------------------
 * */

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
                sum += input_image[{i,j}];

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

    img::image<vec3f> non_local_means_denoiser(const img::image<vec3f> &symmetrized_image,
    const std::vector<img::image<vec3f>> &aux_images,
    int height, int width, int half_patch_window = 3, int half_search_window = 10,float hf = 25.0f, float sigma = 50.0f,
    bool b_variance = false, vec3f mean_pixel = zero3f) 
    {
        auto pad = half_patch_window + half_search_window;
        auto epsilon = 1e-7f;
        auto patch_window_dimension = (half_patch_window * 2) + 1;

        //define denoised output image 
        auto output_image = img::image<vec3f>({width,height},zero3f);
        
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
                                    dist2 += ((pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2)) - (var_p + min(var_p,var_q))) / ( (hf * hf) * (var_p + var_q) + epsilon);//( (epsilon + var_q + var_p) / (2 * pow(sigma,2)) );
                                } 
                                else dist2 += (pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2)) - (2 * pow(sigma,2));
                                //dist2 += (pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2));
                            }
                        }
                        //dist2 -= (2 * pow(sigma,2));
                        //dist2 = max(0.0f,dist2 - 2 * (sigma * sigma));
                        auto den_dist =  3 * pow(patch_window_dimension,2);
                        dist2 = max(0.0f, dist2/den_dist);

                        //calculate weights of auxiliary images ( normal , albedo etc.)
                        auto w_aux = 1.0f;
                        for(auto i = 0; i < aux_images.size(); i++)
                        {
                            auto &p = aux_images[i][{x1 + pad, x2 + pad}] ;
                            auto &q = aux_images[i][{y1 + pad, y2 + pad}] ;
                            auto dist_albedo = pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2);
                            w_aux *= exp(-(dist_albedo / (2 * pow(sigma,2) ) ) );
                            //printf("dist albedo %f \n",dist_albedo);
                            //printf("w albedo %f \n",w_albedo);
                        }

                        //calculate weight of (x,y)
                        auto &p = symmetrized_image[{x1 + pad, x2 + pad}] ;
                        auto &q = symmetrized_image[{y1 + pad, y2 + pad}] ;
                        auto dist = pow(p.x - q.x,2) + pow(p.y - q.y,2) + pow(p.z - q.z,2) ;//- (2 * (sigma * sigma)); 
                        auto w = (exp(-dist / (2 * pow(sigma,2))) * exp(-dist2 / ( pow(hf,2) * (2*pow(sigma,2)) ) ))
                        * (!aux_images.empty() ? w_aux : 1); 

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
        return output_image;
    }


}  // namespace yocto::pathtrace
