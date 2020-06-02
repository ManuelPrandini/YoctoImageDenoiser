add_subdirectory(yscenetrace)
add_subdirectory(yimage_intel_denoiser)
add_subdirectory(yimage_nlm_denoiser)

if(YOCTO_OPENGL)
add_subdirectory(ysceneitraces)
endif(YOCTO_OPENGL)
