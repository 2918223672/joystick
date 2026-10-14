#include "common_pubsub.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

/* 静态资源池，避免在单片机上使用malloc */
static CommonNode_t node_pool[NODE_COUNT];
static Publisher_t publisher_pool[COMMON_MAX_TOPIC_COUNT];
static Subscriber_t subscriber_pool[COMMON_MAX_SUBSCRIBER_COUNT];

/* 检查字符串是否非空且不超过最大长度 */
static uint8_t IsNameValid(const char *name, uint16_t max_len)
{
    uint16_t length = 0;

    if (name == NULL || name[0] == '\0')
    {
        return 0;
    }

    while (name[length] != '\0')
    {
        length++;

        if (length > max_len)
        {
            return 0;
        }
    }

    return 1;
}

/* 复制名称，并保证目标字符串以'\0'结束 */
static void CopyName(char *destination, const char *source, uint16_t max_len)
{
    uint16_t index = 0;

    while (index < max_len && source[index] != '\0')
    {
        destination[index] = source[index];
        index++;
    }

    destination[index] = '\0';
}

/* 根据话题名称查找已经存在的话题 */
static Publisher_t *FindPublisher(const char *name)
{
    uint8_t index;

    for (index = 0; index < COMMON_MAX_TOPIC_COUNT; index++)
    {
        if (publisher_pool[index].topic_registered != 0 && strcmp(publisher_pool[index].topic_name, name) == 0)
        {
            return &publisher_pool[index];
        }
    }

    return NULL;
}

/* 创建一个话题节点，但不把它标记为发布者 */
static Publisher_t *CreateTopic(const char *name, uint16_t data_len)
{
    uint8_t index;

    for (index = 0; index < COMMON_MAX_TOPIC_COUNT; index++)
    {
        if (publisher_pool[index].topic_registered == 0)
        {
            memset(&publisher_pool[index], 0, sizeof(Publisher_t));
            CopyName(publisher_pool[index].topic_name, name, COMMON_MAX_TOPIC_NAME_LEN);
            publisher_pool[index].data_len = data_len;
            publisher_pool[index].topic_registered = 1;
            return &publisher_pool[index];
        }
    }

    return NULL;
}

CommonNode_t *NodeRegister(NodeId_t id, const char *name)
{
    CommonNode_t *node;

    if (id <= NODE_NONE || id >= NODE_COUNT || !IsNameValid(name, COMMON_NODE_NAME_LEN))
    {
        return NULL;
    }

    taskENTER_CRITICAL();

    node = &node_pool[id];

    if (node->registered != 0)
    {
        if (strcmp(node->name, name) == 0)
        {
            taskEXIT_CRITICAL();
            return node;
        }

        taskEXIT_CRITICAL();
        return NULL;
    }

    memset(node, 0, sizeof(CommonNode_t));
    node->id = id;
    CopyName(node->name, name, COMMON_NODE_NAME_LEN);
    node->registered = 1;

    taskEXIT_CRITICAL();
    return node;
}

Publisher_t *PubRegister(CommonNode_t *node, const char *name, uint16_t data_len)
{
    Publisher_t *publisher;

    if (node == NULL || node->registered == 0 || !IsNameValid(name, COMMON_MAX_TOPIC_NAME_LEN) || data_len == 0 || data_len > COMMON_MAX_MESSAGE_SIZE)
    {
        return NULL;
    }

    taskENTER_CRITICAL();

    publisher = FindPublisher(name);

    if (publisher == NULL)
    {
        publisher = CreateTopic(name, data_len);
    }

    if (publisher == NULL || publisher->data_len != data_len)
    {
        taskEXIT_CRITICAL();
        return NULL;
    }

    /*
     * SubRegister可能已经提前创建了话题。
     * 这时再由真正的发布节点补充owner_node。
     */
    if (publisher->owner_node != NULL && publisher->owner_node != node)
    {
        taskEXIT_CRITICAL();
        return NULL;
    }

    publisher->owner_node = node;

    taskEXIT_CRITICAL();
    return publisher;
}

Subscriber_t *SubRegister(CommonNode_t *node, const char *name, uint16_t data_len)
{
    Publisher_t *publisher;
    Subscriber_t *subscriber = NULL;
    uint8_t index;

    if (node == NULL || node->registered == 0 || !IsNameValid(name, COMMON_MAX_TOPIC_NAME_LEN) || data_len == 0 || data_len > COMMON_MAX_MESSAGE_SIZE)
    {
        return NULL;
    }

    taskENTER_CRITICAL();

    publisher = FindPublisher(name);

    if (publisher == NULL)
    {
        publisher = CreateTopic(name, data_len);
    }

    if (publisher == NULL || publisher->data_len != data_len)
    {
        taskEXIT_CRITICAL();
        return NULL;
    }

    for (index = 0; index < COMMON_MAX_SUBSCRIBER_COUNT; index++)
    {
        if (subscriber_pool[index].registered == 0)
        {
            subscriber = &subscriber_pool[index];
            memset(subscriber, 0, sizeof(Subscriber_t));
            subscriber->data_len = data_len;
            subscriber->owner_node = node;
            subscriber->publisher = publisher;
            subscriber->registered = 1;
            break;
        }
    }

    if (subscriber == NULL)
    {
        taskEXIT_CRITICAL();
        return NULL;
    }

    /* 将订阅者挂到该话题的订阅者链表尾部 */
    if (publisher->first_subscriber == NULL)
    {
        publisher->first_subscriber = subscriber;
    }
    else
    {
        Subscriber_t *last = publisher->first_subscriber;

        while (last->next != NULL)
        {
            last = last->next;
        }

        last->next = subscriber;
    }

    taskEXIT_CRITICAL();
    return subscriber;
}

uint8_t PubPushMessage(Publisher_t *pub, const void *data_ptr)
{
    Subscriber_t *subscriber;
    uint8_t delivered_count = 0;

    if (pub == NULL || data_ptr == NULL || pub->topic_registered == 0 || pub->owner_node == NULL)
    {
        return 0;
    }

    taskENTER_CRITICAL();

    subscriber = pub->first_subscriber;

    while (subscriber != NULL)
    {
        /*
         * 队列满时丢弃旧数据，保留最新数据。
         * 这适合摇杆、姿态和控制量这类“只关心最新值”的消息。
         */
        if (subscriber->message_count >= COMMON_QUEUE_DEPTH)
        {
            subscriber->front_index = (uint8_t)((subscriber->front_index + 1) % COMMON_QUEUE_DEPTH);
            subscriber->message_count--;
        }

        memcpy(subscriber->queue[subscriber->back_index], data_ptr, pub->data_len);

        subscriber->back_index = (uint8_t)((subscriber->back_index + 1) % COMMON_QUEUE_DEPTH);
        subscriber->message_count++;
        delivered_count++;
        subscriber = subscriber->next;
    }

    taskEXIT_CRITICAL();
    return delivered_count;
}

uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr)
{
    if (sub == NULL || data_ptr == NULL || sub->registered == 0)
    {
        return 0;
    }

    taskENTER_CRITICAL();

    if (sub->message_count == 0)
    {
        taskEXIT_CRITICAL();
        return 0;
    }

    memcpy(data_ptr, sub->queue[sub->front_index], sub->data_len);

    sub->front_index = (uint8_t)((sub->front_index + 1) % COMMON_QUEUE_DEPTH);
    sub->message_count--;

    taskEXIT_CRITICAL();
    return 1;
}
