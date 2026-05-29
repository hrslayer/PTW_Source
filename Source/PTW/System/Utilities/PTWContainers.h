#pragma once
#include "CoreMinimal.h"
#include "Algo/Heapify.h"

/**
 * 커스텀 우선순위 큐 컨테이너입니다.
 */
template <typename ElementType>
class TPriorityQueue
{
public:
	TPriorityQueue() {}
	
	void Push(const ElementType& Item)
	{
		Array.HeapPush(Item);
	}
	
	void Push(ElementType&& Item)
	{
		Array.HeapPush(MoveTemp(Item));
	}
	
	ElementType Pop()
	{
		check(Array.Num() > 0);
		ElementType TopItem;
		Array.HeapPop(TopItem);
		return TopItem;
	}
	
	const ElementType& Top() const
	{
		check(Array.Num() > 0);
		return Array.HeapTop();
	}
	
	bool IsEmpty() const
	{
		return Array.Num() == 0;
	}
	
	int32 Num() const
	{
		return Array.Num();
	}
	
	void Empty()
	{
		Array.Empty();
	}

	bool Contains(const ElementType& Item) const
	{
		return Array.Contains(Item);
	}
	
	void Swap(TPriorityQueue& Other)
	{
		::Swap(Array, Other.Array);
	}
	
private:
	TArray<ElementType> Array;
};