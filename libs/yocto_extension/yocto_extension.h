//
// # Yocto/Extension: Tiny Yocto/GL extension
//
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
//

#ifndef _YOCTO_EXTENSION_H_
#define _YOCTO_EXTENSION_H_

// -----------------------------------------------------------------------------
// INCLUDES
// -----------------------------------------------------------------------------
#include <yocto/yocto_image.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_commonio.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageDenoise/oidn.hpp>
#include "ext/image_io.h"
#include "ext/arg_parser.h"


#include <atomic>
#include <future>
#include <memory>

// -----------------------------------------------------------------------------
// ALIASES
// -----------------------------------------------------------------------------
namespace yocto::extension {

// Namespace aliases
namespace ext = yocto::extension;
namespace img = yocto::image;
namespace cli = yocto::commonio;

// Math defitions
using math::bbox3f;
using math::byte;
using math::frame3f;
using math::identity3x4f;
using math::ray3f;
using math::rng_state;
using math::vec2f;
using math::vec2i;
using math::vec3b;
using math::vec3f;
using math::vec3i;
using math::vec4f;
using math::vec4i;
using math::zero2f;
using math::zero3f;

}  // namespace yocto::pathtrace

// -----------------------------------------------------------------------------
// HIGH LEVEL API
// -----------------------------------------------------------------------------
namespace yocto::extension {

    //create OpenImageDenoise device and return it.
    oidn::DeviceRef create_device(int num_threads = -1, int set_affinity = -1, int verbose = -1);

    //set filter to device with also albedo image and normal image as input. Return the filter created.
    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    img::image<vec3f>& normal_image, 
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT");

    //set filter to device. Return the filter created.
    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT");

    //set filter to device with also albedo image as input. Return the filter created.
    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT");


    //execute filter
    void denoise( oidn::FilterRef& filter);

    //add pad to input image setting black pixels on board
    img::image<vec3f> add_padding( const img::image<vec3f> &input_image,int pad);

    //get mean pixel value of the input image
    vec3f get_mean_pixel( const img::image<vec3f> & input_image);

    //mapping color from (0,1) to (0,255) or viceversa based on boolean parameter
    void mapping_colors(img::image<vec3f> &input_image, bool to_255 = false);

    //apply non local means denoiser on symmetrized image ( input images with padding )
    img::image<vec3f> non_local_means_denoiser(const img::image<vec3f> &symmetrized_image,
    const std::vector<img::image<vec3f>> &aux_images,
    int height, int width, int half_patch_window = 3, int half_search_window = 10,float hf = 25.0f, float sigma = 50.0f,
    bool b_variance = false, vec3f mean_pixel = zero3f)  ;
    
}  // namespace yocto::pathtrace

#endif
