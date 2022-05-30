// Copyright (c) 2022 Michael Breen (https://mbreen.com)
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

static const char* program_name;

// Usage message split on the program name.
static const char* const usage[] = {
        "Usage: ", " A B\n\n"
        "Read numbers in base A from stdin and output in base B.\n"
        "The alphabet used is the environment variable DIGITS.\n"
        "DIGITS must be printable ASCII and contain at least as\n"
        "many characters as A or B, whichever is greater.\n"
        "If DIGITS is not set, the default alphabet is\n"
        "0123456789abcdefghijklmnopqrstuvwxyz\n\n"
        "Examples:\n"
        "  # convert hexadecimal to decimal:\n"
        "  echo deadbeef | ", " 16 10\n"
        "  # convert a date to vigesimal, custom alphabet:\n"
        "  export DIGITS=0123456789cjzwfsbvxq\n"
        "  echo 2021-12-25 | ", " 10 20\n\n"
        "Author: Michael Breen (https://mbreen.com)"
        // VERSION
        };

// Output the program name followed by msg and '\n' to stderr.
static void error_msg(const char *msg) {
    fputs(program_name, stderr);
    fputs(": ", stderr);
    fputs(msg, stderr);
    fputs("\n", stderr);
}

// Exit with Usage, optionally preceded by an error message.
static void exit_usage(const char* msg) {
    if (msg)
        error_msg(msg);
    size_t i = 0;
    for (; i < sizeof(usage)/sizeof(usage[0]) - 1; ++i) {
        fputs(usage[i], stderr);
        fputs(program_name, stderr);
    }
    fputs(usage[i], stderr);
    exit(1);
}

static const char* get_alphabet(void) {
    const char *digchars = getenv("DIGITS");
    if (!digchars || !digchars[0])
        return "0123456789abcdefghijklmnopqrstuvwxyz";
    return digchars;
}

// Report an error in the alphabet and exit.
static void err_digits(const char* msg) {
    error_msg(msg);
    fputs("DIGITS: ", stderr);
    fputs(get_alphabet(), stderr);
    fputs("\n\n", stderr);
    exit_usage(NULL);
}

static void overflow(void) {
    error_msg("overflow");
    exit(2);
}

#define isalpha(x) (((c)|0x20) >= 'a' && ((c)|0x20) <= 'z')
#define isdecimal(x) ((unsigned) (x) - '0' < 10)

// Numeric value of digit or radix. For a digit, -1 means "none".
typedef signed char digval;

// Convert an argument of 1 or 2 decimal digits to a number.
static digval arg2num(char *arg) {
    if (!isdecimal(arg[0]))
        exit_usage("arguments must be 1 or 2 decimal digits");
    int val = arg[0] - '0';
    if (isdecimal(arg[1]) && !arg[2])
        val = val * 10 + arg[1] - '0';
    else if (arg[1])
        exit_usage("arguments must be 1 or 2 decimal digits");
    return (digval) val;
}

// Report printable/DEL/non-ASCII character as invalid and exit.
static void invalid_input_digit(int c) {
    fputs(program_name, stderr);
    fputs(": ", stderr);
    fputs("input character ",stderr);
    if (c < 0x7f) {
        fputs("'", stderr);
        fputc(c, stderr);
        fputs("' not in alphabet\n", stderr);
    } else {
        fputs("outside ASCII range\n", stderr);
    }
    exit(1);
}

#define MAX_LEN 4096
typedef struct {
    digval radix;
    int len;
    digval digits[MAX_LEN]; // least significant first
} numbr;

// Make *num = *num * factor + addend.
static void mult_add(numbr *num, digval factor, digval addend) {
    int j, carry = addend;
    for (j = 0; carry || (factor && j < num->len); ++j) {
        if (j < num->len)
            carry += num->digits[j] * factor;
        else if (j >= MAX_LEN)
            overflow();
        num->digits[j] = (digval) (carry % num->radix);
        carry /= num->radix;
    }
    num->len = j;
}
    
static numbr out;

int main(int argc, char **argv) {
    program_name = argv[0];
    for (const char *ip = program_name; *ip; ++ip)
        if (*ip == '/')
            program_name = ip + 1;
    if (argc != 3)
        exit_usage(NULL);
    digval base_in = arg2num(argv[1]);
    out.len = 0;
    out.radix = arg2num(argv[2]);
    if (base_in < 2 || out.radix < 2)
        exit_usage("minimum base 2");
    const char *digit_chars = get_alphabet();
    digval digit_vals[0x80];
    for (int j = 0; j < 0x80; ++j)
        digit_vals[j] = -1;
    for (digval v = 0; (size_t) v < strlen(digit_chars); ++v) {
        int i = digit_chars[v];
        if (i < ' ' || i > '~')
            err_digits("$DIGITS must be printable ASCII");
        if (digit_vals[i] != -1)
            err_digits("digit repeated in $DIGITS");
        digit_vals[i] = v;
    }
    if ((int) strlen(digit_chars) < (
            base_in > out.radix ? base_in : out.radix))
        err_digits("$DIGITS too short");
    int digit_read = 0;
    for (;;) {
        int c = getchar();
        digval val = c & 0x80 ? -1 : digit_vals[c];
        if (val >= 0 && val < base_in) {
            // Accumulate number.
            mult_add(&out, base_in, val);
            digit_read = 1;
        } else {
            if (isalpha(c) || isdecimal(c) || c >= 0x7f)
                invalid_input_digit(c);
            // Output accumulated number, if any.
            if (digit_read) {
                if (!out.len)
                    out.digits[out.len++] = 0;
                for (int j = out.len - 1; j >= 0; --j)
                    putchar(digit_chars[out.digits[j]]);
                digit_read = out.len = 0;
            }
            if (c == EOF)
                return 0;
            putchar(c);
        }
    }
}
