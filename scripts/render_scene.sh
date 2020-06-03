#script used to render scene in image in ldr and hdr using yocto pathrace.
#It produce also the normal and albedo images.
#you can pass in the following array the scene name and when you
#launch the command insert the right resolution and samples to find
#the right image name.

name_image=""
file_json=""
resolution=0
samples=0
ext_jpg=".jpg"
ext_hdr=".hdr"
ext_hdr_folder="hdr"
ext_ldr_folder="ldr"

#parse argument to get resolution and samples parameters
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

    \? ) echo "Usage: render_scene.sh [-r resolution] [-s samples]"
      ;;
  esac

done

#"11_bathroom1" "13_bedroom" "15_classroom"
## declare an array of test names
declare -a arr=("extra_dining_room" "extra_living_room")

## now loop through the above array
for name_image in "${arr[@]}"
do
    #check scene
    file_json=tests/scenes/${name_image}/*.json
    #RENDERING JPG
    echo $name_image
  
    echo "render jpg"

    echo "pathrace "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_ldr_folder}/${name_image}/${name_image}_${resolution}_${samples}${ext_jpg} -s ${samples} -r ${resolution} 
    echo "normal "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_ldr_folder}/${name_image}/${name_image}_normal_${resolution}_${samples}${ext_jpg} -s ${samples} -r ${resolution} -t normal
    echo "albedo "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_ldr_folder}/${name_image}/${name_image}_albedo_${resolution}_${samples}${ext_jpg} -s ${samples} -r ${resolution} -t albedo
  
    #RENDERING HDR
    echo "render hdr"

    echo "pathrace "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_hdr_folder}/${name_image}/${name_image}_${resolution}_${samples}${ext_hdr} -s ${samples} -r ${resolution} 
    echo "normal "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_hdr_folder}/${name_image}/${name_image}_normal_${resolution}_${samples}${ext_hdr} -s ${samples} -r ${resolution} -t normal
    echo "albedo "
    ./bin/yscenetrace ${file_json} -o tests/images/${ext_hdr_folder}/${name_image}/${name_image}_albedo_${resolution}_${samples}${ext_hdr} -s ${samples} -r ${resolution} -t albedo

done

exit 0

