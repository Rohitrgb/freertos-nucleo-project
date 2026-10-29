#include "unity.h"
#include "debounce.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

typedef struct {
    bool     raw;
    uint32_t t_ms;
} sample_t;

static debounce_t btn;
static int pressed_count;
static int released_count;

void setUp(void)
{
    pressed_count  = 0;
    released_count = 0;
    TEST_ASSERT_TRUE(debounce_init(&btn, 20U, false));   /* 20 ms, starts released */
}

void tearDown(void) {}

/* Replay samples and count the events that come out. */
static void feed(const sample_t *s, size_t n)
{
    for (size_t i = 0U; i < n; i++) {
        debounce_event_t e = debounce_update(&btn, s[i].raw, s[i].t_ms);
        if (e == DEBOUNCE_PRESSED)  { pressed_count++; }
        if (e == DEBOUNCE_RELEASED) { released_count++; }
    }
}

/* A clean press, recognised at t = 120 ms. */
static void press_cleanly(void)
{
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true, 100U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_PRESSED, debounce_update(&btn, true, 120U));
}

void test_starts_released_with_no_events(void)
{
    TEST_ASSERT_FALSE(debounce_is_pressed(&btn));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE, debounce_update(&btn, false, 0U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE, debounce_update(&btn, false, 1000U));
}

void test_clean_press_reported_after_debounce_time(void)
{
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true, 100U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true, 119U));  /* 19 ms: too soon */
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_PRESSED, debounce_update(&btn, true, 120U));  /* 20 ms: accepted */
    TEST_ASSERT_TRUE(debounce_is_pressed(&btn));
}

void test_press_is_reported_only_once(void)
{
    press_cleanly();
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE, debounce_update(&btn, true, 121U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE, debounce_update(&btn, true, 500U));
}

void test_bouncy_press_gives_exactly_one_event(void)
{
    static const sample_t bounce[] = {
        {true, 100U}, {false, 103U}, {true, 105U}, {false, 108U}, {true, 110U},
        {true, 115U}, {true, 120U},  {true, 125U}, {true, 130U},  {true, 135U},
    };
    feed(bounce, LEN(bounce));
    TEST_ASSERT_EQUAL_INT(1, pressed_count);
    TEST_ASSERT_EQUAL_INT(0, released_count);
    TEST_ASSERT_TRUE(debounce_is_pressed(&btn));
}

void test_short_glitch_is_ignored(void)
{
    static const sample_t glitch[] = {
        {true, 100U}, {false, 110U}, {false, 150U}, {false, 200U},
    };
    feed(glitch, LEN(glitch));
    TEST_ASSERT_EQUAL_INT(0, pressed_count);
    TEST_ASSERT_EQUAL_INT(0, released_count);
    TEST_ASSERT_FALSE(debounce_is_pressed(&btn));
}

void test_change_of_mind_restarts_the_clock(void)
{
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true,  100U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, false, 110U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true,  115U));  /* clock restarts here */
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true,  134U));  /* 19 ms since 115 */
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_PRESSED, debounce_update(&btn, true,  135U));  /* 20 ms since 115 */
}

void test_clean_release_reported_after_debounce_time(void)
{
    press_cleanly();
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,     debounce_update(&btn, false, 300U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,     debounce_update(&btn, false, 319U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_RELEASED, debounce_update(&btn, false, 320U));
    TEST_ASSERT_FALSE(debounce_is_pressed(&btn));
}

void test_bouncy_release_gives_exactly_one_event(void)
{
    static const sample_t bounce[] = {
        {false, 300U}, {true, 302U}, {false, 305U}, {true, 306U}, {false, 308U},
        {false, 320U}, {false, 328U}, {false, 340U},
    };
    press_cleanly();
    feed(bounce, LEN(bounce));
    TEST_ASSERT_EQUAL_INT(0, pressed_count);
    TEST_ASSERT_EQUAL_INT(1, released_count);
    TEST_ASSERT_FALSE(debounce_is_pressed(&btn));
}

void test_works_when_the_millisecond_counter_wraps(void)
{
    /* 0xFFFFFFF0 + 20 ms wraps past zero and lands on 4 */
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true, 0xFFFFFFF0U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,    debounce_update(&btn, true, 0x00000003U));  /* 19 ms */
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_PRESSED, debounce_update(&btn, true, 0x00000004U));  /* 20 ms */
}

void test_zero_debounce_time_reacts_immediately(void)
{
    debounce_t fast;
    TEST_ASSERT_TRUE(debounce_init(&fast, 0U, false));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_PRESSED,  debounce_update(&fast, true,  50U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_RELEASED, debounce_update(&fast, false, 60U));
}

void test_can_start_in_the_pressed_state(void)
{
    debounce_t held;
    TEST_ASSERT_TRUE(debounce_init(&held, 20U, true));
    TEST_ASSERT_TRUE(debounce_is_pressed(&held));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,     debounce_update(&held, true,  0U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_NONE,     debounce_update(&held, false, 10U));
    TEST_ASSERT_EQUAL_INT(DEBOUNCE_RELEASED, debounce_update(&held, false, 30U));
}

void test_init_rejects_null(void)
{
    TEST_ASSERT_FALSE(debounce_init(NULL, 20U, false));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_released_with_no_events);
    RUN_TEST(test_clean_press_reported_after_debounce_time);
    RUN_TEST(test_press_is_reported_only_once);
    RUN_TEST(test_bouncy_press_gives_exactly_one_event);
    RUN_TEST(test_short_glitch_is_ignored);
    RUN_TEST(test_change_of_mind_restarts_the_clock);
    RUN_TEST(test_clean_release_reported_after_debounce_time);
    RUN_TEST(test_bouncy_release_gives_exactly_one_event);
    RUN_TEST(test_works_when_the_millisecond_counter_wraps);
    RUN_TEST(test_zero_debounce_time_reacts_immediately);
    RUN_TEST(test_can_start_in_the_pressed_state);
    RUN_TEST(test_init_rejects_null);
    return UNITY_END();
}
