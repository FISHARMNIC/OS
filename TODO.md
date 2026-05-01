userspace should not directly expose kernel includes since most of those will cause faults

give framebuffer user access and syscall to get mapped fb ptr

backspace doesnt work after usng arrows for terminal history

there should be a fdirsize syscall for fls

argv should be the same as unix -> i think argv[0] is path, 1 is exec name, 2 is argument 0, etc

test malloc implementation and implement a real break
    -> then add a user malloc somehow? should probably be more than mapping a kmalloc to ring3, that could be unsafe?

FIXED: Running exec twice causes memory corruption
    Fix was ret() callback was nesting stack too much. wasn't needed anyways