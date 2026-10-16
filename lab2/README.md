Задание 2.

ssh -p 2207 user@127.0.0.1
ssh user@192.168.56.101

& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" snapshot "ubuntu-var07" take "clean"

touch /tmp/marker
ls /tmp/marker

& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" controlvm "ubuntu-var07" poweroff
& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" snapshot "ubuntu-var07" restore "clean"
& "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe" startvm "ubuntu-var07" --type headless



Задание 3.

cmake -S bench -B build
cmake --build build --config Release
.\build\mc-var.exe --threads 1          # Ninja кладет прямо в /build
.\build\mc-var.exe --threads 2

scp -P 2207 -r "D:\Projects\bsu\cloud-lab\bench" user@127.0.0.1:/home/user/
ssh -p 2207 user@127.0.0.1

sudo apt update
sudo apt install -y build-essential cmake
cd ~/bench
cmake -S . -B build
cmake --build build
./build/mc-var --threads 1
./build/mc-var --threads 2

cp -r /mnt/d/Projects/bsu/cloud-lab/bench ~/projects/
cd ~/projects/bench
rm -rf build
CXX=/usr/bin/g++-13 cmake -S . -B build
cmake --build build
./build/mc-var --threads 1



Задание 4.

aws --endpoint-url http://127.0.0.1:5001 ec2 describe-instance-types `
  --instance-types t3.micro g5.xlarge p4d.24xlarge `
  --query "InstanceTypes[]. [InstanceType,VCpuInfo.DefaultVCpus,MemoryInfo.SizeInMiB,GpuInfo.Gpus[0].Name,GpuInfo.Gpus[0].MemoryInfo.SizeInMiB]" `
  --output table

$env:AWS_CLI_FILE_ENCODING = "UTF-8"
aws --endpoint-url http://127.0.0.1:5001 ec2 run-instances --image-id ami-12345678 --instance-type t3.micro `
  --user-data file://user-data.sh `
  --tag-specifications 'ResourceType=instance,Tags=[{Key=Name,Value=var07}]' `
  --query "Instances[0].InstanceId" --output text
Получили: i-b406d7569704fb578

ЭТУ КОМАНДУ:
aws --endpoint-url http://127.0.0.1:5001 ec2 describe-instance-attribute --instance-id i-b406d7569704fb578 --attribute userData `
  --query "UserData.Value" --output text | base64 --decode
ЗАМЕНИЛ НА:
[System.Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((aws --endpoint-url http://127.0.0.1:5001 ec2 describe-instance-attribute --instance-id i-b406d7569704fb578 --attribute userData --query "UserData.Value" --output text)))

Жизненный цикл инстанса:

aws --endpoint-url http://127.0.0.1:5001 ec2 stop-instances --instance-ids i-b406d7569704fb578
aws --endpoint-url http://127.0.0.1:5001 ec2 describe-instances --instance-ids i-b406d7569704fb578 --query "Reservations[].Instances[].State.Name"
Состояние: stopping → stopped (выше)

aws --endpoint-url http://127.0.0.1:5001 ec2 start-instances --instance-ids i-b406d7569704fb578
Состояние: pending → running (выше)

aws --endpoint-url http://127.0.0.1:5001 ec2 terminate-instances --instance-ids i-b406d7569704fb578
Состояние: shutting-down → terminated (выше)

(выводы команд есть в 4-logs.log)