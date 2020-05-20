#Script created to generate the images to use as Image Denoise input from scenes. 
#-color -albedo -normal

#CORNELLBOX

printf "generate CORNELLBOX \n"
'
printf "LDR \n\n"

printf "pathrace \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_1024_64.jpg -s 64 -r 1024

printf "albedo \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_albedo_1024_64.jpg -s 64 -r 1024 -t color

printf "normal \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_normal_1024_64.jpg -s 64 -r 1024 -t normal


printf "naive \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_naive_1024_64.jpg -s 64 -r 1024 -t naive

printf "HDR \n\n"

printf "pathrace \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/highres/01_cornellbox/01_cornellbox_1280_64.hdr -s 64 -r 1280

printf "albedo \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/highres/01_cornellbox/01_cornellbox_albedo_1280_64.hdr -s 64 -r 1280 -t color

printf "normal \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/highres/01_cornellbox/01_cornellbox_normal_1280_64.hdr -s 64 -r 1280 -t normal

printf "CORNELLBOX finished!\n"

#BATHROOM
'
printf "generate BATHROOM \n"
#./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/highres/11_bathroom1/prova_albedo_bathroom1_1280_64.jpg -s 64 -r 1280 -t color
./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/highres/11_bathroom1/prova_eyelight_bathroom1_1280_64.jpg -s 64 -r 1280 -t eyelight
'
printf "HDR \n"
printf "pathrace \n"
./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/highres/11_bathroom1/bathroom1_1280_64.hdr -s 64 -r 1280

printf "albedo \n"
./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/highres/11_bathroom1/bathroom1_albedo_1280_64.hdr -s 64 -r 1280 -t color

printf "normal \n"
./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/highres/11_bathroom1/bathroom1_normal_1280_64.hdr -s 64 -r 1280 -t normal

printf "eyelight \n"
./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/higres/11_bathroom1/bathroom1_eyelight_1280_64.hdr -s 64 -r 1280 -t eyelight

#printf "naive \n"
#./bin/yscenetrace tests/scenes/11_bathroom1/bathroom1.json -o tests/images/higres/11_bathroom1/bathroom1_naive_1280_64.jpg -s 64 -r 1280 -t naive
'
printf "BATHROOM finished!\n"

#AREALIGHT
'
printf "generate AREALIGHT \n"
printf "pathrace \n"


./bin/yscenetrace tests/scenes/05_arealight/arealight.json -o tests/images/lowres/05_arealight/arealight_1024_64.jpg -s 64 -r 1024

printf "albedo \n"
./bin/yscenetrace tests/scenes/05_arealight/arealight.json -o tests/images/lowres/05_arealight/arealight_albedo_1024_64.jpg -s 64 -r 1024 -t color

printf "normal \n"
./bin/yscenetrace tests/scenes/05_arealight/arealight.json -o tests/images/lowres/05_arealight/arealight_normal_1024_64.jpg -s 64 -r 1024 -t normal

printf "eyelight \n"
./bin/yscenetrace tests/scenes/05_arealight/arealight.json -o tests/images/lowres/05_arealight/arealight_eyelight_1024_64.jpg -s 64 -r 1024 -t eyelight

printf "naive \n"
./bin/yscenetrace tests/scenes/05_arealight/arealight.json -o tests/images/lowres/11_bathroom1/05_arealight/arealight_naive_1024_64.jpg -s 64 -r 1024 -t naive

printf "AREALIGHT finished!\n"
'
#HAIR
'
printf "generate HAIR \n"

printf "pathrace \n"
./bin/yscenetrace tests/scenes/10_hair/hair.json -o tests/images/lowres/10_hair/hair_1024_64.jpg -s 64 -r 1024

printf "albedo \n"
./bin/yscenetrace tests/scenes/10_hair/hair.json -o tests/images/lowres/10_hair/hair_albedo_1024_64.jpg -s 64 -r 1024 -t color

printf "normal \n"
./bin/yscenetrace tests/scenes/10_hair/hair.json -o tests/images/lowres/10_hair/hair_normal_1024_64.jpg -s 64 -r 1024 -t normal

printf "eyelight \n"
./bin/yscenetrace tests/scenes/10_hair/hair.json -o tests/images/lowres/10_hair/hair_eyelight_1024_64.jpg -s 64 -r 1024 -t eyelight

printf "naive \n"
./bin/yscenetrace tests/scenes/10_hair/hair.json -o tests/images/lowres/10_hair/hair_naive_1024_64.jpg -s 64 -r 1024 -t naive

printf "HAIR finished!\n"

'