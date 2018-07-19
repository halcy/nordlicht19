ffmpeg -i $1 temp.wav
sox -n -r 32000 -c 1 silence.wav trim 0.0 15.0
sox temp.wav -c 1 -r 32000 music.wav
sox music.wav silence.wav -c 1 -r 32000 music.raw
mv music.raw $2
rm temp.wav
rm music.wav
rm silence.wav
