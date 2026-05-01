backspace doesnt work after using arrows for terminal history

there should be a fdirsize syscall for fls -> just does same as ls but just a counter

argv should be the same as unix -> I think argv[0] is path, 1 is exec name, 2 is argument 0, etc

fat32 lfn parsing still broken (i.e. eventsss.elf -> eventssself.) and lfn chaining not implemented at al

test malloc implementation and implement a real break
    -> then add a user malloc somehow? should probably be more than mapping a kmalloc to ring3, that could be unsafe?

FIXED: Running exec twice causes memory corruption
    Fix was ret() callback was nesting stack too much. wasn't needed anyways