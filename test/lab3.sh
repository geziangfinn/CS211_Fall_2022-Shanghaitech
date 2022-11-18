riscv64-unknown-elf-gcc -march=rv64ia ../test/lib.c ../test/lab3-core0.c -e _start -Ttext=0x10000 -o ../riscv-elf/lab3-core0.riscv
riscv64-unknown-elf-gcc -march=rv64ia ../test/lib.c ../test/lab3-core1.c -e _start -Ttext=0x60000 -o ../riscv-elf/lab3-core1.riscv
./Simulator -c0 ../riscv-elf/lab3-core0.riscv -c1 ../riscv-elf/lab3-core1.riscv