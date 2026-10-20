#include "unity.h"
#include "ringbuf.h"

#define CAP 8U

static ringbuf_t rb;
static uint8_t storage[CAP];

void setUp(void)    { TEST_ASSERT_TRUE(ringbuf_init(&rb, storage, CAP)); }
void tearDown(void) {}

void test_starts_empty(void)
{
    TEST_ASSERT_TRUE(ringbuf_is_empty(&rb));
    TEST_ASSERT_FALSE(ringbuf_is_full(&rb));
    TEST_ASSERT_EQUAL_UINT32(0U, ringbuf_count(&rb));
}

void test_put_then_get_returns_same_byte(void)
{
    uint8_t out = 0U;
    TEST_ASSERT_TRUE(ringbuf_put(&rb, 0x42U));
    TEST_ASSERT_TRUE(ringbuf_get(&rb, &out));
    TEST_ASSERT_EQUAL_HEX8(0x42U, out);
    TEST_ASSERT_TRUE(ringbuf_is_empty(&rb));
}

void test_get_from_empty_fails_and_leaves_output_alone(void)
{
    uint8_t out = 0xAAU;
    TEST_ASSERT_FALSE(ringbuf_get(&rb, &out));
    TEST_ASSERT_EQUAL_HEX8(0xAAU, out);
}

void test_bytes_come_out_in_fifo_order(void)
{
    uint8_t out = 0U;
    for (uint8_t i = 0U; i < 5U; i++) { TEST_ASSERT_TRUE(ringbuf_put(&rb, i)); }
    for (uint8_t i = 0U; i < 5U; i++) {
        TEST_ASSERT_TRUE(ringbuf_get(&rb, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
}

void test_fills_to_capacity_then_rejects(void)
{
    for (uint8_t i = 0U; i < CAP; i++) { TEST_ASSERT_TRUE(ringbuf_put(&rb, i)); }
    TEST_ASSERT_TRUE(ringbuf_is_full(&rb));
    TEST_ASSERT_FALSE(ringbuf_put(&rb, 0xFFU));
    TEST_ASSERT_EQUAL_UINT32(CAP, ringbuf_count(&rb));
}

void test_wraps_around_the_storage_array(void)
{
    uint8_t out = 0U;
    for (uint8_t i = 0U; i < 20U; i++) {       /* 20 bytes through 8 slots */
        TEST_ASSERT_TRUE(ringbuf_put(&rb, i));
        TEST_ASSERT_TRUE(ringbuf_get(&rb, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
}

void test_counters_survive_uint32_overflow(void)
{
    uint8_t out = 0U;
    rb.head = 0xFFFFFFFEU;                     /* deliberate white-box setup */
    rb.tail = 0xFFFFFFFEU;
    for (uint8_t i = 0U; i < 6U; i++) { TEST_ASSERT_TRUE(ringbuf_put(&rb, i)); }
    TEST_ASSERT_EQUAL_UINT32(6U, ringbuf_count(&rb));
    for (uint8_t i = 0U; i < 6U; i++) {
        TEST_ASSERT_TRUE(ringbuf_get(&rb, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
    TEST_ASSERT_TRUE(ringbuf_is_empty(&rb));
}

void test_init_rejects_bad_capacity(void)
{
    ringbuf_t bad;
    uint8_t buf[6];
    TEST_ASSERT_FALSE(ringbuf_init(&bad, buf, 6U));
    TEST_ASSERT_FALSE(ringbuf_init(&bad, buf, 0U));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_starts_empty);
    RUN_TEST(test_put_then_get_returns_same_byte);
    RUN_TEST(test_get_from_empty_fails_and_leaves_output_alone);
    RUN_TEST(test_bytes_come_out_in_fifo_order);
    RUN_TEST(test_fills_to_capacity_then_rejects);
    RUN_TEST(test_wraps_around_the_storage_array);
    RUN_TEST(test_counters_survive_uint32_overflow);
    RUN_TEST(test_init_rejects_bad_capacity);
    return UNITY_END();
}
