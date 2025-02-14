//Вариант #10 неотрицательное целое число произвольной длины реализованых в виде масссива десятичных цифр 

#pragma once
#include <iostream>
#include "Iterator.h"

using namespace std;

class BigNumber
{
   //Число начинается с ячейки 0, а заканчивается в size - 1
   //BigNumber 123  ->   [0] = 3   [1] = 2   [2] = 1
   short* number;   //Большое число записанное в виде массива целых неотрицательных чисел
   size_t size;                  //Количество цифр в числе
   size_t capacity;              //Количество цифр для которых выделена память

public:

   BigNumber(const size_t new_cap_ = 100);

   BigNumber(const short* number_, const size_t size_);

   ~BigNumber();

   BigNumber(const BigNumber& other_);

   void Set_Number(const short* new_number_, size_t new_size_);

   size_t Get_Capacity() const;

   size_t Get_Size() const;

   short& operator[](size_t index_) const;

   short& operator[](size_t index_);

   operator bool() const;

   BigNumber& operator++();

   BigNumber operator++(int);

   BigNumber& operator--();

   BigNumber operator--(int);

   BigNumber& operator=(const BigNumber& other_);

   BigNumber& operator=(const int& other_);

   BigNumber operator+(const BigNumber& other_) const;

   BigNumber operator*(const BigNumber& other_) const;

   BigNumber operator*(const short& digit_) const;

   //Усеченый минус
   BigNumber operator-(const BigNumber& other_) const;

   BigNumber operator/(const BigNumber& other_) const;

   BigNumber operator%(const BigNumber& other_) const;

   bool operator!() const;

   bool operator<(const BigNumber& other_) const;

   bool operator==(const BigNumber& other_) const;

   bool operator<=(const BigNumber& other_) const;

   bool operator>(const BigNumber& other_) const;

   bool operator>=(const BigNumber& other_) const;

   bool operator!=(const BigNumber& other_) const;

   //Полное очищение массива
   void Clear();

   //Сдвиг чисел. Увеличение разрядов на index_
   void Number_Shift(size_t index_);

   //Вставка цифры в конец числа
   void Push_Back(short digit_);

   //Вставка массива в конец числа
   void Push_Back(short* digit_, size_t size_);

   //Расширение массива на capacity * 2
   void Expansion();

   //Расширение массива до new_cap_, если new_cap_ > capacity
   void Expansion(size_t new_cap_);

   //Вывод должен быть от size - 1 до 0, т.к. конец числа в 0 ячейке, а начало в size - 1
   // BigNumber 123  ->   [0] = 3   [1] = 2   [2] = 1
   friend ostream& operator<<(ostream& stream, const BigNumber& object_);

   //Ввод должен быть от size - 1 до 0, т.к. конец числа в 0 ячейке, а начало в size - 1
   // BigNumber 123  ->   [0] = 3   [1] = 2   [2] = 1
   friend istream& operator>>(istream& stream, BigNumber& object_);

   Iterator begin() const
   {
      return Iterator(number);
   }

   Iterator end() const
   {
      return Iterator(nullptr);
   }

};


