/**
 * @file dsl.c
 * @brief Implements the command station DSL parser API.
 * @author Mikey Austin
 * @date 2012-2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>

#include "dsl.h"

#define T               DSL_result_T
#define DSL_MAX_TOK_LEN 20
#define DSL_TOK_FORWARD 128
#define DSL_TOK_REVERSE 129
#define DSL_TOK_STOP    130
#define DSL_TOK_ADDR    131
#define DSL_TOK_SPEED   132
#define DSL_TOK_ALL     133
#define DSL_TOK_NUMBER  134
#define DSL_TOK_SHOW    135
#define DSL_TOK_STATUS  136
#define DSL_TOK_HELP    137
#define DSL_TOK_RAW     138
#define DSL_TOK_HEX     139
#define DSL_TOK_CACHE   140
#define DSL_TOK_CLEAR   141

#define DSL_CMP(str, len, tok) ((len == strlen_P(PSTR(tok))) \
                                && !strncmp_P(str, PSTR(tok), len))

/**
 * Private module scanner.
 */
static struct
{
    /* Flush the input. */
    void (*flush)(void);

    /* Semantic value of the scanned token. */
    union {
        int  i;
        char s[DSL_MAX_TOK_LEN + 1];
    } value;
    
    /* Flag to indicate last char was end of input. */
    int seen_end;
} DSL_scanner;

/**
 * Private module parser state.
 */
static struct
{
    int curr;

    /**
     * This variable stores the parser result object.
     */
    T result;
} DSL_parser;

/**
 * Reset the scanner internal state.
 */
static void DSL_scanner_reset(void);

/**
 * Scan the input stream until a token is found.
 */
static int DSL_next_token(void);

/**
 * Start parsing the token stream.
 */
static int DSL_parse(void);

/**
 * Test current token against token argument and advance
 * on success.
 */
static int DSL_accept(int tok);

/**
 * Advance token stream by one.
 */
static void DSL_advance(void);

/**
 * Test the token argument against current token, but do 
 * not advance stream on success.
 */
static int DSL_accept_no_advance(int tok);

/**
 * Grammar specific functions to implement the recursive descent parsing.
 */
static int DSL_grammar_forward(void);
static int DSL_grammar_reverse(void);
static int DSL_grammar_stop(void);
static int DSL_grammar_addr(void);
static int DSL_grammar_speed(void);
static int DSL_grammar_show(void);
static int DSL_grammar_cache(void);
static int DSL_grammar_help(void);
static int DSL_grammar_raw(void);

extern void
DSL_module_init(void (*flush)(void))
{
    DSL_scanner.flush = flush;
}

extern int
DSL_parser_start(T *result)
{
    DSL_scanner_reset();

    if(result != NULL)
    {
        /* Create a new result object. */
        *result = (T) malloc(sizeof(**result));
        if(*result == NULL)
        {
            return DSL_PARSE_ERROR;
        }

        /* Set the parser result object. */
        DSL_parser.result = *result;
        DSL_parser.result->type = DSL_RES_TYPE_UNDEF;
    }

    /* Start the token stream for this parse. */
    DSL_advance();

    if(DSL_parse() == DSL_PARSE_ERROR)
    {
        /* Clear the input. */
        DSL_scanner.flush();

        /* Trash the result. */
        if(DSL_parser.result != NULL)
        {
            switch(DSL_parser.result->type)
            {
                case DSL_RES_TYPE_RAW:
                case DSL_RES_TYPE_DCC:
                    DCC_packet_destroy(DSL_parser.result->payload.packet);
                    break;

                case DSL_RES_TYPE_SYS:
                    Sys_cmd_destroy(DSL_parser.result->payload.cmd, NULL);
                    break;
            }

            free(DSL_parser.result);
            *result = DSL_parser.result = NULL;
        }

        return DSL_PARSE_ERROR;
    }

    /* Finish off the packet. */
    if(DSL_parser.result->type == DSL_RES_TYPE_DCC)
    {
        DCC_set_checksum(DSL_parser.result->payload.packet);
        DCC_set_packet_end(DSL_parser.result->payload.packet);
    }

    return DSL_PARSE_OK;
}

static void
DSL_scanner_reset(void)
{
    DSL_scanner.seen_end = 0;
}

static int
DSL_next_token(void)
{
    char *tok;
    int c, c2, tok_i = 0, num_i;

    if(DSL_scanner.seen_end)
    {
        return 0;
    }

    /* For convenience point tok to scanner value string buffer. */
    tok = DSL_scanner.value.s;

    /* Start scanning characters until we find a token. */
    for(;;)
    {
        c = getchar();

        /* Scan for hexadecimal token. */
        if(isdigit(c) && c == '0')
        {
            if(isalpha(c2 = getchar()) && c2 == 'x')
            {
                /* This is definately a hexadecimal string. */
                while((tok_i < DSL_MAX_TOK_LEN)
                    && (isalpha(c = getchar()) || isdigit(c)))
                {
                    tok[tok_i++] = (isdigit(c) ? c : toupper(c));
                }

                tok[tok_i] = '\0';

                /* Now check that this hex string has an even number of characters. */
                if(strlen(tok) > 0 && (strlen(tok) % 2) != 0)
                {
                    /* Zero or odd number of characters. */
                    return 0;
                }

                if(c < 0)
                {
                    DSL_scanner.seen_end = 1;
                }
                else
                {
                    ungetc(c, stdin);
                }

                return DSL_TOK_HEX;
            }
            else
            {
                ungetc(c2, stdin);
            }
        }

        /* Scan for numbers. */
        if(isdigit(c))
        {
            for(num_i = (c - '0'); isdigit(c = getchar()); )
            {
                num_i = (num_i * 10) + (c - '0');
            }

            /* Set the semantic value for this number token. */
            DSL_scanner.value.i = num_i;

            if(c < 0)
            {
                DSL_scanner.seen_end = 1;
            }
            else
            {
                ungetc(c, stdin);
            }

            return DSL_TOK_NUMBER;
        }

        /* Scan for keywords. */
        if(isalpha(c))
        {
            do
            {
                tok[tok_i++] = tolower(c);
            }
            while(isalpha(c = getchar()) && tok_i < DSL_MAX_TOK_LEN);

            if(c < 0)
            {
                DSL_scanner.seen_end = 1;
            }
            else
            {
                ungetc(c, stdin);
            }

            if(DSL_CMP(tok, tok_i, "raw"))
            {
                return DSL_TOK_RAW;
            }
            else if(DSL_CMP(tok, tok_i, "forward") || DSL_CMP(tok, tok_i, "fw"))
            {
                return DSL_TOK_FORWARD;
            }
            else if(DSL_CMP(tok, tok_i, "reverse") || DSL_CMP(tok, tok_i, "rv"))
            {
                return DSL_TOK_REVERSE;
            }
            else if(DSL_CMP(tok, tok_i, "stop"))
            {
                return DSL_TOK_STOP;
            }
            else if(DSL_CMP(tok, tok_i, "addr") || DSL_CMP(tok, tok_i, "ad"))
            {
                return DSL_TOK_ADDR;
            }
            else if(DSL_CMP(tok, tok_i, "speed") || DSL_CMP(tok, tok_i, "sp"))
            {
                return DSL_TOK_SPEED;
            }
            else if(DSL_CMP(tok, tok_i, "all"))
            {
                return DSL_TOK_ALL;
            }
            else if(DSL_CMP(tok, tok_i, "show"))
            {
                return DSL_TOK_SHOW;
            }
            else if(DSL_CMP(tok, tok_i, "cache"))
            {
                return DSL_TOK_CACHE;
            }
            else if(DSL_CMP(tok, tok_i, "clear"))
            {
                return DSL_TOK_CLEAR;
            }
            else if(DSL_CMP(tok, tok_i, "status"))
            {
                return DSL_TOK_STATUS;
            }
            else if(DSL_CMP(tok, tok_i, "help"))
            {
                return DSL_TOK_HELP;
            }
            else
            {
                /* Unknown token. */
                return 0;
            }
        }

        /* Process remaining characters. */
        switch(c)
        {
            case ' ':
            case '\t':
                /* Ignore whitespace. */
                break;

            default:
                /* Unknown character. */
                return 0;
        }
    }
}

static int
DSL_parse(void)
{
    if(!(DSL_grammar_raw()
        || DSL_grammar_help()
        || DSL_grammar_show()
        || DSL_grammar_cache()
        || DSL_grammar_forward()
        || DSL_grammar_reverse()
        || DSL_grammar_stop()))
    {
        return DSL_PARSE_ERROR;
    }

    return DSL_PARSE_OK;
}

static int
DSL_accept(int tok)
{
    if(DSL_accept_no_advance(tok))
    {
        DSL_parser.curr = DSL_next_token();
        return 1;
    }

    return 0;

}

static int
DSL_accept_no_advance(int tok)
{
    return (tok == DSL_parser.curr);
}

static void
DSL_advance(void)
{
    DSL_parser.curr = DSL_next_token();
}

static int
DSL_grammar_help(void)
{
    if(DSL_accept(DSL_TOK_HELP))
    {
        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_SYS;
            DSL_parser.result->payload.cmd = Sys_cmd_create(SYS_CMD_TYPE_HELP, NULL);
        }

        /* Help command has no arguments. */
        return DSL_PARSE_OK;
    }

    return DSL_PARSE_ERROR;
}


static int
DSL_grammar_show(void)
{
    uint8_t cmd_type;

    if(DSL_accept(DSL_TOK_SHOW))
    {
        switch(DSL_parser.curr)
        {
            case DSL_TOK_STATUS:
                cmd_type = SYS_CMD_TYPE_STATUS;
                break;

            default:
                return DSL_PARSE_ERROR;
        }

        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_SYS;
            DSL_parser.result->payload.cmd = Sys_cmd_create(cmd_type, NULL);
        }

        return DSL_PARSE_OK;
    }

    return DSL_PARSE_ERROR;
}

static int
DSL_grammar_cache(void)
{
    uint8_t cmd_type = 0;
    int *address;
    void *args = NULL;

    if(DSL_accept(DSL_TOK_CACHE))
    {
        switch(DSL_parser.curr)
        {
            case DSL_TOK_CLEAR:
                cmd_type = SYS_CMD_TYPE_CACHE_CLEAR;
                break;

            case DSL_TOK_SHOW:
                DSL_advance();
                if(DSL_accept_no_advance(DSL_TOK_NUMBER))
                {
                    cmd_type = SYS_CMD_TYPE_CACHE_SHOW;
                    address = (int*) malloc(sizeof(int));
                    *address = DSL_scanner.value.i;
                    args = (void*) address;
                }
                else
                {
                    /* We expected an address. */
                    return DSL_PARSE_ERROR;
                }
                break;

            default:
                return DSL_PARSE_ERROR;
        }

        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_SYS;
            DSL_parser.result->payload.cmd = Sys_cmd_create(cmd_type, args);
        }

        return DSL_PARSE_OK;
    }

    return DSL_PARSE_ERROR;
}

static int
DSL_grammar_forward(void)
{
    if(DSL_accept(DSL_TOK_FORWARD))
    {
        /* Setup DCC packet payload before checking the rest of the syntax. */
        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_DCC;
            DSL_parser.result->payload.packet = DCC_baseline_packet_create();
        }

        if((DSL_grammar_addr() && DSL_grammar_speed())
            || (DSL_grammar_speed() && DSL_grammar_addr()))
        {
            /* Semantic action. */
            if(DSL_parser.result)
            {
                DCC_set_preamble(DSL_parser.result->payload.packet);
                DCC_set_speed_direction_preamble(DSL_parser.result->payload.packet);
                DCC_set_direction(DSL_parser.result->payload.packet, DCC_DIRECTION_FORWARD);
            }

            /* Address and speed are compulsory. */
            return DSL_PARSE_OK;
        }
    }

    return DSL_PARSE_ERROR;
}

static int
DSL_grammar_reverse(void)
{
    if(DSL_accept(DSL_TOK_REVERSE))
    {
        /* Setup DCC packet payload. */
        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_DCC;
            DSL_parser.result->payload.packet = DCC_baseline_packet_create();
        }

        if((DSL_grammar_addr() && DSL_grammar_speed())
            || (DSL_grammar_speed() && DSL_grammar_addr()))
        {
            /* Semantic action. */
            if(DSL_parser.result)
            {
                DCC_set_preamble(DSL_parser.result->payload.packet);
                DCC_set_speed_direction_preamble(DSL_parser.result->payload.packet);
                DCC_set_direction(DSL_parser.result->payload.packet, DCC_DIRECTION_REVERSE);
            }

            /* Address and speed are compulsory. */
            return DSL_PARSE_OK;
        }
    }

    return DSL_PARSE_ERROR;
}

static int
DSL_grammar_stop(void)
{
    if(DSL_accept(DSL_TOK_STOP))
    {
        /* Setup DCC packet payload. */
        if(DSL_parser.result)
        {
            DSL_parser.result->type = DSL_RES_TYPE_DCC;
            DSL_parser.result->payload.packet = DCC_baseline_packet_create();
            DCC_set_preamble(DSL_parser.result->payload.packet);
        }

        /* Address is optional. */
        if(DSL_grammar_addr())
        {
            /* Semantic action for specific loco. */
            if(DSL_parser.result && DSL_parser.result->payload.packet)
            {
                DCC_set_speed_direction_preamble(DSL_parser.result->payload.packet);
                DCC_set_speed(DSL_parser.result->payload.packet, (unsigned char) 0);
            }

            return DSL_PARSE_OK;
        }
        else if(DSL_accept(DSL_TOK_ALL))
        {
            /* Semantic action for emergency broadcast stop. */
            if(DSL_parser.result && DSL_parser.result->payload.packet)
            {
                DCC_special_emergency_stop_packet(DSL_parser.result->payload.packet);
            }

            return DSL_PARSE_OK;
        }
        else
        {
            /* Semantic action for broadcast stop while keeping power to motors. */
            if(DSL_parser.result && DSL_parser.result->payload.packet)
            {
                DCC_special_broadcast_stop_packet(DSL_parser.result->payload.packet);
            }

            return DSL_PARSE_OK;
        }
    }

    return DSL_PARSE_ERROR;
}

static int
DSL_grammar_addr(void)
{
    if(!DSL_accept(DSL_TOK_ADDR))
    {
        return DSL_PARSE_ERROR;
    }

    if(DSL_accept_no_advance(DSL_TOK_NUMBER))
    {
        /* Semantic action. */
        if(DSL_parser.result && DSL_parser.result->payload.packet)
        {
            DCC_set_address(DSL_parser.result->payload.packet,
                (unsigned char) DSL_scanner.value.i);
        }

        DSL_advance();

        return DSL_PARSE_OK;
    }
    else
    {
        return DSL_PARSE_ERROR;
    }
}

static int
DSL_grammar_speed(void)
{
    if(!DSL_accept(DSL_TOK_SPEED))
    {
        return DSL_PARSE_ERROR;
    }

    if(DSL_accept_no_advance(DSL_TOK_NUMBER))
    {
        /* Semantic action. */
        if(DSL_parser.result && DSL_parser.result->payload.packet)
        {
            DCC_set_speed(DSL_parser.result->payload.packet,
                (unsigned char) DSL_scanner.value.i);
        }

        DSL_advance();

        return DSL_PARSE_OK;
    }
    else
    {
        return DSL_PARSE_ERROR;
    }
}

static int
DSL_grammar_raw(void)
{
    int i;

    if(DSL_accept(DSL_TOK_RAW) && DSL_accept_no_advance(DSL_TOK_HEX))
    {
        /* Semantic action. */
        if(DSL_parser.result && DSL_parser.result->payload.packet)
        {
            /* Setup result. */
            DSL_parser.result->type = DSL_RES_TYPE_RAW;
            DSL_parser.result->payload.packet = DCC_packet_create(strlen(DSL_scanner.value.s) / 2);

            /* Convert hexadecimal string into array of bytes and populate packet. */
            for(i=0; i < (strlen(DSL_scanner.value.s) / 2); i++)
            {
                sscanf(DSL_scanner.value.s + (i * 2), "%2hhx",
                    &DSL_parser.result->payload.packet->bytes[i]);
            }
        }

        DSL_advance();

        return DSL_PARSE_OK;
    }

    return DSL_PARSE_ERROR;
}
