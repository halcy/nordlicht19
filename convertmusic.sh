ffmpeg -i $1 temp.wav
sox temp.wav -c 1 -r 32000 music.raw
mv music.raw $2
rm temp.wav
