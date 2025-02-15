#ifndef DRIVER_AS608_H
#define DRIVER_AS608_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define AS608_HEADER 0xEF01
#define AS608_ADDRESS 0xFFFFFFFF
#define  AS608_MAX_PACKET_SIZE 64

/**
 * @brief chip command definition
 */
#define AS608_COMMAND_GET_IMAGE             0x01        /**< get image command */
#define AS608_COMMAND_GEN_CHAR              0x02        /**< generate char command */
#define AS608_COMMAND_MATCH                 0x03        /**< match command */
#define AS608_COMMAND_SEARCH                0x04        /**< search command */
#define AS608_COMMAND_REG_MODEL             0x05        /**< reg model command */
#define AS608_COMMAND_STORE_CHAR            0x06        /**< store char command */
#define AS608_COMMAND_LOAD_CHAR             0x07        /**< load char command */
#define AS608_COMMAND_UP_CHAR               0x08        /**< up char command */
#define AS608_COMMAND_DOWN_CHAR             0x09        /**< down char command */
#define AS608_COMMAND_UP_IMAGE              0x0A        /**< up image command */
#define AS608_COMMAND_DOWN_IMAGE            0x0B        /**< down image command */
#define AS608_COMMAND_DELETE_CHAR           0x0C        /**< delete char command */
#define AS608_COMMAND_EMPTY                 0x0D        /**< empty command */
#define AS608_COMMAND_WRITE_REG             0x0E        /**< write reg command */
#define AS608_COMMAND_READ_SYS_PARA         0x0F        /**< read sys para command */
#define AS608_COMMAND_ENROLL                0x10        /**< enroll command */
#define AS608_COMMAND_IDENTIFY              0x11        /**< identify command */
#define AS608_COMMAND_SET_PWD               0x12        /**< set password command */
#define AS608_COMMAND_VFY_PWD               0x13        /**< verify password command */
#define AS608_COMMAND_GET_RANDOM_CODE       0x14        /**< get random code command */
#define AS608_COMMAND_SET_CHIP_ADDR         0x15        /**< set chip addr command */
#define AS608_COMMAND_READ_INFO_PAGE        0x16        /**< read info page command */
#define AS608_COMMAND_PORT_CONTROL          0x17        /**< port control command */
#define AS608_COMMAND_WRITE_NOTEPAD         0x18        /**< write notepad command */
#define AS608_COMMAND_READ_NOTEPAD          0x19        /**< read notepad command */
#define AS608_COMMAND_BURN_CODE             0x1A        /**< burn code command */
#define AS608_COMMAND_HIGH_SPEED_SEARCH     0x1B        /**< high speed search command */
#define AS608_COMMAND_GEN_BIN_IMAGE         0x1C        /**< generate bin image command */
#define AS608_COMMAND_VALID_TEMPLATE_NUM    0x1D        /**< valid template num command */
#define AS608_COMMAND_USER_GPIO             0x1E        /**< user gpio command */
#define AS608_COMMAND_READ_INDEX_TABLE      0x1F        /**< read index table command */

/**
 * @brief chip type definition
 */
#define AS608_TYPE_COMMAND         0x01        /**< command type */
#define AS608_TYPE_DATA            0x02        /**< data type */
#define AS608_TYPE_RESPONSE        0x07        /**< response type */
#define AS608_TYPE_END             0x08        /**< end type */

#pragma pack(1)  // 按1字节对齐
typedef struct {
	uint16_t Head;
	uint32_t Address;
	uint8_t Identifier;
	uint16_t Length;
	uint8_t ConfirmCode;
	uint8_t Params[32];
	uint16_t CheckSum;
} As608_PacketInfo_t;
#pragma pack()

typedef enum
{
    AS608_BOOL_FALSE = 0x00,        /**< disable */
    AS608_BOOL_TRUE  = 0x01,        /**< enable */
} as608_bool_t;

typedef enum
{
    AS608_LEVEL_1 = 0x0001,        /**< level 1 */
    AS608_LEVEL_2 = 0x0002,        /**< level 2 */
    AS608_LEVEL_3 = 0x0003,        /**< level 3 */
    AS608_LEVEL_4 = 0x0004,        /**< level 4 */
    AS608_LEVEL_5 = 0x0005,        /**< level 5 */
} as608_level_t;

typedef enum
{
    AS608_PACKET_SIZE_32_BYTES  = 0x0000,        /**< 32 bytes */
    AS608_PACKET_SIZE_64_BYTES  = 0x0001,        /**< 64 bytes */
    AS608_PACKET_SIZE_128_BYTES = 0x0002,        /**< 128 bytes */
    AS608_PACKET_SIZE_256_BYTES = 0x0003,        /**< 256 bytes */
} as608_packet_size_t;

typedef enum
{
    AS608_BUFFER_NUMBER_1 = 0x01,        /**< buffer 1 */
    AS608_BUFFER_NUMBER_2 = 0x02,        /**< buffer 2 */
} as608_buffer_number_t;

typedef enum
{
    AS608_SENSOR_TYPE_FPC1011C = 0x0000,        /**< fpc1011c */
    AS608_SENSOR_TYPE_C500     = 0x0002,        /**< c500 */
    AS608_SENSOR_TYPE_S500     = 0x0003,        /**< s500*/
    AS608_SENSOR_TYPE_XWSEMI   = 0x0007,        /**< xwsemi */
    AS608_SENSOR_TYPE_CUSTOM   = 0x0009,        /**< custom */
} as608_sensor_type_t;

typedef enum
{
    AS608_BURN_CODE_MODE_INFO = 0x00,        /**< only flash info */
    AS608_BURN_CODE_MODE_FULL = 0x01,        /**< full */
} as608_burn_code_mode_t;

typedef enum
{
    AS608_IMAGE_BIN        = 0x00,        /**< bin image */
    AS608_IMAGE_NO_FEATURE = 0x01,        /**< no feature */
    AS608_IMAGE_FEATURE    = 0x02,        /**< feature */
} as608_image_t;

typedef enum
{
    AS608_GPIO_NUMBER_0 = 0x00,        /**< number 0 */
    AS608_GPIO_NUMBER_1 = 0x01,        /**< number 1 */
} as608_gpio_number_t;

typedef enum
{
    AS608_GPIO_LEVEL_LOW  = 0x00,        /**< level low */
    AS608_GPIO_LEVEL_HIGH = 0x01,        /**< level high */
} as608_gpio_level_t;

typedef enum
{
    AS608_STATUS_OK                          = 0x00,        /**< ok */
    AS608_STATUS_FRAME_ERROR                 = 0x01,        /**< frame error */
    AS608_STATUS_NO_FINGERPRINT              = 0x02,        /**< no fingerprint */
    AS608_STATUS_INPUT_ERROR                 = 0x03,        /**< fingerprint image error */
    AS608_STATUS_IMAGE_TOO_DRY               = 0x04,        /**< fingerprint image too dry */
    AS608_STATUS_IMAGE_TOO_WET               = 0x05,        /**< fingerprint image too wet */
    AS608_STATUS_IMAGE_TOO_CLUTTER           = 0x06,        /**< fingerprint too clutter */
    AS608_STATUS_IMAGE_TOO_FEW_FEATURE       = 0x07,        /**< fingerprint feature too few */
    AS608_STATUS_NOT_MATCH                   = 0x08,        /**< not match */
    AS608_STATUS_NOT_FOUND                   = 0x09,        /**< not found */
    AS608_STATUS_FEATURE_COMBINE_ERROR       = 0x0A,        /**< feature combine error */
    AS608_STATUS_LIB_ADDR_OVER               = 0x0B,        /**< fingerprint lib addr is over */
    AS608_STATUS_LIB_READ_ERROR              = 0x0C,        /**< fingerprint lib read error */
    AS608_STATUS_UPLOAD_FEATURE_ERROR        = 0x0D,        /**< upload feature error */
    AS608_STATUS_NO_FRAME                    = 0x0E,        /**< no frame */
    AS608_STATUS_UPLOAD_IMAGE_ERROR          = 0x0F,        /**< upload image error */
    AS608_STATUS_LIB_DELETE_ERROR            = 0x10,        /**< delete lib error */
    AS608_STATUS_LIB_CLEAR_ERROR             = 0x11,        /**< clear lib error */
    AS608_STATUS_ENTER_LOW_POWER_ERROR       = 0x12,        /**< enter low power error */
    AS608_STATUS_COMMAND_INVALID             = 0x13,        /**< command invalid */
    AS608_STATUS_RESET_ERROR                 = 0x14,        /**< reset error */
    AS608_STATUS_BUFFER_INVALID              = 0x15,        /**< buffer invalid */
    AS608_STATUS_UPDATE_ERROR                = 0x16,        /**< update error */
    AS608_STATUS_NO_MOVE                     = 0x17,        /**< no move */
    AS608_STATUS_FLASH_ERROR                 = 0x18,        /**< flash error */
    AS608_STATUS_F0_RESPONSE                 = 0xF0,        /**< f0 response */
    AS608_STATUS_F1_RESPONSE                 = 0xF1,        /**< f1 response */
    AS608_STATUS_FLASH_WRITE_SUM_ERROR       = 0xF2,        /**< flash sum error */
    AS608_STATUS_FLASH_WRITE_HEADER_ERROR    = 0xF3,        /**< flash header error */
    AS608_STATUS_FLASH_WRITE_LENGTH_ERROR    = 0xF4,        /**< flash length error */
    AS608_STATUS_FLASH_WRITE_LENGTH_TOO_LONG = 0xF5,        /**< flash length too long */
    AS608_STATUS_FLASH_WRITE_ERROR           = 0xF6,        /**< flash write error */
    AS608_STATUS_UNKNOWN                     = 0x19,        /**< unknown */
    AS608_STATUS_REG_INVALID                 = 0x1A,        /**< reg invalid */
    AS608_STATUS_DATA_INVALID                = 0x1B,        /**< data invalid */
    AS608_STATUS_NOTE_PAGE_INVALID           = 0x1C,        /**< note page invalid */
    AS608_STATUS_PORT_INVALID                = 0x1D,        /**< port invalid */
    AS608_STATUS_ENROOL_ERROR                = 0x1E,        /**< enrool error */
    AS608_STATUS_LIB_FULL                    = 0x1F,        /**< lib full */
} as608_status_t;

typedef struct as608_params_s
{
    uint16_t status;                        /**< current status */
    as608_sensor_type_t sensor_type;        /**< sensor type */
    uint16_t fingerprint_size;              /**< fingerprint size */
    as608_level_t level;                    /**< level */
    uint32_t address;                       /**< device address */
    as608_packet_size_t packet_size;        /**< packet size */
    uint16_t n_9600;                        /**< n times of 9600 */
} as608_params_t;

#endif

void AS608_Init(void);
uint8_t AS608_Read(void);
uint8_t AS608_SendCommand(uint8_t Identifier,uint16_t Packet_Length,uint8_t Command,uint8_t* Params);
uint8_t PS_GetImage(as608_status_t* status);
