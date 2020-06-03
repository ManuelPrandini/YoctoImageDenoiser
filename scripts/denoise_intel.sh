#script used to denoise image in ldr and hdr using INTEL algoritm
#you can pass in the following array the scene name and when you
#launch the command insert the right resolution and samples to find
#the right image name.

name_image=""
resolution=""
samples=""

## declare an array of test names
declare -a arr=("extra_dining_room" "extra_living_room")

#parse the options relative to resolution and samples
while getopts ":r:s:" opt; do
  case ${opt} in
    r ) 
    # process option r
    resolution=${OPTARG}
      ;;

    s ) 
    # process option s
    samples=${OPTARG}
      ;;

    \? ) echo "Usage: denoise_intel.sh [-r resolution] [-s samples]"
      ;;
  esac

done

## now loop through the above array
for name_image in "${arr[@]}"
do
  echo ${name_image}
  
  #LDR
  echo "denoise ldr"
  ./bin/yimage_intel_denoiser tests/images/ldr/${name_image}/${name_image}_${resolution}_${samples}.jpg --alb tests/images/ldr/${name_image}/${name_image}_albedo_${resolution}_${samples}.jpg --nrm tests/images/ldr/${name_image}/${name_image}_normal_${resolution}_${samples}.jpg -o out/ldr/intel/${name_image}_${resolution}_${samples}.jpg 
  
  #HDR
  echo "denoise hdr"
  ./bin/yimage_intel_denoiser tests/images/hdr/${name_image}/${name_image}_${resolution}_${samples}.hdr --alb tests/images/hdr/${name_image}/${name_image}_albedo_${resolution}_${samples}.hdr --nrm tests/images/hdr/${name_image}/${name_image}_normal_${resolution}_${samples}.hdr -o out/hdr/intel/${name_image}_${resolution}_${samples}.hdr --hdr
  #NLM
done
