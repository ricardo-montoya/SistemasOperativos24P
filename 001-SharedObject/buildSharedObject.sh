echo "imprime_lib.o : imprime_lib.c"
echo "gcc -c imprime_lib.c -o imprime_lib.o"
      gcc -c imprime_lib.c -o imprime_lib.o

echo "imprime_lib.so : imprime_lib.o"
echo "gcc -shared imprime_lib.o -o imprime_lib.so"
     gcc -shared imprime_lib.o -o libimprime_lib.so


echo "imprimeSOexe : imprime_main.c"
echo "gcc -L/home/daego/Documents/SistemasOperativosUAM/001-SharedObject/ -Wall -o imprimeSOexe imprime_main.c -limprime_lib"
      gcc -L/home/daego/Documents/SistemasOperativosUAM/001-SharedObject/ -Wall -o imprimeSOexe imprime_main.c -limprime_lib
      
echo "to clean: rm imprime_main.o imprime_lib.o imprime_lib.so imprimeSOexe"

echo "LD_LIBRARY_PATH=/home/daego/Documents/SistemasOperativosUAM/001-SharedObject/:$LD_LIBRARY_PATH"
      export LD_LIBRARY_PATH=/home/daego/Documents/SistemasOperativosUAM/001-SharedObject/:$LD_LIBRARY_PATH

echo "Ejecucion con carga del SharedObject"
./imprimeSOexe
