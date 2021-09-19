package queueImplement;

public class QueueLinked {
    public QueueNode front; // 队首指针
    public QueueNode rear;  // 队尾指针

    public QueueLinked() {
        this.rear = null;
        this.front = null;
    }   //将一个对象追加到队列的尾部

    public void enqueue(double[] obj) {
        //如果队列是空的
        if (rear == null && front == null) {
            rear = new QueueNode(obj);
            front = rear;
        } else {
            QueueNode node = new QueueNode(obj);
            rear.next = node;
            rear = rear.next;
        }
    }
    //队首对象出队
    //return 出队的对象，队列空时返回null
    public double[] dequeue() {
        //如果队列空
        if (front == null) {
            return null;
        }
        //如果队列中只剩下一个对象
        if (front == rear && rear != null) {
            QueueNode node = front;
            rear = null;
            front = null;
            return node.data;
        }
        double[] obj = front.data;
        front = front.next;
        return obj;
    }

    public int queueSize() {
        if (front == null ){
            return 0;
        }
        if (front == rear && rear != null){
            return 1;
        }
        int z = 0;
        QueueNode node = front;
        while(node != null){
            z++;
            node = node.next;
        }
        return z;
    }

    public boolean isEmptyQue(){
        if (this.front == null){
            return true;
        }else{
            return false;
        }
    }

}
