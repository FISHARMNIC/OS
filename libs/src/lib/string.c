#include <sys/string.h>
#include <cpu.h>

uint32_t strlen(const char *str)
{
    uint32_t len = 0;
    while (str[len])
    {
        len++;
    }
    return len;
}

uint32_t strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
        {
            return *s1 - *s2;
        }
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

void *memcpy(void *dest, const void *src, uint32_t size)
{

    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (uint32_t i = 0; i < size; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

void *memset(void *dest, const uint8_t src, uint32_t size)
{
    uint8_t *d = (uint8_t *)dest;

    for (uint32_t i = 0; i < size; i++)
    {
        d[i] = src;
    }

    return dest;
}

char *strchr(const char *s, char c)
{
    do
    {
        if (*s == c)
        {
            return (char *)s;
        }
    } while (*(s++));
    return NULLPTR;
}


char *strtok_r(char *str, const char *delim, char **saveptr)
{
    if (str == NULLPTR)
        str = *saveptr;

    if (str == NULLPTR)
        return NULLPTR;

    // skip leading delimiters
    while (*str)
    {
        int is_delim = 0;

        for (const char *d = delim; *d; d++)
        {
            if (*str == *d)
            {
                is_delim = 1;
                break;
            }
        }

        if (!is_delim)
            break;

        str++;
    }

    if (*str == '\0')
    {
        *saveptr = NULLPTR;
        return NULLPTR;
    }

    char *token = str;

    while (*str)
    {
        for (const char *d = delim; *d; d++)
        {
            if (*str == *d)
            {
                *str = '\0';
                *saveptr = str + 1;
                return token;
            }
        }
        str++;
    }

    *saveptr = NULLPTR;
    return token;
}

void str_toupper(char *s)
{
    if (!s) return;

    while (*s)
    {
        if (*s >= 'a' && *s <= 'z')
            *s -= ('a' - 'A');
        s++;
    }
}

char* strcat(char* dest, char* extra)
{
    while(*(dest++) != 0);
    dest--;

    while(*extra != 0)
    {
        *(dest++) = *(extra++);
    }

    return dest;
}