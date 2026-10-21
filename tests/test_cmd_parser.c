#include "unity.h"
#include "cmd_parser.h"
#include "ringbuf.h"
#include <stdio.h>

static int  led_calls;
static int  status_calls;
static char led_arg[CMD_LINE_MAX];

static void handle_led(const char *arg)
{
    led_calls++;
    (void)snprintf(led_arg, sizeof led_arg, "%s", arg);
}

static void handle_status(const char *arg)
{
    (void)arg;
    status_calls++;
}

static const cmd_entry_t table[] = {
    {"led",    handle_led},
    {"status", handle_status},
};

static cmd_parser_t parser;

void setUp(void)
{
    led_calls = 0;
    status_calls = 0;
    led_arg[0] = '\0';
    TEST_ASSERT_TRUE(cmd_parser_init(&parser, table, sizeof table / sizeof table[0]));
}

void tearDown(void) {}

/* Helpers: feed bytes, return the last meaningful result. */
static cmd_result_t feed_str(const char *s)
{
    cmd_result_t last = CMD_NONE;
    for (; *s != '\0'; s++) {
        cmd_result_t r = cmd_parser_feed(&parser, (uint8_t)*s);
        if (r != CMD_NONE) { last = r; }
    }
    return last;
}

static cmd_result_t feed_repeat(char c, size_t count)
{
    cmd_result_t last = CMD_NONE;
    for (size_t i = 0U; i < count; i++) {
        cmd_result_t r = cmd_parser_feed(&parser, (uint8_t)c);
        if (r != CMD_NONE) { last = r; }
    }
    return last;
}

void test_runs_command_with_argument(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("led on\n"));
    TEST_ASSERT_EQUAL_INT(1, led_calls);
    TEST_ASSERT_EQUAL_STRING("on", led_arg);
}

void test_runs_command_without_argument(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("status\n"));
    TEST_ASSERT_EQUAL_INT(1, status_calls);
}

void test_nothing_runs_until_line_ends(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_NONE, feed_str("led on"));
    TEST_ASSERT_EQUAL_INT(0, led_calls);
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("\n"));
    TEST_ASSERT_EQUAL_INT(1, led_calls);
}

void test_crlf_runs_command_once(void)
{
    (void)feed_str("status\r\n");
    TEST_ASSERT_EQUAL_INT(1, status_calls);
}

void test_unknown_command_is_reported(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_ERR_UNKNOWN, feed_str("bogus\n"));
    TEST_ASSERT_EQUAL_INT(0, led_calls);
    TEST_ASSERT_EQUAL_INT(0, status_calls);
}

void test_blank_lines_are_ignored(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_NONE, feed_str("\n\r\n   \n"));
    TEST_ASSERT_EQUAL_INT(0, led_calls);
    TEST_ASSERT_EQUAL_INT(0, status_calls);
}

void test_extra_spaces_are_trimmed(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("   led    on   \n"));
    TEST_ASSERT_EQUAL_STRING("on", led_arg);
}

void test_argument_keeps_inner_spaces(void)
{
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("led on now\n"));
    TEST_ASSERT_EQUAL_STRING("on now", led_arg);
}

void test_line_at_limit_accepted_one_over_rejected(void)
{
    (void)feed_repeat('a', CMD_LINE_MAX - 1U);
    TEST_ASSERT_EQUAL_INT(CMD_ERR_UNKNOWN, feed_str("\n"));   /* fits, just not a command */
    (void)feed_repeat('a', CMD_LINE_MAX);
    TEST_ASSERT_EQUAL_INT(CMD_ERR_TOO_LONG, feed_str("\n"));
}

void test_overlong_line_rejected_then_parser_recovers(void)
{
    (void)feed_repeat('a', 500U);
    TEST_ASSERT_EQUAL_INT(CMD_ERR_TOO_LONG, feed_str("\n"));
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("status\n"));
    TEST_ASSERT_EQUAL_INT(1, status_calls);
}

void test_non_printable_bytes_reject_the_line(void)
{
    (void)feed_str("le");
    (void)cmd_parser_feed(&parser, 0x01U);
    (void)feed_str("d on");
    TEST_ASSERT_EQUAL_INT(CMD_ERR_BAD_CHAR, feed_str("\n"));
    TEST_ASSERT_EQUAL_INT(0, led_calls);

    (void)cmd_parser_feed(&parser, 0xFFU);
    TEST_ASSERT_EQUAL_INT(CMD_ERR_BAD_CHAR, feed_str("\n"));
}

void test_random_garbage_never_breaks_the_parser(void)
{
    uint32_t seed = 12345U;
    for (uint32_t i = 0U; i < 20000U; i++) {
        seed = (seed * 1664525U) + 1013904223U;
        (void)cmd_parser_feed(&parser, (uint8_t)(seed >> 24));
    }
    (void)feed_str("\n");               /* end whatever line was in progress */
    led_calls = 0;
    status_calls = 0;
    TEST_ASSERT_EQUAL_INT(CMD_OK, feed_str("status\n"));
    TEST_ASSERT_EQUAL_INT(1, status_calls);
}

void test_works_with_ring_buffer_in_between(void)
{
    ringbuf_t rx;
    uint8_t   storage[16];
    uint8_t   b = 0U;
    const char *msg = "led on\n";

    TEST_ASSERT_TRUE(ringbuf_init(&rx, storage, 16U));
    for (const char *s = msg; *s != '\0'; s++) {      /* the "interrupt" side */
        TEST_ASSERT_TRUE(ringbuf_put(&rx, (uint8_t)*s));
    }
    while (ringbuf_get(&rx, &b)) {                    /* the "main loop" side */
        (void)cmd_parser_feed(&parser, b);
    }
    TEST_ASSERT_EQUAL_INT(1, led_calls);
    TEST_ASSERT_EQUAL_STRING("on", led_arg);
}

void test_init_rejects_bad_arguments(void)
{
    cmd_parser_t p;
    TEST_ASSERT_FALSE(cmd_parser_init(NULL, table, 2U));
    TEST_ASSERT_FALSE(cmd_parser_init(&p, NULL, 2U));
    TEST_ASSERT_FALSE(cmd_parser_init(&p, table, 0U));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_runs_command_with_argument);
    RUN_TEST(test_runs_command_without_argument);
    RUN_TEST(test_nothing_runs_until_line_ends);
    RUN_TEST(test_crlf_runs_command_once);
    RUN_TEST(test_unknown_command_is_reported);
    RUN_TEST(test_blank_lines_are_ignored);
    RUN_TEST(test_extra_spaces_are_trimmed);
    RUN_TEST(test_argument_keeps_inner_spaces);
    RUN_TEST(test_line_at_limit_accepted_one_over_rejected);
    RUN_TEST(test_overlong_line_rejected_then_parser_recovers);
    RUN_TEST(test_non_printable_bytes_reject_the_line);
    RUN_TEST(test_random_garbage_never_breaks_the_parser);
    RUN_TEST(test_works_with_ring_buffer_in_between);
    RUN_TEST(test_init_rejects_bad_arguments);
    return UNITY_END();
}
