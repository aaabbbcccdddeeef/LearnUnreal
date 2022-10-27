#include "QxOctree.h"

#include <stdexcept>

bool TQxOctreeNode::HasAnyObjects()
{
    if (Elements.Num() > 0)
    {
        return true;
    }
    for (auto Child : Children)
    {
        if (Child->HasAnyObjects())
        {
            return true;
        }
    }
    return false;
}

bool TQxOctreeNode::Add(UObject* InElement, const FBoxCenterAndExtent& InBounds)
{
    bool bAddResult = false;
    FBox tmp1 = InBounds.GetBox();
    FBox tmp2 = Bounds.GetBox();
    if (Encapsulates(tmp2, tmp1))
    {
        SubAdd(InElement, InBounds);
            
        bAddResult = true;
    }
    else
    {
        bAddResult = false;
    }
    return bAddResult;
}

void TQxOctreeNode::GetWithInFrustum(const FConvexVolume& InFrustum, TArray<UObject*>& OutElements)
{
    if (!InFrustum.IntersectBox(Bounds.Center, Bounds.Extent))
    {
        return;
    }

    for (OctreeObject& Element : Elements)
    {
        if (InFrustum.IntersectBox(Element.ObjBounds.Center, Element.ObjBounds.Extent))
        {
            bIsSelected = true; 
            OutElements.Add(Element.Object);
        }
    }

    for (auto Child : Children)
    {
        Child->GetWithInFrustum(InFrustum, OutElements);
    }
}

bool TQxOctreeNode::IsColliding(const FBoxCenterAndExtent& CheckBounds)
{
    if (!Bounds.GetBox().Intersect(CheckBounds.GetBox()))
    {
        return false;
    }

    for (OctreeObject& Element : Elements)
    {
        if (Element.ObjBounds.GetBox().Intersect(CheckBounds.GetBox()))
        {
            return true;
        }
    }

    for (auto Child : Children)
    {
        if (Child->IsColliding(CheckBounds))
        {
            return true;
        }
    }
    return false;
}

void TQxOctreeNode::GetCollidings(const FBoxCenterAndExtent& CheckBounds, TArray<UObject*>& Result)
{
    if (!Bounds.GetBox().Intersect(CheckBounds.GetBox()))
    {
        return;
    }

    for (OctreeObject& Element : Elements)
    {
        if (Element.ObjBounds.GetBox().Intersect(CheckBounds.GetBox()))
        {
            bIsSelected = true;
            Result.Add(Element.Object);
        }
    }

    for (auto Child : Children)
    {
        Child->GetCollidings(CheckBounds, Result);
    }
}

void TQxOctreeNode::GetCollidings(const FVector& Start, const FVector& End, TArray<UObject*>& OutResult)
{
    if (!FMath::LineBoxIntersection(Bounds.GetBox(), Start, End, End - Start))
    {
        return;
    }

    for (OctreeObject& Element : Elements)
    {
        if (FMath::LineBoxIntersection(Element.ObjBounds.GetBox(), Start, End, End - Start))
        {
            bIsSelected = true;
            OutResult.Add(Element.Object);
        }
    }

    for (auto Child : Children)
    {
        Child->GetCollidings(Start, End, OutResult);
    }
}

void TQxOctree::AddElement(UObject* InElement, const FBoxCenterAndExtent& Bounds)
{
    int GrowCount = 0;
    while (!RootNode->Add(InElement, Bounds))
    {
        Grow(Bounds.Center - RootNode->Bounds.Center);
        GrowCount++;
        if (GrowCount > 20)
        {
            UE_LOG(QxAccelerate, Error, TEXT("Abort add operation  because it try to grow bounds more than 20 times"));
            return;
        }
        ElementCount++;
    }
}
