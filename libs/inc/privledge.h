#ifndef PRIVLEDGE_H
#define PRIVLEDGE_H

#include <stdint.h>

typedef void (*user_entry_fn_t)(void);


/**
 * @brief Switches from ring 0 to ring 3 and starts executing at entry
 *
 * @param entry Ring-3 entry point
 * @param user_stack_top Top of user-mode stack (must be mapped/writable)
 */
void privledge_enter_user_mode(user_entry_fn_t entry, uint32_t user_stack_top);

#endif
