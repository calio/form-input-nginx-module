sudo pkill nginx
sudo rm /usr/local/nginx/logs/*
cd ~/ngx
make -j2
sudo make install
sudo nginx
curl -d "a=calio&b=agentzh&c=cc&d=debug"  localhost:8088
cat /usr/local/nginx/logs/error.log
