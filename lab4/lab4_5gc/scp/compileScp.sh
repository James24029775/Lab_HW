
sudo rm -rf /media/sf_shared_folder/scp
sudo cp -r /home/mns2022/project1/scp/ /media/sf_shared_folder/scp
go build -o bin/scp scp.go
chmod +x bin/scp