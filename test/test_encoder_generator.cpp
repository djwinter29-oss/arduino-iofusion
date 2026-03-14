#include <unity.h>

#include "encoder_generator.h"
#include "test_support.h"

namespace {

struct EncoderGeneratorMirror {
  uint8_t pinA;
  uint8_t pinB;
  uint8_t state;
  volatile uint8_t* portAOut;
  volatile uint8_t* portBOut;
  uint8_t maskA;
  uint8_t maskB;
  uint8_t pinUp;
  uint8_t pinDown;
  volatile uint8_t* upPortIn;
  volatile uint8_t* downPortIn;
  uint8_t upMask;
  uint8_t downMask;
  bool activeHigh;
  volatile int32_t position;
  volatile bool directionUp;
};

}  // namespace

void test_encoder_generator_branches() {
  EncoderGenerator enc;
  TEST_ASSERT_FALSE(enc.begin(9, 9, 2, 3));
  TEST_ASSERT_TRUE(enc.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  enc.onTick();
  enc.onTick();

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  enc.onTick();

  setDigitalPin(2, true);
  setDigitalPin(3, true);
  enc.onTick();

  TEST_ASSERT_EQUAL_INT32(1, enc.getPosition());
  TEST_ASSERT_FALSE(enc.getDirection());

  enc.reset();
  TEST_ASSERT_EQUAL_INT32(0, enc.getPosition());
  TEST_ASSERT_TRUE(enc.getDirection());

  EncoderGenerator activeLowEnc;
  TEST_ASSERT_TRUE(activeLowEnc.begin(EncoderGenerator::Config{9, 10, 2, 3, true, false}));

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  activeLowEnc.onTick();
  activeLowEnc.onTick();

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  activeLowEnc.onTick();

  TEST_ASSERT_EQUAL_INT32(1, activeLowEnc.getPosition());
  TEST_ASSERT_FALSE(activeLowEnc.getDirection());
}

void test_encoder_generator_config_edges() {
  EncoderGenerator encoder;
  EncoderGenerator separatedPorts;

  encoder.onTick();
  TEST_ASSERT_EQUAL_INT32(0, encoder.getPosition());
  TEST_ASSERT_TRUE(encoder.getDirection());

  mockNullOutputPort = digitalPinToPort(9);
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockNullOutputPort = -1;
  mockNullOutputPort = digitalPinToPort(10);
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockNullOutputPort = -1;
  mockZeroMaskPin = 9;
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockZeroMaskPin = -1;
  mockZeroMaskPin = 10;
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockZeroMaskPin = -1;
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{64, 10, 2, 3, false, true}));
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 64, 2, 3, false, true}));

  mockNullOutputPort = digitalPinToPort(16);
  TEST_ASSERT_FALSE(separatedPorts.begin(EncoderGenerator::Config{8, 16, 24, 32, false, true}));
  mockNullOutputPort = -1;
  mockZeroMaskPin = 16;
  TEST_ASSERT_FALSE(separatedPorts.begin(EncoderGenerator::Config{8, 16, 24, 32, false, true}));
  mockZeroMaskPin = -1;

  mockNullInputPort = digitalPinToPort(2);
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockNullInputPort = -1;
  mockNullInputPort = digitalPinToPort(32);
  TEST_ASSERT_FALSE(separatedPorts.begin(EncoderGenerator::Config{8, 16, 24, 32, false, true}));
  mockNullInputPort = -1;
  mockZeroMaskPin = 2;
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, true}));
  mockZeroMaskPin = -1;
  mockZeroMaskPin = 32;
  TEST_ASSERT_FALSE(separatedPorts.begin(EncoderGenerator::Config{8, 16, 24, 32, false, true}));
  mockZeroMaskPin = -1;
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 64, 3, false, true}));
  TEST_ASSERT_FALSE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 64, false, true}));
  TEST_ASSERT_TRUE(encoder.begin(EncoderGenerator::Config{9, 10, 2, 3, false, false}));

  setDigitalPin(2, true);
  setDigitalPin(3, true);
  encoder.onTick();
  TEST_ASSERT_EQUAL_INT32(0, encoder.getPosition());

  setDigitalPin(2, false);
  setDigitalPin(3, true);
  encoder.onTick();
  TEST_ASSERT_EQUAL_INT32(1, encoder.getPosition());
  TEST_ASSERT_TRUE(encoder.getDirection());

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  encoder.onTick();
  TEST_ASSERT_EQUAL_INT32(0, encoder.getPosition());
  TEST_ASSERT_FALSE(encoder.getDirection());

  EncoderGeneratorMirror& mirror = reinterpret_cast<EncoderGeneratorMirror&>(encoder);
  mirror.portAOut = nullptr;
  mirror.portBOut = nullptr;
  setDigitalPin(2, false);
  setDigitalPin(3, true);
  encoder.onTick();
  encoder.reset();
  TEST_ASSERT_EQUAL_INT32(0, encoder.getPosition());
}