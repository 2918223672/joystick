#include "app_bus.h"

#include "common_pubsub.h"

/* 输入、OLED、USB和蓝牙节点 */
static CommonNode_t *input_node;
static CommonNode_t *oled_node;
static CommonNode_t *usb_node;
static CommonNode_t *bluetooth_node;

/* 输入话题的发布者和输出节点订阅者 */
static Publisher_t *key_publisher;
static Publisher_t *joystick_publisher;
static Subscriber_t *oled_joystick_subscriber;
static Subscriber_t *usb_key_subscriber;
static Subscriber_t *usb_joystick_subscriber;
static Subscriber_t *bluetooth_key_subscriber;

uint8_t App_BusInit(void)
{
    input_node = NodeRegister(NODE_INPUT, "input");
    oled_node = NodeRegister(NODE_OLED, "oled");
    usb_node = NodeRegister(NODE_USB, "usb");
    bluetooth_node = NodeRegister(NODE_BLUETOOTH, "bluetooth");

    if (input_node == 0 || oled_node == 0 ||
        usb_node == 0 || bluetooth_node == 0)
    {
        return 0;
    }

    key_publisher = PubRegister(input_node, TOPIC_KEY_EVENT, sizeof(KeyData_t));
    joystick_publisher = PubRegister(input_node, TOPIC_JOYSTICK_DATA, sizeof(JoystickData_t));
    oled_joystick_subscriber = SubRegister(oled_node, TOPIC_JOYSTICK_DATA, sizeof(JoystickData_t));
    usb_key_subscriber = SubRegister(usb_node, TOPIC_KEY_EVENT, sizeof(KeyData_t));
    usb_joystick_subscriber = SubRegister(usb_node, TOPIC_JOYSTICK_DATA, sizeof(JoystickData_t));
    bluetooth_key_subscriber = SubRegister(bluetooth_node, TOPIC_KEY_EVENT, sizeof(KeyData_t));

    if (key_publisher == 0 || joystick_publisher == 0 ||
        oled_joystick_subscriber == 0 ||
        usb_key_subscriber == 0 ||
        usb_joystick_subscriber == 0 || bluetooth_key_subscriber == 0)
    {
        return 0;
    }

    return 1;
}

uint8_t App_BusPublishKey(const KeyData_t *data)
{
    if (key_publisher == 0 || data == 0)
    {
        return 0;
    }

    return PubPushMessage(key_publisher, data) > 0;
}

uint8_t App_BusPublishJoystick(const JoystickData_t *data)
{
    if (joystick_publisher == 0 || data == 0)
    {
        return 0;
    }

    return PubPushMessage(joystick_publisher, data) > 0;
}

uint8_t App_BusReceiveOledJoystick(JoystickData_t *data)
{
    if (oled_joystick_subscriber == 0 || data == 0)
    {
        return 0;
    }

    return SubGetMessage(oled_joystick_subscriber, data);
}

uint8_t App_BusReceiveUsbKey(KeyData_t *data)
{
    if (usb_key_subscriber == 0 || data == 0)
    {
        return 0;
    }

    return SubGetMessage(usb_key_subscriber, data);
}

uint8_t App_BusReceiveUsbJoystick(JoystickData_t *data)
{
    if (usb_joystick_subscriber == 0 || data == 0)
    {
        return 0;
    }

    return SubGetMessage(usb_joystick_subscriber, data);
}

uint8_t App_BusReceiveBluetoothKey(KeyData_t *data)
{
    if (bluetooth_key_subscriber == 0 || data == 0)
    {
        return 0;
    }

    return SubGetMessage(bluetooth_key_subscriber, data);
}
