// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Source: https://stackoverflow.com/questions/19101284/implementing-an-atomic-queue-in-qt5
#pragma once

#include <QAtomicPointer>

namespace noda {
  template <class T>
  class AtomicQueue {
	struct QueueNode {
	  QueueNode(T *value) :
	      next(NULL), data(value) {}
	  ~QueueNode()
	  { /*if ( next ) delete next;*/
	  }
	  QueueNode *next;
	  T *data;
	};

  public:
	AtomicQueue()
	{
	  m_front = new QueueNode(new T());
	  m_tail.store(m_front);
	  m_divider.store(m_front);
	}

	~AtomicQueue()
	{
	  QueueNode *node = m_front;
	  while(node->next) {
		QueueNode *n = node->next;
		delete node;
		node = n;
	  }

	  if(m_front)
		delete m_front;
	}

	void push(T *value)
	{
	  m_tail.load()->next = new QueueNode(value);
	  m_tail = m_tail.load()->next; // This moves the QueueNode into the atomic pointer, making it safe :)
	  while(m_front != m_divider.load()) {
		QueueNode *tmp = m_front;
		m_front = m_front->next;
		delete tmp;
	  }
	}

	bool peek(T *&result)
	{
	  if(m_divider.load() != m_tail.load()) {
		// Problem area
		QueueNode *next = m_divider.load()->next;
		if(next) {
		  result = next->data;
		  return true;
		}
	  }
	  return false;
	}

	bool pop(T *&result)
	{
	  bool res = this->peek(result);
	  if(res) {
		m_divider = m_divider.load()->next;
	  }
	  return res;
	}

  private:
	QueueNode *m_front;
	QAtomicPointer<QueueNode> m_divider, m_tail;
  };
} // namespace noda