package queueImplement;

public class QueueNode {
    public double[] data; // 节点存储的数据
    public QueueNode next; // 指向下个节点的指针

    public QueueNode() {
        this(null, null);
    }

    public QueueNode(double[] data) {
        this(data, null);
    }

    public QueueNode(double[] data, QueueNode next) {
        this.data = data;
        this.next = next;
    }

}
