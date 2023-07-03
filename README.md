# backlight

Basic backlight utility for intel iGPU laptops that don't have xbacklight support but do show up in `sys`.

## Usage

Build the program, copy it to bin, and set it suid root so that it's allowed to write the backlight file.

```bash
make
sudo make install
```

Once installed, you can adjust the backlight in either raw units (which vary between laptop models) or in percentages of max brightness. Numbers prefixed with a + or - are treated as relative changes.

```bash
# Increase brightness by 5%
backlight +5%
# Set brightness to half of max
backlight 50%
# Turn off backlight
backlight 0
# Increment backlight brightness by 1000 raw units
backlight 1000
```
