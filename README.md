# NicoOS

`./run-host.sh`

<img width="665" height="559" alt="Screenshot 2026-04-25 at 11 24 12 AM" src="https://github.com/user-attachments/assets/1ecc7212-c09b-40fc-b2d7-7e88bf98875e" />


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
