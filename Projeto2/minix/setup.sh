#!/bin/bash
if [ -z "$1" ];then
echo "Bem vindos ao assistente de criação de sistema minix By Pedro"
echo "Menu:"
echo "[-c] para criar todo sistema de arquivos  Apenas uma vez!"
echo "[-m] para montar"
echo "[-t] Efetuar todo teste com o mesmo modulo"
else
if [ $1 = "-c" ]; then
echo "criador de sistemas de arquivos"
sudo dd if=/dev/zero of=file.img bs=1k count=10000
echo "transformando file.img em um arquivo de bloco"
sudo losetup /dev/loop0 file.img
echo "montando sistema de arquivo minix no arquivo file.img"
sudo mkfs.minix -c /dev/loop0 10000
sudo umount /dev/loop0
sudo mkdir /proj2
sudo mount /dev/loop0 /proj2 
fi
if [ $1 = "-t" ]; then
echo "Efetuando teste com mesmo modulo"
sudo losetup /dev/loop0 file.img
sudo umount /dev/loop0
sudo rmmod minix2
make
sudo insmod minix2.ko
sudo mount /dev/loop0 /proj2 
fi
fi

