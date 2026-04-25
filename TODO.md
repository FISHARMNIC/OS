FIXED: Running exec twice causes memory corruption
    Fix was ret() callback was nesting stack too much. wasn't needed anyways