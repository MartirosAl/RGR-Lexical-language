#pragma once
class Iterator
{
   short* arr;

public:
   Iterator(short* begin) : arr(begin) {}
   const short& operator*() const
   {
      if (!arr)
         throw "There is no pointer";
      return *arr;
   }

   short& operator*()
   {
      if (!arr)
         throw "There is no pointer";
      return *arr;
   }

   short* operator->()
   {
      if (!arr)
         throw "There is no pointer";
      return arr;
   }

   Iterator operator++()
   {
      if (!arr)
         throw "There is no pointer";
      return Iterator(arr++);
   }

   Iterator operator++(int)
   {
      if (!arr)
         throw "There is no pointer";

      Iterator temp = (*this);
      arr++;
      return Iterator(temp);
   }

   bool operator==(const Iterator& other_) const { return arr == other_.arr; }

   bool operator!=(const Iterator& other_) const { return arr != other_.arr; }
};