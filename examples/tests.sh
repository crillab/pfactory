cd ../; make clean;cd examples/
cd ../; make -j;cd examples/

for i in $(ls -d */); 
do 
    echo ${i}; 
    cd ${i};
    echo
    echo
    pwd
    valgrind ./${i::-1}
    read -n 1 -s -r -p "Press any key to continue"
    cd ..;
done
