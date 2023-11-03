#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to handle GPIO chip initialization and setup
struct gpiod_chip* initializeGPIO() {
    // Open the GPIO chip associated with /dev/gpiochip0
    // Returns a pointer to the initialized chip or NULL in case of an error
    struct gpiod_chip* chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
    }
    return chip;
}

// Function to set up a GPIO line and configure it as an output
struct gpiod_line_bulk setupGPIOLine(struct gpiod_chip* chip, int offset, const char* consumer) {
    // Configuration for requesting a GPIO line
    struct gpiod_line_request_config config;
    struct gpiod_line_bulk lines;
    int values[1] = {0};
    int err;

    unsigned int offsets[1] = {offset};

    // Get the GPIO line specified by 'offset'
    err = gpiod_chip_get_lines(chip, offsets, 1, &lines);
    if (err) {
        perror("gpiod_chip_get_lines");
    } else {
        // Configure the GPIO line for output and assign the specified 'consumer' name
        memset(&config, 0, sizeof(config));
        config.consumer = consumer;
        config.request_type = GPIOD_LINE_REQUEST_DIRECTION_OUTPUT;
        config.flags = 0;

        // Request the GPIO line with the given configuration
        err = gpiod_line_request_bulk(&lines, &config, values);
        if (err) {
            perror("gpiod_line_request_bulk");
            gpiod_line_release_bulk(&lines);
        }
    }

    return lines;
}

// Function to set the GPIO line value
void setGPIOLineValue(struct gpiod_line_bulk lines, int value) {
    int err;
    int values[1] = {value};
    // Set the value of the GPIO line(s) to the specified 'value'
    err = gpiod_line_set_value_bulk(&lines, values);
    if (err) {
        perror("gpiod_line_set_value_bulk");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <GPIO_PIN> <on/off>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int pinNumber = atoi(argv[1]);
    const char* action = argv[2]; // Action (on or off)

    // Initialize the GPIO chip
    struct gpiod_chip *chip = initializeGPIO();
    if (!chip) {
        return EXIT_FAILURE;
    }

    struct gpiod_line_bulk lines = setupGPIOLine(chip, pinNumber, "relay-control");
    if (lines.num_lines == 0) {
        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    if (strcmp(action, "on") == 0) {
        // Turn on the relay (set the GPIO line to high)
        setGPIOLineValue(lines, 1);
        printf("Relay is ON.\n");
    } else if (strcmp(action, "off") == 0) {
        // Turn off the relay (set the GPIO line to low)
        setGPIOLineValue(lines, 0);
        printf("Relay is OFF.\n");
    } else {
        fprintf(stderr, "Invalid action. Use 'on' or 'off'.\n");
    }

    // Cleanup
    gpiod_line_release_bulk(&lines);
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}
