#ifdef JM_COT_DOUBLYLINKEDNODES
class CF_DoublyLinkedNodes_WeakRef<Class T>
{
	ref CF_DoublyLinkedNode_WeakRef<T> m_Head;

	int m_Count;

	void ~CF_DoublyLinkedNodes_WeakRef()
	{
		m_Head = null;
	}

	CF_DoublyLinkedNode_WeakRef<T> Add(T value)
	{
		auto node = new CF_DoublyLinkedNode_WeakRef<T>(value);

		Add(node);

		return node;
	}

	void Add(CF_DoublyLinkedNode_WeakRef<T> node)
	{
		node.Link(m_Head, null);

		m_Head = node;

		m_Count++;
	}

	void Remove(CF_DoublyLinkedNode_WeakRef<T> node)
	{
		if (!node)
			return;

		bool isHead = m_Head == node;

		if (!isHead && node.m_Next == null && node.m_Prev == null)
			return;

		auto next = node.m_Next;

		node.Unlink();

		if (isHead)
			m_Head = next;

		m_Count--;
	}
}

class CF_DoublyLinkedNode_WeakRef<Class T>
{
	ref CF_DoublyLinkedNode_WeakRef<T> m_Next;
	ref CF_DoublyLinkedNode_WeakRef<T> m_Prev;

	T m_Value;

	void CF_DoublyLinkedNode_WeakRef(T value)
	{
		m_Value = value;
	}

	void Link(CF_DoublyLinkedNode_WeakRef<T> next, CF_DoublyLinkedNode_WeakRef<T> prev)
	{
		if (next == this || prev == this || (next != null && next == prev))
		{
			Error(string.Format("Trying to create circular reference (next=%1, prev=%2)", next, prev));
			return;
		}

		m_Next = next;
		m_Prev = prev;

		if (next)
			next.m_Prev = this;

		if (prev)
			prev.m_Next = this;
	}

	void Unlink()
	{
		T value;
		m_Value = value;

		if (m_Next)
			m_Next.m_Prev = m_Prev;

		if (m_Prev)
			m_Prev.m_Next = m_Next;

		m_Next = null;
		m_Prev = null;
	}
}
#endif
