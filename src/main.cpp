#include <iostream>
#include <cstdlib>
#include <RtMidi.h>
#include <chrono>

struct Message {
    Message(unsigned char b0, unsigned char b1, unsigned char b2, int delta_time)
        : bytes { b0, b1, b2 }, delta_time(delta_time) {}
    std::vector<unsigned char> bytes;
    int delta_time;
};

void sleep_for(float time) {
    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();

        auto time_diff = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
        if (time_diff >= time) {
            break;
        }

    }
}

bool done;
static void interrupt(int ignore){
    done = true;
}

void enable_if_true(RtMidiOut* midi_out, int* states, int button, bool condition) {
    int state = condition ? 0x01 : 0x00;
    if (states[button] != state) {
        states[button]  = state;
        std::vector<unsigned char> message = { 0x90, (unsigned char)button, (unsigned char)state };
        midi_out->sendMessage(&message);
    }
}

int main(int argc, const char** argv) {
    RtMidiIn*  midi_in  = new RtMidiIn();
    RtMidiOut* midi_out = new RtMidiOut();
    std::vector<unsigned char> message;
    int states[256];
    for (int i = 0x00; i < 0xff; ++i) {
        states[i] = 0;
    }

    done = false;
    (void) signal(SIGINT, interrupt);

    // Search for the MIDI input
    for (auto i = midi_in->getPortCount() - 1; i >= 0; --i) {
        if (midi_in->getPortName(i) == "APC MINI") {
            midi_in->openPort(i);
            break;
        }
    }
    if (!midi_in->isPortOpen()) {
        printf("Couldn't find 'APC MINI' midi input device.\n");
        goto cleanup;
    }

    // Search for the MIDI output
    for (auto i = midi_out->getPortCount() - 1; i >= 0; --i) {
        if (midi_out->getPortName(i) == "APC MINI") {
            midi_out->openPort(i);
            break;
        }
    }
    if (!midi_out->isPortOpen()) {
        printf("Couldn't find 'APC MINI' midi output device.\n");
        goto cleanup;
    }

    while (!done) {
        midi_in->getMessage(&message);
        if (message.empty() || message.size() < 3) {
            continue;
        }

        // Note on 0x01 : solid green
        // Note on 0x02 : flashing green

        // Note on 0x03 : solid red
        // Note on 0x04 : flashing red

        // Note on 0x05 : solid yellow
        // Note on 0x06 : flashing yellow

        if (message[0] == 0x90) {
            // Note on
            printf("NOTE ON  %02x %02x\n", (int)message[1], (int)message[2]);
            message[2] = states[message[1]] = (states[message[1]] + 1) % 0x07;
            midi_out->sendMessage(&message);
        } else if (message[0] == 0x80) {
            // Note off
            printf("NOTE OFF %02x %02x\n", (int)message[1], (int)message[2]);
        } else if (message[0] == 0xb0) {
            // Control message
            printf("CONTROL  %02x %02x\n", (int)message[1], (int)message[2]);
            enable_if_true(midi_out, states, 0x00, 0x10 <= message[2]);
            enable_if_true(midi_out, states, 0x08, 0x20 <= message[2]);
            enable_if_true(midi_out, states, 0x10, 0x30 <= message[2]);
            enable_if_true(midi_out, states, 0x18, 0x40 <= message[2]);
            enable_if_true(midi_out, states, 0x20, 0x50 <= message[2]);
            enable_if_true(midi_out, states, 0x28, 0x60 <= message[2]);
            enable_if_true(midi_out, states, 0x30, 0x70 <= message[2]);
            enable_if_true(midi_out, states, 0x38, 0x7f <= message[2]);
        }

    }

cleanup:
    delete midi_in;
    delete midi_out;
    return 0;
}
