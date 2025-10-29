# SPDX-License-Identifier: MIT | Author: Rohit Patil
#!/usr/bin/env python3
import sys
import os
import yaml
import textwrap
from jinja2 import Template

# Supported C/C++ integer types that can appear in the event definitions.
_supported_type_list = ["uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t"]

# --------------------------------------------------------------------------- #
# Generic file generator ---------------------------------------------------- #
# --------------------------------------------------------------------------- #
class GenerateFile:
    """
    Base class that handles writing a header file and appending events to it.
    The file path is stored in ``outfile`` and the stream ID (used in templates)
    is stored in ``stream_id``.
    """
    def __init__(self, file_path, streamId):
        self.outfile = file_path
        self.bHeaderAdded = False
        self.stream_id = streamId

    # --------------------------------------------------------------------- #
    # Header generation --------------------------------------------------- #
    # --------------------------------------------------------------------- #
    def add_header(self, tmpl_str):
        inputs = {}
        inputs["stream_id"] = self.stream_id
        template = Template(tmpl_str)
        with open(self.outfile, 'w+') as f:
            out_str = template.render(**inputs)
            f.write(out_str)

    # --------------------------------------------------------------------- #
    # Event generation ---------------------------------------------------- #
    # --------------------------------------------------------------------- #
    def add_event(self, tmpl_str, event):
        inputs = {}
        inputs["stream_id"] = self.stream_id
        inputs["evt"] = event
        print(inputs)
        template = Template(tmpl_str)
        with open(self.outfile, 'a') as f:
            out_str = template.render(**inputs)
            f.write(out_str)

# --------------------------------------------------------------------------- #
# C++ header file generator ------------------------------------------------- #
# --------------------------------------------------------------------------- #
class CppHeaderFile(GenerateFile):
    def __init__(self, dpath, streamId):
        super().__init__(f"{dpath}/event_types.hpp", streamId)
        self._create()

    # --------------------------------------------------------------------- #
    # Header skeleton ----------------------------------------------------- #
    # --------------------------------------------------------------------- #
    def _create(self):
        """
        Write a minimal header that includes the common event definition,
        guards, and defines ``EVENT_STREAM_ID``.
        """
        c_code_tmpl = """

        #include <event.hpp>

        #pragma once

        #define EVENT_STREAM_ID  {{ stream_id }}

        """
        clean_template = textwrap.dedent(c_code_tmpl)
        super().add_header(clean_template)

    # --------------------------------------------------------------------- #
    # Event type definition ----------------------------------------------- #
    # --------------------------------------------------------------------- #
    def addEvent(self, event):
        """
        Append a struct and ``EventId`` specialization for the given event.
        The struct is marked with ``__attribute__((packed))`` to avoid
        padding between fields.
        """
        c_code_tmpl = """
        typedef struct {
            {%- for f in evt.params %}
            {{ f.type }} {{ f.name }};
            {%- endfor %}
        } __attribute__((packed)) {{ evt.name }}_t;

        template <>
        struct EventId<{{ evt.name }}_t> {
            static constexpr uint32_t value = {{ evt.id }};
        };
        """
        clean_template = textwrap.dedent(c_code_tmpl)
        super().add_event(clean_template, event)

# --------------------------------------------------------------------------- #
# Babeltrace metadata generator --------------------------------------------- #
# --------------------------------------------------------------------------- #
class BabeltraceMetadata(GenerateFile):
    """
    Generates a Babeltrace (CTF) configuration file that describes the
    trace format and all events.  The main file is named simply ``metadata``.
    """
    def __init__(self, dpath, streamId):
        super().__init__(f"{dpath}/metadata", streamId)
        self._create()

    def _create(self):
        """
        Write the core trace definition (types, trace properties,
        clock, packet header, etc.).  The only dynamic part is
        ``{{ stream_id }}``.
        """
        bb_config_hdr = """\
        /* CTF 1.8 */

        typedef integer { size = 64; align = 8; signed = false; } uint64_t;
        typedef integer { size = 32; align = 8; signed = false; } uint32_t;
        typedef integer { size = 16; align = 8; signed = false; } uint16_t;
        typedef integer { size = 8; align = 8; signed = false; }  uint8_t;
        typedef integer { size = 32; align = 8; signed = true; }  int32_t;
        typedef integer { size = 16; align = 8; signed = true; }  int16_t;
        typedef integer { size = 8; align = 8; signed = true; }   int8_t;

        trace {
            major = 1;
            minor = 8;
            byte_order = le;

             /* Packet header: must contain stream_id */
             packet.header := struct {
                 uint32_t stream_id;
             };

        };

        clock {
             name = myclock;
             freq = 1000000000; /* 1 GHz = ns */
        };

        stream {
             id = {{ stream_id }};

             packet.context := struct {
                 uint32_t events_discarded;
                 uint32_t packet_size;
                 uint32_t content_size;
                 uint32_t packet_seq_count;
             };

             event.header := struct {
                 uint32_t id;
                 uint64_t timestamp;
             };
        };

        """
        clean_template = textwrap.dedent(bb_config_hdr)
        super().add_header(clean_template)

    # --------------------------------------------------------------------- #
    # Individual event definition ----------------------------------------- #
    # --------------------------------------------------------------------- #
    def addEvent(self, event):
        """
        Append an ``event`` block to the CTF configuration.  Each event
        contains its name, id, stream ID and a struct of fields.
        """
        bb_config_event = """
        event {
            name = {{ evt.name }};
            id   = {{ evt.id }};
            stream_id = {{ stream_id }};

            fields := struct {
                {%- for f in evt.params %}
                {{ f.type }} {{ f.name }};
                {%- endfor %}
            };
        };

        """
        clean_template = textwrap.dedent(bb_config_event)
        super().add_event(clean_template, event)

# --------------------------------------------------------------------------- #
# YAML parsing utilities ---------------------------------------------------- #
# --------------------------------------------------------------------------- #
def parse_yaml_file(file_path):
    """
    Generator that yields one event at a time from a potentially large
    YAML file.  The YAML is expected to be a list of dictionaries,
    each containing an ``events`` key whose value is a list of events.
    """
    with open(file_path, 'r') as f:
        data = yaml.safe_load(f)

    for event_entry in data:
        events = event_entry.get('events', [])
        for ev in events:
            yield (ev)

# --------------------------------------------------------------------------- #
# Validation utilities ------------------------------------------------------ #
# --------------------------------------------------------------------------- #
def check_argument(event):
    """
    Validate that the event dictionary contains a string name, a non‑negative
    integer ID and parameters that use only supported types.
    If validation fails, print an error message and exit.
    """
    event_name = event['name']
    event_id = int(event['id'])
    if not isinstance(event_name, str):
        print(f"group:{gName} event name not a string")
        sys.exit(-1)
    if event_id < 0:
        print(f"{event_name} event Id negative not supported")
        sys.exit(-1)

    params = event.get('params', [])
    for p in params:
        t = p['type']
        n = p['name']
        if not isinstance(n, str):
            print(f"group:{gName} event:{event_name} parameter not have name in string format {n}")
            sys.exit(-1)
        if not isinstance(t, str):
            print(f"group:{gName} event:{event_name} parameter {n} not have type in string format {t}")
            sys.exit(-1)
        if t not in _supported_type_list:
            print(f"group:{gName} event:{event_name} have unsupported type {t}")
            sys.exit(-1)

# --------------------------------------------------------------------------- #
# Main entry point ---------------------------------------------------------- #
# --------------------------------------------------------------------------- #
def main(yaml_file, out_path):
    """
    High‑level driver that creates the C++ header and Babeltrace
    metadata files, then iterates over all events in the YAML file,
    validates them, and writes their definitions to both outputs.
    """
    c_file = CppHeaderFile(out_path, 0)
    bb_file = BabeltraceMetadata(out_path, 0)

    for event in parse_yaml_file(yaml_file):
        check_argument(event)
        c_file.addEvent(event)
        bb_file.addEvent(event)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <yaml_file>")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
