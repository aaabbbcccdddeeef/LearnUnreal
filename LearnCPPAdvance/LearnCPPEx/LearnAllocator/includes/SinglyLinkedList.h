#pragma once

// 单链表结构
template<class T>
class SinglyLinkedList
{
public:
    struct Node
    {
        T data;
        Node* next;
    };

    Node* head;

public:
    SinglyLinkedList()
    {
    }

    void insert(Node* previousNode, Node* newNode);
    void remove(Node* previousNods, Node* deleteNode);
};

template <class T>
void SinglyLinkedList<T>::insert(Node* previousNode, Node* newNode)
{
    if (previousNode == nullptr) //这个表示插入的newNode是head之前的node
    {
        if (head != nullptr) // 如果链表中已经有元素了
        {
            newNode->next = head;
        }
        else 
        {
            newNode->next = nullptr;
        }
        head = newNode;
    }
    else
    {
        if (previousNode->next = nullptr) // is last node
        {
            previousNode->next = newNode;
            newNode->next = nullptr;
        }
        else
        {
            newNode->next = previousNode->next;
            previousNode->next = newNode;
        }
    }
}

template <class T>
void SinglyLinkedList<T>::remove(Node* previousNods, Node* deleteNode)
{
    if (previousNods == nullptr)
    {
        if (deleteNode->next == nullptr)
        {
            head = nullptr;
        }
        else
        {
            head = deleteNode->next;
        }
    }
    else
    {
        previousNods->next = deleteNode->next;
    }
}


