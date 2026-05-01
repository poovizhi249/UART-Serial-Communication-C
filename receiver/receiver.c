#include <windows.h>
#include <stdio.h>

int main() {
    HANDLE hSerial;
    hSerial = CreateFile("COM6", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM6\n");
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    printf("Receiver: Listening...\n");

    unsigned char byte, buffer[256];
    DWORD bytesRead;
    int state = 0, length = 0, index = 0;
    unsigned char checksum = 0, received_checksum = 0;

    while (1) {
        if (!ReadFile(hSerial, &byte, 1, &bytesRead, NULL) || bytesRead == 0)
            continue;

        switch (state) {
            case 0:
                if (byte == 0x02) {
                    state = 1;
                    index = 0;
                    checksum = 0;
                }
                break;
            case 1:
                length = byte;
                state = 2;
                break;
            case 2:
                buffer[index++] = byte;
                checksum ^= byte;
                if (index == length)
                    state = 3;
                break;
            case 3:
                received_checksum = byte;
                state = 4;
                break;
            case 4:
                if (byte == 0x03) {
                    if (checksum == received_checksum) {
                        buffer[length] = '\0';
                        printf("Receiver: %s\n", buffer);
                    } else {
                        printf("Receiver: Checksum error. Message discarded.\n");
                    }
                } else {
                    printf("Receiver: Invalid end byte.\n");
                }
                state = 0;
                break;
        }
    }

    CloseHandle(hSerial);
    return 0;
}
