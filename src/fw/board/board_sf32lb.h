#pragma once

#include "display.h"

#include "drivers/button_id.h"
#include "debug/power_tracking.h"
#include <mcu.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  GPIO_OType_PP,
  GPIO_OType_OD,
} GPIOOType_TypeDef;

typedef enum {
  GPIO_PuPd_NOPULL,
  GPIO_PuPd_UP,
  GPIO_PuPd_DOWN,
} GPIOPuPd_TypeDef;

typedef enum {
  GPIO_Speed_2MHz,
  GPIO_Speed_50MHz,
  GPIO_Speed_200MHz
} GPIOSpeed_TypeDef;

// Power Configuration
/////////////////////////////////////////////////////////////////////////////

#define GPIO_Port_NULL ((GPIO_TypeDef *) 0)
#define GPIO_Pin_NULL ((uint16_t)0x0000)
//! Guaranteed invalid IRQ priority
#define IRQ_PRIORITY_INVALID (1 << __NVIC_PRIO_BITS)

// This is generated in order to faciliate the check within the IRQ_MAP macro below
#include "bf0_hal.h"

// This is generated in order to faciliate the check within the IRQ_MAP macro below
enum {
  #define IRQ_DEF(num, irq) IS_VALID_IRQ__##irq,
  #if defined(MICRO_FAMILY_SF32LB)
  #  include "irq_sf32lb.def"
  #else
  #  error need IRQ table for new micro family
  #endif
  #undef IRQ_DEF
  };

//! Creates a trampoline to the interrupt handler defined within the driver
#define IRQ_MAP(irq, handler, device) \
  void irq##_IRQHandler(void) { \
    handler(device); \
  } \
  _Static_assert(IS_VALID_IRQ__##irq || true, "(See comment below)")
/*
 * The above static assert checks that the requested IRQ is valid by checking that the enum
 * value (generated above) is declared. The static assert itself will not trip, but you will get
 * a compilation error from that line if the IRQ does not exist within irq_stm32*.def.
 */

// There are a lot of DMA streams and they are very straight-forward to define. Let's use some
// macro magic to make it a bit less tedious and error-prone.
#define CREATE_DMA_STREAM(cnum, snum) \
  static DMAStreamState s_dma##cnum##_stream##snum##_state; \
  static DMAStream DMA##cnum##_STREAM##snum##_DEVICE = { \
    .state = &s_dma##cnum##_stream##snum##_state, \
    .controller = &DMA##cnum##_DEVICE, \
    .periph = DMA##cnum##_Stream##snum, \
    .irq_channel = DMA##cnum##_Stream##snum##_IRQn, \
  }; \
  IRQ_MAP(DMA##cnum##_Stream##snum, dma_stream_irq_handler, &DMA##cnum##_STREAM##snum##_DEVICE)

typedef struct {
  GPIO_TypeDef* const peripheral; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
} ExtiConfig;

typedef enum {
  AccelThresholdLow, ///< A sensitive state used for stationary mode
  AccelThresholdHigh, ///< The accelerometer's default sensitivity
  AccelThreshold_Num,
} AccelThreshold;

typedef struct {
  const char* const name; ///< Name for debugging purposes.
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
  ExtiConfig exti;
  GPIOPuPd_TypeDef pull;
} ButtonConfig;

typedef struct {
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
} ButtonComConfig;

typedef struct {
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
} InputConfig;

#define ADC_TypeDef GPADC_TypeDef
typedef struct {
  ADC_TypeDef *const adc; ///< One of ADCX. For example ADC1.
  const uint8_t adc_channel; ///< One of ADC_Channel_*
  uint32_t clock_ctrl;  ///< Peripheral clock control flag
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint16_t gpio_pin; ///< One of GPIO_Pin_*
} ADCInputConfig;


#define TIM_TypeDef GPT_TypeDef
#define TIM_OCInitTypeDef void

typedef struct {
  union {
    TIM_TypeDef* const peripheral; ///< A TIMx peripheral GTIM
    LPTIM_TypeDef* const lp_peripheral; ///< A LPTIMx peripheral
  };
  const uint32_t config_clock;   ///< One of RCC_APB1Periph_TIMx. For example, RCC_APB1Periph_TIM3.
  void (* const init)(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct); ///< One of TIM_OCxInit
  void (* const preload)(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload); ///< One of TIM_OCxPreloadConfig
} TimerConfig;

typedef struct {
  const TimerConfig timer;
  const uint8_t irq_channel;
} TimerIrqConfig;

typedef struct {
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
  bool active_high; ///< Pin is active high or active low
} OutputConfig;

//! Alternate function pin configuration
//! Used to configure a pin for use by a peripheral
typedef struct {
  GPIO_TypeDef* const gpio; ///< One of GPIOX. For example, GPIOA.
  const uint32_t gpio_pin; ///< One of GPIO_Pin_X.
  const uint16_t gpio_pin_source; ///< One of GPIO_PinSourceX.
  const uint8_t  gpio_af; ///< One of GPIO_AF_X
} AfConfig;

typedef struct {
  uint16_t value;
  uint16_t resolution;
  int enabled;
  uint16_t channel;
  uint8_t  is_comp; /* Is complementary*/  
} PwmState;

typedef struct {
  OutputConfig output;
  TimerConfig timer;
  AfConfig afcfg;
  PwmState  *state;
  
} PwmConfig;

typedef struct {
  int axes_offsets[3];
  bool axes_inverts[3];
  uint32_t shake_thresholds[AccelThreshold_Num];
  uint32_t double_tap_threshold;
} AccelConfig;

typedef struct {
  int axes_offsets[3];
  bool axes_inverts[3];
} MagConfig;

typedef struct {
  AfConfig i2s_ck;
  AfConfig i2s_sd;
  SPI_TypeDef *spi;
  uint32_t spi_clock_ctrl;
  uint16_t gain;

  //! Pin we use to control power to the microphone. Only used on certain boards.
  OutputConfig mic_gpio_power;
} MicConfig;

typedef enum {
  OptionNotPresent = 0, // FIXME
  OptionActiveLowOpenDrain,
  OptionActiveHigh
} PowerCtl5VOptions;

typedef enum {
  ActuatorOptions_Ctl = 1 << 0, ///< GPIO is used to enable / disable vibe
  ActuatorOptions_Pwm = 1 << 1, ///< PWM control
  ActuatorOptions_IssiI2C = 1 << 2, ///< I2C Device, currently used for V1_5 -> OG steel backlight
  ActuatorOptions_HBridge = 1 << 3, //< PWM actuates an H-Bridge, requires ActuatorOptions_PWM
} ActuatorOptions;

typedef enum {
  CC2564A = 0,
  CC2564B,
  DA14681,
} BluetoothController;

typedef struct {
  // Audio Configuration
  /////////////////////////////////////////////////////////////////////////////
  const bool has_mic;
  const MicConfig mic_config;

  // IO expander rst ctrl
  const OutputConfig ioe_rst;

  // Vibe rst ctrl pin
  const OutputConfig vibe_rst;

  // Ambient Light Configuration
  /////////////////////////////////////////////////////////////////////////////
  const uint32_t ambient_light_dark_threshold;
  const uint32_t ambient_k_delta_threshold;
  const OutputConfig photo_en;
  const bool als_always_on;

  // Debug Serial Configuration
  /////////////////////////////////////////////////////////////////////////////
  const ExtiConfig dbgserial_int;
  const InputConfig dbgserial_int_gpio;

  // MFi Configuration
  /////////////////////////////////////////////////////////////////////////////
  const OutputConfig mfi_reset_pin;

  // Display Configuration
  /////////////////////////////////////////////////////////////////////////////
  const OutputConfig lcd_com; //!< This needs to be pulsed regularly to keep the sharp display fresh.

  //! Controls power to the sharp display
  const PowerCtl5VOptions power_5v0_options;
  const OutputConfig power_ctl_5v0;

  const uint8_t backlight_on_percent; // percent of max possible brightness
  const uint8_t backlight_max_duty_cycle_percent; // Calibrated such that the preceived brightness
                    // of "backlight_on_percent = 100" (and all other values, to a reasonable
                    // tolerance) is identical across all platforms. >100% isn't possible, so
                    // future backlights must be at least as bright as Tintin's.

  // FPC Pinstrap Configuration
  /////////////////////////////////////////////////////////////////////////////
  const InputConfig fpc_pinstrap_1;
  const InputConfig fpc_pinstrap_2;

  // GPIO Configuration
  /////////////////////////////////////////////////////////////////////////////
  const uint16_t num_avail_gpios;
} BoardConfig;

// Button Configuration
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  const ButtonConfig buttons[NUM_BUTTONS];
  const ButtonComConfig button_com;
  const bool active_high;
} BoardConfigButton;

typedef struct {
  const uint32_t numerator;
  const uint32_t denominator;
} VMonScale;

// Power Configuration
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  const ExtiConfig pmic_int;
  const InputConfig pmic_int_gpio;

  //! Voltage rail control lines
  const OutputConfig rail_4V5_ctrl;
  const OutputConfig rail_6V6_ctrl;
  const GPIOOType_TypeDef rail_6V6_ctrl_otype;

  //! Scaling factor for battery vmon
  const VMonScale battery_vmon_scale;
  //! Tells us if the USB cable plugged in.
  const InputConfig vusb_stat;
  const ExtiConfig vusb_exti;
  //! Tells us whether the charger thinks we're charging or not.
  const InputConfig chg_stat;
  //! Tell the charger to use 2x current to charge faster (MFG only).
  const OutputConfig chg_fast;
  //! Enable the charger. We may want to disable this in MFG, normally it's always on.
  const OutputConfig chg_en;

  //! Interrupt that fires when the USB cable is plugged in
  const bool has_vusb_interrupt;

  const bool wake_on_usb_power;

  const int charging_cutoff_voltage;
  const int charging_status_led_voltage_compensation;

  //! Percentage for watch only mode
  const uint8_t low_power_threshold;

  //! Approximate hours of battery life
  const uint8_t battery_capacity_hours;

  const InputConfig charger_int_gpio;
  const ExtiConfig charger_exti;
} BoardConfigPower;

typedef struct {
  const AccelConfig accel_config;
  const InputConfig accel_int_gpios[2];
  const ExtiConfig accel_ints[2];
} BoardConfigAccel;

typedef struct {
  const MagConfig mag_config;
  const InputConfig mag_int_gpio;
  const ExtiConfig mag_int;
} BoardConfigMag;

typedef struct {
  const ActuatorOptions options;
  const OutputConfig ctl;
  const PwmConfig pwm;
  const uint16_t vsys_scale; //< Voltage to scale duty cycle to in mV. 0 if no scaling should occur.
                             //< For example, Silk VBat may droop to 3.3V, so we scale down vibe
                             //< duty cycle so that 100% duty cycle will always be 3.3V RMS.
} BoardConfigActuator;

typedef struct {
  const OutputConfig power_en; //< Enable power supply to the accessory connector.
  const InputConfig int_gpio;
  const ExtiConfig exti;
} BoardConfigAccessory;

typedef struct {
  const BluetoothController controller;

  union {
    //! Used with CC2564x
    const OutputConfig shutdown;

    //! Used with DA14681
    const OutputConfig reset;
  };

  struct {
    const InputConfig int_gpio;
    const ExtiConfig int_exti;
  } wakeup;
} BoardConfigBTCommon;

typedef struct {
  const bool output_enabled;
  const AfConfig af_cfg;
  const InputConfig an_cfg;
} BoardConfigMCO1;

typedef struct {
  const OutputConfig cs;
} BoardConfigBTSPI;

typedef struct {
  const AfConfig rx_af_cfg;
  const AfConfig tx_af_cfg;

  USART_TypeDef* const rx_uart;
  USART_TypeDef* const tx_uart;

  const uint32_t rx_clk_control;
  const uint32_t tx_clk_control;
} BoardConfigBTUART;

// REVISIT:
// This enum exists to allow older roll-your-own SPI
// drivers to continue to work until they are reworked
// to use the new SPI API.  It can go away once the
// new API is used exclusively

typedef enum {
  SpiPeriphClockAPB1,
  SpiPeriphClockAPB2
} SpiPeriphClock;

typedef struct {
  SPI_TypeDef *spi;
  GPIO_TypeDef * const spi_gpio;

  const uint32_t spi_clk;
  const SpiPeriphClock spi_clk_periph;

  const AfConfig mosi;
  const AfConfig clk;
  const OutputConfig cs;

  const OutputConfig on_ctrl;
  const GPIOOType_TypeDef on_ctrl_otype;
} BoardConfigSharpDisplay;

typedef const struct DMARequest DMARequest;
typedef const struct UARTDevice UARTDevice;
typedef const struct SPIBus SPIBus;
typedef const struct SPISlavePort SPISlavePort;
typedef const struct I2CBus I2CBus;
typedef const struct I2CSlavePort I2CSlavePort;
typedef const struct HRMDevice HRMDevice;
typedef const struct VoltageMonitorDevice VoltageMonitorDevice;
typedef const struct TemperatureSensor TemperatureSensor;
typedef const struct MicDevice MicDevice;
typedef const struct QSPIPort QSPIPort;
typedef const struct QSPIFlash QSPIFlash;
typedef const struct ICE40LPDevice ICE40LPDevice;


typedef const struct TouchSensor {
  I2CSlavePort *i2c;
  InputConfig int_gpio;
  ExtiConfig int_exti;
  //OutputConfig reset_gpio;
} TouchSensor;


void board_early_init(void);
void board_init(void);

#include "board_definitions.h"

