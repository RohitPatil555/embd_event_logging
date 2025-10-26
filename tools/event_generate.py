#!/usr/bin/env python3
import sys
import os
import yaml
import textwrap
from jinja2 import Template

_supported_type_list = ["uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t"]

class GenerateFile:
    def __init__(self, file_path, streamId):
        self.outfile = file_path
        self.bHeaderAdded = False
        self.stream_id = streamId

    def add_header(self, tmpl_str):
        inputs = {}
        inputs["stream_id"] = self.stream_id
        template = Template(tmpl_str)
        with open(self.outfile, 'w+') as f:
            out_str = template.render(**inputs)
            f.write(out_str)

    def add_event(self, tmpl_str, event):
        inputs = {}
        inputs["stream_id"] = self.stream_id
        inputs["evt"] = event
        print(inputs)
        template = Template(tmpl_str)
        with open(self.outfile, 'a') as f:
            out_str = template.render(**inputs)
            f.write(out_str)


class CppHeaderFile(GenerateFile):
    def __init__(self, dpath, streamId):
        super().__init__(f"{dpath}/event_types.hpp", streamId)
        self._create()

    def _create(self):
        c_code_tmpl = """

        #include <event.hpp>

        #pragma once

        #define EVENT_STREAM_ID  {{ stream_id }}

        """
        clean_template = textwrap.dedent(c_code_tmpl)
        super().add_header(clean_template)

    def addEvent(self, event):
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


class BabeltraceMetadata(GenerateFile):
    def __init__(self, dpath, streamId):
        super().__init__(f"{dpath}/metadata", streamId)
        self._create()

    def _create(self):
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

    def addEvent(self, event):
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

def parse_yaml_file(file_path):
    """
    Generator that yields one event at a time from a potentially large YAML file.
    Supports 'include' directive inside 'events:'.
    """
    with open(file_path, 'r') as f:
        data = yaml.safe_load(f)

    for event_entry in data:
        events = event_entry.get('events', [])
        for ev in events:
            yield (ev)



def check_argument(event):
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

def main(yaml_file, out_path):
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
