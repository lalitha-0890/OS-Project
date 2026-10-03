#include "common.h"
#include <stdarg.h>

void print_header(const char *title)
{
    printf("\n");
    printf("===========================================\n");
    printf("  %s\n", title);
    printf("===========================================\n");
}

void print_subheader(const char *title)
{
    printf("\n--- %s ---\n", title);
}

void print_step(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("    ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void print_arrow(void)
{
    printf("            |\n");
    printf("            v\n");
}

void pause_output(void)
{
    printf("\n[Press Enter to continue...]");
    fflush(stdout);
    /* Only wait if stdin is an interactive terminal */
    if (isatty(STDIN_FILENO)) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
}
