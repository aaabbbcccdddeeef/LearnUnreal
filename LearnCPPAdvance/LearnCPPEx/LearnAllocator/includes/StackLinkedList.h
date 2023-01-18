#pragma once

template<class T>
class StackLinkedList
{
public:
    struct Node
    {
        T data;
        Node* next;
    };

    Node* head;

public:
    StackLinkedList() = default;
    StackLinkedList(StackLinkedList& stckLinedList) = delete;

    void Push(Node* newNode);
    Node* Pop();
};

template <class T>
void StackLinkedList<T>::Push(Node* newNode)
{
    newNode->next = head;
    head = newNode;
}

template <class T>
typename StackLinkedList<T>::Node* StackLinkedList<T>::Pop()
{
    Node* top = head;
    head = head->next;
    return top;
}


