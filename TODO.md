P1:
* test malloc implementation and implement a real break
    -> then add a user malloc somehow? should probably be more than mapping a kmalloc to ring3, that could be unsafe?
    -> implement user malloc, currently uses kmalloc. same code but no faking the break, just change lib to take break address in mm_init and stop faking the break for uland
* when hitting enter in bin/test/event.elf there is double newline with 2x prompt for some reason
    -> holding enter exits (1st newline)
    -> releasing makes second

P2:
* backspace doesnt work after using arrows for terminal history
* argv should be the same as unix -> I think argv[0] is path, 1 is exec name, 2 is argument 0, etc
* fat32 lfn parsing still broken (i.e. eventsss.elf -> eventssself.) and lfn chaining not implemented at al
* fs writing