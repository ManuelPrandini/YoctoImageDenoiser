#Script created to generate the images to use as Image Denoise input from scenes. 
#CORNELLBOX
#-color -albedo -normal
'
printf "generate CORNELLBOX \n"
printf "pathrace \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_512_256.jpg -s 256 -r 512
printf "albedo \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_albedo_512_256.jpg -s 256 -r 512 -t eyelight

printf "normal \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_normal_512_256.jpg -s 256 -r 512 -t normal
'
printf "naive \n"
./bin/yscenetrace tests/scenes/01_cornellbox/cornellbox.json -o tests/images/lowres/01_cornellbox/01_cornellbox_naive_512_256.jpg -s 256 -r 512 -t naive
printf "CORNELLBOX finished!\n"