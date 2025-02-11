/******************************************************************************
* File Name:   mqtt_client_config.h
*
* Description: This file contains all the configuration macros used by the
*              MQTT client in this example.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef MQTT_CLIENT_CONFIG_H_
#define MQTT_CLIENT_CONFIG_H_

#include "cy_mqtt_api.h"

/*******************************************************************************
* Macros
********************************************************************************/

/***************** MQTT CLIENT CONNECTION CONFIGURATION MACROS *****************/
/* MQTT Broker/Server address and port used for the MQTT connection. */
#define MQTT_BROKER_ADDRESS               "46d1a266bf304f3692ae6246fc153715.s1.eu.hivemq.cloud"
#define MQTT_PORT                         8883

/* Set this macro to 1 if a secure (TLS) connection to the MQTT Broker is
 * required to be established, else 0.
 */
#define MQTT_SECURE_CONNECTION            ( 1 )

/* Configure the user credentials to be sent as part of MQTT CONNECT packet */
#define MQTT_USERNAME                     "Test1234"
#define MQTT_PASSWORD                     "Test1234"


/********************* MQTT MESSAGE CONFIGURATION MACROS **********************/
/* The MQTT topics to be used by the publisher and subscriber. */
#define MQTT_PUB_TOPIC                    "ledstatus"
#define MQTT_SUB_TOPIC                    "ledstatus"

/* Set the QoS that is associated with the MQTT publish, and subscribe messages.
 * Valid choices are 0, 1, and 2. Other values should not be used in this macro.
 */
#define MQTT_MESSAGES_QOS                 ( 1 )

/* Configuration for the 'Last Will and Testament (LWT)'. It is an MQTT message
 * that will be published by the MQTT broker if the MQTT connection is
 * unexpectedly closed. This configuration is sent to the MQTT broker during
 * MQTT connect operation and the MQTT broker will publish the Will message on
 * the Will topic when it recognizes an unexpected disconnection from the client.
 *
 * If you want to use the last will message, set this macro to 1 and configure
 * the topic and will message, else 0.
 */
#define ENABLE_LWT_MESSAGE                ( 0 )
#if ENABLE_LWT_MESSAGE
    #define MQTT_WILL_TOPIC_NAME          MQTT_PUB_TOPIC "/will"
    #define MQTT_WILL_MESSAGE             ("MQTT client unexpectedly disconnected!")
#endif

/* MQTT messages which are published on the MQTT_PUB_TOPIC that controls the
 * device (user LED in this example) state in this code example.
 */
#define MQTT_DEVICE_ON_MESSAGE            "TURN ON"
#define MQTT_DEVICE_OFF_MESSAGE           "TURN OFF"


/******************* OTHER MQTT CLIENT CONFIGURATION MACROS *******************/
/* A unique client identifier to be used for every MQTT connection. */
#define MQTT_CLIENT_IDENTIFIER            "psoc6-mqtt-client"

/* The timeout in milliseconds for MQTT operations in this example. */
#define MQTT_TIMEOUT_MS                   ( 5000 )

/* The keep-alive interval in seconds used for MQTT ping request. */
#define MQTT_KEEP_ALIVE_SECONDS           ( 60 )

/* Every active MQTT connection must have a unique client identifier. If you
 * are using the above 'MQTT_CLIENT_IDENTIFIER' as client ID for multiple MQTT
 * connections simultaneously, set this macro to 1. The device will then
 * generate a unique client identifier by appending a timestamp to the
 * 'MQTT_CLIENT_IDENTIFIER' string. Example: 'psoc6-mqtt-client5927'
 */
#define GENERATE_UNIQUE_CLIENT_ID         ( 1 )

/* The longest client identifier that an MQTT server must accept (as defined
 * by the MQTT 3.1.1 spec) is 23 characters. However some MQTT brokers support
 * longer client IDs. Configure this macro as per the MQTT broker specification.
 */
#define MQTT_CLIENT_IDENTIFIER_MAX_LEN    ( 23 )

/* As per Internet Assigned Numbers Authority (IANA) the port numbers assigned
 * for MQTT protocol are 1883 for non-secure connections and 8883 for secure
 * connections. In some cases there is a need to use other ports for MQTT like
 * port 443 (which is reserved for HTTPS). Application Layer Protocol
 * Negotiation (ALPN) is an extension to TLS that allows many protocols to be
 * used over a secure connection. The ALPN ProtocolNameList specifies the
 * protocols that the client would like to use to communicate over TLS.
 *
 * This macro specifies the ALPN Protocol Name to be used that is supported
 * by the MQTT broker in use.
 * Note: For AWS IoT, currently "x-amzn-mqtt-ca" is the only supported ALPN
 *       ProtocolName and it is only supported on port 443.
 *
 * Uncomment the below line and specify the ALPN Protocol Name to use this
 * feature.
 */
// #define MQTT_ALPN_PROTOCOL_NAME           "x-amzn-mqtt-ca"

/* Server Name Indication (SNI) is extension to the Transport Layer Security
 * (TLS) protocol. As required by some MQTT Brokers, SNI typically includes the
 * hostname in the Client Hello message sent during TLS handshake.
 *
 * Specify the SNI Host Name to use this extension
 * as specified by the MQTT Broker.
 */
#define MQTT_SNI_HOSTNAME                 (MQTT_BROKER_ADDRESS)

/* A Network buffer is allocated for sending and receiving MQTT packets over
 * the network. Specify the size of this buffer using this macro.
 *
 * Note: The minimum buffer size is defined by 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE'
 * macro in the MQTT library. Please ensure this macro value is larger than
 * 'CY_MQTT_MIN_NETWORK_BUFFER_SIZE'.
 */
#define MQTT_NETWORK_BUFFER_SIZE          ( 2 * CY_MQTT_MIN_NETWORK_BUFFER_SIZE )

/* Maximum MQTT connection re-connection limit. */
#define MAX_MQTT_CONN_RETRIES            (150u)

/* MQTT re-connection time interval in milliseconds. */
#define MQTT_CONN_RETRY_INTERVAL_MS      (2000)


/**************** MQTT CLIENT CERTIFICATE CONFIGURATION MACROS ****************/

/* Configure the below credentials in case of a secure MQTT connection. */
/* PEM-encoded client certificate */
#define CLIENT_CERTIFICATE      \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDyTCCArGgAwIBAgIUIeZ05m3Vd2bzRp3ca4qt6dAeATYwDQYJKoZIhvcNAQEL\n"\
"BQAwgYwxCzAJBgNVBAYTAkJFMRAwDgYDVQQIDAdMaW1idXJnMRUwEwYDVQQHDAxI\n"\
"ZXJrLWRlLVN0YWQxETAPBgNVBAoMCHVoYXNzZWx0MQwwCgYDVQQLDANpaXcxDjAM\n"\
"BgNVBAMMBWJyZW50MSMwIQYJKoZIhvcNAQkBFhR2cmV5c2JyZW50QGdtYWlsLmNv\n"\
"bTAeFw0yNDEwMTgxNTEzNDFaFw0yNTEwMTgxNTEzNDFaMIGMMQswCQYDVQQGEwJC\n"\
"RTEQMA4GA1UECAwHTGltYnVyZzEVMBMGA1UEBwwMSGVyay1kZS1TdGFkMREwDwYD\n"\
"VQQKDAh1aGFzc2VsdDEMMAoGA1UECwwDaWl3MQ4wDAYDVQQDDAVicmVudDEjMCEG\n"\
"CSqGSIb3DQEJARYUdnJleXNicmVudEBnbWFpbC5jb20wggEiMA0GCSqGSIb3DQEB\n"\
"AQUAA4IBDwAwggEKAoIBAQDsAVI3sEN/8LFCOYVVTD1vuz/iwaWxHEif18+pgM3K\n"\
"YaPQdI3wLOUMmmr2f3Z7PwEeHh7cwpyki9uYR9oSgEt4BtPE5KNADc4LIMSa9xO5\n"\
"s+uVKp6cScsQqEMzLxWuK8Q4uqW3sGyR4Y2NjY+/M0Un3Uv0eO6L9a2o00r9Wsam\n"\
"JCrSl8I2MYufJ8vUzFUWR5S9FSAoaRsIsEdXet5i4xNIt5XMI0vsccNy4zbnR64X\n"\
"uyDmtiSPE1xUmNM+T/LZboZGd1gra9NzQNuedNTEzEQpG0lhCg+oYqimw6GwX8Kt\n"\
"qIy3p4/esFL9aRnC6QNVh8cJG3qhvtWRDf78suSWQlJzAgMBAAGjITAfMB0GA1Ud\n"\
"DgQWBBSCn0B2y8h0OS8y8DvsneUdFPVMvzANBgkqhkiG9w0BAQsFAAOCAQEAQtT2\n"\
"NT+vw9VTZxiBFFD2wlSiNzJ/EpIo91XSQT+3zn2MUkttSls+tfkM9l+R7DiKXMQh\n"\
"7MbYdFBxkJxwFYS8D/+wIghfGVFJHSu13P/GSDW5dKaOWDj3224Zkx0G47bBQkHw\n"\
"EZZFZiOUgeELfBPbjJJ2mt6xrn8xfR63MxAgWnI802RlsQpx6PBC2I2fcKKAHbIu\n"\
"5PVpKzuj02e2uoD5EKVEEP8Vt4vGIwPVxAqQQdWextD8MfC/Q/i+8XVnR8zyKKvl\n"\
"HkI0Soq22EJiS6IOVjO8QiBBCVq54ZITIEEfu41vcuBEikmBqOUWqMLD8IdyS7lc\n"\
"8O36R7FGD8EVmrNXvw==\n"\
"-----END CERTIFICATE-----"

/* PEM-encoded client private key */
#define CLIENT_PRIVATE_KEY          \
"-----BEGIN PRIVATE KEY-----\n"\
"MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDsAVI3sEN/8LFC\n"\
"OYVVTD1vuz/iwaWxHEif18+pgM3KYaPQdI3wLOUMmmr2f3Z7PwEeHh7cwpyki9uY\n"\
"R9oSgEt4BtPE5KNADc4LIMSa9xO5s+uVKp6cScsQqEMzLxWuK8Q4uqW3sGyR4Y2N\n"\
"jY+/M0Un3Uv0eO6L9a2o00r9WsamJCrSl8I2MYufJ8vUzFUWR5S9FSAoaRsIsEdX\n"\
"et5i4xNIt5XMI0vsccNy4zbnR64XuyDmtiSPE1xUmNM+T/LZboZGd1gra9NzQNue\n"\
"dNTEzEQpG0lhCg+oYqimw6GwX8KtqIy3p4/esFL9aRnC6QNVh8cJG3qhvtWRDf78\n"\
"suSWQlJzAgMBAAECggEAWCcwLjPtKptdoqZ4qsFFC3iOZuU9PjgZ2nbSjvZZJ/ir\n"\
"pz/6CpXCYqBFRtiaiXOht725em1I9F93QVzJVHLOfcFTFa7vWYwug9KUDWwv1oQh\n"\
"gYC/aRJZQtoO683xE7MuYqA5KTFBhIYTCSYw5NLCSJ8s8V7Af8/fM3nk/1iKbeDJ\n"\
"0q39PChL/yaA7IrYvS5/WaCoCvf9qqu/5NdQLem50zb4/TEnOXkny1VIW73tEold\n"\
"R5JpWh+JXHyIGKB3ZxHfOOdJWqddXD4qUxDBszSBFJ1RUJA/+hXmyVz/xnG4Kn47\n"\
"25FHZgud5hOxTnYAN5QGXaYyf9bh5UqB0a6wRBn1LQKBgQD9+uDII+mGuBuKRqwb\n"\
"MaAe3KxgLETo3cPZTnpcwiNyrKe363JheNRaJmx24JFs7fm3woc1ocXdFM4ejk2Q\n"\
"jO2z+D4kbl5LSxj5/RfS5030lqbiUYgn3pTWchuo+j4Pr1qd58gRN0RmfInDQOWg\n"\
"9KvwtKo7oz/BqHOgTe+PIq48bQKBgQDt4dhT0sbD5dhhNdK96/Ys6rxyVGosJcYk\n"\
"tbg87cFJpM0w+nPD3ynXFC33r+5DSqlSwIORigCktAvZ87ZFtlZ5oFkQabPUng+q\n"\
"cUXIepOq8fy0QsKWCA9kdaHbh52fRJOBJNSHuQGPERln2wq5SADftWfhLsPpzye3\n"\
"IyxUdyW+XwKBgQCyDMa4iDSWdUsZTSIqjkeqWxmc8mv8tDswhIvTq2YeZlvN3Al7\n"\
"u7Fi4bz4Oca5jruZz43qU1ba/lLhqPFHfQTHPiiSca05K5R+7Kde+zjzVwn3byD8\n"\
"klXVYHlzpLsNU+k6+dqP9ybEklohv8U7aUuAm6cb1/5CTU2B/CENP0rWoQKBgQCz\n"\
"o7CspGiLuT/tMSsjV1TBWMbUg+Dl0+TNBhnKkTU8+uIlkYzoptc+37NS7527XV9+\n"\
"6Xix6IhR2NF2VvAXtvmYwblghqJR9scXzJCJM9/d4P4X1q6BhDazg42mbcRfaPcH\n"\
"OJvynoiJ1Lzdy/FOOAzI44I98AXsxA5W443vf0aWewKBgQC5VLffXx4vpshK9JHH\n"\
"P6ffU/rJ4sFRy8P9EWWhIFg4/RKQo78agqpvaypB3LFS79hicSVsUIsioYs8AJVK\n"\
"ao2bV9TxM65kNMlgTb9KTA6pL+VzIbvxmrZXgcIe9df8dIhJPtAJnKNyxtxy40A+\n"\
"lTM4+Jhz7E5iDwGXRevI7d1J4A==\n"\
"-----END PRIVATE KEY-----"

/* PEM-encoded Root CA certificate */
#define ROOT_CA_CERTIFICATE     \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----"


/******************************************************************************
* Global Variables
*******************************************************************************/
extern cy_mqtt_broker_info_t broker_info;
extern cy_awsport_ssl_credentials_t  *security_info;
extern cy_mqtt_connect_info_t connection_info;


#endif /* MQTT_CLIENT_CONFIG_H_ */
