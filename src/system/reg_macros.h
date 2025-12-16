#pragma once

// Macro to declare a memory-mapped register with mirror
#define DECLARE_MEMREG(name) \
    uint8_t name##_read(void); \
    void name##_write(uint8_t val); \
    uint8_t name##_set_bit(uint8_t bit, bool val); \
    uint8_t name##_toggle_bit(uint8_t bit); \
    bool name##_get_bit(uint8_t bit);

// Macro to define a memory-mapped register with mirror
#define DEFINE_MEMREG(name, addr) \
    static uint8_t mirror_##name; \
    uint8_t name##_read(void) { \
        return mirror_##name; \
    } \
    void name##_write(uint8_t val) { \
        /* write addr first. this is important for the memory bank register */ \
        /* because the mirror will be affected */ \
        *(volatile uint8_t*) (addr) = val; \
        /* treat just the write as volatile since I'm okay with read-side */ \
        /* optimizations */ \
        *(volatile uint8_t*) &mirror_##name = val; \
    } \
    uint8_t name##_set_bit(uint8_t bit, bool val) { \
        uint8_t res = bits_update(mirror_##name, bit, val); \
        name##_write(res); \
        return res; \
    } \
    uint8_t name##_toggle_bit(uint8_t bit) { \
        uint8_t res = bits_toggle(mirror_##name, bit); \
        name##_write(res); \
        return res; \
    } \
    bool name##_get_bit(uint8_t bit) { \
        return bits_get(mirror_##name, bit); \
    }

// Macro to declare a read-only register
#define DECLARE_READREG(name) \
    uint8_t name##_read(void);

// Macro to define a read-only register
#define DEFINE_READREG(name, addr) \
    uint8_t name##_read(void) { \
        volatile uint8_t *ptr = (volatile uint8_t*) (addr); \
        return *ptr; \
    }

// Macro to declare a write-only register
#define DECLARE_WRITEREG(name) \
    void name##_write(uint8_t val);

// Macro to define a write-only register
#define DEFINE_WRITEREG(name, addr) \
    void name##_write(uint8_t val) { \
        *(volatile uint8_t*) (addr) = val; \
    }
