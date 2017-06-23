# !/bin/bash
for k in 32 64 128 256 512 1024 2048 4096
do
	./main $k >> fs.dat
	# ./main $k >> fs_click.dat
done
