Certainly\! Here is the content rewritten in clear, professional language and formatted using Markdown.

# Library Quick Start Guide: A Professional Setup

This guide provides a professional, step-by-step walkthrough for integrating and using this event tracing library.

-----

## Step 1: Define Event Configuration (YAML File)

You must create a **YAML file** to define your events and their parameters. This file contains the specification for all traceable events.

Use the following required syntax:

```yaml
- events:
  - name: event1
    id: 1
    params:
      - name: field1
        type: uint8_t
      - ...
  - ...
```

> **Note:** Currently, only **signed and unsigned integer types (8, 16, and 32 bits)** are supported for event parameters.

-----

## Step 2: Integrate with the Build System (CMake)

Configure your project's CMake file to integrate the library. You must set the following two variables:

  * **`EVENT_DESCRIPTION_FILE`**: The full path to your event YAML configuration file.
  * **`EVENT_GENERATED_OUT_DIR`**: The directory where the build process will generate output files, including the essential `event_types.hpp` and the **Babeltrace metadata file**.

**Example CMake Configuration:**

```cmake
set(EVENT_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/example.yml)
set(EVENT_GENERATED_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
```

Finally, include the library in your main project:

```cmake
add_subdirectory(<library code checkout path>)
```

-----

## Step 3: Implement Event Logging in Application Code

This step covers how to generate events and send them to the collection framework.

### Generating an Event

To generate and populate an event:

1.  Reference the event name (e.g., `"event1"`) from your YAML file.

2.  Instantiate a variable of the generated event structure type (e.g., `Event<event1_t>`):

    ```cpp
    Event<event1_t> evt1;
    ```

3.  Update the event parameters (e.g., `"field1"`) using the `getParam()` method:

    ```cpp
    evt1.getParam()->field1 = 2;
    ```

4.  Log the populated event using the framework's singleton instance:

    ```cpp
    eventCollector * inst = eventCollector::getInstance();
    inst->pushEvent( &evt1 );
    ```

### Posting Event Packets to an Interface

To enable offline analysis, you must **periodically extract and dump** collected event packets. This is typically done in a dedicated thread that posts the data over a socket or writes it to a file.

**Packet Extraction Example:**

```cpp
auto pkt = inst->getSendPacket();

if ( pkt.has_value() ) {
  auto data = pkt.value();
  // Call your interface function to dump the binary packet data
  <interface_to_dump_packet>( reinterpret_cast<const char *>( data.data() ), data.size() );

  // Important: Free the packet buffer after a successful transmission/dump.
  inst->sendPacketCompleted();
} else {
  cerr << "fail to get event packet" << endl;
  // Implement a retry mechanism.
}
```

-----

## Step 4: Babeltrace Analysis Setup

Use **Babeltrace** to analyze the collected event data.

1.  Create a new directory for tracing, such as **`traces`**.

2.  Copy the build-generated **`metadata`** file into the `traces` directory.

3.  Copy all your collected **packet dump binary files** into the `traces` directory.

4.  Execute the following command to confirm Babeltrace is correctly reading the trace data:

    ```bash
    babeltrace2 traces/
    ```

    A console output confirms successful operation.

-----

## Step 5: Advanced Event Analysis

To analyze the system behavior from the event stream, you can:

1.  You can **create custom analysis plugins** for **Babeltrace**.
2.  Develop a **Python utility** to process the trace data.

For detailed documentation on Babeltrace, please refer to the official Babeltrace Introduction [here](https://babeltrace.org/docs/v2.0/man7/babeltrace2-intro.7/).
