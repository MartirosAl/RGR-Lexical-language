#include "BigNumber.h"

BigNumber::BigNumber(const size_t new_capacity_)
{
   if (new_capacity_ == 0)
   {
      number = new short[100];
      capacity = 100;
   }
   else
   {
      number = new short[new_capacity_];
      capacity = new_capacity_;
   }
   size = 1;
   number[0] = 0;
}

BigNumber::BigNumber(const short* number_, const size_t size_)
{
   //Выделяю память кратную 100 и больше size_
   size_t i = 1;
   for (i = 1; 100 * i < size_; i++);
   capacity = 100 * i;
   number = new short[capacity];

   size = size_;

   for (i = 0; i < size; i++)
      number[i] = number_[i];
}

BigNumber::~BigNumber()
{
   if (number)
      delete[] number;
   number = nullptr;
   size = 0;
   capacity = 0;
}

BigNumber::BigNumber(const BigNumber& other_)
{
   if (other_.size == 0)
      throw "Uninitialized variables";

   size = other_.size;
   capacity = other_.capacity;
   number = new short[other_.capacity];


   for (size_t i = 0; i < other_.size; i++)
      number[i] = other_[i];

}

void BigNumber::Set_Number(const short* new_number_, size_t new_size_)
{
   if (size != new_size_)
   {
      Clear();
      Expansion(new_size_);
      size = new_size_;
   }

   for (size_t i = 0; i < new_size_; i++)
      number[i] = new_number_[i];
}

size_t BigNumber::Get_Capacity() const
{
   return capacity;
}

size_t BigNumber::Get_Size() const
{
   return size;
}

short& BigNumber::operator[](size_t index_) const
{
   if (size == 0)
      throw "Uninitialized variables";

   if (index_ >= capacity)
      throw "Out of range";
   return number[index_];
}

short& BigNumber::operator[](size_t index_) 
{
   if (size == 0)
      throw "Uninitialized variables";

   if (index_ >= capacity)
   {
      Expansion(index_ + 1);
   }
   return number[index_];
}

BigNumber::operator bool() const
{
   return (size > 1 || (size == 1 && number[0] != 0));
}


BigNumber& BigNumber::operator++()
{
   if (size == 0)
      throw "Uninitialized variables";

   short s_one[1] = { 1 };
   BigNumber one(s_one, 1);
   (*this) = (*this) + one;
   return *this;
}

BigNumber BigNumber::operator++(int)
{
   if (size == 0)
      throw "Uninitialized variables";

   BigNumber temp = (*this);
   short s_one[1] = { 1 };
   BigNumber one(s_one, 1);
   (*this) = (*this) + one;
   return temp;
}

BigNumber& BigNumber::operator--()
{
   if (size == 0)
      throw "Uninitialized variables";

   short s_one[1] = { 1 };
   BigNumber one(s_one, 1);
   (*this) = (*this) - one;
   return *this;
}

BigNumber BigNumber::operator--(int)
{
   if (size == 0)
      throw "Uninitialized variables";

   BigNumber temp = (*this);
   short s_one[1] = { 1 };
   BigNumber one(s_one, 1);
   (*this) = (*this) - one;
   return temp;
}

BigNumber& BigNumber::operator=(const BigNumber& other_)
{
   if (other_.size == 0)
      throw "Uninitialized variables";

   if (other_.size >= capacity)
   {
      this->Clear();
      capacity = other_.capacity;
      number = new short[other_.capacity];
   }

   size = other_.size;
   for (size_t i = 0; i < size; i++)
      number[i] = other_[i];

   return *this;
}

BigNumber& BigNumber::operator=(const int& other_)
{
   if (other_ < 0)
      throw "Assignment error";
   if (other_ == 0)
   {
      if (size == capacity)
         Expansion();
      number[0] = 0;
      size = 1;
      return *this;
   }
   int i = 0;
   size = 0;
   int temp = other_;
   for (; temp > 0; temp = temp / 10)
   {
      if (size == capacity)
         Expansion();
      number[i] = temp%10;
      size++;
      i++;
   }

   return *this;
}

BigNumber BigNumber::operator+(const BigNumber& other_) const
{
   if (other_.size == 0 || size == 0)
      throw "Uninitialized variables";

   BigNumber max_number = (size > other_.size ? *this : other_);
   BigNumber min_number = (size > other_.size ? other_ : *this);


   //Выбираем где больше места + 1, чтобы не трогать Expansion 
   //Можно было реализовать и с *max_number, но тогда нужно было бы делать расширение, а это менее эффективно
   BigNumber result((max_number.capacity) + 1);
   result.size = max_number.size;

   short adddigit = 0;

   for (size_t i = 0; i < min_number.size; i++)
   {
      result[i] = (max_number[i] + min_number[i] + adddigit) % 10;
      adddigit = (max_number[i] + min_number[i] + adddigit) / 10;
   }

   //Смотрим что больше и дополняем до него
   for (size_t i = min_number.size; i < max_number.size; i++)
   {
      result[i] = (adddigit + max_number[i]) % 10;
      adddigit = (adddigit + max_number[i]) / 10;
   }

   if (adddigit)
   {
      result.size++;
      result[result.size - 1] = adddigit;
   }
   
   return result;
}

BigNumber BigNumber::operator*(const short& digit_) const
{
   if (size == 0)
      throw "Uninitialized variables";

   if (digit_ > 9 || digit_ < 0)
      throw "Wrong variable";

   if (size == 1 && number[0] == 0 || digit_ == 0)
      return BigNumber();

   BigNumber result(size + 1);
   result.size = size;

   short digit = 0;

   for (size_t i = 0; i < size; i++)
   {
      digit += number[i] * digit_;
      result[i] = digit % 10;
      digit = digit / 10;
   }

   if (digit != 0)
   {
      result.size++;
      result[result.size - 1] = digit;
   }

   return result;
}

BigNumber BigNumber::operator*(const BigNumber& other_) const
{
   if (other_.size == 0 || size == 0)
      throw "Uninitialized variables";

   if (size == 1 && number[0] == 0 || other_.size == 1 && other_[0] == 0)
      return BigNumber();

   BigNumber result(size + other_.size + 1);
   result.size = size + other_.size - 1;
   for (size_t i = 0; i < size + other_.size - 1; i++)
      result[i] = 0;


   for (size_t i = 0; i < size; i++)
   { 
      short digit = 0;
      for (size_t j = 0; j < other_.size; j++)
      {
         result[i + j] += number[i] * other_[j] + digit;
         digit = result[i + j] / 10;
         result[i+j] -= digit * 10;
      }
      if (digit)
      {
         if (result.size <= i + other_.size)
            result.size++;
         result[i + other_.size] = digit;
      }
   }

   return result;
}


BigNumber BigNumber::operator-(const BigNumber& other_) const
{
   if (other_.size == 0 || size == 0)
      throw "Uninitialized variables";

   if (*this <= other_)
   {
      BigNumber result(1);
      result.size = 1;
      result[0] = 0;
      return result;
   }

   if (other_.size == 1 && other_[0] == 0)
      return *this;
   
   BigNumber max_number = *this;
   BigNumber min_number = other_;
   
   BigNumber result = *this;

   short subdigit = 0;

   for (size_t i = 0; i < min_number.size; i++)
   {
      result[i] = max_number[i] - min_number[i] - subdigit;
      if (result[i] < 0)
      {
         subdigit = 1;
         result[i] = 10 + result[i];
      }
      else 
         subdigit = 0;
   }

   for (size_t i = min_number.size; i < max_number.size && subdigit; i++)
   {
      result[i] = max_number[i] - subdigit;
      if (result[i] < 0)
      {
         subdigit = 1;
         result[i] = 10 + result[i];
      }
      else
         subdigit = 0;
   }

   for (size_t i = result.size; i > 1 && result[i - 1] == 0; i--)
      result.size--;

   return result;
}

BigNumber BigNumber::operator/(const BigNumber& other_) const 
{   
   if (other_.size == 0 || size == 0)
      throw "Uninitialized variables";

   if (other_.size == 1 && other_.number[0] == 0)
      throw "You can't divide by zero";

   if (*this <= other_)
   {
      BigNumber result(1);
      return result;
   }

   if (other_.size == 1 && other_[0] == 1) 
      return *this;  

   BigNumber cur(1);
   BigNumber result(*this);

   for (size_t i = size; i > 0; i--)
   {
      cur.Push_Back(number[i - 1]);
      
      short x = 0;
      short left = 0;
      short right = 9;
      while (left <= right)
      {
         short middle = (left + right) / 2;

         if (other_ * middle <= cur)
         {
            x = middle;
            left = middle + 1;
         }
         else
            right = middle - 1;
      }

      result[i - 1] = x;
      
      cur = (cur - (other_ * x));
   }
   
   size_t temp = result.size;
   for (; temp > 1 && result[temp - 1] == 0; temp--);
   result.size = temp;

   return result;
}

BigNumber BigNumber::operator%(const BigNumber& other_) const
{
   if (other_.size == 0 || size == 0)
      throw "Uninitialized variables";

   if (other_.size == 1 && other_.number[0] == 0)
      throw "You can't divide by zero";

   if (*this <= other_)
   {
      BigNumber result(1);
      return result;
   }

   if (other_.size == 1 && other_[0] == 1)
   {
      BigNumber result(1);
      return result;
   }

   BigNumber cur(1);

   BigNumber result(*this);

   for (size_t i = size; i > 0; i--)
   {
      cur.Push_Back(number[i - 1]);

      short x = 0;
      short left = 0;
      short right = 9;
      while (left <= right)
      {
         short middle = (left + right) / 2;

         if (other_ * middle <= cur)
         {
            x = middle;
            left = middle + 1;
         }
         else
            right = middle - 1;
      }

      result[i - 1] = x;
      cur = (cur - (other_ * x));
   }

   return cur;
}

bool BigNumber::operator!() const
{
   return (!size || (size <= 1 && number[0] == 0));
}

bool BigNumber::operator<(const BigNumber& other_) const
{
   if (this == &other_)
      return false;

   if (size != other_.size)
      return size < other_.size;

   for (size_t i = size; i > 0; i--)
      if (number[i - 1] != other_[i - 1])
         return number[i - 1] < other_[i - 1];

   return false;
}

bool BigNumber::operator==(const BigNumber& other_) const
{
   if (this == &other_)
      return true;

   if (size != other_.size)
      return false;

   for (size_t i = size; i > 0; i--)
      if (number[i - 1] != other_[i - 1])
         return false;

   return true;
}

bool BigNumber::operator<=(const BigNumber& other_) const
{
   if (this == &other_)
      return true;

   if (size != other_.size)
      return size < other_.size;

   for (size_t i = size; i > 0; i--)
      if (number[i - 1] != other_[i - 1])
         return number[i - 1] < other_[i - 1];

   return true;
}

bool BigNumber::operator>(const BigNumber& other_) const
{
   if (this == &other_)
      return false;

   if (size != other_.size)
      return size > other_.size;

   for (size_t i = size; i > 0; i--)
      if (number[i - 1] != other_[i - 1])
         return number[i - 1] > other_[i - 1];

   return false;
}

bool BigNumber::operator>=(const BigNumber& other_) const
{
   if (this == &other_)
      return true;

   if (size != other_.size)
      return size > other_.size;

   for (size_t i = size; i > 0; i--)
      if (number[i - 1] != other_[i - 1])
         return number[i - 1] > other_[i - 1];

   return true;
}

bool BigNumber::operator!=(const BigNumber& other_) const
{
   return !(*this == other_);
}

int reverseDigits(int num)
{
   int rev_num = 0;
   while (num > 0) {
      rev_num = rev_num * 10 + num % 10;
      num = num / 10;
   }
   return rev_num;
}

void BigNumber::Clear()
{
   delete[] number;
   number = nullptr;
   size = 0;
   capacity = 0;
}

void BigNumber::Number_Shift(const size_t index_)
{
   if (index_ == 0)
      return;

   if (capacity < (size + index_))
      Expansion();
   

   for (size_t i = size; i > 0; i--)
      number[i + index_ - 1] = number[i - 1];

   for (size_t i = 0; i < index_; i++)
      number[i] = 0;

   size += index_;

   //Удаление лишних нулей, например, если был только один ноль
   for (size_t i = size; i > 1 && number[i - 1] == 0; i--)
      size--;
}

void BigNumber::Push_Back(short digit_)
{
   if (digit_ < 0 || digit_ > 9)
      throw "Wrong digit";

   Number_Shift(1);
   number[0] = digit_;
}

void BigNumber::Push_Back(short* digit_, size_t size_)
{
   //Проверка массива на правильность содержания цифр
   for (size_t i = 0; i < size_; i++)
      if (digit_[i] < 0 || digit_[i] > 9)
         throw "Wrong digit";

   Number_Shift(size_);

   for (size_t i = 0; i < size_; i++)
      number[i] = digit_[i];
   
}

void BigNumber::Expansion()
{
   short* temp_arr = new short[capacity *= 2];

   if (capacity > 0)
   {
      for (size_t i = 0; i < size; i++)
         temp_arr[i] = number[i];
      delete[] number;
   }
   
   number = temp_arr;

}

void BigNumber::Expansion(size_t new_capacity_)
{
   if (new_capacity_ <= capacity)
      return;

   short* temp_arr = new short[capacity = new_capacity_];

   for (size_t i = 0; i < size; i++)
      temp_arr[i] = number[i];

   delete[] number;
  
   number = temp_arr;

}

ostream& operator<<(ostream& stream, const BigNumber& object_)
{
   for (size_t i = object_.size; i > 0; i--)
      stream << object_[i-1];

   return stream;
}

istream& operator>>(istream& stream, BigNumber& object_)
{
   //Переременная для востановления первоначального массива в случае неправильного ввода
   BigNumber mercy = object_;

   if (object_.size != 0)
      object_.Clear();

   object_.number = new short[100];
   object_.capacity = 100;

   char digit[2];

   //23456789

   while (stream.peek() != '\n' && stream.peek() != ' ' && !stream.eof())
   {
      stream.get(digit, 2);
      if (digit[0] < '0' || digit[0] > '9')
      {
         stream.clear();
         stream.setstate(ios::failbit);

         object_ = mercy;
         cerr << "Corruption input!";
         return stream;
      }

      object_.Push_Back(digit[0] - '0');
   }
   stream.ignore();//Игнорируем \n или пробел

   return stream;
}