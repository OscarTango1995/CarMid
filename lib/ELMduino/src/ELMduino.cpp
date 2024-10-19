#include "ELMduino.h"

/*
 bool ELM327::begin(Stream &stream, const bool& debug, const uint16_t& timeout, const char& protocol, const uint16_t& payloadLen, const byte& dataTimeout)

 Description:
 ------------
  * Constructor for the ELM327 Class; initializes ELM327

 Inputs:
 -------
  * Stream &stream      - Reference to Serial port connected to ELM327
  * bool debug          - Specify whether or not to print debug statements to "Serial"
  * uint16_t timeout    - Time in ms to wait for a query response
  * char protocol       - Protocol ID to specify the ELM327 to communicate with the ECU over
  * uint16_t payloadLen - Maximum number of bytes expected to be returned by the ELM327 after a query
  * byte dataTimeout    - Number of ms to wait after receiving data before the ELM327 will
                          return the data - see https://www.elmelectronics.com/help/obd/tips/#UnderstandingOBD
 Return:
 -------
  * bool - Whether or not the ELM327 was properly initialized
*/
bool ELM327::begin(Stream &stream, const bool &debug, const uint16_t &timeout, const char &protocol, const uint16_t &payloadLen, const byte &dataTimeout)
{
    elm_port = &stream;
    PAYLOAD_LEN = payloadLen;
    debugMode = debug;
    timeout_ms = timeout;

    payload = (char *)malloc(PAYLOAD_LEN + 1); // allow for terminating '\0'

    // test if serial port is connected
    if (!elm_port)
        return false;

    // try to connect
    if (!initializeELM(protocol, dataTimeout))
        return false;

    return true;
}

/*
 bool ELM327::initializeELM(const char& protocol, const byte& dataTimeout)

 Description:
 ------------
  * Initializes ELM327

 Inputs:
 -------
  * char protocol    - Protocol ID to specify the ELM327 to communicate with the ECU over
  * byte dataTimeout - Number of ms to wait after receiving data before the ELM327 will
                       return the data - see https://www.elmelectronics.com/help/obd/tips/#UnderstandingOBD

 Return:
 -------
  * bool - Whether or not the ELM327 was propperly initialized

 Notes:
 ------
  * Protocol - Description
  * 0        - Automatic
  * 1        - SAE J1850 PWM (41.6 kbaud)
  * 2        - SAE J1850 PWM (10.4 kbaud)
  * 3        - ISO 9141-2 (5 baud init)
  * 4        - ISO 14230-4 KWP (5 baud init)
  * 5        - ISO 14230-4 KWP (fast init)
  * 6        - ISO 15765-4 CAN (11 bit ID, 500 kbaud)
  * 7        - ISO 15765-4 CAN (29 bit ID, 500 kbaud)
  * 8        - ISO 15765-4 CAN (11 bit ID, 250 kbaud)
  * 9        - ISO 15765-4 CAN (29 bit ID, 250 kbaud)
  * A        - SAE J1939 CAN (29 bit ID, 250* kbaud)
  * B        - User1 CAN (11* bit ID, 125* kbaud)
  * C        - User2 CAN (11* bit ID, 50* kbaud)

  * --> *user adjustable
*/
bool ELM327::initializeELM(const char &protocol, const byte &dataTimeout)
{
    char command[10] = {'\0'};
    connected = false;

    sendCommand_Blocking(SET_ALL_TO_DEFAULTS);
    delay(100);

    sendCommand_Blocking(RESET_ALL);
    delay(100);

    sendCommand_Blocking(ECHO_OFF);
    delay(100);

    sendCommand_Blocking(PRINTING_SPACES_OFF);
    delay(100);

    sendCommand_Blocking(ALLOW_LONG_MESSAGES);
    delay(100);

    // // Set data timeout
    sprintf(command, SET_TIMEOUT_TO_H_X_4MS, dataTimeout / 4);
    sendCommand_Blocking(command);
    delay(100);

    // Automatic searching for protocol requires setting the protocol to AUTO and then
    // sending an OBD command to initiate the protocol search. The OBD command "0100"
    // requests a list of supported PIDs 0x00 - 0x20 and is guaranteed to work
    if ((String)protocol == "0")
    {
        // Tell the ELM327 to do an auto protocol search. If a valid protocol is found, it will be saved to memory.
        // Some ELM clones may not have memory enabled and thus will perform the search every time.
        sprintf(command, SET_PROTOCOL_TO_AUTO_H_SAVE, protocol);
        if (sendCommand_Blocking(command) == ELM_SUCCESS)
        {
            if (strstr(payload, RESPONSE_OK) != NULL)
            {
                // Protocol search can take a comparatively long time. Temporarily set
                // the timeout value to 30 seconds, then restore the previous value.
                uint16_t prevTimeout = timeout_ms;
                timeout_ms = 30000;

                int8_t state = sendCommand_Blocking("0100");

                if (state == ELM_SUCCESS)
                {
                    timeout_ms = prevTimeout;
                    connected = true;
                    return connected;
                }
                else if (state == ELM_BUFFER_OVERFLOW)
                {
                    while (elm_port->available())
                        elm_port->read();
                }

                timeout_ms = prevTimeout;
            }
        }
    }
    else
    {
        // Set protocol
        sprintf(command, TRY_PROT_H_AUTO_SEARCH, protocol);

        if (sendCommand_Blocking(command) == ELM_SUCCESS)
        {
            if (strstr(payload, RESPONSE_OK) != NULL)
            {
                connected = true;
                return connected;
            }
        }
    }

    if (debugMode)
    {
        Serial.print(F("Setting protocol via "));
        Serial.print(TRY_PROT_H_AUTO_SEARCH);
        Serial.print(F(" did not work - trying via "));
        Serial.println(SET_PROTOCOL_TO_H_SAVE);
    }

    // Set protocol and save
    sprintf(command, SET_PROTOCOL_TO_H_SAVE, protocol);

    if (sendCommand_Blocking(command) == ELM_SUCCESS)
    {
        if (strstr(payload, RESPONSE_OK) != NULL)
        {
            connected = true;
            return connected;
        }
    }
    
    if (debugMode)
    {
        Serial.print(F("Setting protocol via "));
        Serial.print(SET_PROTOCOL_TO_H_SAVE);
        Serial.println(F(" did not work"));
    }

    return connected;
}

/*
 void ELM327::formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses)

 Description:
 ------------
  * Creates a query stack to be sent to ELM327

 Inputs:
 -------
  * uint16_t service - Service number of the queried PID
  * uint32_t pid     - PID number of the queried PID
  * uint8_t num_responses - see function header for "queryPID()"

 Return:
 -------
  * void
*/
void ELM327::formatQueryArray(uint8_t service, uint16_t pid, uint8_t num_responses)
{
    if (debugMode)
    {
        Serial.print(F("Service: "));
        Serial.println(service);
        Serial.print(F("PID: "));
        Serial.println(pid);
    }

    isMode0x22Query = (service == 0x22 && pid <= 0xFF); // mode 0x22 responses always zero-pad the pid to 4 chars, even for a 2-char pid

    query[0] = ((service >> 4) & 0xF) + '0';
    query[1] = (service & 0xF) + '0';

    // determine PID length (standard queries have 16-bit PIDs,
    // but some custom queries have PIDs with 32-bit values)
    if (pid & 0xFF00)
    {
        if (debugMode)
            Serial.println(F("Long query detected"));

        longQuery = true;

        query[2] = ((pid >> 12) & 0xF) + '0';
        query[3] = ((pid >> 8) & 0xF) + '0';
        query[4] = ((pid >> 4) & 0xF) + '0';
        query[5] = (pid & 0xF) + '0';

        if (specifyNumResponses)
        {
            if (num_responses > 0xF)
            {
                query[6] = ((num_responses >> 4) & 0xF) + '0';
                query[7] = (num_responses & 0xF) + '0';
                query[8] = '\0';

                upper(query, 8);
            }
            else
            {
                query[6] = (num_responses & 0xF) + '0';
                query[7] = '\0';
                query[8] = '\0';

                upper(query, 7);
            }
        }
        else
        {
            query[6] = '\0';
            query[7] = '\0';
            query[8] = '\0';

            upper(query, 6);
        }
    }
    else
    {
        if (debugMode)
            Serial.println(F("Normal length query detected"));

        longQuery = false;

        query[2] = ((pid >> 4) & 0xF) + '0';
        query[3] = (pid & 0xF) + '0';

        if (specifyNumResponses)
        {
            if (num_responses > 0xF)
            {
                query[4] = ((num_responses >> 4) & 0xF) + '0';
                query[5] = (num_responses & 0xF) + '0';
                query[6] = '\0';
                query[7] = '\0';
                query[8] = '\0';

                upper(query, 6);
            }
            else
            {
                query[4] = (num_responses & 0xF) + '0';
                query[5] = '\0';
                query[6] = '\0';
                query[7] = '\0';
                query[8] = '\0';

                upper(query, 5);
            }
        }
        else
        {
            query[4] = '\0';
            query[5] = '\0';
            query[6] = '\0';
            query[7] = '\0';
            query[8] = '\0';

            upper(query, 4);
        }
    }

    if (debugMode)
    {
        Serial.print(F("Query string: "));
        Serial.println(query);
    }
}

/*
 void ELM327::upper(char string[], uint8_t buflen)

 Description:
 ------------
  * Converts all elements of char array string[] to
  uppercase ascii

 Inputs:
 -------
  * uint8_t string[] - Char array
  * uint8_t buflen   - Length of char array

 Return:
 -------
  * void
*/
void ELM327::upper(char string[], uint8_t buflen)
{
    for (uint8_t i = 0; i < buflen; i++)
    {
        if (string[i] > 'Z')
            string[i] -= 32;
        else if ((string[i] > '9') && (string[i] < 'A'))
            string[i] += 7;
    }
}

/*
 bool ELM327::timeout()

 Description:
 ------------
  * Determines if a time-out has occurred

 Inputs:
 -------
  * void

 Return:
 -------
  * bool - whether or not a time-out has occurred
*/
bool ELM327::timeout()
{
    currentTime = millis();
    if ((currentTime - previousTime) >= timeout_ms)
        return true;
    return false;
}

/*
 uint8_t ELM327::ctoi(uint8_t value)

 Description:
 ------------
  * converts a decimal or hex char to an int

 Inputs:
 -------
  * uint8_t value - char to be converted

 Return:
 -------
  * uint8_t - int value of parameter "value"
*/
uint8_t ELM327::ctoi(uint8_t value)
{
    if (value >= 'A')
        return value - 'A' + 10;
    else
        return value - '0';
}

/*
 int8_t ELM327::nextIndex(char const *str,
                          char const *target,
                          uint8_t numOccur)

 Description:
 ------------
  * Finds and returns the first char index of
  numOccur'th instance of target in str

 Inputs:
 -------
  * char const *str    - string to search target within
  * char const *target - String to search for in str
  * uint8_t numOccur   - Which instance of target in str

 Return:
 -------
  * int8_t - First char index of numOccur'th
  instance of target in str. -1 if there is no
  numOccur'th instance of target in str
*/
int8_t ELM327::nextIndex(char const *str,
                         char const *target,
                         uint8_t numOccur = 1)
{
    char const *p = str;
    char const *r = str;
    uint8_t count;

    for (count = 0;; ++count)
    {
        p = strstr(p, target);

        if (count == (numOccur - 1))
            break;

        if (!p)
            break;

        p++;
    }

    if (!p)
        return -1;

    return p - r;
}

/*
 double ELM327::conditionResponse(const uint8_t &numExpectedBytes, const float &scaleFactor, const float &bias)

 Description:
 ------------
  * Converts the ELM327's response into it's correct, numerical value. Returns 0 if numExpectedBytes > numPayChars

 Inputs:
 -------
  * uint64_t response        - ELM327's response
  * uint8_t numExpectedBytes - Number of valid bytes from the response to process
  * double scaleFactor       - Amount to scale the response by
  * float bias               - Amount to bias the response by

 Return:
 -------
  * double - Converted numerical value
*/
double ELM327::conditionResponse(const uint8_t &numExpectedBytes, const double &scaleFactor, const float &bias)
{
    uint8_t numExpectedPayChars = numExpectedBytes * 2;
    uint8_t payCharDiff = numPayChars - numExpectedPayChars;

    if (numExpectedBytes > 8)
    {
        if (debugMode)
            Serial.println(F("WARNING: Number of expected response bytes is greater than 8 - returning 0"));

        return 0;
    }

    if (numPayChars < numExpectedPayChars)
    {
        if (debugMode)
            Serial.println(F("WARNING: Number of payload chars is less than the number of expected response chars returned by ELM327 - returning 0"));

        return 0;
    }
    else if (numPayChars & 0x1)
    {
        if (debugMode)
            Serial.println(F("WARNING: Number of payload chars returned by ELM327 is an odd value - returning 0"));

        return 0;
    }
    else if (numExpectedPayChars == numPayChars)
    {
        if (scaleFactor == 1 && bias == 0) // No scale/bias needed
        {
            return response;
        }
        else
        {
            return (response * scaleFactor) + bias;
        }
    }

    // If there were more payload bytes returned than we expected, test the first and last bytes in the
    // returned payload and see which gives us a higher value. Sometimes ELM327's return leading zeros
    // and others return trailing zeros. The following approach gives us the best chance at determining
    // where the real data is. Note that if the payload returns BOTH leading and trailing zeros, this
    // will not give accurate results!

    if (debugMode)
        Serial.println(F("Looking for lagging zeros"));

    uint16_t numExpectedBits = numExpectedBytes * 8;
    uint64_t laggingZerosMask = 0;

    for (uint16_t i = 0; i < numExpectedBits; i++)
        laggingZerosMask |= (1 << i);

    if (!(laggingZerosMask & response)) // Detect all lagging zeros in `response`
    {
        if (debugMode)
            Serial.println(F("Lagging zeros found"));

        if (scaleFactor == 1 && bias == 0) // No scale/bias needed
        {
            return (response >> (4 * payCharDiff));
        }
        else
        {
            return ((response >> (4 * payCharDiff)) * scaleFactor) + bias;
        }
    }
    else
    {
        if (debugMode)
            Serial.println(F("Lagging zeros not found - assuming leading zeros"));

        if (scaleFactor == 1 && bias == 0) // No scale/bias needed
        {
            return response;
        }
        else
        {
            return (response * scaleFactor) + bias;
        }
    }
}

/*
 void ELM327::flushInputBuff()

 Description:
 ------------
  * Flushes input serial buffer

 Inputs:
 -------
  * void

 Return:
 -------
  * void
*/
void ELM327::flushInputBuff()
{
    if (debugMode)
        Serial.println(F("Clearing input serial buffer"));

    while (elm_port->available())
        elm_port->read();
}

/*
  void ELM327::queryPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses)

  Description:
  ------------
  * create a PID query command string and send the command

  Inputs:
  -------
  * uint8_t service       - The diagnostic service ID. 01 is "Show current data"
  * uint16_t pid          - The Parameter ID (PID) from the service
  * uint8_t num_responses - Number of lines of data to receive - see ELM datasheet "Talking to the vehicle".
                            This can speed up retrieval of information if you know how many responses will be sent.
                            Basically the OBD scanner will not wait for more responses if it does not need to go through
                            final timeout. Also prevents OBD scanners from sending mulitple of the same response.

  Return:
  -------
  * void
*/
void ELM327::queryPID(const uint8_t &service, const uint16_t &pid, const uint8_t &num_responses)
{
    formatQueryArray(service, pid, num_responses);
    sendCommand(query);
}

/*
 void ELM327::queryPID(char queryStr[])

 Description:
 ------------
  * Queries ELM327 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * char queryStr[] - Query string (service and PID)

 Return:
 -------
  * void
*/
void ELM327::queryPID(char queryStr[])
{
    if (strlen(queryStr) <= 4)
        longQuery = false;
    else
        longQuery = true;

    sendCommand(queryStr);
}

/*
 double ELM327::processPID(const uint8_t& service, const uint16_t& pid, const uint8_t& num_responses, const uint8_t& numExpectedBytes, const float& scaleFactor, const float& bias)

 Description:
 ------------
  * Queries ELM327 for a specific type of vehicle telemetry data

 Inputs:
 -------
  * uint8_t service          - The diagnostic service ID. 01 is "Show current data"
  * uint16_t pid             - The Parameter ID (PID) from the service
  * uint8_t num_responses    - Number of lines of data to receive - see ELM datasheet "Talking to the vehicle".
                               This can speed up retrieval of information if you know how many responses will be sent.
                               Basically the OBD scanner will not wait for more responses if it does not need to go through
                               final timeout. Also prevents OBD scanners from sending mulitple of the same response.
  * uint8_t numExpectedBytes - Number of valid bytes from the response to process
  * float scaleFactor        - Amount to scale the response by
  * float bias               - Amount to bias the response by

 Return:
 -------
  * double - The PID value if successfully received, else 0.0
*/

double ELM327::processPID(const uint8_t &service, const uint16_t &pid, const uint8_t &num_responses, const uint8_t &numExpectedBytes, const double &scaleFactor, const float &bias)
{
    // Send the query
    queryPID(service, pid, num_responses);

    // Wait for response by continuously checking
    while (true)
    {
        // Call get_response() to check if a message has been received
        get_response();

        // If the response is successful, process and return the value
        if (nb_rx_state == ELM_SUCCESS)
        {
            // Find and process the response
            findResponse(service, pid);
            return conditionResponse(numExpectedBytes, scaleFactor, bias);
        }
        // If the response is not successful and is not still waiting (ELM_GETTING_MSG), return an error (e.g., 0)
        else if (nb_rx_state != ELM_GETTING_MSG)
        {
            return 0.0; // Return 0 or any other error indicator
        }

        // Optionally add a short delay here to avoid continuously hammering the loop
        delay(10);
    }
}

// double ELM327::processPID(const uint8_t &service, const uint16_t &pid, const uint8_t &num_responses, const uint8_t &numExpectedBytes, const double &scaleFactor, const float &bias)
// {
//     if (nb_query_state == SEND_COMMAND)
//     {
//         queryPID(service, pid, num_responses);
//         nb_query_state = WAITING_RESP;
//     }
//     else if (nb_query_state == WAITING_RESP)
//     {
//         get_response();
//         if (nb_rx_state == ELM_SUCCESS)
//         {
//             nb_query_state = SEND_COMMAND; // Reset the query state machine for next command

//             findResponse(service, pid);

//             return conditionResponse(numExpectedBytes, scaleFactor, bias);
//         }
//         else if (nb_rx_state != ELM_GETTING_MSG)
//             nb_query_state = SEND_COMMAND; // Error or timeout, so reset the query state machine for next command
//     }
//     return 0.0;
// }

/*
 uint32_t ELM327::supportedPIDs_1_20()

 Description:
 ------------
  * Determine which of PIDs 0x1 through 0x20 are supported (bit encoded)

 Inputs:
 -------
  * void

 Return:
 -------
  * uint32_t - Bit encoded booleans of supported PIDs 0x1-0x20
*/
uint32_t ELM327::supportedPIDs_1_20()
{
    return (uint32_t)processPID(SERVICE_01, SUPPORTED_PIDS_1_20, 1, 4);
}

/*
 float ELM327::engineLoad()

 Description:
 ------------
  * Find the current engine load in %

 Inputs:
 -------
  * void

 Return:
 -------
  * float - Engine load %
*/
float ELM327::engineLoad()
{
    return processPID(SERVICE_01, ENGINE_LOAD, 1, 1, 100.0 / 255.0);
}

/*
 float ELM327::engineCoolantTemp()

 Description:
 ------------
  * Find the current engine coolant temp in C

 Inputs:
 -------
  * void

 Return:
 -------
  * float - Engine load %
*/
float ELM327::engineCoolantTemp()
{
    return processPID(SERVICE_01, ENGINE_COOLANT_TEMP, 1, 1, 1, -40.0);
}

/*
 uint8_t ELM327::manifoldPressure()

 Description:
 ------------
  * Find intake manifold absolute pressure in kPa

 Inputs:
 -------
  * void

 Return:
 -------
  * uint8_t - Intake manifold absolute pressure in kPa
*/
uint8_t ELM327::manifoldPressure()
{
    return (uint8_t)processPID(SERVICE_01, INTAKE_MANIFOLD_ABS_PRESSURE, 1, 1);
}

/*
 float ELM327::rpm()

 Description:
 ------------
  * Queries and parses received message for/returns vehicle RMP data

 Inputs:
 -------
  * void

 Return:
 -------
  * float - Vehicle RPM
*/
float ELM327::rpm()
{
    return processPID(SERVICE_01, ENGINE_RPM, 1, 2, 1.0 / 4.0);
}

/*
 int32_t ELM327::kph()

 Description:
 ------------
  *  Queries and parses received message for/returns vehicle speed data (kph)

 Inputs:
 -------
  * void

 Return:
 -------
  * int32_t - Vehicle speed in kph
*/
int32_t ELM327::kph()
{
    return (int32_t)processPID(SERVICE_01, VEHICLE_SPEED, 1, 1);
}

/*
 float ELM327::throttle()

 Description:
 ------------
  *  Find throttle position in %

 Inputs:
 -------
  * void

 Return:
 -------
  * float - Throttle position in %
*/
float ELM327::throttle()
{
    return processPID(SERVICE_01, THROTTLE_POSITION, 1, 1, 100.0 / 255.0);
}

/*
 void ELM327::sendCommand(const char *cmd)

 Description:
 ------------
  * Sends a command/query for Non-Blocking PID queries

 Inputs:
 -------
  * const char *cmd - Command/query to send to ELM327

 Return:
 -------
  * void
*/
void ELM327::sendCommand(const char *cmd)
{
    // clear payload buffer
    memset(payload, '\0', PAYLOAD_LEN + 1);

    // reset input serial buffer and number of received bytes
    recBytes = 0;
    flushInputBuff();
    connected = false;

    // Reset the receive state ready to start receiving a response message
    nb_rx_state = ELM_GETTING_MSG;

    if (debugMode)
    {
        Serial.print(F("Sending the following command/query: "));
        Serial.println(cmd);
    }

    elm_port->print(cmd);
    elm_port->print('\r');

    // prime the timeout timer
    previousTime = millis();
    currentTime = previousTime;
}

/*
 obd_rx_states ELM327::sendCommand_Blocking(const char* cmd)

 Description:
 ------------
    * Sends a command/query and waits for a respoonse (blocking function)
    Sometimes it's desirable to use a blocking command, e.g when sending an AT command.
    This function removes the need for the caller to set up a loop waiting for the command to finish.
    Caller is free to parse the payload string if they need to use the response.

 Inputs:
 -------
  * const char *cmd - Command/query to send to ELM327

 Return:
 -------
  * int8_t - the ELM_XXX status of getting the OBD response
*/
int8_t ELM327::sendCommand_Blocking(const char *cmd)
{
    sendCommand(cmd);
    while (get_response() == ELM_GETTING_MSG)
        ;
    return nb_rx_state;
}

/*
 obd_rx_states ELM327::get_response(void)

 Description:
 ------------
  * Non Blocking (NB) receive OBD scanner response. Must be called repeatedly until
    the status progresses past ELM_GETTING_MSG.

 Inputs:
 -------
  * void

 Return:
 -------
  * int8_t - the ELM_XXX status of getting the OBD response
*/
int8_t ELM327::get_response(void)
{
    // buffer the response of the ELM327 until either the
    // end marker is read or a timeout has occurred
    // last valid idx is PAYLOAD_LEN but want to keep one free for terminating '\0'
    // so limit counter to < PAYLOAD_LEN
    if (!elm_port->available())
    {
        nb_rx_state = ELM_GETTING_MSG;
        if (timeout())
            nb_rx_state = ELM_TIMEOUT;
    }
    else
    {
        char recChar = elm_port->read();

        if (debugMode)
        {
            Serial.print(F("\tReceived char: "));
            // display each received character, make non-printables printable
            if (recChar == '\f')
                Serial.println(F("\\f"));
            else if (recChar == '\n')
                Serial.println(F("\\n"));
            else if (recChar == '\r')
                Serial.println(F("\\r"));
            else if (recChar == '\t')
                Serial.println(F("\\t"));
            else if (recChar == '\v')
                Serial.println(F("\\v"));
            // convert spaces to underscore, easier to see in debug output
            else if (recChar == ' ')
                Serial.println(F("_"));
            // display regular printable
            else
                Serial.println(recChar);
        }

        // this is the end of the OBD response
        if (recChar == '>')
        {
            if (debugMode)
                Serial.println(F("Delimiter found."));

            nb_rx_state = ELM_MSG_RXD;
        }
        else if (!isalnum(recChar) && (recChar != ':') && (recChar != '.'))
            // discard all characters except for alphanumeric and decimal places.
            // decimal places needed to extract floating point numbers, e.g. battery voltage
            nb_rx_state = ELM_GETTING_MSG; // Discard this character
        else
        {
            if (recBytes < PAYLOAD_LEN)
            {
                payload[recBytes] = recChar;
                recBytes++;
                nb_rx_state = ELM_GETTING_MSG;
            }
            else
                nb_rx_state = ELM_BUFFER_OVERFLOW;
        }
    }

    // Message is still being received (or is timing out), so exit early without doing all the other checks
    if (nb_rx_state == ELM_GETTING_MSG)
        return nb_rx_state;

    // End of response delimiter was found
    if (debugMode && nb_rx_state == ELM_MSG_RXD)
    {
        Serial.print(F("All chars received: "));
        Serial.println(payload);
    }

    if (nb_rx_state == ELM_TIMEOUT)
    {
        if (debugMode)
        {
            Serial.print(F("Timeout detected with overflow of "));
            Serial.print((currentTime - previousTime) - timeout_ms);
            Serial.println(F("ms"));
        }
        return nb_rx_state;
    }

    if (nb_rx_state == ELM_BUFFER_OVERFLOW)
    {
        if (debugMode)
        {
            Serial.print(F("OBD receive buffer overflow (> "));
            Serial.print(PAYLOAD_LEN);
            Serial.println(F(" bytes)"));
        }
        return nb_rx_state;
    }

    // Now we have successfully received OBD response, check if the payload indicates any OBD errors
    if (nextIndex(payload, RESPONSE_UNABLE_TO_CONNECT) >= 0)
    {
        if (debugMode)
            Serial.println(F("ELM responded with error \"UNABLE TO CONNECT\""));

        nb_rx_state = ELM_UNABLE_TO_CONNECT;
        return nb_rx_state;
    }

    connected = true;

    if (nextIndex(payload, RESPONSE_NO_DATA) >= 0)
    {
        if (debugMode)
            Serial.println(F("ELM responded with error \"NO DATA\""));

        nb_rx_state = ELM_NO_DATA;
        return nb_rx_state;
    }

    if (nextIndex(payload, RESPONSE_STOPPED) >= 0)
    {
        if (debugMode)
            Serial.println(F("ELM responded with error \"STOPPED\""));

        nb_rx_state = ELM_STOPPED;
        return nb_rx_state;
    }

    if (nextIndex(payload, RESPONSE_ERROR) >= 0)
    {
        if (debugMode)
            Serial.println(F("ELM responded with \"ERROR\""));

        nb_rx_state = ELM_GENERAL_ERROR;
        return nb_rx_state;
    }

    nb_rx_state = ELM_SUCCESS;
    return nb_rx_state;
}

/*
 uint64_t ELM327::findResponse(uint8_t &service)

 Description:
 ------------
  * Parses the buffered ELM327's response and returns the queried data

 Inputs:
 -------
  * void

 Return:
 -------
  * uint64_t - Query response value
*/
uint64_t ELM327::findResponse(const uint8_t &service, const uint8_t &pid)
{
    uint8_t firstDatum = 0;
    char header[7] = {'\0'};

    if (longQuery)
    {
        header[0] = query[0] + 4;
        header[1] = query[1];
        header[2] = query[2];
        header[3] = query[3];
        header[4] = query[4];
        header[5] = query[5];
    }
    else
    {
        header[0] = query[0] + 4;
        header[1] = query[1];
        if (isMode0x22Query) // mode 0x22 responses always zero-pad the pid to 4 chars, even for a 2-char pid
        {
            header[2] = '0';
            header[3] = '0';
            header[4] = query[2];
            header[5] = query[3];
        }
        else
        {
            header[2] = query[2];
            header[3] = query[3];
        }
    }

    if (debugMode)
    {
        Serial.print(F("Expected response header: "));
        Serial.println(header);
    }

    int8_t firstHeadIndex = nextIndex(payload, header);
    int8_t secondHeadIndex = nextIndex(payload, header, 2);

    if (firstHeadIndex >= 0)
    {
        if (longQuery | isMode0x22Query)
            firstDatum = firstHeadIndex + 6;
        else
            firstDatum = firstHeadIndex + 4;

        // Some ELM327s (such as my own) respond with two
        // "responses" per query. "numPayChars" represents the
        // correct number of bytes returned by the ELM327
        // regardless of how many "responses" were returned
        if (secondHeadIndex >= 0)
        {
            if (debugMode)
                Serial.println(F("Double response detected"));

            numPayChars = secondHeadIndex - firstDatum;
        }
        else
        {
            if (debugMode)
                Serial.println(F("Single response detected"));

            numPayChars = recBytes - firstDatum;
        }

        response = 0;
        for (uint8_t i = 0; i < numPayChars; i++)
        {
            uint8_t payloadIndex = firstDatum + i;
            uint8_t bitsOffset = 4 * (numPayChars - i - 1);
            response = response | ((uint64_t)ctoi(payload[payloadIndex]) << bitsOffset);
        }

        // It is useful to have the response bytes
        // broken-out because some PID algorithms (standard
        // and custom) require special operations for each
        // byte returned
        responseByte_0 = response & 0xFF;
        responseByte_1 = (response >> 8) & 0xFF;
        responseByte_2 = (response >> 16) & 0xFF;
        responseByte_3 = (response >> 24) & 0xFF;
        responseByte_4 = (response >> 32) & 0xFF;
        responseByte_5 = (response >> 40) & 0xFF;
        responseByte_6 = (response >> 48) & 0xFF;
        responseByte_7 = (response >> 56) & 0xFF;

        if (debugMode)
        {
            Serial.println(F("64-bit response: "));
            Serial.print(F("\tresponseByte_0: "));
            Serial.println(responseByte_0);
            Serial.print(F("\tresponseByte_1: "));
            Serial.println(responseByte_1);
            Serial.print(F("\tresponseByte_2: "));
            Serial.println(responseByte_2);
            Serial.print(F("\tresponseByte_3: "));
            Serial.println(responseByte_3);
            Serial.print(F("\tresponseByte_4: "));
            Serial.println(responseByte_4);
            Serial.print(F("\tresponseByte_5: "));
            Serial.println(responseByte_5);
            Serial.print(F("\tresponseByte_6: "));
            Serial.println(responseByte_6);
            Serial.print(F("\tresponseByte_7: "));
            Serial.println(responseByte_7);
        }

        return response;
    }

    if (debugMode)
        Serial.println(F("Response not detected"));

    return 0;
}

/*
 void ELM327::printError()

 Description:
 ------------
  * Prints appropriate error description if an error has occurred

 Inputs:
 -------
  * void

 Return:
 -------
  * void
*/
void ELM327::printError()
{
    Serial.print(F("Received: "));
    Serial.println(payload);

    if (nb_rx_state == ELM_SUCCESS)
        Serial.println(F("ELM_SUCCESS"));
    else if (nb_rx_state == ELM_NO_RESPONSE)
        Serial.println(F("ERROR: ELM_NO_RESPONSE"));
    else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
        Serial.println(F("ERROR: ELM_BUFFER_OVERFLOW"));
    else if (nb_rx_state == ELM_UNABLE_TO_CONNECT)
        Serial.println(F("ERROR: ELM_UNABLE_TO_CONNECT"));
    else if (nb_rx_state == ELM_NO_DATA)
        Serial.println(F("ERROR: ELM_NO_DATA"));
    else if (nb_rx_state == ELM_STOPPED)
        Serial.println(F("ERROR: ELM_STOPPED"));
    else if (nb_rx_state == ELM_TIMEOUT)
        Serial.println(F("ERROR: ELM_TIMEOUT"));
    else if (nb_rx_state == ELM_BUFFER_OVERFLOW)
        Serial.println(F("ERROR: BUFFER OVERFLOW"));
    else if (nb_rx_state == ELM_GENERAL_ERROR)
        Serial.println(F("ERROR: ELM_GENERAL_ERROR"));
    else
        Serial.println(F("No error detected"));

    delay(100);
}

/*
 bool ELM327::isPidSupported(uint8_t pid)

 Description:
 ------------
  * Checks if a particular PID is supported by the connected ECU.

  * This is a convenience method that selects the correct supportedPIDS_xx_xx() query and parses
    the bit-encoded result, returning a simple Boolean value indicating PID support from the ECU.

 Inputs:
 -------
  * uint8_t pid - the PID to check for support.

 Return:
 -------
  * bool - Whether or not the queried PID is supported by the ECU.
*/
bool ELM327::isPidSupported(uint8_t pid)
{
    uint8_t pidInterval = (pid / PID_INTERVAL_OFFSET) * PID_INTERVAL_OFFSET;

    switch (pidInterval)
    {
    case SUPPORTED_PIDS_1_20:
        supportedPIDs_1_20();
        break;
    default:
        break;
    }

    if (nb_rx_state == ELM_SUCCESS)
    {
        return ((response >> (32 - pid)) & 0x1);
    }
    return false;
}
