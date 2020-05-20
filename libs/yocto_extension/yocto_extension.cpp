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


    oidn::DeviceRef create_device(int num_threads = -1, int set_affinity = -1, int verbose = -1)
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
        if (verbose >= 0)
            device.set("verbose", verbose);

        //commit all setting
        device.commit();

        return device;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    img::image<vec3f>& normal_image, 
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT")
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

        if (max_memory_mb >= 0)
            filter.set("max_memory_mb", max_memory_mb);

        filter.commit();

        return filter;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT")
    { 
        auto filter = device.newFilter(filter_type.c_str());

        cli::print_info("set filter for color image");
        filter.setImage("color", color_image.data(), oidn::Format::Float3, width, height);

        filter.setImage("output", output_image.data(), oidn::Format::Float3, width, height);

        if (hdr)
            filter.set("hdr", true);
        if (srgb)
            filter.set("srgb", true);

        if (max_memory_mb >= 0)
            filter.set("max_memory_mb", max_memory_mb);

        filter.commit();

        return filter;
    }

    oidn::FilterRef set_filter_to_device(oidn::DeviceRef& device, int width, int height,
    img::image<vec3f>& color_image, img::image<vec3f>& output_image,  
    img::image<vec3f>& albedo_image,
    bool hdr = false, bool srgb = false, int max_memory_mb = -1, std::string filter_type = "RT")
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

        if (max_memory_mb >= 0)
            filter.set("max_memory_mb", max_memory_mb);

        filter.commit();

        return filter;
    }

    void denoise( oidn::FilterRef& filter)
    {
        filter.execute();
    }

}  // namespace yocto::pathtrace
