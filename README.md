# NicoOS

`./run-host.sh`

## Working
* Interrupts
    * mouse
    * keyboard
    * syscalls
* Filesystem
* Userspace
    * stdio syscalls
    * fs syscalls
* elf execution in userspace

## Issues
* Running `exec` twice causes memory corruption
* First line in terminal will have '%' appended to the start