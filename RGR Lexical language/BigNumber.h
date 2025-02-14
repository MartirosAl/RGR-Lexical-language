//������� #10 ��������������� ����� ����� ������������ ����� ������������ � ���� �������� ���������� ���� 

#pragma once
#include <iostream>
#include "Iterator.h"

using namespace std;

class BigNumber
{
   //����� ���������� � ������ 0, � ������������� � size - 1
   //BigNumber 123  ->   [0] = 3   [1] = 2   [2] = 1
   short* number;   //������� ����� ���������� � ���� ������� ����� ��������������� �����
   size_t size;                  //���������� ���� � �����
   size_t capacity;              //���������� ���� ��� ������� �������� ������

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

   //�������� �����
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

   //������ �������� �������
   void Clear();

   //����� �����. ���������� �������� �� index_
   void Number_Shift(size_t index_);

   //������� ����� � ����� �����
   void Push_Back(short digit_);

   //������� ������� � ����� �����
   void Push_Back(short* digit_, size_t size_);

   //���������� ������� �� capacity * 2
   void Expansion();

   //���������� ������� �� new_cap_, ���� new_cap_ > capacity
   void Expansion(size_t new_cap_);

   //����� ������ ���� �� size - 1 �� 0, �.�. ����� ����� � 0 ������, � ������ � size - 1
   // BigNumber 123  ->   [0] = 3   [1] = 2   [2] = 1
   friend ostream& operator<<(ostream& stream, const BigNumber& object_);

   //���� ������ ���� �� size - 1 �� 0, �.�. ����� ����� � 0 ������, � ������ � size - 1
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


