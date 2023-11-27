gcc -o Hash src/hash/main.c src/bin/function.c  # create executable Hash

sudo cp Hash /usr/local/bin/  # copy Hash executable to /usr/local/bin
sudo chmod +x /usr/local/bin/Hash  # grant execution rights

