#ifndef COMMON_PUBSUB_H
#define COMMON_PUBSUB_H

#include "common_node.h"

/*
 * 静态发布/订阅资源配置。
 * C8T6内存有限，因此不使用malloc动态申请内存。
 */
#define COMMON_MAX_TOPIC_NAME_LEN   24
#define COMMON_MAX_TOPIC_COUNT      12
#define COMMON_MAX_SUBSCRIBER_COUNT 8
#define COMMON_QUEUE_DEPTH          4
#define COMMON_MAX_MESSAGE_SIZE     16

typedef struct CommonPublisher Publisher_t;
typedef struct CommonSubscriber Subscriber_t;

/* 一个话题的发布者实例 */
struct CommonPublisher
{
    char topic_name[COMMON_MAX_TOPIC_NAME_LEN + 1]; /* 话题名称 */
    uint16_t data_len;                               /* 话题数据长度 */
    CommonNode_t *owner_node;                        /* 发布该话题的节点 */
    Subscriber_t *first_subscriber;                  /* 第一个订阅者 */
    uint8_t topic_registered;                        /* 话题节点是否已经创建 */
};

/* 一个话题的订阅者实例 */
struct CommonSubscriber
{
    uint8_t queue[COMMON_QUEUE_DEPTH][COMMON_MAX_MESSAGE_SIZE]; /* 消息缓存 */
    uint16_t data_len;                                          /* 消息数据长度 */
    uint8_t front_index;                                        /* 队头索引 */
    uint8_t back_index;                                         /* 队尾索引 */
    uint8_t message_count;                                      /* 当前消息数量 */
    CommonNode_t *owner_node;                                   /* 所属节点 */
    Publisher_t *publisher;                                     /* 所订阅的话题 */
    Subscriber_t *next;                                         /* 同话题的下一个订阅者 */
    uint8_t registered;                                         /* 订阅者是否已经注册 */
};

/*
 * 注册一个功能节点。
 * 同一个NodeId只能注册一次，重复使用相同名称注册时返回原节点。
 */
CommonNode_t *NodeRegister(NodeId_t id, const char *name);

/*
 * 注册一个话题发布者。
 * 一个话题只允许一个发布节点，但可以有多个订阅节点。
 */
Publisher_t *PubRegister(CommonNode_t *node, const char *name, uint16_t data_len);

/*
 * 注册一个话题订阅者。
 * 同一个话题可以被多个节点订阅，每个订阅者拥有独立缓存。
 */
Subscriber_t *SubRegister(CommonNode_t *node, const char *name, uint16_t data_len);

/*
 * 发布数据给该话题的所有订阅者。
 * 返回成功写入的订阅者数量。
 */
uint8_t PubPushMessage(Publisher_t *pub, const void *data_ptr);

/*
 * 从订阅者缓存中取出一条数据。
 * 返回1表示取到数据，返回0表示当前没有新数据。
 */
uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr);

#endif
