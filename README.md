# NicoOS

`./run-host.sh`

<img width="907" height="740" alt="Screenshot 2026-04-29 at 2 35 57 PM" src="https://github.com/user-attachments/assets/f9d59eff-4be5-49b9-a76b-b087eb14fd9f" />

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
