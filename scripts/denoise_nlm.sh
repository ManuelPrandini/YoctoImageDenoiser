resolution=""
samples=""


## declare an array of test names
declare -a arr=("15_classroom" "17_kitchen" )

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
  ./bin/yimage_nlm_denoiser tests/images/ldr/${name_image}/${name_image}_${resolution}_${samples}.jpg --alb tests/images/ldr/${name_image}/${name_image}_albedo_${resolution}_${samples}.jpg --nrm tests/images/ldr/${name_image}/${name_image}_normal_${resolution}_${samples}.jpg -o out/ldr/nlm/${name_image}_${resolution}_${samples}.jpg -p 5 -s 15 -h 50 -a 150
  
  #HDR
  echo "denoise hdr"
  ./bin/yimage_nlm_denoiser tests/images/hdr/${name_image}/${name_image}_${resolution}_${samples}.hdr --alb tests/images/hdr/${name_image}/${name_image}_albedo_${resolution}_${samples}.hdr --nrm tests/images/hdr/${name_image}/${name_image}_normal_${resolution}_${samples}.hdr -o out/hdr/nlm/${name_image}_${resolution}_${samples}.hdr -p 5 -s 15 -h 50 -a 150
  #NLM
  
done