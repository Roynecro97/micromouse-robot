# micromouse-robot

Robot for Micromouse.

## Using ESP-IDF

### Setting Up ESP-IDF

1. Install ESP-IDF 5.1.4 using the VSCode extension or by following the
   [guide][install guide].
2. Install `idf-requirements.txt` in the ESP-IDF python virtualenv.

   For example, if you're using Python 3.12 and a POSIX environment with bash:

   ```sh
   source ~/.espressif/python_env/idf5.1_py3.12_env/bin/activate
   pip install -r idf-requirements.txt
   ```

   Or in a single command:

   ```sh
   ~/.espressif/python_env/idf5.1_py3.12_env/bin/pip install -r idf-requirements.txt
   ```

   This step is required to make sure the ESP-IDF dependencies are using the
   correct versions because the ESP-IDF installation allows for API breaking
   updates.

[install guide]: https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32/get-started/index.html#installation

### Configuring the Project

(This stage is optional)

Use `idf.py menuconfig` to configure the project.

### Running on the ESP32

Use `idf.py build` to build the project.

Use `idf.py flash` to build (if needed) and flash the ESP32.

Use `idf.py monitor` to view the console output.
To exit the monitor, use `Ctrl + ]`.
See more keyboard shortcuts [here][monitor shortcuts].

`idf.py flash monitor` performs build+flash+monitor.

[monitor shortcuts]: https://docs.espressif.com/projects/esp-idf/en/v5.1.4/esp32/api-guides/tools/idf-monitor.html#keyboard-shortcuts

## Running the Tests

TODO: Add tests and how to run them
