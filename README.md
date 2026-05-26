# USB Audio + MIDI Example using TinyUSB and STM32H732

This project is an example of a USB Audio Class 2 (UAC2) device based on the " `audio_test` example from the [TinyUSB](https://docs.tinyusb.org/en/latest/) library for the STM32H732 microcontroller on the [WeAct STM32H7xx Core Board](https://github.com/WeActStudio/MiniSTM32H7xx).

The project was configured using STM32CubeMX version 6.16.1 and compiled with STM32CubeIDE version 2.1.1.

When the board is connected to a PC, the operating system (tested only on Linux Mint 22.3) recognizes it as:

* a USB microphone;

* a USB MIDI device.

In this example, the microphone continuously outputs a 1 kHz sine wave.

Regarding MIDI support, only MIDI message reception was implemented in this example. However, MIDI transmission can be implemented in a very similar way. Some `typedefs` were created to simplify the handling of MIDI message data. These definitions can be found in the `midi.h` file.

You can see examples of how these structures are used inside the `while(1)` loop in `main()`. When a MIDI Control Change message is received on controller number 20 (`0x14`) with a value greater than 63, the onboard LED starts blinking faster (100 ms period). Values lower than 64 make the LED return to the default blinking period used in the original TinyUSB example.

## Important notes

For this specific microcontroller, a few adjustments were required to make UAC2 work properly. The most important one is ensuring that the USB peripheral can access a valid RAM region.

A dedicated RAM section called `.usb_ram` was added to the linker script `STM32H743VITX_FLASH.ld`. Be aware that STM32CubeMX may overwrite this modification when regenerating the project files, so make sure to verify it after running CubeMX.

### Debug Messages and SWO

For TinyUSB debug messages and `printf()` output, this project uses the SWO (Serial Wire Output) feature on pin `PB3`.

SWO is a unidirectional debug communication channel that allows the microcontroller to send data to the host computer through the onboard ST-LINK without significantly interrupting CPU execution. This is extremely important when debugging USB applications.

Using a regular USART for debug output may introduce excessive delays, and the USB host may timeout during device enumeration, causing USB errors.

TinyUSB debug messages are disabled by default in this example. If desired, they can be enabled by changing the `CFG_TUSB_DEBUG` definition in the `tusb_config.h` file.

### USB Audio Synchronization Mode

The USB audio streaming mode was configured as **Synchronous**. The descriptor defined by the `TUD_AUDIO20_MIC_ONE_CH_DESCRIPTOR` macro was modified for this purpose.

The device always transmits the same number of samples every millisecond (USB Full Speed mode).

However, the default TinyUSB macros used to create audio descriptors automatically add one extra sample to the endpoint size. This behavior exists to support **Asynchronous** and **Adaptive** synchronization modes.

In my case, the audio stream only worked correctly after modifying `audio_device.c` by commenting line 1841 and adding line 1842, because there is an `ASSERT` that fails when the endpoint size is configured using the fixed number of samples without the additional sample.

With the extra sample enabled, the audio stream consistently caused a `HardFault` on the STM32H743 for reasons that I still do not fully understand. I am not sure whether this is the best or most correct solution.

Interestingly, the same issue did not occur when testing on an STM32F411.

## Audio Callbacks

The audio class callbacks and related variables were moved to the files:

- `audio_cb.h`
- `audio_cb.c`

This was done to keep `main.c` shorter and easier to read.

Since this project is only intended as an example, the volume and mute control callbacks are currently implemented only as placeholders and are not effectively used by the application.
