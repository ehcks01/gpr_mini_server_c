/*
    바퀴에서 발생한 엔코더 이벤트를 큐에 쌓았다가 처리하려고 함.
    이미 지나간 자리를 나중에 취득하는건 의미가 없어보여서 중단.
*/

#define QUEUE_MAX 2000
int front_queue = -1;
int rear_queue = -1;
char queue[QUEUE_MAX];

char empty_queue(void)
{
    if (front_queue == rear_queue)
        return 1;
    else
        return 0;
}

char full_queue(void)
{
    int tmp = (rear_queue + 1) % QUEUE_MAX;
    if (tmp == front_queue)
        return 1;
    else
        return 0;
}
void add_queue(char isFront)
{
    if (!full_queue())
    {
        rear_queue = (rear_queue + 1) % QUEUE_MAX;
        queue[rear_queue] = isFront;
    }
}

char delete_queue()
{
    if (!empty_queue())
    {
        front_queue = (front_queue + 1) % QUEUE_MAX;
    }
    return queue[front_queue];
}