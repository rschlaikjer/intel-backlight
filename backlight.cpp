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

int main(int argc, char **argv) {
  // Check args
  if (argc != 2) {
    fprintf(stderr, "%s [+-]brightness\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Parse
  const char *amount_str = argv[1];
  const bool is_relative = (amount_str[0] == '+' || amount_str[0] == '-');
  int amount = std::stoi(amount_str);

  // Open fd
  int fd = open(BACKLIGHT_PATH, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed to open backlight device: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // If it's relative, need to load the previous value
  if (is_relative) {
    char buf[64];
    memset(buf, 0, sizeof(buf));
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    if (bytes_read < 0) {
      fprintf(stderr, "Failed to read backlight setting: %s\n",
              strerror(errno));
      return EXIT_FAILURE;
    }
    const int prev_amount = std::stoi(buf);
    amount += prev_amount;
  }

  // Write new backlight value
  std::string out_str = std::to_string(amount);
  ssize_t written = ::write(fd, out_str.data(), out_str.size());
  if (written < 0) {
    fprintf(stderr, "Failed to write backlight setting: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
