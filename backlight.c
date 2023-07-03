#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

static const char *BACKLIGHT_PATH =
    "/sys/class/backlight/intel_backlight/brightness";
static const char *MAX_BRIGHTNESS_PATH =
    "/sys/class/backlight/intel_backlight/max_brightness";

int read_file(const char *path) {
  // Open file
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
    return -1;
  }

  // Read contents
  char buf[64];
  memset(buf, 0, sizeof(buf));
  ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
  if (bytes_read < 0) {
    fprintf(stderr, "Failed to read %s: %s\n", path, strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);

  // Parse as integer
  char *endptr;
  int value = strtol(buf, &endptr, 10);
  if (buf == endptr) {
    return -1;
  }

  return value;
}

int main(int argc, char **argv) {
  // Check args
  if (argc != 2) {
    fprintf(stderr, "%s [+-]brightness[%%]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse
  const char *amount_str = argv[1];
  const bool is_relative = (amount_str[0] == '+' || amount_str[0] == '-');
  char *endptr;
  int amount = strtol(amount_str, &endptr, 10);
  if (endptr == amount_str) {
    fprintf(stderr, "Failed to parse argument as a number: %s\n", amount_str);
    return EXIT_FAILURE;
  }

  // If the number was followed by a percent sign, it's in percent of max
  // brightness val not in raw units
  const bool is_percent = *endptr == '%';

  // Read the max brightness / current brightness values
  const int max_brightness = read_file(MAX_BRIGHTNESS_PATH);
  const int current_brightness = read_file(BACKLIGHT_PATH);
  if (max_brightness == -1) {
    fprintf(stderr, "Failed to read max brightness\n");
    return EXIT_FAILURE;
  }
  if (current_brightness == -1) {
    fprintf(stderr, "Failed to read current brightness\n");
    return EXIT_FAILURE;
  }

  // Work out what we actually want to write back
  int new_brightness = -1;
  if (is_relative && is_percent) {
    float delta_percent = ((float)amount) / 100.0;
    int delta_ticks = delta_percent * max_brightness;
    new_brightness = current_brightness + delta_ticks;
  } else if (is_relative && !is_percent) {
    new_brightness = current_brightness + amount;
  } else if (!is_relative && is_percent) {
    float percent_amount = ((float)amount) / 100.0;
    new_brightness = percent_amount * max_brightness;
  } else if (!is_relative && !is_percent) {
    new_brightness = amount;
  } else {
    fprintf(stderr, "Unhandled combination\n");
    asm volatile("UD2");
  }

  // Bound the brightness value
  if (new_brightness < 0) {
    new_brightness = 0;
  }
  if (new_brightness > max_brightness) {
    new_brightness = max_brightness;
  }

  // Open fd
  int fd = open(BACKLIGHT_PATH, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed to open backlight device: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // Write new backlight value
  const int written = dprintf(fd, "%d", new_brightness);
  if (written < 0) {
    fprintf(stderr, "Failed to write backlight setting: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
