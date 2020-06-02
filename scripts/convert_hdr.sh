#convert HDR INTEL to png
for fullfile in out/hdr/intel/* ; do
    filename=$(basename -- "$fullfile")
    extension="${filename##*.}"
    filename="${filename%.*}"
    ./bin/yimageproc ${fullfile} -o out/png/intel/${filename}.png
done

#convert HDR NLM to png
for fullfile in out/hdr/nlm/* ; do
    filename=$(basename -- "$fullfile")
    extension="${filename##*.}"
    filename="${filename%.*}"
    ./bin/yimageproc ${fullfile} -o out/png/nlm/${filename}.png
done

