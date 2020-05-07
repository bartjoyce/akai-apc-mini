#include <stdio.h>
#include <unistd.h>
#include <RtMidi.h>

bool done;
static void interrupt(int ignore){
    done = true;
}

static RtMidiIn*  midi_in;
static RtMidiOut* midi_out;
static int states[256];

void midi_in_callback(double time_stamp, std::vector<unsigned char>* message_ptr, void* user_data) {
    auto& message = *message_ptr;
    if (message.empty() || message.size() < 3) {
        return;
    }

    // Grid buttons:
    //  Note on 0x00 : no lights
    //  Note on 0x01 : green  solid
    //  Note on 0x02 : green  flashing
    //  Note on 0x03 : red    solid
    //  Note on 0x04 : red    flashing
    //  Note on 0x05 : yellow solid
    //  Note on 0x06 : yellow flashing

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
    }
}

int main(int argc, const char** argv) {
    std::vector<unsigned char> message;

    // Init global variables
    midi_in  = new RtMidiIn;
    midi_out = new RtMidiOut;
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

    midi_in->setCallback(&midi_in_callback, NULL);

    // Loop until interrupted
    while (!done) {
        sleep(1);
    }

cleanup:
    delete midi_in;
    delete midi_out;
    return 0;
}
