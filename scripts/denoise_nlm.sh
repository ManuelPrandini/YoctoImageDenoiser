#script used to denoise image in ldr and hdr using nlm algoritm
#you can pass in the following array the scene name and when you
#launch the command insert the right resolution and samples to find
#the right image name.
#in the options I not added the parameters relative to nlm because
#for hdr and ldr can be different so  I prefer to set them manually

resolution=""
samples=""

## declare an array of test names
declare -a arr=("extra_dining_room" "extra_living_room" )

#get option relative to resolution and samples
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

    \? ) echo "Usage: denoise_nlm.sh [-r resolution] [-s samples]"
      ;;
  esac

done

## now loop through the above array
for name_image in "${arr[@]}"
do
  echo ${name_image}
  #LDR
  echo "denoise ldr"
  ./bin/yimage_nlm_denoiser tests/images/ldr/${name_image}/${name_image}_${resolution}_${samples}.jpg --alb tests/images/ldr/${name_image}/${name_image}_albedo_${resolution}_${samples}.jpg --nrm tests/images/ldr/${name_image}/${name_image}_normal_${resolution}_${samples}.jpg -o out/ldr/nlm/${name_image}_${resolution}_${samples}.jpg -p 3 -s 10 -h 20 -a 100
  
  #HDR
  echo "denoise hdr"
  ./bin/yimage_nlm_denoiser tests/images/hdr/${name_image}/${name_image}_${resolution}_${samples}.hdr --alb tests/images/hdr/${name_image}/${name_image}_albedo_${resolution}_${samples}.hdr --nrm tests/images/hdr/${name_image}/${name_image}_normal_${resolution}_${samples}.hdr -o out/hdr/nlm/${name_image}_${resolution}_${samples}.hdr -p 3 -s 10 -h 20 -a 100
  #NLM
  
done