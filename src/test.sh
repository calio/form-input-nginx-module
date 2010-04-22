sudo pkill nginx
sudo rm /usr/local/nginx/logs/*
cd ~/ngx
make -j2
sudo make install
sudo nginx
curl -v -d ""  localhost:8088/bar3
cat /usr/local/nginx/logs/error.log
