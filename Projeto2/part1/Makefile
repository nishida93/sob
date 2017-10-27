# Alex Venturini  - RA: 15294739 
# Luan Bonomi - RA: 15108780
# Pedro Catalini - RA: 15248354
# Matheus Nishida - RA: 12212692
# Daniel Toloto  - RA: 16436065	
# Leonardo Guissi - RA: 15108244


obj-m += projetoMod.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
